// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "Rpcs/PossibleRpcService.h"

namespace SpatialGDK
{
FBetterRpcService::FBetterRpcService(SpatialInterface* Sender)
	: ReliableClientSender(Sender, FMonotonicRingBufferWriter(SpatialConstants::SERVER_ENDPOINT_COMPONENT_ID, 1, 2, 3),
						   FOverflowBufferWriter(SpatialConstants::SERVER_ENDPOINT_COMPONENT_ID, 1))
	, ReliableServerSender(Sender, FMonotonicRingBufferWriter(SpatialConstants::CLIENT_ENDPOINT_COMPONENT_ID, 1, 2, 3))
	, UnreliableServerClientSender(Sender, FMonotonicRingBufferWriter(SpatialConstants::CLIENT_ENDPOINT_COMPONENT_ID, 1, 2, 3))
	, MulticastSender(Sender, FMonotonicRingBufferWriter(SpatialConstants::MULTICAST_RPCS_COMPONENT_ID, 1, 2, 3))
	, RpcSender(Sender, FCommandRpcWriter(SpatialConstants::SERVER_TO_SERVER_COMMAND_ENDPOINT_COMPONENT_ID, 1))
{
}

void FBetterRpcService::InitializeAuthEntity(Worker_EntityId EntityId, const EntityViewElement& EntityState)
{
	// Setup the count and acks on ring buffer rpcs.
	// Re-snd the overflowed RPCs for reliable client RPCs.
	//
}

void FBetterRpcService::InitializeNonAuthEntity(Worker_EntityId EntityId, const EntityViewElement& EntityState) {}

void FBetterRpcService::CleanupNonAuthEntity(Worker_EntityId EntityId)
{
	ReliableClientSender.ClearEntity(EntityId);
	ReliableServerSender.ClearEntity(EntityId);
	UnreliableServerClientSender.ClearEntity(EntityId);
}

void FBetterRpcService::SendRpc(Worker_EntityId EntityId, RPCPayload Payload, ERPCType RpcType)
{
	switch (RpcType)
	{
	case ERPCType::ClientReliable:
		ReliableClientSender.Send(EntityId, MoveTemp(Payload));
		break;
	case ERPCType::ClientUnreliable:
		UnreliableServerClientSender.Send(EntityId, MoveTemp(Payload));
		break;
	case ERPCType::ServerReliable:
		ReliableServerSender.Send(EntityId, MoveTemp(Payload));
		break;
	case ERPCType::ServerUnreliable:
		UnreliableServerClientSender.Send(EntityId, MoveTemp(Payload));
		break;
	case ERPCType::NetMulticast:
		MulticastSender.Send(EntityId, MoveTemp(Payload));
		break;
	case ERPCType::CrossServer:
		RpcSender.SendAndFlush(EntityId, MoveTemp(Payload));
		break;
	default:
		checkNoEntry();
	}
}

void FBetterRpcService::SendAndFlushRpc(Worker_EntityId EntityId, RPCPayload Payload, ERPCType RpcType)
{
	switch (RpcType)
	{
	case ERPCType::ClientReliable:
		ReliableClientSender.SendAndFlush(EntityId, MoveTemp(Payload));
		break;
	case ERPCType::ClientUnreliable:
		UnreliableServerClientSender.SendAndFlush(EntityId, MoveTemp(Payload));
		break;
	case ERPCType::ServerReliable:
		ReliableServerSender.SendAndFlush(EntityId, MoveTemp(Payload));
		break;
	case ERPCType::ServerUnreliable:
		UnreliableServerClientSender.SendAndFlush(EntityId, MoveTemp(Payload));
		break;
	case ERPCType::NetMulticast:
		MulticastSender.SendAndFlush(EntityId, MoveTemp(Payload));
		break;
	case ERPCType::CrossServer:
		RpcSender.SendAndFlush(EntityId, MoveTemp(Payload));
		break;
	default:
		checkNoEntry();
	}
}

void FBetterRpcService::FlushRpcs(Worker_EntityId EntityId)
{
	ReliableClientSender.FlushEntity(EntityId);
	UnreliableServerClientSender.FlushEntity(EntityId);
	ReliableServerSender.FlushEntity(EntityId);
	UnreliableServerClientSender.FlushEntity(EntityId);
	MulticastSender.FlushEntity(EntityId);
}

void FBetterRpcService::FlushAll()
{
	ReliableClientSender.FlushAll();
	UnreliableServerClientSender.FlushAll();
	ReliableServerSender.FlushAll();
	UnreliableServerClientSender.FlushAll();
	MulticastSender.FlushAll();
}

} // namespace SpatialGDK
