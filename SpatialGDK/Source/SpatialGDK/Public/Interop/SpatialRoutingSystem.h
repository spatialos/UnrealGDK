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
	SpatialRoutingSystem(const FSubView& InSubView, Worker_EntityId InRoutingWorkerSystemEntityId)
		: SubView(InSubView)
		, RoutingWorkerSystemEntityId(InRoutingWorkerSystemEntityId)
	{
	}

	void Init(SpatialOSWorkerInterface* Connection);
	void Advance(SpatialOSWorkerInterface* Connection);
	void Flush(SpatialOSWorkerInterface* Connection);
	void Destroy(SpatialOSWorkerInterface* Connection);

private:
	const FSubView& SubView;

	struct RoutingComponents
	{
		TOptional<CrossServerEndpoint> Sender;
		CrossServer::ReaderState SenderACKState;
		CrossServer::RPCSchedule ReceiverSchedule;
		CrossServer::WriterState ReceiverState;
		TOptional<CrossServerEndpointACK> ReceiverACK;
	};

	void ProcessUpdate(Worker_EntityId, const ComponentChange& Change, RoutingComponents& Components);

	void OnSenderChanged(Worker_EntityId, RoutingComponents& Components);
	void TransferRPCsToReceiver(Worker_EntityId ReceiverId, RoutingComponents& Components);

	void OnReceiverACKChanged(Worker_EntityId, RoutingComponents& Components);
	void WriteACKToSender(CrossServer::RPCKey RPCKey, RoutingComponents& SenderComponents, CrossServer::Result Result);
	void ClearReceiverSlot(Worker_EntityId Receiver, CrossServer::RPCKey RPCKey, RoutingComponents& ReceiverComponents);

	typedef TPair<Worker_EntityId_Key, Worker_ComponentId> EntityComponentId;

	Schema_ComponentUpdate* GetOrCreateComponentUpdate(EntityComponentId EntityComponentIdPair);

	TMap<Worker_EntityId_Key, RoutingComponents> RoutingWorkerView;

	TMap<EntityComponentId, Schema_ComponentUpdate*> PendingComponentUpdatesToSend;

	void CreateRoutingPartition(SpatialOSWorkerInterface* Connection);
	Worker_RequestId RoutingWorkerRequest;
	Worker_EntityId RoutingPartition;
	Worker_EntityId RoutingWorkerSystemEntityId;
	TSet<Worker_EntityId_Key> ReceiversToInspect;
};
} // namespace SpatialGDK
