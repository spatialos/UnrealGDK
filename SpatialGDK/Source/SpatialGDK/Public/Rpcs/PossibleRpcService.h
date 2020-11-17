// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "Rpcs/CommandRpcSender.h"
#include "Rpcs/MulticastRpcSender.h"
#include "Rpcs/ReliableClientRpcSender.h"
#include "Rpcs/ReliableServerRpcSender.h"
#include "Rpcs/UnreliableServerClientRpcSender.h"
#include "SpatialView/ViewCoordinator.h"
#include "Utils/RPCContainer.h"

namespace SpatialGDK
{
class FBetterRpcService
{
public:
	FBetterRpcService(SpatialInterface* Sender);

	void InitializeAuthEntity(Worker_EntityId EntityId, const EntityViewElement& EntityState);
	void InitializeNonAuthEntity(Worker_EntityId EntityId, const EntityViewElement& EntityState);
	void CleanupNonAuthEntity(Worker_EntityId EntityId);

	void SendRpc(Worker_EntityId EntityId, RPCPayload Payload, ERPCType RpcType);
	void SendAndFlushRpc(Worker_EntityId EntityId, RPCPayload Payload, ERPCType RpcType);

	void FlushRpcs(Worker_EntityId EntityId);
	void FlushAll();

private:
	FReliableClientRpcSender ReliableClientSender;
	FReliableServerRpcSender ReliableServerSender;
	FUnreliableServerClientRpcSender UnreliableServerClientSender;
	FMulticastRpcSender MulticastSender;
	FCommandRpcSender RpcSender;
};
} // namespace SpatialGDK
