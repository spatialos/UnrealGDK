// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "Rpcs/ReliableServerRpcSender.h"

namespace SpatialGDK
{
FReliableServerRpcSender::FReliableServerRpcSender(SpatialInterface* Sender, FMonotonicRingBufferWriter RingBufferWriter)
	: RingBufferWriter(RingBufferWriter)
	, Sender(Sender)
{
}

void FReliableServerRpcSender::Send(Worker_EntityId EntityId, RPCPayload Rpc)
{
	EntityIdToRpcData[EntityId].MessagesToSend.Add(MoveTemp(Rpc));
}

void FReliableServerRpcSender::SendAndFlush(Worker_EntityId EntityId, RPCPayload Rpc)
{
	FRpcData& Data = EntityIdToRpcData[EntityId];
	Data.MessagesToSend.Add(MoveTemp(Rpc));
	WriteToEntity(EntityId, Data);
}

void FReliableServerRpcSender::FlushEntity(Worker_EntityId EntityId)
{
	WriteToEntity(EntityId, EntityIdToRpcData[EntityId]);
}

void FReliableServerRpcSender::FlushAll()
{
	// Either iterate over the map or just record which IDs need updating.
	for (auto& Element : EntityIdToRpcData)
	{
		WriteToEntity(Element.Key, Element.Value);
	}
}

void FReliableServerRpcSender::ClearEntity(Worker_EntityId EntityId)
{
	EntityIdToRpcData.Remove(EntityId);
}

void FReliableServerRpcSender::WriteToEntity(Worker_EntityId EntityId, FRpcData& Data)
{
	const int32 RpcSlotsAvailable = RingBufferWriter.GetNumberOfSlots() - (Data.LastSent - Data.LastAck);
	const int32 NumberToSend = FMath::Min(Data.MessagesToSend.Num(), RpcSlotsAvailable);

	if (NumberToSend > 0)
	{
		Sender->SendUpdate(EntityId, RingBufferWriter.CreateUpdate(Data.MessagesToSend.GetData(), NumberToSend, Data.LastSent));
		Data.MessagesToSend.RemoveAt(0, NumberToSend);
	}
}

} // namespace SpatialGDK
