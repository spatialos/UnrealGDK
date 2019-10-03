#ifndef GDK_ENTITY_COMPONENT_UPDATE_H
#define GDK_ENTITY_COMPONENT_UPDATE_H
#include "gdk/component_update.h"
#include "gdk/entity_component_id.h"
#include "gdk/worker_sdk.h"

namespace gdk {

struct EntityComponentUpdate {
  gdk::EntityId EntityId;
  gdk::ComponentUpdate Update;

  EntityComponentId GetEntityComponentId() const {
    return EntityComponentId{EntityId, Update.GetComponentId()};
  }
};

// Comparator that sorts by entity ID first and then by component ID.
struct EntityFirstEntityComponentUpdateComparator {
  using is_transparent = void;

  constexpr bool operator()(const EntityComponentId& lhs, const EntityComponentId& rhs) const {
    return EntityFirstEntityComponentIdComparator{}(lhs, rhs);
  }

  constexpr bool operator()(const EntityComponentUpdate& lhs, const EntityComponentId& rhs) const {
    return this->operator()(lhs.GetEntityComponentId(), rhs);
  }

  constexpr bool operator()(const EntityComponentId& lhs, const EntityComponentUpdate& rhs) const {
    return this->operator()(lhs, rhs.GetEntityComponentId());
  }

  constexpr bool operator()(const EntityComponentUpdate& lhs,
                            const EntityComponentUpdate& rhs) const {
    return this->operator()(lhs.GetEntityComponentId(), rhs.GetEntityComponentId());
  }
};

}  // namespace gdk
#endif  // GDK_ENTITY_COMPONENT_UPDATE_H
