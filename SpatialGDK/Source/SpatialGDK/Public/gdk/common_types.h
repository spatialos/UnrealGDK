#ifndef GDK_COMMON_TYPES_H
#define GDK_COMMON_TYPES_H
#include <cstdint>

namespace gdk {

using EntityId = std::int64_t;
using ComponentId = std::uint32_t;
using RequestId = std::uint32_t;
using FieldId = std::uint32_t;

}  // namespace gdk
#endif  // GDK_COMMON_TYPES_H