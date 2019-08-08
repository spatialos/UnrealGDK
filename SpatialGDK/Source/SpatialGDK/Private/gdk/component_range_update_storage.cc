#include "gdk/internal/component_range_update_storage.h"

#include "gdk/entity_component_id.h"

namespace gdk {

void ComponentRangeUpdateStorage::AddComponentUpdate(EntityId entityId, ComponentData&& update) {
  EntityComponentData entityComponentData{entityId, std::move(update)};
  const auto id = entityComponentData.GetEntityComponentId();
  data.Insert(std::move(entityComponentData));

  updates.TryMoveUpdateTo(&events, id);
}

void ComponentRangeUpdateStorage::AddComponentUpdate(EntityId entityId, ComponentUpdate&& update) {
  EntityComponentUpdate entityComponentUpdate{entityId, std::move(update)};
  if (data.TryApplyUpdate(entityComponentUpdate)) {
    events.InsertOrMerge(std::move(entityComponentUpdate));
  } else {
    updates.InsertOrMerge(std::move(entityComponentUpdate));
  }
}

void ComponentRangeUpdateStorage::RemoveComponent(EntityId entityId, ComponentId componentId) {
  const EntityComponentId id{entityId, componentId};
  if (data.Remove(id)) {
    events.Remove(id);
  } else {
    updates.Remove(id);
  }
}

void ComponentRangeUpdateStorage::RemoveEntity(EntityId entityId) {
  if (data.ClearEntityId(entityId)) {
    events.ClearEntityId(entityId);
  }
  updates.ClearEntityId(entityId);
}

void ComponentRangeUpdateStorage::Clear() {
  data.Clear();
  updates.Clear();
  events.Clear();
}

const std::vector<EntityComponentUpdate>& ComponentRangeUpdateStorage::GetUpdates() const {
  return updates.GetUnderlying();
}

const std::vector<EntityComponentUpdate>& ComponentRangeUpdateStorage::GetEvents() const {
  return events.GetUnderlying();
}

const std::vector<EntityComponentData>& ComponentRangeUpdateStorage::GetCompleteUpdates() const {
  return data.GetUnderlying();
}

}  // namespace gdk
