#ifndef GDK_INTERNAL_COMPONENT_UPDATE_LIST_H
#define GDK_INTERNAL_COMPONENT_UPDATE_LIST_H
#include "gdk/worker_sdk.h"
#include "gdk/entity_component_id.h"
#include "gdk/entity_component_update.h"
#include "gdk/collections/flat_set.h"
#include "gdk/utils/try_move.h"
#include <algorithm>
#include <cstddef>
#include <vector>

namespace gdk {

class ComponentUpdateList {
public:
  ComponentUpdateList() = default;

  explicit ComponentUpdateList(size_t capacity) : c(capacity) {}

  ~ComponentUpdateList() = default;

  // Moveable, not copyable
  ComponentUpdateList(const ComponentUpdateList&) = delete;
  ComponentUpdateList(ComponentUpdateList&&) = default;
  ComponentUpdateList& operator=(const ComponentUpdateList&) = delete;
  ComponentUpdateList& operator=(ComponentUpdateList&&) = default;

  void Clear() {
    c.clear();
  }

  bool ClearEntityId(EntityId entityId) {
    const auto lb = c.lower_bound(EntityComponentId{entityId, 0});
    if (lb != c.end() && lb->EntityId == entityId) {
      // Use the lower bound of the next entity ID to make sure all components are covered.
      const auto ub = c.lower_bound(EntityComponentId{entityId + 1, 0});
      c.erase(lb, ub);
      return true;
    }
    return false;
  }

  bool Remove(const EntityComponentId& id) {
    return c.erase(id);
  }

  void InsertOrMerge(EntityComponentUpdate&& update) {
    decltype(c)::iterator it;
    bool success;
    std::tie(it, success) = c.insert(TryMove(update));
    if (!success) {
      it->Update.Merge(std::move(update.Update));
    }
  }

  bool TryMoveUpdateTo(ComponentUpdateList* destination, const EntityComponentId& id) {
    auto it = c.find(id);
    if (it == c.end()) {
      return false;
    }
    destination->InsertOrMerge(std::move(*it));
    c.erase(it);
    return true;
  }

  std::vector<EntityComponentUpdate>& GetUnderlying() {
    return c.GetUnderlying();
  }

  const std::vector<EntityComponentUpdate>& GetUnderlying() const {
    return c.GetUnderlying();
  }

private:
  FlatSet<EntityComponentUpdate, EntityFirstEntityComponentUpdateComparator> c;
};

}  // namespace gdk
#endif  // GDK_INTERNAL_COMPONENT_UPDATE_LIST_H
