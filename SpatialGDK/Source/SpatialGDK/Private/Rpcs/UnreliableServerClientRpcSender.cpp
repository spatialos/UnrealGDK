// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "Rpcs/UnreliableServerClientRpcSender.h"

namespace SpatialGDK
{
FUnreliableServerClientRpcSender::FUnreliableServerClientRpcSender(SpatialInterface* Sender, FMonotonicRingBufferWriter RingBufferWriter)
	: RingBufferWriter(RingBufferWriter)
	, Sender(Sender)
{
}

void FUnreliableServerClientRpcSender::Send(Worker_EntityId EntityId, RPCPayload Rpc)
{
	EntityIdToRpcData[EntityId].MessagesToSend.Add(MoveTemp(Rpc));
}

void FUnreliableServerClientRpcSender::SendAndFlush(Worker_EntityId EntityId, RPCPayload Rpc)
{
	FRpcData& Data = EntityIdToRpcData[EntityId];
	Data.MessagesToSend.Add(MoveTemp(Rpc));
	WriteToEntity(EntityId, Data);
}

void FUnreliableServerClientRpcSender::FlushEntity(Worker_EntityId EntityId)
{
	WriteToEntity(EntityId, EntityIdToRpcData[EntityId]);
}

void FUnreliableServerClientRpcSender::FlushAll()
{
	// Either iterate over the map or just record which IDs need updating.
	for (auto& Element : EntityIdToRpcData)
	{
		WriteToEntity(Element.Key, Element.Value);
	}
}

void FUnreliableServerClientRpcSender::ClearEntity(Worker_EntityId EntityId)
{
	EntityIdToRpcData.Remove(EntityId);
}

void FUnreliableServerClientRpcSender::WriteToEntity(Worker_EntityId EntityId, FRpcData& Data)
{
	const int32 RpcSlotsAvailable = RingBufferWriter.GetNumberOfSlots() - (Data.LastSent - Data.LastAck);
	const int32 NumberToSend = FMath::Min(Data.MessagesToSend.Num(), RpcSlotsAvailable);

	if (NumberToSend > 0)
	{
		Sender->SendUpdate(EntityId, RingBufferWriter.CreateUpdate(Data.MessagesToSend.GetData(), NumberToSend, Data.LastSent));
	}
	Data.MessagesToSend.Empty();
}

} // namespace SpatialGDK
