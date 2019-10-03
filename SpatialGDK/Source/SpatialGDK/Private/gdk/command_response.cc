#include "gdk/command_response.h"

namespace gdk {

CommandResponse::CommandResponse(Schema_CommandResponse* response, ComponentId componentId,
                                 CommandIndex commandIndex)
: componentId(componentId), commandIndex(commandIndex), response(response) {}

CommandResponse::CommandResponse(const Worker_CommandResponse& response)
: CommandResponse(Schema_CopyCommandResponse(response.schema_type), response.component_id,
                  response.command_index) {}

CommandResponse::CommandResponse(ComponentId componentId, FieldId commandIndex)
: CommandResponse(Schema_CreateCommandResponse(), componentId, commandIndex) {}

CommandResponse CommandResponse::Clone() const {
  return CommandResponse{Schema_CopyCommandResponse(response.get()), componentId, commandIndex};
}

Schema_CommandResponse* CommandResponse::Release() && {
  return response.release();
}

Schema_CommandResponse* CommandResponse::GetUnderlying() const {
  return response.get();
}

Schema_Object* CommandResponse::GetResponseObject() {
  return Schema_GetCommandResponseObject(response.get());
}

const Schema_Object* CommandResponse::GetResponseObject() const {
  return Schema_GetCommandResponseObject(response.get());
}

ComponentId CommandResponse::GetComponentId() const {
  return componentId;
}

FieldId CommandResponse::GetCommandIndex() const {
  return commandIndex;
}

void CommandResponse::Deleter::operator()(Schema_CommandResponse* response) const noexcept {
  Schema_DestroyCommandResponse(response);
}

}  // namespace gdk
