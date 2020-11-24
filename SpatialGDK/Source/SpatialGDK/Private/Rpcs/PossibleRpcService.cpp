// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "Rpcs/PossibleRpcService.h"

namespace SpatialGDK
{
FExperimentRpcService::FExperimentRpcService(SpatialInterface* Sender)
	: ReliableClientSender(Sender, FMonotonicRingBufferWriter(SpatialConstants::SERVER_ENDPOINT_COMPONENT_ID, 1, 2, 3),
						   FOverflowBufferWriter(SpatialConstants::SERVER_ENDPOINT_COMPONENT_ID, 1))
	, ReliableServerSender(Sender, FMonotonicRingBufferWriter(SpatialConstants::CLIENT_ENDPOINT_COMPONENT_ID, 1, 2, 3))
	, UnreliableServerClientSender(Sender, FMonotonicRingBufferWriter(SpatialConstants::CLIENT_ENDPOINT_COMPONENT_ID, 1, 2, 3))
	, MulticastSender(Sender, FMonotonicRingBufferWriter(SpatialConstants::MULTICAST_RPCS_COMPONENT_ID, 1, 2, 3))
	, RpcSender(Sender, FCommandRpcWriter(SpatialConstants::SERVER_TO_SERVER_COMMAND_ENDPOINT_COMPONENT_ID, 1))
{
}

void FExperimentRpcService::InitializeAuthEntity(Worker_EntityId EntityId, const EntityViewElement& EntityState)
{
	// Setup the count and acks on ring buffer rpcs.
	// Re-send the overflowed RPCs for reliable client RPCs. We could make this something the sender does with some initialisation function.
	// In fact I think I'm in favour of that although it's slightly less efficient than reading through the view in one go.
}

void FExperimentRpcService::InitializeNonAuthEntity(Worker_EntityId EntityId, const EntityViewElement& EntityState) {}

void FExperimentRpcService::CleanupNonAuthEntity(Worker_EntityId EntityId)
{
	ReliableClientSender.ClearEntity(EntityId);
	ReliableServerSender.ClearEntity(EntityId);
	UnreliableServerClientSender.ClearEntity(EntityId);
}

void FExperimentRpcService::SendRpc(Worker_EntityId EntityId, RPCPayload Payload, ERPCType RpcType)
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

void FExperimentRpcService::SendAndFlushRpc(Worker_EntityId EntityId, RPCPayload Payload, ERPCType RpcType)
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

void FExperimentRpcService::FlushRpcs(Worker_EntityId EntityId)
{
	ReliableClientSender.FlushEntity(EntityId);
	UnreliableServerClientSender.FlushEntity(EntityId);
	ReliableServerSender.FlushEntity(EntityId);
	UnreliableServerClientSender.FlushEntity(EntityId);
	MulticastSender.FlushEntity(EntityId);
}

void FExperimentRpcService::FlushAll()
{
	ReliableClientSender.FlushAll();
	UnreliableServerClientSender.FlushAll();
	ReliableServerSender.FlushAll();
	UnreliableServerClientSender.FlushAll();
	MulticastSender.FlushAll();
}

} // namespace SpatialGDK
