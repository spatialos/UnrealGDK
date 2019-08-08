#ifndef GDK_ENTITY_VISIBILITY_CHANGE_STORAGE_H
#define GDK_ENTITY_VISIBILITY_CHANGE_STORAGE_H
#include "internal/sorted_component_data.h"
#include "internal/utils/sorted_array.h"
#include "visible_entity.h"
#include <vector>

namespace gdk {

class ComponentUpdate;

class EntityVisibilityChangeStorage {
public:
  EntityVisibilityChangeStorage() = default;
  ~EntityVisibilityChangeStorage() = default;

  // Moveable, not copyable
  EntityVisibilityChangeStorage(const EntityVisibilityChangeStorage&) = delete;
  EntityVisibilityChangeStorage(EntityVisibilityChangeStorage&&) = default;
  EntityVisibilityChangeStorage& operator=(const EntityVisibilityChangeStorage&) = delete;
  EntityVisibilityChangeStorage& operator=(EntityVisibilityChangeStorage&&) = default;

  bool TryAddEntity(VisibleEntity&& entity);
  void RemoveEntity(EntityId entityId);

  bool TrySetStartingComponent(EntityId entityId, ComponentData&& data);
  bool TryUpdateStartingComponent(EntityId entityId, const ComponentUpdate& update);
  /** Consumes component if successful */
  bool TryAddComponent(EntityId entityId, ComponentData&& data);
  bool TryAddComponentId(EntityId entityId, ComponentId componentId);
  bool TryRemoveComponent(EntityId entityId, ComponentId componentId);

  void Clear();

  const std::vector<VisibleEntity>& GetEntitiesAdded() const;
  const std::vector<EntityId>& GetEntitiesRemoved() const;

private:
  using Comparator = ConvertingSortedArrayComparator<VisibleEntity, EntityId>;

  bool TryCancelEntityAdd(EntityId entityId);
  bool TryCancelEntityRemove(EntityId entityId);

  SortedArray<VisibleEntity, Comparator> entitiesAdded;
  SortedArray<EntityId> entitiesRemoved;
};

}  // namespace gdk
#endif  // GDK_ENTITY_VISIBILITY_CHANGE_STORAGE_H
