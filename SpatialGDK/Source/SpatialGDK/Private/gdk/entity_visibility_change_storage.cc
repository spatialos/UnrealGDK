#include "gdk/internal/entity_visibility_change_storage.h"

#include "gdk/component_update.h"
#include "gdk/visible_entity.h"
#include <algorithm>

namespace gdk {

bool EntityVisibilityChangeStorage::TryAddEntity(VisibleEntity&& entity) {
  if (TryCancelEntityRemove(entity.GetEntityId())) {
    return false;
  }
  entitiesAdded.insert(std::move(entity));
  return true;
}

void EntityVisibilityChangeStorage::RemoveEntity(EntityId entityId) {
  if (TryCancelEntityAdd(entityId)) {
    return;
  }
  entitiesRemoved.insert(entityId);
}

bool EntityVisibilityChangeStorage::TrySetStartingComponent(EntityId entityId,
                                                            ComponentData&& data) {
  auto it = entitiesAdded.find(entityId);
  if (it == entitiesAdded.end()) {
    return false;
  }
  return it->TrySetStartingComponent(std::move(data));
}

bool EntityVisibilityChangeStorage::TryUpdateStartingComponent(EntityId entityId,
                                                               const ComponentUpdate& update) {
  auto it = entitiesAdded.find(entityId);
  if (it == entitiesAdded.end()) {
    return false;
  }
  return it->TryUpdateStartingComponent(update);
}

bool EntityVisibilityChangeStorage::TryAddComponent(EntityId entityId, ComponentData&& data) {
  auto it = entitiesAdded.find(entityId);
  if (it == entitiesAdded.end()) {
    return false;
  }
  it->AddStartingComponent(std::move(data));
  return true;
}

bool EntityVisibilityChangeStorage::TryAddComponentId(EntityId entityId, ComponentId componentId) {
  auto it = entitiesAdded.find(entityId);
  if (it == entitiesAdded.end()) {
    return false;
  }
  it->AddComponentId(componentId);
  return true;
}

bool EntityVisibilityChangeStorage::TryRemoveComponent(EntityId entityId, ComponentId componentId) {
  // todo remove starting entities
  auto it = entitiesAdded.find(entityId);
  if (it == entitiesAdded.end()) {
    return false;
  }
  it->RemoveComponentId(componentId);
  return true;
}

void EntityVisibilityChangeStorage::Clear() {
  entitiesAdded.clear();
  entitiesRemoved.clear();
}

const std::vector<VisibleEntity>& EntityVisibilityChangeStorage::GetEntitiesAdded() const {
  return entitiesAdded.GetUnderlying();
}

const std::vector<EntityId>& EntityVisibilityChangeStorage::GetEntitiesRemoved() const {
  return entitiesRemoved.GetUnderlying();
}

bool EntityVisibilityChangeStorage::TryCancelEntityAdd(EntityId entityId) {
  return entitiesAdded.erase_first(entityId);
}

bool EntityVisibilityChangeStorage::TryCancelEntityRemove(EntityId entityId) {
  return entitiesRemoved.erase_first(entityId);
}

}  // namespace gdk
