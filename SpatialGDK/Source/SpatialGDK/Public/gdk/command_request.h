#ifndef GDK_COMMAND_REQUEST_H
#define GDK_COMMAND_REQUEST_H
#include "gdk/worker_sdk.h"
#include <memory>

namespace gdk {

/** An RAII wrapper for command requests. */
class CommandRequest {
public:
  explicit CommandRequest(Schema_CommandRequest* request, ComponentId componentId,
                          CommandIndex commandIndex);
  explicit CommandRequest(const Worker_CommandRequest& request);
  explicit CommandRequest(ComponentId componentId, CommandIndex commandIndex);

  ~CommandRequest() = default;

  // Moveable, not copyable
  CommandRequest(const CommandRequest&) = delete;
  CommandRequest(CommandRequest&&) = default;
  CommandRequest& operator=(const CommandRequest&) = delete;
  CommandRequest& operator=(CommandRequest&&) = default;

  CommandRequest Clone() const;
  Schema_CommandRequest* Release() &&;

  Schema_Object* GetRequestObject();
  const Schema_Object* GetRequestObject() const;

  ComponentId GetComponentId() const;
  FieldId GetCommandIndex() const;

  Schema_CommandRequest* GetUnderlying() const;

private:
  struct Deleter {
    void operator()(Schema_CommandRequest* request) const noexcept;
  };

  ComponentId componentId;
  CommandIndex commandIndex;
  std::unique_ptr<Schema_CommandRequest, Deleter> request;
};

}  // namespace gdk
#endif  // GDK_COMMAND_REQUEST_H
