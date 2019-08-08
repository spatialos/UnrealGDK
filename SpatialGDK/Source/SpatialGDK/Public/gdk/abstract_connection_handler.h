#ifndef GDK_ABSTRACT_CONNECTION_HANDLER_H
#define GDK_ABSTRACT_CONNECTION_HANDLER_H
#include "gdk/op_list.h"
#include <cstddef>
#include <string>
#include <vector>

namespace gdk {

class OpList;
class CommandMetadataManager;
struct MessagesToSend;

class AbstractConnectionHandler {
public:
  virtual ~AbstractConnectionHandler() = default;

  virtual void Advance() = 0;
  virtual size_t GetOpListCount() = 0;
  virtual OpList GetNextOpList() = 0;
  virtual void SendMessages(MessagesToSend&& messages) = 0;
  virtual const std::string& GetWorkerId() const = 0;
  virtual const std::vector<std::string>& GetWorkerAttributes() const = 0;
};

}  // namespace gdk
#endif  // GDK_ABSTRACT_CONNECTION_HANDLER_H
