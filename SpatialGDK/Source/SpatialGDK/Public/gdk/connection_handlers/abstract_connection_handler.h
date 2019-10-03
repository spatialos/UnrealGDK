#ifndef GDK_ABSTRACT_CONNECTION_HANDLER_H
#define GDK_ABSTRACT_CONNECTION_HANDLER_H
#include "gdk/op_lists/op_list.h"
#include <cstddef>
#include <string>
#include <vector>

namespace gdk {

class OpList;
struct MessagesToSend;

/**
 * Base class that presents external messages consumes the local changes.
 *
 * Receives of external messages and produces an OpList representing them.
 * Consumes MessagesToSend instances.
 *
 * todo possibly move some common implementation here so that the interface can be better defined.
 */
class AbstractConnectionHandler {
public:
  virtual ~AbstractConnectionHandler() = default;

  /**
   * Should be called to indicate a new tick has started.
   * Ensures all external messages, up to this, point have been received.
   */
  virtual void Advance() = 0;
  /** The number of OpList instances queued. */
  virtual size_t GetOpListCount() = 0;
  /** Gets the next queued OpList. If there is no OpList queued then an empty one is returned. */
  virtual OpList GetNextOpList() = 0;
  /** Consumes messages and sends them to the deployment. */
  virtual void SendMessages(MessagesToSend&& messages) = 0;
  /** Return the unique ID for the worker. */
  virtual const std::string& GetWorkerId() const = 0;
  /** Returns the attributes for the worker. */
  virtual const std::vector<std::string>& GetWorkerAttributes() const = 0;
};

}  // namespace gdk
#endif  // GDK_ABSTRACT_CONNECTION_HANDLER_H
