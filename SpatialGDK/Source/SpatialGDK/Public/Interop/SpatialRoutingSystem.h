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
class SpatialRoutingSystem
{
public:
	SpatialRoutingSystem(const FSubView& InSubView, FSpatialEntityId InRoutingWorkerSystemEntityId)
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

	void ProcessUpdate(FSpatialEntityId, const ComponentChange& Change, RoutingComponents& Components);

	void OnSenderChanged(FSpatialEntityId, RoutingComponents& Components);
	void TransferRPCsToReceiver(FSpatialEntityId ReceiverId, RoutingComponents& Components);

	void OnReceiverACKChanged(FSpatialEntityId, RoutingComponents& Components);
	void WriteACKToSender(CrossServer::RPCKey RPCKey, RoutingComponents& SenderComponents, CrossServer::Result Result);
	void ClearReceiverSlot(FSpatialEntityId Receiver, CrossServer::RPCKey RPCKey, RoutingComponents& ReceiverComponents);

	typedef TPair<FSpatialEntityId, Worker_ComponentId> EntityComponentId;

	Schema_ComponentUpdate* GetOrCreateComponentUpdate(EntityComponentId EntityComponentIdPair);

	TMap<FSpatialEntityId, RoutingComponents> RoutingWorkerView;

	TMap<EntityComponentId, Schema_ComponentUpdate*> PendingComponentUpdatesToSend;

	Worker_RequestId RoutingWorkerRequest;
	FSpatialEntityId RoutingWorkerSystemEntityId;
	TSet<FSpatialEntityId> ReceiversToInspect;
};
} // namespace SpatialGDK
