#ifndef GDK_MESSAGES_TO_SEND_H
#define GDK_MESSAGES_TO_SEND_H
#include "gdk/command_messages.h"
#include "gdk/entity_component_messages.h"
#include "gdk/misc_messages.h"
#include <vector>

namespace gdk {

// todo probably split this out into messages grouped by potential atomicity or by theme.
// May want to pool these.
struct MessagesToSend {
  std::vector<LogMessage> Logs;
  std::vector<MetricsMessage> Metrics;
  std::vector<EntityComponentUpdate> Updates;
  std::vector<EntityComponentData> ComponentsAdded;
  std::vector<EntityComponentId> ComponentsRemoved;
  std::vector<CommandRequestToSend> CommandRequests;
  std::vector<CommandResponseToSend> CommandResponse;
  std::vector<CommandFailure> CommandFailures;
  std::vector<ReserveEntityIdsRequest> ReserveEntityIdsRequests;
  std::vector<CreateEntityRequest> CreateEntityRequests;
  std::vector<DeleteEntityRequest> DeleteEntityRequests;
  std::vector<EntityQueryRequest> EntityQueryRequests;
  std::vector<EntityComponentId> AuthorityLossAcks;
  std::vector<InterestChange> InterestChanges;
};

}  // namespace gdk
#endif  // GDK_MESSAGES_TO_SEND_H
