#ifndef GDK_COMPONENT_RANGE_DATA_STORAGE_H
#define GDK_COMPONENT_RANGE_DATA_STORAGE_H
#include "component_data.h"
#include "entity_component_id.h"
#include "entity_component_messages.h"
#include <vector>

namespace gdk {

class ComponentRangeDataStorage {
public:
  ComponentRangeDataStorage() = default;
  ~ComponentRangeDataStorage() = default;

  // Moveable, not copyable.
  ComponentRangeDataStorage(const ComponentRangeDataStorage&) = delete;
  ComponentRangeDataStorage(ComponentRangeDataStorage&&) = default;
  ComponentRangeDataStorage& operator=(const ComponentRangeDataStorage&) = delete;
  ComponentRangeDataStorage& operator=(ComponentRangeDataStorage&&) = default;

  /** Fails if the component has been marked as removed. Will consume data on success. */
  bool TryAddComponent(EntityId entityId, ComponentData&& data);
  void RemoveComponent(EntityId entityId, ComponentId componentId);
  void RemoveEntity(EntityId entityId);

  bool TryMergeComponentUpdate(EntityId entityId, const ComponentUpdate& update);
  /** Fails if the component has not been added this tick. Will consume data on success. */
  bool TryApplyComponentData(EntityId entityId, ComponentData&& data);

  void Clear();

  const std::vector<EntityComponentData>& GetComponentsAdded() const;
  const std::vector<EntityComponentId>& GetComponentsRemoved() const;

private:
  bool TryEraseComponentRemoved(EntityId entityId, ComponentId componentId);
  bool TryEraseComponentAdded(EntityId entityId, ComponentId componentId);

  std::vector<EntityComponentData> componentsAdded;
  std::vector<EntityComponentId> componentsRemoved;
};

}  // namespace gdk
#endif  //  GDK_COMPONENT_RANGE_DATA_STORAGE_H
