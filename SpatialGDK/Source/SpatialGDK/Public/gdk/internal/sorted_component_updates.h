#ifndef GDK_SORTED_COMPONENT_UPDATES_H
#define GDK_SORTED_COMPONENT_UPDATES_H
#include "gdk/common_types.h"
#include "gdk/entity_component_id.h"
#include "gdk/entity_component_messages.h"
#include "gdk/internal/utils/sorted_array.h"
#include "gdk/internal/utils/try_move.h"
#include <algorithm>
#include <cstddef>
#include <vector>

namespace gdk {

class SortedComponentUpdates {
public:
  SortedComponentUpdates() = default;

  explicit SortedComponentUpdates(size_t capacity) : c(capacity) {}

  ~SortedComponentUpdates() = default;

  // Moveable, not copyable
  SortedComponentUpdates(const SortedComponentUpdates&) = delete;
  SortedComponentUpdates(SortedComponentUpdates&&) = default;
  SortedComponentUpdates& operator=(const SortedComponentUpdates&) = delete;
  SortedComponentUpdates& operator=(SortedComponentUpdates&&) = default;

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
    return c.erase_first(id);
  }

  void InsertOrMerge(EntityComponentUpdate&& update) {
    decltype(c)::iterator it;
    bool success;
    std::tie(it, success) = c.try_insert(TryMove(update));
    if (!success) {
      it->Update.Merge(std::move(update.Update));
    }
  }

  bool TryMoveUpdateTo(SortedComponentUpdates* destination, const EntityComponentId& id) {
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
  using Comparator = ConvertingSortedArrayComparator<EntityComponentUpdate, EntityComponentId>;

  SortedArray<EntityComponentUpdate, Comparator> c;
};

}  // namespace gdk
#endif  // GDK_SORTED_COMPONENT_UPDATES_H
