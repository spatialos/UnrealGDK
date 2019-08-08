#ifndef GDK_TRY_MOVE_H
#define GDK_TRY_MOVE_H
#include <type_traits>
namespace gdk {

// Does the same thing as std::move but denotes that the object might still be safe to use.
// Should only be used in arguments to functions that conditionally move (e.g. try_emplace).
// Avoids linters detecting use-after-move errors in cases where it is known to be safe.
template <typename T>
constexpr std::remove_reference_t<T>&& TryMove(T&& t) {
  return static_cast<std::remove_reference_t<T>&&>(t);
}

}  // namespace gdk
#endif  // GDK_TRY_MOVE_H
