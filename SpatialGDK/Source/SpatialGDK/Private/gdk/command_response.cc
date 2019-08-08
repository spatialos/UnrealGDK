#include "gdk/command_response.h"

#include <WorkerSDK/improbable/c_schema.h>
#include <WorkerSDK/improbable/c_worker.h>

namespace gdk {

CommandResponse::CommandResponse(Schema_CommandResponse* response) : response(response) {}

CommandResponse::CommandResponse(const Worker_CommandResponse& response)
: response(Schema_CopyCommandResponse(response.schema_type)) {}

CommandResponse::CommandResponse(ComponentId componentId, FieldId commandIndex)
: response(Schema_CreateCommandResponse(componentId, commandIndex)) {}

CommandResponse CommandResponse::Clone() const {
  return CommandResponse{Schema_CopyCommandResponse(response.get())};
}

Schema_CommandResponse* CommandResponse::Release() && {
  return response.release();
}

Schema_CommandResponse* CommandResponse::GetUnderlying() {
  return response.get();
}

const Schema_CommandResponse* CommandResponse::GetUnderlying() const {
  return response.get();
}

Schema_Object* CommandResponse::GetResponseObject() {
  return Schema_GetCommandResponseObject(response.get());
}

const Schema_Object* CommandResponse::GetResponseObject() const {
  return Schema_GetCommandResponseObject(response.get());
}

ComponentId CommandResponse::GetComponentId() const {
  return Schema_GetCommandResponseComponentId(response.get());
}

FieldId CommandResponse::GetCommandIndex() const {
  return Schema_GetCommandResponseCommandIndex(response.get());
}

void CommandResponse::Deleter::operator()(Schema_CommandResponse* response) const noexcept {
  Schema_DestroyCommandResponse(response);
}

}  // namespace gdk
