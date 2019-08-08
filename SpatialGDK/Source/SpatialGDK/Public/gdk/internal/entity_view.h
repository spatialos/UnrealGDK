#ifndef GDK_ENTITY_VIEW_H
#define GDK_ENTITY_VIEW_H
#include "gdk/common_types.h"
#include <unordered_map>
#include <vector>

namespace gdk {

class EntityView {
public:
  EntityView() = default;
  ~EntityView() = default;

  // Moveable, not copyable
  EntityView(const EntityView&) = delete;
  EntityView(EntityView&&) = default;
  EntityView& operator=(const EntityView&) = delete;
  EntityView& operator=(EntityView&&) = default;

  void AddEntity(EntityId entityId);
  void RemoveEntity(EntityId entityId);
  void AddComponent(EntityId entityId, ComponentId componentId);
  void RemoveComponent(EntityId entityId, ComponentId componentId);

  // Returns true if the entity component is present in the view. False otherwise.
  bool HasComponent(EntityId entityId, ComponentId componentId) const;

  // todo - should this check if the entity exists
  const std::vector<ComponentId>& GetComponentIds(EntityId entityId) const;

private:
  std::unordered_map<EntityId, std::vector<ComponentId>> entities;
};

}  // namespace gdk
#endif  // GDK_ENTITY_VIEW_H
