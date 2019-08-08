#include "gdk/internal/component_range_data_storage.h"

#include "gdk/component_update.h"
#include <algorithm>
#include <utility>

namespace gdk {

bool ComponentRangeDataStorage::TryAddComponent(EntityId entityId, ComponentData&& data) {
  if (TryEraseComponentRemoved(entityId, data.GetComponentId())) {
    return false;
  }
  componentsAdded.emplace_back(EntityComponentData{entityId, std::move(data)});
  return true;
}

void ComponentRangeDataStorage::RemoveComponent(EntityId entityId, ComponentId componentId) {
  if (TryEraseComponentAdded(entityId, componentId)) {
    return;
  }
  componentsRemoved.emplace_back(EntityComponentId{entityId, componentId});
}

// Currently we are still assuming that all the remove component messages will have gone through.
// So we don't need to remove anything from componentsAdded.
void ComponentRangeDataStorage::RemoveEntity(EntityId entityId) {
  const auto removedIt = std::remove_if(componentsRemoved.begin(), componentsRemoved.end(),
                                        [entityId](const EntityComponentId& entityComponentId) {
                                          return entityComponentId.EntityId == entityId;
                                        });
  componentsRemoved.erase(removedIt, componentsRemoved.end());

  const auto addedIt = std::remove_if(
      componentsAdded.begin(), componentsAdded.end(),
      [entityId](const EntityComponentData& data) { return data.EntityId == entityId; });
  componentsAdded.erase(addedIt, componentsAdded.end());
}

bool ComponentRangeDataStorage::TryMergeComponentUpdate(EntityId entityId,
                                                        const ComponentUpdate& update) {
  for (auto& component : componentsAdded) {
    if (component.EntityId == entityId &&
        component.Data.GetComponentId() == update.GetComponentId()) {
      component.Data.ApplyUpdate(update);
      return true;
    }
  }
  return false;
}

bool ComponentRangeDataStorage::TryApplyComponentData(EntityId entityId, ComponentData&& data) {
  for (auto& component : componentsAdded) {
    if (component.EntityId == entityId &&
        component.Data.GetComponentId() == data.GetComponentId()) {
      component.Data = std::move(data);
      return true;
    }
  }
  return false;
}

void ComponentRangeDataStorage::Clear() {
  componentsAdded.clear();
  componentsRemoved.clear();
}

const std::vector<EntityComponentData>& ComponentRangeDataStorage::GetComponentsAdded() const {
  return componentsAdded;
}

const std::vector<EntityComponentId>& ComponentRangeDataStorage::GetComponentsRemoved() const {
  return componentsRemoved;
}

bool ComponentRangeDataStorage::TryEraseComponentRemoved(EntityId entityId,
                                                         ComponentId componentId) {
  const auto it = std::find(componentsRemoved.begin(), componentsRemoved.end(),
                            EntityComponentId{entityId, componentId});
  if (it == componentsRemoved.end()) {
    return false;
  }
  componentsRemoved.erase(it);
  return true;
}

bool ComponentRangeDataStorage::TryEraseComponentAdded(EntityId entityId, ComponentId componentId) {
  const auto it =
      std::find_if(componentsAdded.begin(), componentsAdded.end(),
                   [entityId, componentId](const EntityComponentData& data) -> bool {
                     return data.EntityId == entityId && data.Data.GetComponentId() == componentId;
                   });

  if (it == componentsAdded.end()) {
    return false;
  }
  componentsAdded.erase(it);
  return true;
}

}  // namespace gdk
