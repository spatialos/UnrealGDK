#ifndef GDK_COMMAND_MESSAGES_H
#define GDK_COMMAND_MESSAGES_H
#include "gdk/command_request.h"
#include "gdk/command_response.h"
#include "gdk/worker_sdk.h"
#include "gdk/entity_query.h"
#include "gdk/entity_snapshot.h"
#include "gdk/entity_state.h"
#include <string>

namespace gdk {

struct CommandResponseReceived {
  gdk::RequestId RequestId;
  CommandResponse Response;
  Worker_StatusCode Status;
  std::string Message;
};

struct CommandRequestReceived {
  gdk::EntityId EntityId;
  gdk::RequestId RequestId;
  CommandRequest Request;
  std::uint32_t TimeoutMillis;
  std::string CallerWorkerId;
  std::vector<std::string> CallerAttributeSet;
};

struct CommandRequestToSend {
  gdk::EntityId EntityId;
  gdk::RequestId RequestId;
  CommandRequest Request;
  // Todo how should metadata related to sending be added?
  // should this be a different field or a different struct?
  // Could be added to the whatever object stores commands to send.
  std::uint32_t Timeout;
};

struct CommandResponseToSend {
  gdk::RequestId RequestId;
  CommandResponse Response;
};

struct CommandFailure {
  gdk::RequestId RequestId;
  std::string Message;
};

struct ReserveEntityIdsRequest {
  gdk::RequestId RequestId;
  std::uint32_t NumberOfEntityIds;
  std::uint32_t TimeoutMillis;
};

struct CreateEntityRequest {
  gdk::RequestId RequestId;
  EntityState Entity;
  gdk::EntityId EntityId;
  std::uint32_t TimeoutMillis;
};

struct DeleteEntityRequest {
  gdk::RequestId RequestId;
  gdk::EntityId EntityId;
  std::uint32_t TimeoutMillis;
};

struct EntityQueryRequest {
  gdk::RequestId RequestId;
  EntityQuery Query;
  std::uint32_t TimeoutMillis;
};

struct CreateEntityResponse {
  gdk::RequestId RequestId;
  Worker_StatusCode StatusCode;
  std::string Message;
  gdk::EntityId EntityId;
};

struct DeleteEntityResponse {
  gdk::RequestId RequestId;
  gdk::EntityId EntityId;
  Worker_StatusCode StatusCode;
  std::string Message;
};

struct ReserveEntityIdsResponse {
  gdk::RequestId RequestId;
  Worker_StatusCode StatusCode;
  std::string Message;
  gdk::EntityId FirstEntityId;
  std::uint32_t NumberOfIds;
};

struct EntityQueryResponse {
  gdk::RequestId RequestId;
  Worker_StatusCode StatusCode;
  std::string Message;
  std::uint32_t ResultCount;
  std::vector<EntitySnapshot> Entities;
};

}  // namespace gdk
#endif  // GDK_COMMAND_MESSAGES_H
