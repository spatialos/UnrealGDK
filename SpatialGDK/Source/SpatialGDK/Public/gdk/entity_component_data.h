#ifndef GDK_ENTITY_COMPONENT_DATA_H
#define GDK_ENTITY_COMPONENT_DATA_H
#include "gdk/component_data.h"
#include "gdk/entity_component_id.h"
#include "gdk/worker_sdk.h"

namespace gdk {

struct EntityComponentData {
  gdk::EntityId EntityId;
  gdk::ComponentData Data;

  EntityComponentId GetEntityComponentId() const {
    return EntityComponentId{EntityId, Data.GetComponentId()};
  }
};

// Comparator that sorts by entity ID first and then by component ID.
struct EntityFirstEntityComponentDataComparator {
  using is_transparent = void;

  constexpr bool operator()(const EntityComponentId& lhs, const EntityComponentId& rhs) const {
    return EntityFirstEntityComponentIdComparator{}(lhs, rhs);
  }

  constexpr bool operator()(const EntityComponentData& lhs, const EntityComponentId& rhs) const {
    return this->operator()(lhs.GetEntityComponentId(), rhs);
  }

  constexpr bool operator()(const EntityComponentId& lhs, const EntityComponentData& rhs) const {
    return this->operator()(lhs, rhs.GetEntityComponentId());
  }

  constexpr bool operator()(const EntityComponentData& lhs, const EntityComponentData& rhs) const {
    return this->operator()(lhs.GetEntityComponentId(), rhs.GetEntityComponentId());
  }
};

}  // namespace gdk
#endif  // GDK_ENTITY_COMPONENT_DATA_H
