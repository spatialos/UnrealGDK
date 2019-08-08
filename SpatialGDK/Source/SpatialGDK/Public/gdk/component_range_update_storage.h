#ifndef GDK_COMPONENT_RANGE_UPDATE_STORAGE_H
#define GDK_COMPONENT_RANGE_UPDATE_STORAGE_H
#include "entity_component_messages.h"
#include "internal/sorted_component_data.h"
#include "internal/sorted_component_updates.h"
#include <vector>

namespace gdk {

class ComponentRangeUpdateStorage {
public:
  ComponentRangeUpdateStorage() = default;
  ~ComponentRangeUpdateStorage() = default;

  // Moveable, not copyable.
  ComponentRangeUpdateStorage(const ComponentRangeUpdateStorage&) = delete;
  ComponentRangeUpdateStorage(ComponentRangeUpdateStorage&&) = default;
  ComponentRangeUpdateStorage& operator=(const ComponentRangeUpdateStorage&) = delete;
  ComponentRangeUpdateStorage& operator=(ComponentRangeUpdateStorage&&) = default;

  void AddComponentUpdate(EntityId entityId, ComponentData&& update);
  void AddComponentUpdate(EntityId entityId, ComponentUpdate&& update);
  void RemoveComponent(EntityId entityId, ComponentId componentId);
  void RemoveEntity(EntityId entityId);

  void Clear();

  const std::vector<EntityComponentUpdate>& GetUpdates() const;
  const std::vector<EntityComponentUpdate>& GetEvents() const;
  const std::vector<EntityComponentData>& GetCompleteUpdates() const;

private:
  SortedComponentUpdates updates;
  SortedComponentUpdates events;
  SortedComponentData data;
};

}  // namespace gdk
#endif  //  GDK_COMPONENT_RANGE_UPDATE_STORAGE_H
