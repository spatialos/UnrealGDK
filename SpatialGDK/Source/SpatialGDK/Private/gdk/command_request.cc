#include "gdk/command_request.h"

namespace gdk {

CommandRequest::CommandRequest(Schema_CommandRequest* request, ComponentId componentId,
                               CommandIndex commandIndex)
: componentId(componentId), commandIndex(commandIndex), request(request) {}

CommandRequest::CommandRequest(const Worker_CommandRequest& request)
: CommandRequest(Schema_CopyCommandRequest(request.schema_type), request.component_id,
                 request.command_index) {}

CommandRequest::CommandRequest(ComponentId componentId, FieldId commandIndex)
: CommandRequest(Schema_CreateCommandRequest(), componentId, commandIndex) {}

CommandRequest CommandRequest::Clone() const {
  return CommandRequest{Schema_CopyCommandRequest(request.get()), componentId, commandIndex};
}

Schema_CommandRequest* CommandRequest::Release() && {
  return request.release();
}

Schema_CommandRequest* CommandRequest::GetUnderlying() const {
  return request.get();
}

Schema_Object* CommandRequest::GetRequestObject() {
  return Schema_GetCommandRequestObject(request.get());
}

const Schema_Object* CommandRequest::GetRequestObject() const {
  return Schema_GetCommandRequestObject(request.get());
}

ComponentId CommandRequest::GetComponentId() const {
  return componentId;
}

FieldId CommandRequest::GetCommandIndex() const {
  return commandIndex;
}

void CommandRequest::Deleter::operator()(Schema_CommandRequest* request) const noexcept {
  Schema_DestroyCommandRequest(request);
}

}  // namespace gdk
