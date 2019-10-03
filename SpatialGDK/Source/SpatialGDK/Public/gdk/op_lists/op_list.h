#ifndef GDK_OP_LIST_H
#define GDK_OP_LIST_H
#include "gdk/op_lists/abstract_op_list.h"
#include <cstddef>
#include <memory>

namespace gdk {

// todo consider making this a union of the known op list types
/**
 * Represents a set of external changes to be applied to the worker.
 * Convenience wrapper giving an abstract op list value semantics.
 */
class OpList {
public:
  explicit OpList() : ops(nullptr) {}
  explicit OpList(std::unique_ptr<AbstractOpList> opList) : ops(std::move(opList)) {}

  ~OpList() = default;

  // Moveable, not copyable
  OpList(const OpList& other) = delete;
  OpList(OpList&& other) = default;
  OpList& operator=(const OpList& other) = delete;
  OpList& operator=(OpList&& other) = default;

  size_t GetCount() const {
    if (ops == nullptr) {
      return 0;
    }
    return ops->GetCount();
  }

  Worker_Op& operator[](size_t index) const {
    return ops->operator[](index);
  }

private:
  std::unique_ptr<AbstractOpList> ops;
};

}  // namespace gdk
#endif  // GDK_OP_LIST_H
