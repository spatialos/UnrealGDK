// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "SpatialView/CommandRetryHandler.h"
#include "SpatialView/ComponentSetData.h"
#include "SpatialView/ConnectionHandler/AbstractConnectionHandler.h"
#include "SpatialView/CriticalSectionFilter.h"
#include "SpatialView/Dispatcher.h"
#include "SpatialView/ReceivedOpEventHandler.h"
#include "SpatialView/SpatialOSWorker.h"
#include "SpatialView/SubView.h"
#include "SpatialView/WorkerView.h"

#include "Templates/UniquePtr.h"

namespace SpatialGDK
{
class SpatialEventTracer;

class ViewCoordinator : public ISpatialOSWorker
{
public:
	explicit ViewCoordinator(TUniquePtr<AbstractConnectionHandler> ConnectionHandler, TSharedPtr<SpatialEventTracer> EventTracer,
							 FComponentSetData ComponentSetData);

	~ViewCoordinator();

	// Movable, Non-copyable.
	ViewCoordinator(const ViewCoordinator&) = delete;
	ViewCoordinator(ViewCoordinator&&) = default;
	ViewCoordinator& operator=(const ViewCoordinator&) = delete;
	ViewCoordinator& operator=(ViewCoordinator&&) = default;

	void Advance(float DeltaTimeS);
	const ViewDelta& GetViewDelta() const;
	void FlushMessagesToSend();

	// Create a subview with the specified tag, filter, and refresh callbacks.
	FSubView& CreateSubView(Worker_ComponentId Tag, const FFilterPredicate& Filter,
							const TArray<FDispatcherRefreshCallback>& DispatcherRefreshCallbacks);
	// Force a refresh of the given entity ID across all subviews. Used when local state changes which could
	// change any subview's filter's truth value for the given entity. Conceptually this can be thought of
	// as marking the entity dirty for all subviews, although the refresh is immediate.
	// Note: It would be possible to only refresh the subviews that require refreshing instead of globally refreshing.
	// This could be achieved by either having systems understand which subviews need refreshing, or by having systems
	// broadcast events which subviews subscribe to in order to trigger a refresh. This global refresh may only be a
	// temporary solution which keeps the API simple while the main systems are Actors and the load balancer.
	// In the future when there could be an unbounded number of user systems this should probably be revisited.
	void RefreshEntityCompleteness(Worker_EntityId EntityId);

	virtual const TArray<EntityDelta>& GetEntityDeltas() const override;
	virtual const TArray<Worker_Op>& GetWorkerMessages() const override;
	virtual const EntityView& GetView() const override;

	virtual const FString& GetWorkerId() const override;

	virtual Worker_EntityId GetWorkerSystemEntityId() const override;

	virtual const ComponentData* GetComponent(Worker_EntityId EntityId, Worker_ComponentId ComponentId) const override;

	virtual void SendAddComponent(Worker_EntityId EntityId, ComponentData Data, const FSpatialGDKSpanId& SpanId = {}) override;
	virtual void SendComponentUpdate(Worker_EntityId EntityId, ComponentUpdate Update, const FSpatialGDKSpanId& SpanId = {}) override;
	virtual void SendRemoveComponent(Worker_EntityId EntityId, Worker_ComponentId ComponentId,
									 const FSpatialGDKSpanId& SpanId = {}) override;

	virtual Worker_RequestId SendEntityCommandRequest(Worker_EntityId EntityId, CommandRequest Request, FRetryData RetryData = NO_RETRIES,
													  const FSpatialGDKSpanId& SpanId = {}) override;
	virtual void SendEntityCommandResponse(Worker_RequestId RequestId, CommandResponse Response,
										   const FSpatialGDKSpanId& SpanId = {}) override;
	virtual void SendEntityCommandFailure(Worker_RequestId RequestId, FString Message, const FSpatialGDKSpanId& SpanId = {}) override;

	virtual Worker_RequestId SendReserveEntityIdsRequest(uint32 NumberOfEntityIds, FRetryData RetryData = NO_RETRIES) override;
	virtual Worker_RequestId SendCreateEntityRequest(TArray<ComponentData> EntityComponents, TOptional<Worker_EntityId> EntityId,
													 FRetryData RetryData = NO_RETRIES, const FSpatialGDKSpanId& SpanId = {}) override;
	virtual Worker_RequestId SendDeleteEntityRequest(Worker_EntityId EntityId, FRetryData RetryData = NO_RETRIES,
													 const FSpatialGDKSpanId& SpanId = {}) override;
	virtual Worker_RequestId SendEntityQueryRequest(EntityQuery Query, FRetryData RetryData = NO_RETRIES) override;

	virtual void SendMetrics(SpatialMetrics Metrics) override;
	virtual void SendLogMessage(Worker_LogLevel Level, const FName& LoggerName, FString Message) override;

	virtual bool HasEntity(Worker_EntityId EntityId) const override;
	virtual bool HasComponent(Worker_EntityId EntityId, Worker_ComponentId ComponentId) const override;
	virtual bool HasAuthority(Worker_EntityId EntityId, Worker_ComponentSetId ComponentSetId) const override;

	CallbackId RegisterComponentAddedCallback(Worker_ComponentId ComponentId, FComponentValueCallback Callback);
	CallbackId RegisterComponentRemovedCallback(Worker_ComponentId ComponentId, FComponentValueCallback Callback);
	CallbackId RegisterComponentValueCallback(Worker_ComponentId ComponentId, FComponentValueCallback Callback);
	CallbackId RegisterAuthorityGainedCallback(Worker_ComponentId ComponentId, FEntityCallback Callback);
	CallbackId RegisterAuthorityLostCallback(Worker_ComponentId ComponentId, FEntityCallback Callback);
	CallbackId RegisterAuthorityLostTempCallback(Worker_ComponentId ComponentId, FEntityCallback Callback);
	void RemoveCallback(CallbackId Id);

	FDispatcherRefreshCallback CreateComponentExistenceRefreshCallback(
		Worker_ComponentId ComponentId,
		const FComponentChangeRefreshPredicate& RefreshPredicate = FSubView::NoComponentChangeRefreshPredicate);
	FDispatcherRefreshCallback CreateComponentChangedRefreshCallback(
		Worker_ComponentId ComponentId,
		const FComponentChangeRefreshPredicate& RefreshPredicate = FSubView::NoComponentChangeRefreshPredicate);
	FDispatcherRefreshCallback CreateAuthorityChangeRefreshCallback(
		Worker_ComponentId ComponentId,
		const FAuthorityChangeRefreshPredicate& RefreshPredicate = FSubView::NoAuthorityChangeRefreshPredicate);

private:
	WorkerView View;
	TUniquePtr<AbstractConnectionHandler> ConnectionHandler;

	FCriticalSectionFilter CriticalSectionFilter;

	Worker_RequestId NextRequestId;
	FDispatcher Dispatcher;

	TArray<TUniquePtr<FSubView>> SubViews;

	FReceivedOpEventHandler ReceivedOpEventHandler;

	TCommandRetryHandler<FReserveEntityIdsRetryHandlerImpl> ReserveEntityIdRetryHandler;
	TCommandRetryHandler<FCreateEntityRetryHandlerImpl> CreateEntityRetryHandler;
	TCommandRetryHandler<FDeleteEntityRetryHandlerImpl> DeleteEntityRetryHandler;
	TCommandRetryHandler<FEntityQueryRetryHandlerImpl> EntityQueryRetryHandler;
	TCommandRetryHandler<FEntityCommandRetryHandlerImpl> EntityCommandRetryHandler;
};

template <class T>
TOptional<T> DeserializeComponent(const ViewCoordinator& Coordinator, Worker_EntityId EntityId)
{
	const ComponentData* Data = Coordinator.GetComponent(EntityId, T::ComponentId);
	if (Data != nullptr)
	{
		return T(Data->GetWorkerComponentData());
	}
	return {};
}

} // namespace SpatialGDK
