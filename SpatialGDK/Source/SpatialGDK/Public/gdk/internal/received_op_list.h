#ifndef GDK_INTERNAL_RECEIVED_OP_LIST_H
#define GDK_INTERNAL_RECEIVED_OP_LIST_H
#include "gdk/abstract_op_list.h"
#include <WorkerSDK/improbable/c_worker.h>
#include <memory>

namespace gdk {

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