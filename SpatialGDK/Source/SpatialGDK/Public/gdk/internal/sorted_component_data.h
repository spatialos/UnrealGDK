#ifndef GDK_SORTED_COMPONENT_DATA_H
#define GDK_SORTED_COMPONENT_DATA_H
#include "gdk/entity_component_id.h"
#include "gdk/entity_component_messages.h"
#include "gdk/internal/utils/sorted_array.h"
#include "gdk/internal/utils/try_move.h"
#include <algorithm>
#include <cstddef>
#include <tuple>
#include <vector>

namespace gdk {

class SortedComponentData {
public:
  SortedComponentData() = default;
  explicit SortedComponentData(size_t capacity) : c(capacity) {}
  ~SortedComponentData() = default;

  // Moveable, not copyable
  SortedComponentData(const SortedComponentData&) = delete;
  SortedComponentData(SortedComponentData&&) = default;
  SortedComponentData& operator=(const SortedComponentData&) = delete;
  SortedComponentData& operator=(SortedComponentData&&) = default;

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
    std::tie(it, success) = c.try_insert(TryMove(data));
    if (!success) {
      *it = std::move(data);
    }
  }

  bool Remove(const EntityComponentId& id) {
    return c.erase_first(id);
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
  using Comparator = ConvertingSortedArrayComparator<EntityComponentData, EntityComponentId>;

  SortedArray<EntityComponentData, Comparator> c;
};

}  // namespace gdk
#endif  // GDK_SORTED_COMPONENT_DATA_H
