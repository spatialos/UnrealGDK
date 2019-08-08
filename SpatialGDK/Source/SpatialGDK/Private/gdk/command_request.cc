#include "gdk/command_request.h"

#include <WorkerSDK/improbable/c_schema.h>
#include <WorkerSDK/improbable/c_worker.h>

namespace gdk {

CommandRequest::CommandRequest(Schema_CommandRequest* request) : request(request) {}

CommandRequest::CommandRequest(const Worker_CommandRequest& request)
: request(Schema_CopyCommandRequest(request.schema_type)) {}

CommandRequest::CommandRequest(ComponentId componentId, FieldId commandIndex)
: request(Schema_CreateCommandRequest(componentId, commandIndex)) {}

CommandRequest CommandRequest::Clone() const {
  return CommandRequest{Schema_CopyCommandRequest(request.get())};
}

Schema_CommandRequest* CommandRequest::Release() && {
  return request.release();
}

Schema_CommandRequest* CommandRequest::GetUnderlying() {
  return request.get();
}

const Schema_CommandRequest* CommandRequest::GetUnderlying() const {
  return request.get();
}

Schema_Object* CommandRequest::GetRequestObject() {
  return Schema_GetCommandRequestObject(request.get());
}

const Schema_Object* CommandRequest::GetRequestObject() const {
  return Schema_GetCommandRequestObject(request.get());
}

ComponentId CommandRequest::GetComponentId() const {
  return Schema_GetCommandRequestComponentId(request.get());
}

FieldId CommandRequest::GetCommandIndex() const {
  return Schema_GetCommandRequestCommandIndex(request.get());
}

void CommandRequest::Deleter::operator()(Schema_CommandRequest* request) const noexcept {
  Schema_DestroyCommandRequest(request);
}

}  // namespace gdk
