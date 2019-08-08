#include "gdk/internal/view_delta.h"

#include <utility>

namespace {
// Returns the value if the element exists and defaultValue otherwise.
template <typename Index, typename Value>
const Value& GetOrDefault(const std::unordered_map<Index, Value>& map, Index index,
                          const Value& defaultValue) {
  auto it = map.find(index);
  if (it == map.end()) {
    return defaultValue;
  }
  return it->second;
}
}  // anonymous namespace

namespace gdk {

const ComponentRangeDataStorage ViewDelta::kEmptyData;
const ComponentRangeUpdateStorage ViewDelta::kEmptyUpdate;
const ComponentRangeRequestStorage ViewDelta::kEmptyRequest;
const ComponentRangeResponseStorage ViewDelta::kEmptyResponse;
const AuthorityChangeStorage ViewDelta::kEmptyAuthority;

ViewDelta::ViewDelta(ComponentRanges ranges)
: connectionMessageReceived(false), connectionStatusCode(), componentRanges(std::move(ranges)) {}

void ViewDelta::AddEntity(VisibleEntity&& entity) {
  GetEntityChangeStorage().TryAddEntity(std::move(entity));
}

void ViewDelta::RemoveEntity(EntityId entityId) {
  GetEntityChangeStorage().RemoveEntity(entityId);
}

void ViewDelta::AddComponent(EntityId entityId, ComponentData&& data) {
  auto& entityStorage = GetEntityChangeStorage();
  entityStorage.TryAddComponentId(entityId, data.GetComponentId());
  auto& componentAddedStorage = GetOrCreateDataStorage(data.GetComponentId());
  if (componentAddedStorage.TryAddComponent(entityId, TryMove(data))) {
    return;
  }
  // If the component can not be added as new (because it was removed in the same tick) then apply
  // it as an update.
  GetOrCreateUpdateStorage(data.GetComponentId()).AddComponentUpdate(entityId, std::move(data));
}

void ViewDelta::RemoveComponent(EntityId entityId, ComponentId componentId) {
  GetEntityChangeStorage().TryRemoveComponent(entityId, componentId);
  GetOrCreateDataStorage(componentId).RemoveComponent(entityId, componentId);
  GetOrCreateUpdateStorage(componentId).RemoveComponent(entityId, componentId);
  GetOrCreateAuthorityStorage(componentId).RemoveComponent(entityId, componentId);
}

void ViewDelta::AddComponentAsUpdate(EntityId entityId, ComponentData&& data) {
  // Try to overwrite components added.
  // If that fails add it as a new update.
  auto& dataStorage = GetOrCreateDataStorage(data.GetComponentId());
  if (dataStorage.TryApplyComponentData(entityId, TryMove(data))) {
    return;
  }
  GetOrCreateUpdateStorage(data.GetComponentId()).AddComponentUpdate(entityId, std::move(data));
}

void ViewDelta::AddUpdate(EntityId entityId, ComponentUpdate&& update) {
  // Try to overwrite components added.
  // If that fails add it as a new update.
  auto& dataStorage = GetOrCreateDataStorage(update.GetComponentId());
  if (dataStorage.TryMergeComponentUpdate(entityId, update)) {
    return;
  }
  GetOrCreateUpdateStorage(update.GetComponentId()).AddComponentUpdate(entityId, std::move(update));
}

void ViewDelta::ChangeAuthority(EntityId entityId, ComponentId componentId,
                                Worker_Authority authority) {
  GetOrCreateAuthorityStorage(componentId).SetAuthority(entityId, componentId, authority);
  if (authority == WORKER_AUTHORITY_NOT_AUTHORITATIVE) {
    RemoveCommandRequests(entityId, componentId);
  }
}

void ViewDelta::AddCommandRequest(CommandRequestReceived&& request) {
  GetOrCreateRequestStorage(request.Request.GetComponentId()).AddRequest(std::move(request));
}

void ViewDelta::AddCommandResponse(CommandResponseReceived&& response) {
  GetOrCreateResponseStorage(response.Response.GetComponentId()).AddResponse(std::move(response));
}

void ViewDelta::AddReserveEntityIdsResponse(ReserveEntityIdsResponse&& response) {
  GetWorldCommandStorage().AddReserveEntityIdsResponse(std::move(response));
}

void ViewDelta::AddCreateEntityResponse(CreateEntityResponse&& response) {
  GetWorldCommandStorage().AddCreateEntityResponse(std::move(response));
}

void ViewDelta::AddDeleteEntityResponse(DeleteEntityResponse&& response) {
  GetWorldCommandStorage().AddDeleteEntityResponse(std::move(response));
}

void ViewDelta::AddEntityQueryResponse(EntityQueryResponse&& response) {
  GetWorldCommandStorage().AddEntityQueryResponse(std::move(response));
}

void ViewDelta::RemoveCommandRequests(EntityId entityId, ComponentId componentId) {
  GetOrCreateRequestStorage(componentId).RemoveEntityComponent(entityId, componentId);
}

void ViewDelta::AddLogMessage(Worker_LogMessageOp logMessageOp) {
  logs.emplace_back(logMessageOp);
}

void ViewDelta::AddMetrics(Worker_MetricsOp metricsOp) {
  metrics.emplace_back(metricsOp);
}

void ViewDelta::AddFlagChange(Worker_FlagUpdateOp flagsOp) {
  flags.emplace_back(flagsOp);
}

void ViewDelta::SetConnectionStatus(Worker_ConnectionStatusCode status,
                                    const std::string& message) {
  connectionMessageReceived = true;
  connectionStatusCode = status;
  connectionStatusMessage = message;
}

const std::vector<VisibleEntity>& ViewDelta::GetEntitiesAdded() const {
  return GetEntityChangeStorage().GetEntitiesAdded();
}

const std::vector<EntityId>& ViewDelta::GetEntitiesRemoved() const {
  return GetEntityChangeStorage().GetEntitiesRemoved();
}

const std::vector<EntityComponentData>&
ViewDelta::GetComponentsAdded(ComponentId componentId) const {
  return GetDataStorage(componentId).GetComponentsAdded();
}

const std::vector<EntityComponentId>&
ViewDelta::GetComponentsRemoved(ComponentId componentId) const {
  return GetDataStorage(componentId).GetComponentsRemoved();
}

const std::vector<EntityComponentUpdate>&
ViewDelta::GetComponentUpdates(ComponentId componentId) const {
  return GetUpdateStorage(componentId).GetUpdates();
}

const std::vector<EntityComponentId>& ViewDelta::GetAuthorityGained(ComponentId componentId) const {
  return GetAuthorityStorage(componentId).GetAuthorityGained();
}

const std::vector<EntityComponentId>& ViewDelta::GetAuthorityLost(ComponentId componentId) const {
  return GetAuthorityStorage(componentId).GetAuthorityLost();
}

const std::vector<EntityComponentId>&
ViewDelta::GetAuthorityLossImminent(ComponentId componentId) const {
  return GetAuthorityStorage(componentId).GetAuthorityLossImminent();
}

const std::vector<EntityComponentId>&
ViewDelta::GetAuthorityLostTemporarily(ComponentId componentId) const {
  return GetAuthorityStorage(componentId).GetAuthorityLostTemporarily();
}

const std::vector<CommandRequestReceived>&
ViewDelta::GetCommandRequests(ComponentId componentId) const {
  return GetRequestStorage(componentId).GetRequests();
}

const std::vector<CommandResponseReceived>&
ViewDelta::GetCommandResponses(ComponentId componentId) const {
  return GetResponseStorage(componentId).GetResponses();
}

const std::vector<ReserveEntityIdsResponse>& ViewDelta::GetReserveEntityIdsResponses() const {
  return GetWorldCommandStorage().GetReserveEntityIdsResponses();
}

const std::vector<CreateEntityResponse>& ViewDelta::GetCreateEntityResponses() const {
  return GetWorldCommandStorage().GetCreateEntityResponses();
}

const std::vector<DeleteEntityResponse>& ViewDelta::GetDeleteEntityResponses() const {
  return GetWorldCommandStorage().GetDeleteEntityResponses();
}

const std::vector<EntityQueryResponse>& ViewDelta::GetEntityQueryResponses() const {
  return GetWorldCommandStorage().GetEntityQueryResponses();
}

const std::vector<EntityComponentData>&
ViewDelta::GetComponentCompleteUpdates(ComponentId componentId) const {
  return GetUpdateStorage(componentId).GetCompleteUpdates();
}

const std::vector<EntityComponentUpdate>&
ViewDelta::GetComponentUpdatesEvents(ComponentId componentId) const {
  return GetUpdateStorage(componentId).GetEvents();
}

const std::vector<Worker_LogMessageOp>& ViewDelta::GetLogMessages() const {
  return logs;
}

const std::vector<Worker_MetricsOp>& ViewDelta::GetMetrics() const {
  return metrics;
}

const std::vector<Worker_FlagUpdateOp>& ViewDelta::GetFlagChanges() const {
  return flags;
}

bool ViewDelta::ReceivedDisconnect() const {
  return connectionMessageReceived;
}

Worker_ConnectionStatusCode ViewDelta::GetConnectionStatusCode() const {
  return connectionStatusCode;
}

const std::string& ViewDelta::GetConnectionMessage() const {
  return connectionStatusMessage;
}

void ViewDelta::Clear() {
  connectionMessageReceived = false;

  entityChangeStorage.Clear();
  worldCommandStorage.Clear();

  for (auto& it : idToDataStorage) {
    it.second.Clear();
  }

  for (auto& it : idToUpdateStorage) {
    it.second.Clear();
  }

  for (auto& it : idToRequestStorage) {
    it.second.Clear();
  }

  for (auto& it : idToResponseStorage) {
    it.second.Clear();
  }

  for (auto& it : idToAuthorityStorage) {
    it.second.Clear();
  }

  metrics.clear();
  logs.clear();
  flags.clear();
}

WorldCommandStorage& ViewDelta::GetWorldCommandStorage() {
  return worldCommandStorage;
}

EntityVisibilityChangeStorage& ViewDelta::GetEntityChangeStorage() {
  return entityChangeStorage;
}

ComponentRangeDataStorage& ViewDelta::GetOrCreateDataStorage(ComponentId id) {
  const auto equivalentId = componentRanges.GetRangeEquivalentId(id);
  return idToDataStorage[equivalentId];
}

ComponentRangeUpdateStorage& ViewDelta::GetOrCreateUpdateStorage(ComponentId id) {
  const auto equivalentId = componentRanges.GetRangeEquivalentId(id);
  return idToUpdateStorage[equivalentId];
}

AuthorityChangeStorage& ViewDelta::GetOrCreateAuthorityStorage(ComponentId id) {
  const auto equivalentId = componentRanges.GetRangeEquivalentId(id);
  return idToAuthorityStorage[equivalentId];
}

ComponentRangeRequestStorage& ViewDelta::GetOrCreateRequestStorage(ComponentId id) {
  const auto equivalentId = componentRanges.GetRangeEquivalentId(id);
  return idToRequestStorage[equivalentId];
}

ComponentRangeResponseStorage& ViewDelta::GetOrCreateResponseStorage(ComponentId id) {
  const auto equivalentId = componentRanges.GetRangeEquivalentId(id);
  return idToResponseStorage[equivalentId];

}

const WorldCommandStorage& ViewDelta::GetWorldCommandStorage() const {
  return worldCommandStorage;
}

const EntityVisibilityChangeStorage& ViewDelta::GetEntityChangeStorage() const {
  return entityChangeStorage;
}

const ComponentRangeDataStorage& ViewDelta::GetDataStorage(ComponentId id) const {
  const auto equivalentId = componentRanges.GetRangeEquivalentId(id);
  return GetOrDefault(idToDataStorage, equivalentId, kEmptyData);
}

const ComponentRangeUpdateStorage& ViewDelta::GetUpdateStorage(ComponentId id) const {
  const auto equivalentId = componentRanges.GetRangeEquivalentId(id);
  return GetOrDefault(idToUpdateStorage, equivalentId, kEmptyUpdate);
}

const AuthorityChangeStorage& ViewDelta::GetAuthorityStorage(ComponentId id) const {
  const auto equivalentId = componentRanges.GetRangeEquivalentId(id);
  return GetOrDefault(idToAuthorityStorage, equivalentId, kEmptyAuthority);
}

const ComponentRangeRequestStorage& ViewDelta::GetRequestStorage(ComponentId id) const {
  const auto equivalentId = componentRanges.GetRangeEquivalentId(id);
  return GetOrDefault(idToRequestStorage, equivalentId, kEmptyRequest);
}

const ComponentRangeResponseStorage& ViewDelta::GetResponseStorage(ComponentId id) const {
  const auto equivalentId = componentRanges.GetRangeEquivalentId(id);
  return GetOrDefault(idToResponseStorage, equivalentId, kEmptyResponse);
}

}  // namespace gdk
