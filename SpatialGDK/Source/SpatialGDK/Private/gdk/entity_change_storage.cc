#include "gdk/view_delta/entity_change_storage.h"

#include "gdk/component_update.h"

namespace gdk {

bool EntityChangeStorage::TryAddEntity(EntityId entityId) {
  if (TryCancelEntityRemove(entityId)) {
    return false;
  }
  entitiesAdded.insert(entityId);
  return true;
}

void EntityChangeStorage::RemoveEntity(EntityId entityId) {
  if (TryCancelEntityAdd(entityId)) {
    return;
  }
  entitiesRemoved.insert(entityId);
}

void EntityChangeStorage::Clear() {
  entitiesAdded.clear();
  entitiesRemoved.clear();
}

const std::vector<EntityId>& EntityChangeStorage::GetEntitiesAdded() const {
  return entitiesAdded.GetUnderlying();
}

const std::vector<EntityId>& EntityChangeStorage::GetEntitiesRemoved() const {
  return entitiesRemoved.GetUnderlying();
}

bool EntityChangeStorage::TryCancelEntityAdd(EntityId entityId) {
  return entitiesAdded.erase(entityId);
}

bool EntityChangeStorage::TryCancelEntityRemove(EntityId entityId) {
  return entitiesRemoved.erase(entityId);
}

}  // namespace gdk
