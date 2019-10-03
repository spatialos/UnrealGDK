#ifndef GDK_INTERNAL_RECEIVED_OP_LIST_H
#define GDK_INTERNAL_RECEIVED_OP_LIST_H
#include "gdk/op_lists/abstract_op_list.h"
#include <memory>

namespace gdk {

/** An RAII wrapper around Worker_OpList. */
class ReceivedOpList : public AbstractOpList {
public:
  explicit ReceivedOpList(Worker_OpList* opList) : ops(opList) {}

  ~ReceivedOpList() = default;

  // Moveable, not copyable
  ReceivedOpList(const ReceivedOpList& other) = delete;
  ReceivedOpList(ReceivedOpList&& other) = default;
  ReceivedOpList& operator=(const ReceivedOpList& other) = delete;
  ReceivedOpList& operator=(ReceivedOpList&& other) = default;

  size_t GetCount() const override {
    return ops->op_count;
  }

  Worker_Op& operator[](size_t index) override {
    return ops->ops[static_cast<std::uint32_t>(index)];
  }

private:
  struct Deleter {
    void operator()(Worker_OpList* opList) const noexcept {
      Worker_OpList_Destroy(opList);
    }
  };
  std::unique_ptr<Worker_OpList, Deleter> ops;
};

}  // namespace gdk
#endif  // GDK_INTERNAL_RECEIVED_OP_LIST_H