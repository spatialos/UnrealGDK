#include "gdk/worker_view.h"

#include "gdk/messages_to_send.h"
#include <cstdint>
#include <utility>

namespace gdk {

WorkerView::WorkerView(const ComponentRanges& componentRanges)
: delta(componentRanges) {}

void WorkerView::EnqueueOpList(OpList&& opList) {
  opListQueue.push_back(std::move(opList));
}

const ViewDelta* WorkerView::GetNextViewDelta() {
  usedOpLists.clear();
  delta.Clear();
  while (!opListQueue.empty()) {
    const auto result = ProcessOpList(opListQueue.front());
    if (result.InUnclosedCriticalSection) {
      MarkOpsAsRead(opListQueue.front(), result.NextOpToProcessIndex);
      break;
    }
    usedOpLists.emplace_back(std::move(opListQueue.front()));
    opListQueue.pop_front();
  }
  return &delta;
}

MessagesToSend WorkerView::FlushLocalChanges() {
  ApplyLocalChanges(currentMessages);
  auto temp = std::move(currentMessages);
  currentMessages = MessagesToSend{};
  return temp;
}

void WorkerView::SendLogMessage(LogMessage logMessage) {
  currentMessages.Logs.emplace_back(std::move(logMessage));
}

void WorkerView::SendMetricsMessage(MetricsMessage metrics) {
  currentMessages.Metrics.emplace_back(std::move(metrics));
}

void WorkerView::SendUpdate(EntityComponentUpdate update) {
  currentMessages.Updates.emplace_back(std::move(update));
}

void WorkerView::SendAddComponent(EntityComponentData data) {
  currentMessages.ComponentsAdded.emplace_back(std::move(data));
}

void WorkerView::SendRemoveComponent(EntityComponentId entityComponentId) {
  currentMessages.ComponentsRemoved.emplace_back(entityComponentId);
}

void WorkerView::SendCommandRequest(CommandRequestToSend request) {
  currentMessages.CommandRequests.emplace_back(std::move(request));
}

void WorkerView::SendCommandResponse(CommandResponseToSend response) {
  currentMessages.CommandResponse.emplace_back(std::move(response));
}

void WorkerView::SendCommandFailure(CommandFailure failure) {
  currentMessages.CommandFailures.emplace_back(std::move(failure));
}

void WorkerView::SendReserveEntityIdsRequest(ReserveEntityIdsRequest request) {
  currentMessages.ReserveEntityIdsRequests.emplace_back(request);
}

void WorkerView::SendCreateEntityRequest(CreateEntityRequest request) {
  currentMessages.CreateEntityRequests.emplace_back(std::move(request));
}

void WorkerView::SendDeleteEntityRequest(DeleteEntityRequest request) {
  currentMessages.DeleteEntityRequests.emplace_back(request);
}

void WorkerView::SendEntityQueryRequest(EntityQueryRequest request) {
  currentMessages.EntityQueryRequests.emplace_back(std::move(request));
}

void WorkerView::SendAuthorityLossAck(EntityComponentId entityComponentId) {
  currentMessages.AuthorityLossAcks.emplace_back(entityComponentId);
}

void WorkerView::SendInterestChange(InterestChange interestChanges) {
  currentMessages.InterestChanges.emplace_back(std::move(interestChanges));
}

bool WorkerView::ContainsCriticalSectionOp(const OpList& opList, size_t startIndex) {
  for (size_t i = startIndex; i < opList.GetCount(); ++i) {
    if (opList[i].op_type == WORKER_OP_TYPE_CRITICAL_SECTION) {
      return true;
    }
  }
  return false;
}

void WorkerView::MarkOpsAsRead(OpList& opList, size_t indexReadTo) {
  for (size_t i = 0; i < indexReadTo; ++i) {
    // todo replace this with a None enum value.
    opList[i].op_type = 0;
  }
}

bool WorkerView::IsCriticalSectionClosed(size_t criticalSectionStartIndex) const {
  // First check the partially processed op list starting just after the open critical section.
  if (ContainsCriticalSectionOp(opListQueue.front(), criticalSectionStartIndex + 1)) {
    return true;
  }
  // Check all other queued critical sections.
  for (size_t i = 1; i < opListQueue.size(); ++i) {
    if (ContainsCriticalSectionOp(opListQueue[i], 0)) {
      return true;
    }
  }
  return false;
}

void WorkerView::ApplyLocalChanges(const MessagesToSend& messagesToSend) {
  for (const auto& component : messagesToSend.ComponentsAdded) {
    view.AddComponent(component.EntityId, component.Data.GetComponentId());
  }
  for (const auto& component : messagesToSend.ComponentsRemoved) {
    view.RemoveComponent(component.EntityId, component.ComponentId);
    delta.RemoveCommandRequests(component.EntityId, component.ComponentId);
  }
}

WorkerView::ProcessOpsResult WorkerView::ProcessOpList(const OpList& opList) {
  for (size_t i = 0; i < opList.GetCount(); ++i) {
    const auto& op = opList[i];
    switch (static_cast<Worker_OpType>(op.op_type)) {
    case WORKER_OP_TYPE_DISCONNECT:
      delta.SetConnectionStatus(
          static_cast<Worker_ConnectionStatusCode>(op.op.disconnect.connection_status_code),
          op.op.disconnect.reason);
      break;
    case WORKER_OP_TYPE_FLAG_UPDATE:
      delta.AddFlagChange(op.op.flag_update);
      break;
    case WORKER_OP_TYPE_LOG_MESSAGE:
      delta.AddLogMessage(op.op.log_message);
      break;
    case WORKER_OP_TYPE_METRICS:
      delta.AddMetrics(op.op.metrics);
      break;
    case WORKER_OP_TYPE_CRITICAL_SECTION:
      // If a critical section is started then check if the closing op is also queued.
      // If it is not then then record where we got to and stop processing.
      if (op.op.critical_section.in_critical_section && !IsCriticalSectionClosed(i)) {
        return ProcessOpsResult{true, i};
      }
      break;
    case WORKER_OP_TYPE_ADD_ENTITY:
      HandleAddEntity(op.op.add_entity);
      break;
    case WORKER_OP_TYPE_REMOVE_ENTITY:
      HandleRemoveEntity(op.op.remove_entity);
      break;
    case WORKER_OP_TYPE_RESERVE_ENTITY_IDS_RESPONSE:
      HandleReserveEntityIdsResponse(op.op.reserve_entity_ids_response);
      break;
    case WORKER_OP_TYPE_CREATE_ENTITY_RESPONSE:
      HandleCreateEntityResponse(op.op.create_entity_response);
      break;
    case WORKER_OP_TYPE_DELETE_ENTITY_RESPONSE:
      HandleDeleteEntityResponse(op.op.delete_entity_response);
      break;
    case WORKER_OP_TYPE_ENTITY_QUERY_RESPONSE:
      HandleEntityQueryResponse(op.op.entity_query_response);
      break;
    case WORKER_OP_TYPE_ADD_COMPONENT:
      HandleAddComponent(op.op.add_component);
      break;
    case WORKER_OP_TYPE_REMOVE_COMPONENT:
      HandleRemoveComponent(op.op.remove_component);
      break;
    case WORKER_OP_TYPE_AUTHORITY_CHANGE:
      HandleAuthorityChange(op.op.authority_change);
      break;
    case WORKER_OP_TYPE_COMPONENT_UPDATE:
      HandleComponentUpdate(op.op.component_update);
      break;
    case WORKER_OP_TYPE_COMMAND_REQUEST:
      HandleCommandRequest(op.op.command_request);
      break;
    case WORKER_OP_TYPE_COMMAND_RESPONSE:
      HandleCommandResponse(op.op.command_response);
      break;
    default:
      // Ignore other ops.
      // todo replace this with a gdk op list enum.
      break;
    }
  }
  return ProcessOpsResult{};
}

void WorkerView::HandleAddEntity(const Worker_AddEntityOp& addEntity) {
  view.AddEntity(addEntity.entity_id);
  delta.AddEntity(addEntity.entity_id);
}

void WorkerView::HandleRemoveEntity(const Worker_RemoveEntityOp& removeEntity) {
  view.RemoveEntity(removeEntity.entity_id);
  delta.RemoveEntity(removeEntity.entity_id);
}

void WorkerView::HandleReserveEntityIdsResponse(const Worker_ReserveEntityIdsResponseOp& response) {
  ReserveEntityIdsResponse r{response.request_id,
                             static_cast<Worker_StatusCode>(response.status_code), response.message,
                             response.first_entity_id, response.number_of_entity_ids};
  delta.AddReserveEntityIdsResponse(std::move(r));
}

void WorkerView::HandleCreateEntityResponse(const Worker_CreateEntityResponseOp& response) {
  CreateEntityResponse r{response.request_id, static_cast<Worker_StatusCode>(response.status_code),
                         response.message, response.entity_id};
  delta.AddCreateEntityResponse(std::move(r));
}

void WorkerView::HandleDeleteEntityResponse(const Worker_DeleteEntityResponseOp& response) {
  DeleteEntityResponse r{response.request_id, response.entity_id,
                         static_cast<Worker_StatusCode>(response.status_code), response.message};
  delta.AddDeleteEntityResponse(std::move(r));
}

void WorkerView::HandleEntityQueryResponse(const Worker_EntityQueryResponseOp& response) {
  const std::uint32_t entityCount = response.results == nullptr ? 0 : response.result_count;
  EntityQueryResponse r{response.request_id, static_cast<Worker_StatusCode>(response.status_code),
                        response.message, response.result_count, std::vector<EntitySnapshot>()};
  r.Entities.reserve(entityCount);
  for (std::uint32_t i = 0; i < entityCount; ++i) {
    r.Entities.emplace_back(EntitySnapshot{response.results[i]});
  }
  delta.AddEntityQueryResponse(std::move(r));
}

void WorkerView::HandleAddComponent(const Worker_AddComponentOp& addComponent) {
  // If the component is present in the view then treat it as an update.
  // If it is not in the view then add it as a new component.
  const auto componentId = addComponent.data.component_id;
  const auto entityId = addComponent.entity_id;

  if (view.HasComponent(entityId, componentId)) {
    HandleCompleteComponentUpdate(addComponent);
  } else {
    view.AddComponent(entityId, componentId);
    delta.AddComponent(entityId, ComponentData{addComponent.data});
  }
}

void WorkerView::HandleRemoveComponent(const Worker_RemoveComponentOp& removeComponent) {
  if (!view.HasComponent(removeComponent.entity_id, removeComponent.component_id)) {
    return;
  }
  view.RemoveComponent(removeComponent.entity_id, removeComponent.component_id);
  delta.RemoveComponent(removeComponent.entity_id, removeComponent.component_id);
}

void WorkerView::HandleAuthorityChange(const Worker_AuthorityChangeOp& authorityChange) {
  delta.ChangeAuthority(authorityChange.entity_id, authorityChange.component_id,
                        static_cast<Worker_Authority>(authorityChange.authority));
}

void WorkerView::HandleComponentUpdate(const Worker_ComponentUpdateOp& componentUpdate) {
  delta.AddUpdate(componentUpdate.entity_id, ComponentUpdate{componentUpdate.update});
}

void WorkerView::HandleCompleteComponentUpdate(const Worker_AddComponentOp& completeUpdate) {
  delta.AddComponentAsUpdate(completeUpdate.entity_id, ComponentData{completeUpdate.data});
}

void WorkerView::HandleCommandRequest(const Worker_CommandRequestOp& request) {
  if (!view.HasComponent(request.entity_id, request.request.component_id)) {
    return;
  }
  CommandRequestReceived r{request.entity_id,
                           request.request_id,
                           CommandRequest{request.request},
                           request.timeout_millis,
                           std::string{request.caller_worker_id},
                           std::vector<std::string>(request.caller_attribute_set.attribute_count)};
  for (std::uint32_t i = 0; i < request.caller_attribute_set.attribute_count; ++i) {
    r.CallerAttributeSet[i] = request.caller_attribute_set.attributes[i];
  }
  delta.AddCommandRequest(std::move(r));
}

void WorkerView::HandleCommandResponse(const Worker_CommandResponseOp& response) {
  const auto status = static_cast<Worker_StatusCode>(response.status_code);
  // If the response was not successful then the op will not contain a valid response.
  // Todo consider replacing with an option type.
  CommandResponse payload = status == WORKER_STATUS_CODE_SUCCESS
      ? CommandResponse{response.response}
      : CommandResponse{response.response.component_id, response.response.command_index};
  CommandResponseReceived r{response.request_id, std::move(payload), status,
                            std::string{response.message}};
  delta.AddCommandResponse(std::move(r));
}

}  // namespace gdk
