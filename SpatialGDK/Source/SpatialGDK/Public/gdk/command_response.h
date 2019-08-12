#ifndef GDK_COMMAND_RESPONSE_H
#define GDK_COMMAND_RESPONSE_H
#include "gdk/common_types.h"
#include <memory>

struct Schema_CommandResponse;
struct Schema_Object;
struct Worker_CommandResponse;

namespace gdk {

/** An RAII wrapper for command responses. */
class CommandResponse {
public:
  explicit CommandResponse(Schema_CommandResponse* response);
  explicit CommandResponse(const Worker_CommandResponse& response);
  explicit CommandResponse(ComponentId componentId, FieldId commandIndex);

  ~CommandResponse() = default;

  // Moveable, not copyable
  CommandResponse(const CommandResponse&) = delete;
  CommandResponse(CommandResponse&&) = default;
  CommandResponse& operator=(const CommandResponse&) = delete;
  CommandResponse& operator=(CommandResponse&&) = default;

  CommandResponse Clone() const;
  Schema_CommandResponse* Release() &&;

  Schema_Object* GetResponseObject();
  const Schema_Object* GetResponseObject() const;

  ComponentId GetComponentId() const;
  FieldId GetCommandIndex() const;

  Schema_CommandResponse* GetUnderlying() const;

private:
  struct Deleter {
    void operator()(Schema_CommandResponse* response) const noexcept;
  };

  std::unique_ptr<Schema_CommandResponse, Deleter> response;
};

}  // namespace gdk
#endif  // GDK_COMMAND_RESPONSE_H
