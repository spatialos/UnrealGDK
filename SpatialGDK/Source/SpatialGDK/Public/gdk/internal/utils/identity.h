#ifndef GDK_IDENTITY_H
#define GDK_IDENTITY_H
#include <utility>

namespace gdk {

// Returns arguments unaltered.
struct Identity {
  template <typename T>
  constexpr T&& operator()(T&& t) const {
    return std::forward<T>(t);
  }
};

}  // namespace gdk
#endif  // GDK_IDENTITY_H
