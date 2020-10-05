// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"

#include "Schema/CrossServerEndpoint.h"
#include "SpatialView/SubView.h"
#include "Utils/CrossServerUtils.h"

DECLARE_LOG_CATEGORY_EXTERN(LogSpatialRoutingSystem, Log, All)

class SpatialOSWorkerInterface;

namespace SpatialGDK
{
struct SpatialRoutingSystem
{
	SpatialRoutingSystem(const FSubView& InSubView, FString InRoutingWorkerId)
		: SubView(InSubView)
		, RoutingWorkerId(InRoutingWorkerId)
	{
	}

	void Init(SpatialOSWorkerInterface* Connection);

	void Advance(SpatialOSWorkerInterface* Connection);

	void Flush(SpatialOSWorkerInterface* Connection);

private:
	const FSubView& SubView;

	struct RoutingComponents
	{
		TOptional<CrossServerEndpointSender> Sender;

		// Allocation slots for Sender ACKSlots
		CrossServer::SlotAlloc SenderACKAlloc;

		// Map of Receiver slots and Sender ACK slots to check when the sender buffer changes.
		CrossServer::RPCAllocMap AllocMap;

		CrossServer::SenderState Receiver;

		TOptional<CrossServerEndpointReceiverACK> ReceiverACK;
	};

	void ProcessUpdate(Worker_EntityId, const ComponentChange& Change, RoutingComponents& Components);

	void OnSenderChanged(Worker_EntityId, RoutingComponents& Components);
	void TransferRPCsToReceiver(Worker_EntityId ReceiverId, RoutingComponents& Components);

	void OnReceiverACKChanged(Worker_EntityId, RoutingComponents& Components);
	void WriteACKToSender(CrossServer::RPCKey RPCKey, RoutingComponents& SenderComponents);
	void ClearReceiverSlot(Worker_EntityId Receiver, CrossServer::RPCKey RPCKey, RoutingComponents& ReceiverComponents);

	typedef TPair<Worker_EntityId_Key, Worker_ComponentId> EntityComponentId;

	Schema_ComponentUpdate* GetOrCreateComponentUpdate(EntityComponentId EntityComponentIdPair);

	TMap<Worker_EntityId_Key, RoutingComponents> RoutingWorkerView;

	TMap<EntityComponentId, Schema_ComponentUpdate*> PendingComponentUpdatesToSend;

	void CreateRoutingWorkerEntity(SpatialOSWorkerInterface* Connection);
	Worker_RequestId RoutingWorkerEntityRequest;
	Worker_EntityId RoutingWorkerEntity;
	FString RoutingWorkerId;
	TSet<Worker_EntityId_Key> ReceiversToInspect;
};
} // namespace SpatialGDK
