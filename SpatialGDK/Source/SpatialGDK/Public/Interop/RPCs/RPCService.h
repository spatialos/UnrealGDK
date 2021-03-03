// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "Interop/RPCs/RPCTypes.h"
#include "SpatialView/SubView.h"

class USpatialNetDriver;

namespace SpatialGDK
{
/**
 * The RPCService aggregates Sender and Receiver in order to route updates and events from the network.
 * It does not deal directly with actor concepts and RPC data, this is pushed to the user side and individual
 * sender/receiver specialization.
 */
class SPATIALGDK_API RPCService
{
public:
	explicit RPCService(const FSubView& InRemoteSubView, const FSubView& InLocalAuthSubView);

	void AdvanceView();

	struct RPCQueueDescription
	{
		Worker_ComponentSetId Authority;
		RPCBufferSender* Sender;
		RPCQueue* Queue;
	};

	struct RPCReceiverDescription
	{
		// Can be 0, in which case the receiver will consider every entity in the view.
		Worker_ComponentSetId Authority;
		RPCBufferReceiver* Receiver;
	};

	void AddRPCQueue(FName QueueName, RPCQueueDescription&& Desc);
	void AddRPCReceiver(FName ReceiverName, RPCReceiverDescription&& Desc);

	TMap<FName, RPCReceiverDescription>& GetReceivers() { return Receivers; }

private:
	void AdvanceSenderQueues();
	void AdvanceReceivers();
	void ProcessUpdatesToSender(Worker_EntityId EntityId, ComponentSpan<ComponentChange> Updates);
	void ProcessUpdatesToReceivers(Worker_EntityId EntityId, const EntityViewElement& ViewElement, ComponentSpan<ComponentChange> Updates);
	void HandleReceiverAuthorityGained(Worker_EntityId EntityId, const EntityViewElement& ViewElement,
									   ComponentSpan<AuthorityChange> AuthChanges);
	void HandleReceiverAuthorityLost(Worker_EntityId EntityId, ComponentSpan<AuthorityChange> AuthChanges);

	// Receiver may or may not need authority to be able to receive RPCs (Client/Server vs Multicast).
	// On the other hand, Senders always need some authority in order to write outgoing RPCs
	// That is why there is not equivalent functions for the senders.
	static constexpr Worker_ComponentSetId NoAuthorityNeeded = 0;
	static bool HasReceiverAuthority(const RPCReceiverDescription& Desc, const EntityViewElement& ViewElement);
	static bool IsReceiverAuthoritySet(const RPCReceiverDescription& Desc, Worker_ComponentSetId ComponentSet);

	const FSubView* RemoteSubView;
	const FSubView* LocalAuthSubView;

	TMap<FName, RPCQueueDescription> Queues;
	TMap<FName, RPCReceiverDescription> Receivers;
};

} // namespace SpatialGDK
