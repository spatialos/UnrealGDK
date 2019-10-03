#ifndef GDK_WORKER_VIEW_H
#define GDK_WORKER_VIEW_H
#include "gdk/entity_view.h"
#include "gdk/view_delta/view_delta.h"
#include "gdk/messages_to_send.h"
#include "gdk/misc_messages.h"
#include "gdk/op_lists/op_list.h"
#include <cstddef>
#include <deque>
#include <vector>

namespace gdk {

/**
 * Represents a worker's view of a simulation.
 * Converts a stream of OpList instances into successive ViewDelta states.
 * Converts local changes into MessagesToSend instances.
 */
class WorkerView {
public:
  explicit WorkerView(const ComponentRanges& componentRanges);

  /** Add an OpList to generate the next ViewDelta. */
  void EnqueueOpList(OpList&& opList);
  /**
   * Use all available OpList instances to create a new ViewDelta.
   * The previous ViewDelta will be invalidated.
   *
   */
  const ViewDelta* GetNextViewDelta();

  /** Ensure all local changes have been applied and populate MessagesToSend. */
  MessagesToSend FlushLocalChanges();

  void SendLogMessage(LogMessage logMessage);
  void SendMetricsMessage(MetricsMessage metrics);
  void SendUpdate(EntityComponentUpdate update);
  void SendAddComponent(EntityComponentData data);
  void SendRemoveComponent(EntityComponentId entityComponentId);
  void SendCommandRequest(CommandRequestToSend request);
  void SendCommandResponse(CommandResponseToSend response);
  void SendCommandFailure(CommandFailure failure);
  void SendReserveEntityIdsRequest(ReserveEntityIdsRequest request);
  void SendCreateEntityRequest(CreateEntityRequest request);
  void SendDeleteEntityRequest(DeleteEntityRequest request);
  void SendEntityQueryRequest(EntityQueryRequest request);
  void SendAuthorityLossAck(EntityComponentId entityComponentId);
  void SendInterestChange(InterestChange interestChanges);

private:
  struct ProcessOpsResult {
    bool InUnclosedCriticalSection;
    size_t NextOpToProcessIndex;
  };

  static bool ContainsCriticalSectionOp(const OpList& opList, size_t startIndex);
  static void MarkOpsAsRead(OpList& opList, size_t indexReadTo);

  // Checks to see if an open critical section is closed in any OpList currently queued.
  bool IsCriticalSectionClosed(size_t criticalSectionStartIndex) const;

  void ApplyLocalChanges(const MessagesToSend& messagesToSend);

  // Processes queued op lists to create a ViewDelta.
  // If a critical section is opened then ops will not be processed until end is queued.
  // Returns true if no unclosed critical sections are found and false otherwise.
  ProcessOpsResult ProcessOpList(const OpList& opList);

  void HandleAddEntity(const Worker_AddEntityOp& addEntity);
  void HandleRemoveEntity(const Worker_RemoveEntityOp& removeEntity);
  void HandleReserveEntityIdsResponse(const Worker_ReserveEntityIdsResponseOp& response);
  void HandleCreateEntityResponse(const Worker_CreateEntityResponseOp& response);
  void HandleDeleteEntityResponse(const Worker_DeleteEntityResponseOp& response);
  void HandleEntityQueryResponse(const Worker_EntityQueryResponseOp& response);
  void HandleAddComponent(const Worker_AddComponentOp& addComponent);
  void HandleRemoveComponent(const Worker_RemoveComponentOp& removeComponent);
  void HandleAuthorityChange(const Worker_AuthorityChangeOp& authorityChange);
  void HandleComponentUpdate(const Worker_ComponentUpdateOp& componentUpdate);
  void HandleCompleteComponentUpdate(const Worker_AddComponentOp& completeUpdate);
  void HandleCommandRequest(const Worker_CommandRequestOp& request);
  void HandleCommandResponse(const Worker_CommandResponseOp& response);

  EntityView view;
  ViewDelta delta;

  std::deque<OpList> opListQueue;
  std::vector<OpList> usedOpLists;

  MessagesToSend currentMessages;
};

}  // namespace gdk
#endif  // GDK_WORKER_VIEW_H
