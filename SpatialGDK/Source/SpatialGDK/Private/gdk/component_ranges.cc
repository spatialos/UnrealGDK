#include "gdk/component_ranges.h"

namespace gdk {

ComponentRanges::ComponentRanges(std::initializer_list<Range> ranges) {
  for (auto& range : ranges) {
    TryAddComponentRange(range); 
  }
}

bool ComponentRanges::TryAddComponentRange(const Range& range) {
  return ranges.try_insert(range).second;
}

ComponentId ComponentRanges::GetRangeEquivalentId(ComponentId id) const {
  const auto it = ranges.find(Range{id, id});
  if (it == ranges.end()) {
    return id;
  }
  return it->Lower;
}

}  // namespace gdk
