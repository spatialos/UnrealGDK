#ifndef GDK_WORKER_VIEW_H
#define GDK_WORKER_VIEW_H
#include "gdk/internal/entity_view.h"
#include "gdk/internal/view_delta.h"
#include "gdk/messages_to_send.h"
#include "gdk/misc_messages.h"
#include "gdk/op_list.h"
#include <cstddef>
#include <deque>
#include <vector>

namespace gdk {

class WorkerView {
public:
  explicit WorkerView(const ComponentRanges& componentRanges);

  void EnqueueOpList(OpList&& opList);
  const ViewDelta* GetNextViewDelta();

  MessagesToSend FlushLocalChanges();

  void SendLogMessage(LogMessage logMessage);
  void SendMetricsMessage(MetricsMessage metrics);
  void SendUpdate(EntityComponentUpdate update);
  void AddComponent(EntityComponentData data);
  void RemoveComponent(EntityComponentId entityComponentId);
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
  static bool ContainsCriticalSectionOp(const OpList& opList, size_t startIndex);

  bool IsCriticalSectionClosed(size_t criticalSectionStartIndex) const;

  void ApplyLocalChanges(const MessagesToSend& messagesToSend);

  void ProcessOpList(const OpList& opList);

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

  bool inOpenCriticalSection;
  size_t firstOpInCurrentList;

  MessagesToSend currentMessages;
};

}  // namespace gdk
#endif  // GDK_WORKER_VIEW_H
