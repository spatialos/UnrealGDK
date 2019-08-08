#include "gdk/spatialos_worker.h"

#include "gdk/abstract_connection_handler.h"
#include "gdk/internal/worker_view.h"
#include "gdk/messages_to_send.h"
#include <WorkerSDK/improbable/c_worker.h>
#include <cstddef>
#include <utility>

namespace gdk {

SpatialOsWorker::SpatialOsWorker(std::unique_ptr<AbstractConnectionHandler> connectionHandler,
         const ComponentRanges& componentRanges)
: connectionHandler(std::move(connectionHandler))
, workerView(componentRanges)
, currentViewDelta(workerView.GetNextViewDelta()) {}

void SpatialOsWorker::Advance() {
  connectionHandler->SendMessages(workerView.FlushLocalChanges());
  connectionHandler->Advance();
  const size_t numberOfOpLists = connectionHandler->GetOpListCount();
  for (size_t i = 0; i < numberOfOpLists; ++i) {
    workerView.EnqueueOpList(connectionHandler->GetNextOpList());
  }
  currentViewDelta = workerView.GetNextViewDelta();
}

void SpatialOsWorker::SendLogMessage(Worker_LogLevel level, EntityId entityId, std::string loggerName,
                         std::string message) {
  workerView.SendLogMessage(LogMessage{level, entityId, std::move(loggerName), std::move(message)});
}

void SpatialOsWorker::SendMetricsMessage(Metrics metrics) {
  workerView.SendMetricsMessage(MetricsMessage{std::move(metrics)});
}

void SpatialOsWorker::SendUpdate(EntityId entityId, ComponentUpdate&& update) {
  workerView.SendUpdate(EntityComponentUpdate{entityId, std::move(update)});
}

void SpatialOsWorker::AddComponent(EntityId entityId, ComponentData&& data) {
  workerView.AddComponent(EntityComponentData{entityId, std::move(data)});
}

void SpatialOsWorker::RemoveComponent(EntityId entityId, ComponentId componentId) {
  workerView.RemoveComponent(EntityComponentId{entityId, componentId});
}

void SpatialOsWorker::SendCommandRequest(EntityId entityId, RequestId requestId, CommandRequest&& request,
                             std::uint32_t timeout) {
  workerView.SendCommandRequest(
      CommandRequestToSend{entityId, requestId, std::move(request), timeout});
}

void SpatialOsWorker::SendCommandResponse(RequestId requestId, CommandResponse&& response) {
  workerView.SendCommandResponse(CommandResponseToSend{requestId, std::move(response)});
}

void SpatialOsWorker::SendCommandFailure(RequestId requestId, std::string message) {
  workerView.SendCommandFailure(CommandFailure{requestId, std::move(message)});
}

void SpatialOsWorker::SendReserveEntityIdsRequest(RequestId requestId, std::uint32_t numberOfEntityIds,
                                      std::uint32_t timeoutMillis) {
  workerView.SendReserveEntityIdsRequest(
      ReserveEntityIdsRequest{requestId, numberOfEntityIds, timeoutMillis});
}

void SpatialOsWorker::SendCreateEntityRequest(RequestId requestId, EntityState&& entity, EntityId entityId,
                                  std::uint32_t timeoutMillis) {
  workerView.SendCreateEntityRequest(
      CreateEntityRequest{requestId, std::move(entity), entityId, timeoutMillis});
}

void SpatialOsWorker::SendDeleteEntityRequest(RequestId requestId, EntityId entityId,
                                  std::uint32_t timeoutMillis) {
  workerView.SendDeleteEntityRequest(DeleteEntityRequest{requestId, entityId, timeoutMillis});
}

void SpatialOsWorker::SendEntityQueryRequest(RequestId requestId, EntityQuery&& query,
                                 std::uint32_t timeoutMillis) {
  workerView.SendEntityQueryRequest(EntityQueryRequest{requestId, std::move(query), timeoutMillis});
}

void SpatialOsWorker::SendAuthorityLossAck(EntityId entityId, ComponentId componentId) {
  workerView.SendAuthorityLossAck(EntityComponentId{entityId, componentId});
}

void SpatialOsWorker::SendInterestChange(EntityId entityId,
                             std::vector<Worker_InterestOverride> interestOverrides) {
  workerView.SendInterestChange(InterestChange{entityId, std::move(interestOverrides)});
}

const std::vector<VisibleEntity>& SpatialOsWorker::GetEntitiesAdded() const {
  return currentViewDelta->GetEntitiesAdded();
}

const std::vector<EntityId>& SpatialOsWorker::GetEntitiesRemoved() const {
  return currentViewDelta->GetEntitiesRemoved();
}

const std::vector<EntityComponentData>& SpatialOsWorker::GetComponentsAdded(ComponentId componentId) const {
  return currentViewDelta->GetComponentsAdded(componentId);
}

const std::vector<EntityComponentId>& SpatialOsWorker::GetComponentsRemoved(ComponentId componentId) const {
  return currentViewDelta->GetComponentsRemoved(componentId);
}

const std::vector<EntityComponentUpdate>& SpatialOsWorker::GetComponentUpdates(ComponentId componentId) const {
  return currentViewDelta->GetComponentUpdates(componentId);
}

const std::vector<EntityComponentId>& SpatialOsWorker::GetAuthorityGained(ComponentId componentId) const {
  return currentViewDelta->GetAuthorityGained(componentId);
}

const std::vector<EntityComponentId>& SpatialOsWorker::GetAuthorityLost(ComponentId componentId) const {
  return currentViewDelta->GetAuthorityLost(componentId);
}

const std::vector<EntityComponentId>& SpatialOsWorker::GetAuthorityLossImminent(ComponentId componentId) const {
  return currentViewDelta->GetAuthorityLossImminent(componentId);
}

const std::vector<EntityComponentId>&
SpatialOsWorker::GetAuthorityLostTemporarily(ComponentId componentId) const {
  return currentViewDelta->GetAuthorityLostTemporarily(componentId);
}

const std::vector<CommandRequestReceived>& SpatialOsWorker::GetCommandRequests(ComponentId componentId) const {
  return currentViewDelta->GetCommandRequests(componentId);
}

const std::vector<CommandResponseReceived>&
SpatialOsWorker::GetCommandResponses(ComponentId componentId) const {
  return currentViewDelta->GetCommandResponses(componentId);
}

const std::vector<ReserveEntityIdsResponse>& SpatialOsWorker::GetReserveEntityIdsResponses() const {
  return currentViewDelta->GetReserveEntityIdsResponses();
}

const std::vector<CreateEntityResponse>& SpatialOsWorker::GetCreateEntityResponses() const {
  return currentViewDelta->GetCreateEntityResponses();
}

const std::vector<DeleteEntityResponse>& SpatialOsWorker::GetDeleteEntityResponses() const {
  return currentViewDelta->GetDeleteEntityResponses();
}

const std::vector<EntityQueryResponse>& SpatialOsWorker::GetEntityQueryResponses() const {
  return currentViewDelta->GetEntityQueryResponses();
}

const std::vector<Worker_LogMessageOp>& SpatialOsWorker::GetLogsReceived() const {
  return currentViewDelta->GetLogMessages();
}

const std::vector<Worker_FlagUpdateOp>& SpatialOsWorker::GetFlagChanges() const {
  return currentViewDelta->GetFlagChanges();
}

const std::vector<Worker_MetricsOp>& SpatialOsWorker::GetMetrics() const {
  return currentViewDelta->GetMetrics();
}

const std::vector<EntityComponentData>& SpatialOsWorker::GetCompleteUpdates(ComponentId componentId) const {
  return currentViewDelta->GetComponentCompleteUpdates(componentId);
}

const std::vector<EntityComponentUpdate>& SpatialOsWorker::GetEvents(ComponentId componentId) const {
  return currentViewDelta->GetComponentUpdatesEvents(componentId);
}

const std::string& SpatialOsWorker::GetWorkerId() const {
  return connectionHandler->GetWorkerId();
}

const std::vector<std::string>& SpatialOsWorker::GetWorkerAttributes() const {
  return connectionHandler->GetWorkerAttributes();
}

bool SpatialOsWorker::HasConnectionStatusChanged() const {
  return currentViewDelta->ReceivedDisconnect();
}

Worker_ConnectionStatusCode SpatialOsWorker::GetConnectionStatusCode() const {
  return currentViewDelta->GetConnectionStatusCode();
}

const std::string& SpatialOsWorker::GetConnectionMessage() const {
  return currentViewDelta->GetConnectionMessage();
}

} // namespace gdk
