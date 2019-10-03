#ifndef GDK_COMPONENT_RANGE_UPDATE_STORAGE_H
#define GDK_COMPONENT_RANGE_UPDATE_STORAGE_H
#include "gdk/entity_component_data.h"
#include "gdk/entity_component_update.h"
#include "gdk/view_delta/component_data_list.h"
#include "gdk/view_delta/component_update_list.h"
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
  ComponentUpdateList updates;
  ComponentUpdateList events;
  ComponentDataList data;
};

}  // namespace gdk
#endif  //  GDK_COMPONENT_RANGE_UPDATE_STORAGE_H
