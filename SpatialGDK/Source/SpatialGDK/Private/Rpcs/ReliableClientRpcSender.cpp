// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "Rpcs/ReliableClientRpcSender.h"

namespace SpatialGDK
{
FReliableClientRpcSender::FReliableClientRpcSender(SpatialInterface* Sender, FMonotonicRingBufferWriter RingBufferWriter,
												   FOverflowBufferWriter OverflowWriter)
	: RingBufferWriter(RingBufferWriter)
	, OverflowWriter(OverflowWriter)
	, Sender(Sender)
{
}

void FReliableClientRpcSender::Send(Worker_EntityId EntityId, RPCPayload Rpc)
{
	EntityIdToRpcData[EntityId].MessagesToSend.Add(MoveTemp(Rpc));
}

void FReliableClientRpcSender::SendAndFlush(Worker_EntityId EntityId, RPCPayload Rpc)
{
	FRpcData& Data = EntityIdToRpcData[EntityId];
	Data.MessagesToSend.Add(MoveTemp(Rpc));
	WriteToEntity(EntityId, Data);
}

void FReliableClientRpcSender::FlushEntity(Worker_EntityId EntityId)
{
	WriteToEntity(EntityId, EntityIdToRpcData[EntityId]);
}

void FReliableClientRpcSender::FlushAll()
{
	// Either iterate over the map or just record which IDs need updating.
	for (auto& Element : EntityIdToRpcData)
	{
		WriteToEntity(Element.Key, Element.Value);
	}
}

void FReliableClientRpcSender::ClearEntity(Worker_EntityId EntityId)
{
	EntityIdToRpcData.Remove(EntityId);
}

void FReliableClientRpcSender::WriteToEntity(Worker_EntityId EntityId, FRpcData& Data)
{
	const int32 RpcSlotsAvailable = RingBufferWriter.GetNumberOfSlots() - (Data.LastSent - Data.LastAck);
	const int32 NumberToSend = FMath::Min(Data.MessagesToSend.Num(), RpcSlotsAvailable);

	// If the number total RPCs queued is larger than the overflow then there have been new messages since the last flush.
	// This means that either the overflow list needs to be rewritten or cleared.
	if (Data.MessagesToSend.Num() != Data.OverflowSize)
	{
		Sender->SendUpdate(
			EntityId, OverflowWriter.CreateUpdate(Data.MessagesToSend.GetData() + NumberToSend, Data.MessagesToSend.Num() - NumberToSend));
	}

	if (NumberToSend > 0)
	{
		Sender->SendUpdate(EntityId, RingBufferWriter.CreateUpdate(Data.MessagesToSend.GetData(), NumberToSend, Data.LastSent));
		Data.MessagesToSend.RemoveAt(0, NumberToSend);
	}
}

} // namespace SpatialGDK
