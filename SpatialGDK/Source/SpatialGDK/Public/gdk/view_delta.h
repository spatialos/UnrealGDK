#ifndef GDK_VIEW_DELTA_H
#define GDK_VIEW_DELTA_H
#include "common_types.h"
#include "component_ranges.h"
#include "internal/authority_change_storage.h"
#include "internal/component_range_data_storage.h"
#include "internal/component_range_request_storage.h"
#include "internal/component_range_response_storage.h"
#include "internal/component_range_update_storage.h"
#include "internal/entity_visibility_change_storage.h"
#include "internal/world_command_storage.h"
#include <WorkerSDK/improbable/c_worker.h>
#include <unordered_map>
#include <vector>

namespace gdk {

class ViewDelta {
public:
  explicit ViewDelta(ComponentRanges ranges);

  ~ViewDelta() = default;

  // Moveable, not copyable
  ViewDelta(const ViewDelta&) = delete;
  ViewDelta(ViewDelta&&) = default;
  ViewDelta& operator=(const ViewDelta&) = delete;
  ViewDelta& operator=(ViewDelta&&) = default;

  void AddEntity(VisibleEntity&& entity);
  void RemoveEntity(EntityId entityId);
  void AddComponent(EntityId entityId, ComponentData&& data);
  void RemoveComponent(EntityId entityId, ComponentId componentId);
  void AddComponentAsUpdate(EntityId entityId, ComponentData&& data);
  void AddUpdate(EntityId entityId, ComponentUpdate&& update);
  void ChangeAuthority(EntityId entityId, ComponentId componentId, Worker_Authority authority);
  void AddCommandRequest(CommandRequestReceived&& request);
  void AddCommandResponse(CommandResponseReceived&& response);
  void AddReserveEntityIdsResponse(ReserveEntityIdsResponse&& response);
  void AddCreateEntityResponse(CreateEntityResponse&& response);
  void AddDeleteEntityResponse(DeleteEntityResponse&& response);
  void AddEntityQueryResponse(EntityQueryResponse&& response);
  void RemoveCommandRequests(EntityId entityId, ComponentId componentId);

  void AddLogMessage(Worker_LogMessageOp logMessageOp);
  void AddMetrics(Worker_MetricsOp metricsOp);
  void AddFlagChange(Worker_FlagUpdateOp flagsOp);

  void SetConnectionStatus(Worker_ConnectionStatusCode status, const std::string& message);

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

  // These are needed while we can not convert from a data object to an update.
  const std::vector<EntityComponentData>&
  GetComponentCompleteUpdates(ComponentId componentId) const;
  const std::vector<EntityComponentUpdate>&
  GetComponentUpdatesEvents(ComponentId componentId) const;

  const std::vector<Worker_LogMessageOp>& GetLogMessages() const;
  const std::vector<Worker_MetricsOp>& GetMetrics() const;
  const std::vector<Worker_FlagUpdateOp>& GetFlagChanges() const;

  bool ReceivedDisconnect() const;
  Worker_ConnectionStatusCode GetConnectionStatusCode() const;
  const std::string& GetConnectionMessage() const;

  void Clear();

private:
  WorldCommandStorage& GetWorldCommandStorage();
  EntityVisibilityChangeStorage& GetEntityChangeStorage();
  ComponentRangeDataStorage& GetOrCreateDataStorage(ComponentId id);
  ComponentRangeUpdateStorage& GetOrCreateUpdateStorage(ComponentId id);
  AuthorityChangeStorage& GetOrCreateAuthorityStorage(ComponentId id);
  ComponentRangeRequestStorage& GetOrCreateRequestStorage(ComponentId id);
  ComponentRangeResponseStorage& GetOrCreateResponseStorage(ComponentId id);

  const WorldCommandStorage& GetWorldCommandStorage() const;
  const EntityVisibilityChangeStorage& GetEntityChangeStorage() const;
  const ComponentRangeDataStorage& GetDataStorage(ComponentId id) const;
  const ComponentRangeUpdateStorage& GetUpdateStorage(ComponentId id) const;
  const AuthorityChangeStorage& GetAuthorityStorage(ComponentId id) const;
  const ComponentRangeRequestStorage& GetRequestStorage(ComponentId id) const;
  const ComponentRangeResponseStorage& GetResponseStorage(ComponentId id) const;

  static const ComponentRangeDataStorage kEmptyData;
  static const ComponentRangeUpdateStorage kEmptyUpdate;
  static const ComponentRangeRequestStorage kEmptyRequest;
  static const ComponentRangeResponseStorage kEmptyResponse;
  static const AuthorityChangeStorage kEmptyAuthority;

  bool connectionMessageReceived;
  Worker_ConnectionStatusCode connectionStatusCode;
  std::string connectionStatusMessage;

  EntityVisibilityChangeStorage entityChangeStorage;
  WorldCommandStorage worldCommandStorage;

  std::unordered_map<ComponentId, ComponentRangeDataStorage> idToDataStorage;
  std::unordered_map<ComponentId, ComponentRangeUpdateStorage> idToUpdateStorage;
  std::unordered_map<ComponentId, ComponentRangeRequestStorage> idToRequestStorage;
  std::unordered_map<ComponentId, ComponentRangeResponseStorage> idToResponseStorage;
  std::unordered_map<ComponentId, AuthorityChangeStorage> idToAuthorityStorage;

  std::vector<Worker_LogMessageOp> logs;
  std::vector<Worker_MetricsOp> metrics;
  std::vector<Worker_FlagUpdateOp> flags;

  ComponentRanges componentRanges;
};

}  // namespace gdk
#endif  // GDK_VIEW_DELTA_H
