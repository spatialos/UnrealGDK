#ifndef GDK_ABSTRACT_OP_LIST_H
#define GDK_ABSTRACT_OP_LIST_H
#include <WorkerSDK/improbable/c_worker.h>
#include <cstddef>
namespace gdk {

class AbstractOpList {
public:
  virtual ~AbstractOpList() = default;

  virtual size_t GetCount() const = 0;
  virtual Worker_Op& operator[](size_t index) = 0;
};

}  // namespace gdk
#endif  // GDK_ABSTRACT_OP_LIST_H
