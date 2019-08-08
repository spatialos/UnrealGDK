#ifndef GDK_COMMAND_MESSAGES_H
#define GDK_COMMAND_MESSAGES_H
#include "gdk/command_request.h"
#include "gdk/command_response.h"
#include "gdk/common_types.h"
#include "gdk/entity_query.h"
#include "gdk/entity_snapshot.h"
#include "gdk/entity_state.h"
#include <WorkerSDK/improbable/c_worker.h>
#include <string>

namespace gdk {

struct CommandResponseReceived {
  RequestId RequestId;
  CommandResponse Response;
  Worker_StatusCode Status;
  std::string Message;
  std::uint32_t CommandId;
};

struct CommandRequestReceived {
  EntityId EntityId;
  RequestId RequestId;
  CommandRequest Request;
  std::uint32_t TimeoutMillis;
  std::string CallerWorkerId;
  std::vector<std::string> CallerAttributeSet;
};

struct CommandRequestToSend {
  EntityId EntityId;
  RequestId RequestId;
  CommandRequest Request;
  // Todo how should metadata related to sending be added?
  // should this be a different field or a different struct?
  // Could be added to the whatever object stores commands to send.
  std::uint32_t Timeout;
};

struct CommandResponseToSend {
  RequestId RequestId;
  CommandResponse Response;
};

struct CommandFailure {
  RequestId RequestId;
  std::string Message;
};

struct ReserveEntityIdsRequest {
  RequestId RequestId;
  std::uint32_t NumberOfEntityIds;
  std::uint32_t TimeoutMillis;
};

struct CreateEntityRequest {
  RequestId RequestId;
  EntityState Entity;
  EntityId EntityId;
  std::uint32_t TimeoutMillis;
};

struct DeleteEntityRequest {
  RequestId RequestId;
  EntityId EntityId;
  std::uint32_t TimeoutMillis;
};

struct EntityQueryRequest {
  RequestId RequestId;
  EntityQuery Query;
  std::uint32_t TimeoutMillis;
};

struct CreateEntityResponse {
  RequestId RequestId;
  Worker_StatusCode StatusCode;
  std::string Message;
  EntityId EntityId;
};

struct DeleteEntityResponse {
  RequestId RequestId;
  EntityId EntityId;
  Worker_StatusCode StatusCode;
  std::string Message;
};

struct ReserveEntityIdsResponse {
  RequestId RequestId;
  Worker_StatusCode StatusCode;
  std::string Message;
  EntityId FirstEntityId;
  std::uint32_t NumberOfIds;
};

struct EntityQueryResponse {
  RequestId RequestId;
  Worker_StatusCode StatusCode;
  std::string Message;
  std::uint32_t ResultCount;
  std::vector<EntitySnapshot> Entities;
};

}  // namespace gdk
#endif  // GDK_COMMAND_MESSAGES_H