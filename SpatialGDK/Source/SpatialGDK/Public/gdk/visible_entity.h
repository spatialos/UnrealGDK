#ifndef GDK_VISIBLE_ENTITY_H
#define GDK_VISIBLE_ENTITY_H
#include "gdk/common_types.h"
#include "gdk/entity_state.h"
#include "gdk/internal/utils/sorted_array.h"  //todo remove public reference
#include <WorkerSDK/improbable/c_worker.h>
#include <vector>

namespace gdk {

class ComponentData;
class ComponentUpdate;

class VisibleEntity {
public:
  explicit VisibleEntity(EntityId entityId);
  explicit VisibleEntity(const Worker_Entity& entity);
  VisibleEntity(EntityId entityId, EntityState&& startingEntity);

  ~VisibleEntity() = default;

  // Moveable, not copyable
  VisibleEntity(const VisibleEntity&) = delete;
  VisibleEntity(VisibleEntity&&) = default;
  VisibleEntity& operator=(const VisibleEntity&) = delete;
  VisibleEntity& operator=(VisibleEntity&&) = default;

  void AddComponentId(ComponentId id);
  void RemoveComponentId(ComponentId id);
  void AddStartingComponent(ComponentData&& data);
  bool TrySetStartingComponent(ComponentData&& data);
  bool TryUpdateStartingComponent(const ComponentUpdate& update);

  EntityId GetEntityId() const;
  const EntityState& GetStartingComponents() const;
  const std::vector<ComponentId>& GetAllComponentIds() const;

  explicit operator EntityId() const {
    return GetEntityId();
  }

private:
  EntityId entityId = 0;
  EntityState startingComponents;
  SortedArray<ComponentId> allComponents;
};

}  // namespace gdk
#endif  // GDK_VISIBLE_ENTITY_H
