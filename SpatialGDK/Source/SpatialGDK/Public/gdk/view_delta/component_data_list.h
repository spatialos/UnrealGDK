#ifndef GDK_INTERNAL_COMPONENT_DATA_LIST_H
#define GDK_INTERNAL_COMPONENT_DATA_LIST_H
#include "gdk/entity_component_id.h"
#include "gdk/entity_component_data.h"
#include "gdk/entity_component_update.h"
#include "gdk/collections/flat_set.h"
#include "gdk/utils/try_move.h"
#include <algorithm>
#include <cstddef>
#include <tuple>
#include <vector>

namespace gdk {

class ComponentDataList {
public:
  ComponentDataList() = default;
  explicit ComponentDataList(size_t capacity) : c(capacity) {}
  ~ComponentDataList() = default;

  // Moveable, not copyable
  ComponentDataList(const ComponentDataList&) = delete;
  ComponentDataList(ComponentDataList&&) = default;
  ComponentDataList& operator=(const ComponentDataList&) = delete;
  ComponentDataList& operator=(ComponentDataList&&) = default;

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

  void Insert(EntityComponentData&& data) {
    decltype(c)::iterator it;
    bool success;
    std::tie(it, success) = c.insert(TryMove(data));
    if (!success) {
      *it = std::move(data);
    }
  }

  bool Remove(const EntityComponentId& id) {
    return c.erase(id);
  }

  bool TryApplyUpdate(const EntityComponentUpdate& update) {
    const auto it = c.find(update.GetEntityComponentId());
    if (it == c.end()) {
      return false;
    }
    it->Data.ApplyUpdate(update.Update);
    return true;
  }

  std::vector<EntityComponentData>& GetUnderlying() {
    return c.GetUnderlying();
  }

  const std::vector<EntityComponentData>& GetUnderlying() const {
    return c.GetUnderlying();
  }

private:
  FlatSet<EntityComponentData, EntityFirstEntityComponentDataComparator> c;
};

}  // namespace gdk
#endif  // GDK_INTERNAL_COMPONENT_DATA_LIST_H
