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

enum class OpTypes {
  kDisconnect = 1,
  kFlagUpdate = 2,
  kLogMessage = 3,
  kMetrics = 4,
  kCriticalSection = 5,
  kAddEntity = 6,
  kRemoveEntity = 7,
  kReserveEntityIdResponse_Deprecated = 8,
  kReserveEntityIdsResponse = 9,
  kCreateEntityResponse = 10,
  kDeleteEntityResponse = 11,
  kEntityQueryResponse = 12,
  kAddComponentResponse = 13,
  kRemoveComponentResponse = 14,
  kAuthorityChange = 15,
  kComponentUpdate = 16,
  kCommandRequest = 17,
  kCommandResponse = 18,
  // The following types are an extension Worker_OpType
  kIgnore = 19,
  kConnected = 19,
};

}  // namespace gdk
#endif  // GDK_ABSTRACT_OP_LIST_H
