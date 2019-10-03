#ifndef GDK_ENTITY_COMPONENT_ID_H
#define GDK_ENTITY_COMPONENT_ID_H
#include "gdk/worker_sdk.h"
#include <functional>

namespace gdk {

struct EntityComponentId {
  gdk::EntityId EntityId;
  gdk::ComponentId ComponentId;

  friend bool operator==(const EntityComponentId& lhs, const EntityComponentId& rhs) {
    return lhs.EntityId == rhs.EntityId && lhs.ComponentId == rhs.ComponentId;
  }
};

struct EntityFirstEntityComponentIdComparator {
  constexpr bool operator()(const EntityComponentId& lhs, const EntityComponentId& rhs) {
    if (lhs.EntityId < rhs.EntityId) {
      return true;
    }
    return lhs.ComponentId < rhs.ComponentId;
  }
};

}  // namespace gdk

namespace std {

template <>
struct hash<gdk::EntityComponentId> {
  size_t operator()(const gdk::EntityComponentId& entityComponentId) const noexcept {
    // Use the boost hash_combine algorithm to combine the worker and component ID hashes.
    const auto entityHash = std::hash<gdk::EntityId>{}(entityComponentId.EntityId);
    const auto componentHash = std::hash<gdk::ComponentId>{}(entityComponentId.ComponentId);
    auto hash = entityHash + 0x9e3779b9;
    hash += componentHash + 0x9e3779b9 + (hash << 6) + (hash >> 2);
    return hash;
  }
};

}  // namespace std
#endif  // GDK_ENTITY_COMPONENT_ID_H
