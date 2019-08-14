#ifndef GDK_SPATIAL_OS_WORKER_H
#define GDK_SPATIAL_OS_WORKER_H
#include "gdk/abstract_connection_handler.h"
#include "gdk/common_types.h"
#include "gdk/component_ranges.h"
#include "gdk/internal/worker_view.h"
#include "gdk/metrics.h"
#include "gdk/visible_entity.h"
#include <WorkerSDK/improbable/c_worker.h>
#include <memory>
#include <vector>

namespace gdk {

/**
 *  Represents a connection to SpatialOS.
 *  A wrapper around a WorkerView and an AbstractConnectionHandler.
 */
class SpatialOsWorker {
public:
  SpatialOsWorker(std::unique_ptr<AbstractConnectionHandler> connectionHandler,
      const ComponentRanges& componentRanges);

  ~SpatialOsWorker();

  // Moveable, not copyable.
  SpatialOsWorker(const SpatialOsWorker&) = delete;
  SpatialOsWorker(SpatialOsWorker&&) = default;
  SpatialOsWorker& operator=(const SpatialOsWorker&) = delete;
  SpatialOsWorker& operator=(SpatialOsWorker&&) = default;

  void FlushSend();
  void Advance();

  void SendLogMessage(Worker_LogLevel level, EntityId entityId, std::string loggerName,
                      std::string message);
  void SendMetricsMessage(Metrics metrics);
  void SendUpdate(EntityId entityId, ComponentUpdate&& update);
  void AddComponent(EntityId entityId, ComponentData&& data);
  void RemoveComponent(EntityId entityId, ComponentId componentId);
  void SendCommandRequest(EntityId entityId, RequestId requestId, CommandRequest&& request,
                          std::uint32_t timeout);
  void SendCommandResponse(RequestId requestId, CommandResponse&& response);
  void SendCommandFailure(RequestId requestId, std::string message);
  void SendReserveEntityIdsRequest(RequestId requestId, std::uint32_t numberOfEntityIds,
                                   std::uint32_t timeoutMillis);
  void SendCreateEntityRequest(RequestId requestId, EntityState&& entity, EntityId entityId,
                               std::uint32_t timeoutMillis);
  void SendDeleteEntityRequest(RequestId requestId, EntityId entityId, std::uint32_t timeoutMillis);
  void SendEntityQueryRequest(RequestId requestId, EntityQuery&& query,
                              std::uint32_t timeoutMillis);
  void SendAuthorityLossAck(EntityId entityId, ComponentId componentId);
  void SendInterestChange(EntityId entityId,
                          std::vector<Worker_InterestOverride> interestOverrides);

  // todo consider replacing vector with a span?
  const std::vector<VisibleEntity>& GetEntitiesAdded() const;
  const std::vector<EntityId>& GetEntitiesRemoved() const;
  const std::vector<EntityComponentData>& GetComponentsAdded(ComponentId componentId) const;
  const std::vector<EntityComponentId>& GetComponentsRemoved(ComponentId componentId) const;
  const std::vector<EntityComponentUpdate>& GetComponentUpdates(ComponentId componentId) const;
  const std::vector<EntityComponentId>& GetAuthorityGained(ComponentId componentId) const;
  const std::vector<EntityComponentId>& GetAuthorityLost(ComponentId componentId) const;
  const std::vector<EntityComponentId>& GetAuthorityLossImminent(ComponentId componentId) const;
  const std::vector<EntityComponentId>& GetAuthorityLostTemporarily(ComponentId componentId) const;
  const std::vector<CommandRequestReceived>& GetCommandRequests(ComponentId componentId) const;
  const std::vector<CommandResponseReceived>& GetCommandResponses(ComponentId componentId) const;
  const std::vector<ReserveEntityIdsResponse>& GetReserveEntityIdsResponses() const;
  const std::vector<CreateEntityResponse>& GetCreateEntityResponses() const;
  const std::vector<DeleteEntityResponse>& GetDeleteEntityResponses() const;
  const std::vector<EntityQueryResponse>& GetEntityQueryResponses() const;
  const std::vector<Worker_LogMessageOp>& GetLogsReceived() const;
  const std::vector<Worker_FlagUpdateOp>& GetFlagChanges() const;
  const std::vector<Worker_MetricsOp>& GetMetrics() const;

  // These are needed while we can not convert from a data object to an update.
  const std::vector<EntityComponentData>& GetCompleteUpdates(ComponentId componentId) const;
  const std::vector<EntityComponentUpdate>& GetEvents(ComponentId componentId) const;

  const std::string& GetWorkerId() const;
  const std::vector<std::string>& GetWorkerAttributes() const;

  bool HasConnectionStatusChanged() const;
  Worker_ConnectionStatusCode GetConnectionStatusCode() const;
  const std::string& GetConnectionMessage() const;

private:
  std::unique_ptr<AbstractConnectionHandler> connectionHandler;
  WorkerView workerView;

  const ViewDelta* currentViewDelta = nullptr;
};

}  // namespace gdk
#endif  // GDK_SPATIAL_OS_WORKER_H
