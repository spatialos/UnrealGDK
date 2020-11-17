// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "Rpcs/MulticastRpcSender.h"

namespace SpatialGDK
{
FMulticastRpcSender::FMulticastRpcSender(SpatialInterface* Sender, FMonotonicRingBufferWriter RingBufferWriter)
	: RingBufferWriter(RingBufferWriter)
	, Sender(Sender)
{
}

void FMulticastRpcSender::Send(Worker_EntityId EntityId, RPCPayload Rpc)
{
	EntityIdToRpcData[EntityId].MessagesToSend.Add(MoveTemp(Rpc));
}

void FMulticastRpcSender::SendAndFlush(Worker_EntityId EntityId, RPCPayload Rpc)
{
	FRpcData& Data = EntityIdToRpcData[EntityId];
	Data.MessagesToSend.Add(MoveTemp(Rpc));
	WriteToEntity(EntityId, Data);
}

void FMulticastRpcSender::FlushEntity(Worker_EntityId EntityId)
{
	WriteToEntity(EntityId, EntityIdToRpcData[EntityId]);
}

void FMulticastRpcSender::FlushAll()
{
	// Either iterate over the map or just record which IDs need updating.
	for (auto& Element : EntityIdToRpcData)
	{
		WriteToEntity(Element.Key, Element.Value);
	}
}

void FMulticastRpcSender::ClearEntity(Worker_EntityId EntityId)
{
	EntityIdToRpcData.Remove(EntityId);
}

void FMulticastRpcSender::WriteToEntity(Worker_EntityId EntityId, FRpcData& Data)
{
	// Effectively write every RPC sent.
	// If there are more RPCs to send than there are slots (call this s) this is equivalent to only the sending the last s RPCs.
	const int32 NumberToSend = FMath::Min(Data.MessagesToSend.Num(), RingBufferWriter.GetNumberOfSlots());
	Sender->SendUpdate(EntityId, RingBufferWriter.CreateUpdate(Data.MessagesToSend.GetData() + Data.MessagesToSend.Num() - NumberToSend,
															   NumberToSend, Data.LastSent));
	Data.MessagesToSend.Empty();
}

} // namespace SpatialGDK
