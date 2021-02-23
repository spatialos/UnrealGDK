// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "Interop/RPCs/RPCTypes.h"
#include "SpatialView/SubView.h"

class USpatialNetDriver;

namespace SpatialGDK
{
class SPATIALGDK_API RPCService
{
public:
	explicit RPCService(const FSubView& InRemoteSubView, const FSubView& InLocalAuthSubView);

	void AdvanceView();

	struct UpdateToSend
	{
		Worker_EntityId EntityId;
		FWorkerComponentUpdate Update;
		FSpatialGDKSpanId SpanId;
	};
	TArray<UpdateToSend> GetRPCsAndAcksToSend();
	TArray<FWorkerComponentData> GetRPCComponentsOnEntityCreation(Worker_EntityId EntityId);

	struct RPCQueueDescription
	{
		Worker_ComponentSetId Authority;
		TSharedPtr<RPCBufferSender> Sender;
		TSharedPtr<RPCQueue> Queue;
	};

	struct RPCReceiverDescription
	{
		Worker_ComponentSetId Authority;
		TSharedPtr<RPCBufferReceiver> Receiver;
	};

	void AddRPCQueue(FName QueueName, RPCQueueDescription&& Desc);
	void AddRPCReceiver(FName ReceiverName, RPCReceiverDescription&& Desc);

private:
	const FSubView* RemoteSubView;
	const FSubView* LocalAuthSubView;

	TMap<FName, RPCQueueDescription> Queues;
	TMap<FName, RPCReceiverDescription> Receivers;
};

} // namespace SpatialGDK
