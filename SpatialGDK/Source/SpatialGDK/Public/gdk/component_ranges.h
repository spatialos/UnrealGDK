#ifndef GDK_COMPONENT_RANGES_H
#define GDK_COMPONENT_RANGES_H
#include "gdk/common_types.h"
#include "gdk/internal/utils/sorted_array.h"  // todo remove public reference.

namespace gdk {

/** Represents a range of component IDs. */
struct Range {
  ComponentId Lower;
  ComponentId Upper;

  friend bool operator<(const Range& lhs, const Range& rhs) {
    return lhs.Upper < rhs.Lower;
  }

  friend bool operator>(const Range& lhs, const Range& rhs) {
    return lhs.Lower > rhs.Upper;
  }

  // Two ranges are considered equal if they overlap
  friend bool operator==(const Range& lhs, const Range& rhs) {
    return !(lhs < rhs || lhs > rhs);
  }
};

/** A set of non-overlapping component ranges that should stored and presented together. */
class ComponentRanges {
public:
  ComponentRanges() = default;
  ComponentRanges(std::initializer_list<Range> ranges);

  bool TryAddComponentRange(const Range& range);
  ComponentId GetRangeEquivalentId(ComponentId id) const;

private:

  SortedArray<Range> ranges;
};

}  // namespace gdk
#endif  // GDK_COMPONENT_RANGES_H
