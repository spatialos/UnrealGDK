#include "gdk/spatialos_connection_handler.h"

#include "gdk/internal/received_op_list.h"
#include "gdk/messages_to_send.h"
#include <chrono>
#include <cstdint>
#include <utility>

namespace gdk {

SpatialOsConnectionHandler::SpatialOsConnectionHandler(Worker_Connection* connection)
: connection(connection), workerId(Worker_Connection_GetWorkerId(connection)) {
  auto* attributes = Worker_Connection_GetWorkerAttributes(connection);
  workerAttributes.reserve(attributes->attribute_count);
  for (std::uint32_t i = 0; i < attributes->attribute_count; ++i) {
    workerAttributes.emplace_back(attributes->attributes[i]);
  }
  pollOpsThread = std::thread{&SpatialOsConnectionHandler::PollOpsInBackground, this};
}

SpatialOsConnectionHandler::~SpatialOsConnectionHandler() {
  connectionCV.notify_one();
  pollOpsThread.join();
}

void SpatialOsConnectionHandler::Advance() {
  std::unique_lock<std::mutex> lock{connectionMutex};
  PollOps();
}

size_t SpatialOsConnectionHandler::GetOpListCount() {
  std::lock_guard<std::mutex> lock{connectionMutex};
  return receivedOpsQueue.size();
}

OpList SpatialOsConnectionHandler::GetNextOpList() {
  std::lock_guard<std::mutex> lock{connectionMutex};
  OpList opList = std::move(receivedOpsQueue.front());
  receivedOpsQueue.pop();
  return opList;
}

void SpatialOsConnectionHandler::SendMessages(MessagesToSend&& messages) {
  std::lock_guard<std::mutex> lock{connectionMutex};
  SendAndClearMessages(&messages);
}

const std::string& SpatialOsConnectionHandler::GetWorkerId() const {
  return workerId;
}

const std::vector<std::string>& SpatialOsConnectionHandler::GetWorkerAttributes() const {
  return workerAttributes;
}

void SpatialOsConnectionHandler::PollOpsInBackground() {
  using namespace std::chrono_literals;
  while (true) {
    std::unique_lock<std::mutex> lock{connectionMutex};
    if (connectionCV.wait_for(lock, 500ms) == std::cv_status::no_timeout) {
      return;
    }
    PollOps();
  }
}

void SpatialOsConnectionHandler::PollOps() {
  auto* opList = Worker_Connection_GetOpList(connection.get(), 0);
  // Loop through the op list and replace internal request Ids with public ones
  for (std::uint32_t i = 0; i < opList->op_count; ++i) {
    auto& op = opList->ops[i];

    RequestId* requestId;
    bool isResponse;

    switch (static_cast<Worker_OpType>(op.op_type)) {
    case WORKER_OP_TYPE_RESERVE_ENTITY_IDS_RESPONSE:
      requestId = &op.reserve_entity_ids_response.request_id;
      isResponse = true;
      break;
    case WORKER_OP_TYPE_CREATE_ENTITY_RESPONSE:
      requestId = &op.create_entity_response.request_id;
      isResponse = true;
      break;
    case WORKER_OP_TYPE_DELETE_ENTITY_RESPONSE:
      requestId = &op.delete_entity_response.request_id;
      isResponse = true;
      break;
    case WORKER_OP_TYPE_ENTITY_QUERY_RESPONSE:
      requestId = &op.entity_query_response.request_id;
      isResponse = true;
      break;
    default:
      requestId = nullptr;
      isResponse = false;
      break;
    }

    // Change the request ID and delete the bookkeeping.
    if (isResponse) {
      const auto it = internalToPublicRequestIds.find(*requestId);
      *requestId = it->second;
      internalToPublicRequestIds.erase(it);
    }
  }

  receivedOpsQueue.emplace(std::make_unique<ReceivedOpList>(opList));
}

void SpatialOsConnectionHandler::SendAndClearMessages(MessagesToSend* messages) {
  Worker_UpdateParameters loopback{/*loopback=*/0};
  Worker_CommandParameters shortCircuit{/*allow_shot_circuit=*/0};

  for (auto& update : messages->Updates) {
    Worker_ComponentUpdate u{nullptr, update.Update.GetComponentId(),
                             std::move(update.Update).Release(), nullptr};
    Worker_Alpha_Connection_SendComponentUpdate(connection.get(), update.EntityId, &u, &loopback);
  }

  for (auto& component : messages->ComponentsAdded) {
    Worker_ComponentData d{nullptr, component.Data.GetComponentId(),
                           std::move(component.Data).Release(), nullptr};
    Worker_Connection_SendAddComponent(connection.get(), component.EntityId, &d, &loopback);
  }

  for (auto& component : messages->ComponentsRemoved) {
    Worker_Connection_SendRemoveComponent(connection.get(), component.EntityId,
                                          component.ComponentId, &loopback);
  }

  for (auto& request : messages->CommandRequests) {
    const auto commandIndex = request.Request.GetCommandIndex();
    auto r = Worker_CommandRequest{nullptr, request.Request.GetComponentId(),
                                   std::move(request.Request).Release(), nullptr};
    const auto id = Worker_Connection_SendCommandRequest(
        connection.get(), request.EntityId, &r, commandIndex, &request.Timeout, &shortCircuit);
    internalToPublicRequestIds[id] = request.RequestId;
  }

  for (auto& response : messages->CommandResponse) {
    auto r = Worker_CommandResponse{nullptr, response.Response.GetComponentId(),
                                    std::move(response.Response).Release(), nullptr};
    Worker_Connection_SendCommandResponse(connection.get(), response.RequestId, &r);
  }

  for (auto& failure : messages->CommandFailures) {
    Worker_Connection_SendCommandFailure(connection.get(), failure.RequestId,
                                         failure.Message.c_str());
  }

  for (auto& request : messages->ReserveEntityIdsRequests) {
    const auto id = Worker_Connection_SendReserveEntityIdsRequest(
        connection.get(), request.NumberOfEntityIds, &request.TimeoutMillis);
    internalToPublicRequestIds[id] = request.RequestId;
  }

  for (auto& request : messages->CreateEntityRequests) {
    auto components = std::move(request.Entity).ReleaseComponentData();
    EntityId* entityIdPtr = request.EntityId == 0 ? nullptr : &request.EntityId;
    const auto id = Worker_Connection_SendCreateEntityRequest(
        connection.get(), static_cast<std::uint32_t>(components.size()), components.data(),
        entityIdPtr, &request.TimeoutMillis);
    internalToPublicRequestIds[id] = request.RequestId;
  }

  for (auto& request : messages->DeleteEntityRequests) {
    const auto id = Worker_Connection_SendDeleteEntityRequest(connection.get(), request.EntityId,
                                                              &request.TimeoutMillis);
    internalToPublicRequestIds[id] = request.RequestId;
  }

  for (auto& request : messages->EntityQueryRequests) {
    const auto id = Worker_Connection_SendEntityQueryRequest(
        connection.get(), request.Query.GetWorkerQuery(), &request.TimeoutMillis);
    internalToPublicRequestIds[id] = request.RequestId;
  }

  for (auto& ack : messages->AuthorityLossAcks) {
    Worker_Connection_SendAuthorityLossImminentAcknowledgement(connection.get(), ack.EntityId,
                                                               ack.ComponentId);
  }

  for (auto& interest : messages->InterestChanges) {
    Worker_Connection_SendComponentInterest(connection.get(), interest.EntityId,
                                            interest.Override.data(),
                                            static_cast<std::uint32_t>(interest.Override.size()));
  }

  for (auto& log : messages->Logs) {
    EntityId* entityId = log.EntityId == 0 ? nullptr : &log.EntityId;
    const auto level = static_cast<uint8_t>(log.Level);
    Worker_LogMessage l{level, log.LoggerName.c_str(), log.Message.c_str(), entityId};
    Worker_Connection_SendLogMessage(connection.get(), &l);
  }

  for (auto& metrics : messages->Metrics) {
    Worker_Connection_SendMetrics(connection.get(), metrics.Metrics.GetMetrics());
  }
}

void SpatialOsConnectionHandler::ConnectionDeleter::operator()(Worker_Connection* connection) const
    noexcept {
  Worker_Connection_Destroy(connection);
}

}  // namespace gdk
