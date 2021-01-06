// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "Interop/RPCs/CrossServerRPCService.h"

#include "EngineClasses/SpatialPackageMapClient.h"
#include "SpatialConstants.h"
#include "Utils/RepLayoutUtils.h"

DEFINE_LOG_CATEGORY(LogCrossServerRPCService);

namespace SpatialGDK
{
CrossServerRPCService::CrossServerRPCService(const ActorCanExtractRPCDelegate InCanExtractRPCDelegate,
											 const ExtractRPCDelegate InExtractRPCCallback, const FSubView& InSubView,
											 FRPCStore& InRPCStore)
	: CanExtractRPCDelegate(InCanExtractRPCDelegate)
	, ExtractRPCCallback(InExtractRPCCallback)
	, SubView(&InSubView)
	, RPCStore(&InRPCStore)
{
}

EPushRPCResult CrossServerRPCService::PushCrossServerRPC(Worker_EntityId EntityId, const RPCSender& Sender,
														 const PendingRPCPayload& Payload, bool bCreatedEntity)
{
	CrossServerEndpoints* Endpoints = CrossServerDataStore.Find(Sender.Entity);
	Schema_Object* EndpointObject = nullptr;
	EntityComponentId SenderEndpointId(Sender.Entity, SpatialConstants::CROSSSERVER_SENDER_ENDPOINT_COMPONENT_ID);

	if (!Endpoints)
	{
		if (bCreatedEntity)
		{
			return EPushRPCResult::EntityBeingCreated;
		}

		EndpointObject = Schema_GetComponentDataFields(RPCStore->GetOrCreateComponentData(SenderEndpointId));
		Endpoints = &CrossServerDataStore.Add(Sender.Entity);
	}
	else
	{
		EndpointObject = Schema_GetComponentUpdateFields(RPCStore->GetOrCreateComponentUpdate(SenderEndpointId));
	}

	CrossServer::WriterState& SenderState = Endpoints->SenderState;

	TOptional<uint32> Slot = SenderState.Alloc.PeekFreeSlot();
	if (!Slot)
	{
		return EPushRPCResult::DropOverflowed;
	}

	uint64 NewRPCId = SenderState.LastSentRPCId++;
	uint32 SlotIdx = Slot.GetValue();

	RPCRingBufferDescriptor Descriptor = RPCRingBufferUtils::GetRingBufferDescriptor(ERPCType::CrossServer);
	uint32 Field = Descriptor.GetRingBufferElementFieldId(ERPCType::CrossServer, SlotIdx + 1);

	Schema_Object* RPCObject = Schema_AddObject(EndpointObject, Field);

	RPCTarget Target(CrossServerRPCInfo(EntityId, NewRPCId));
	CrossServer::WritePayloadAndCounterpart(EndpointObject, Payload.Payload, Target, SlotIdx);

	Schema_ClearField(EndpointObject, Descriptor.LastSentRPCFieldId);
	Schema_AddUint64(EndpointObject, Descriptor.LastSentRPCFieldId, SenderState.LastSentRPCId);

	CrossServer::RPCKey RPCKey(Sender.Entity, NewRPCId);
	CrossServer::SentRPCEntry Entry;
	Entry.Target = Target;
	Entry.SourceSlot = SlotIdx;

	SenderState.Mailbox.Add(RPCKey, Entry);
	SenderState.Alloc.CommitSlot(SlotIdx);

	return EPushRPCResult::Success;
}

void CrossServerRPCService::AdvanceView()
{
	const FSubViewDelta& SubViewDelta = SubView->GetViewDelta();
	for (const EntityDelta& Delta : SubViewDelta.EntityDeltas)
	{
		switch (Delta.Type)
		{
		case EntityDelta::UPDATE:
		{
			for (const ComponentChange& Change : Delta.ComponentUpdates)
			{
				ComponentUpdate(Delta.EntityId, Change.ComponentId, Change.Update);
			}
			break;
		}
		case EntityDelta::ADD:
			PopulateDataStore(Delta.EntityId);
			break;
		case EntityDelta::REMOVE:
		case EntityDelta::TEMPORARILY_REMOVED:
			CrossServerDataStore.Remove(Delta.EntityId);
			RPCStore->PendingComponentUpdatesToSend.Remove(
				EntityComponentId(Delta.EntityId, SpatialConstants::CROSSSERVER_SENDER_ENDPOINT_COMPONENT_ID));
			RPCStore->PendingComponentUpdatesToSend.Remove(
				EntityComponentId(Delta.EntityId, SpatialConstants::CROSSSERVER_RECEIVER_ACK_ENDPOINT_COMPONENT_ID));
			if (Delta.Type == EntityDelta::TEMPORARILY_REMOVED)
			{
				PopulateDataStore(Delta.EntityId);
			}
			break;
		default:
			break;
		}
	}
}

void CrossServerRPCService::ProcessChanges()
{
	const FSubViewDelta& SubViewDelta = SubView->GetViewDelta();
	for (const EntityDelta& Delta : SubViewDelta.EntityDeltas)
	{
		switch (Delta.Type)
		{
		case EntityDelta::UPDATE:
		{
			for (const ComponentChange& Change : Delta.ComponentUpdates)
			{
				ProcessComponentChange(Delta.EntityId, Change.ComponentId);
			}
			break;
		}
		case EntityDelta::ADD:
			EntityAdded(Delta.EntityId);
			break;
		case EntityDelta::REMOVE:

			break;
		case EntityDelta::TEMPORARILY_REMOVED:
			EntityAdded(Delta.EntityId);
			break;
		default:
			break;
		}
	}
}

void CrossServerRPCService::EntityAdded(const Worker_EntityId EntityId)
{
	for (const Worker_ComponentId ComponentId : SubView->GetView()[EntityId].Authority)
	{
		if (!IsCrossServerEndpoint(ComponentId))
		{
			continue;
		}
		OnEndpointAuthorityGained(EntityId, ComponentId);
	}
	CrossServerEndpoints* Endpoints = CrossServerDataStore.Find(EntityId);
	HandleRPC(EntityId, Endpoints->ReceivedRPCs.GetValue());
	UpdateSentRPCsACKs(EntityId, Endpoints->ACKedRPCs.GetValue());
}

void CrossServerRPCService::ComponentUpdate(const Worker_EntityId EntityId, const Worker_ComponentId ComponentId,
											Schema_ComponentUpdate* Update)
{
	if (!IsCrossServerEndpoint(ComponentId))
	{
		return;
	}

	if (CrossServerEndpoints* Endpoints = CrossServerDataStore.Find(EntityId))
	{
		switch (ComponentId)
		{
		case SpatialConstants::CROSSSERVER_RECEIVER_ENDPOINT_COMPONENT_ID:
			Endpoints->ReceivedRPCs->ApplyComponentUpdate(Update);
			break;

		case SpatialConstants::CROSSSERVER_SENDER_ACK_ENDPOINT_COMPONENT_ID:
			Endpoints->ACKedRPCs->ApplyComponentUpdate(Update);
			break;
		default:
			break;
		}
	}
}

void CrossServerRPCService::ProcessComponentChange(const Worker_EntityId EntityId, const Worker_ComponentId ComponentId)
{
	if (!IsCrossServerEndpoint(ComponentId))
	{
		return;
	}

	if (CrossServerEndpoints* Endpoints = CrossServerDataStore.Find(EntityId))
	{
		switch (ComponentId)
		{
		case SpatialConstants::CROSSSERVER_RECEIVER_ENDPOINT_COMPONENT_ID:
			HandleRPC(EntityId, Endpoints->ReceivedRPCs.GetValue());
			break;

		case SpatialConstants::CROSSSERVER_SENDER_ACK_ENDPOINT_COMPONENT_ID:
			UpdateSentRPCsACKs(EntityId, Endpoints->ACKedRPCs.GetValue());
			break;
		default:
			break;
		}
	}
}

void CrossServerRPCService::PopulateDataStore(const Worker_EntityId EntityId)
{
	const EntityViewElement& Entity = SubView->GetView()[EntityId];

	Schema_ComponentData* SenderACKData =
		Entity.Components.FindByPredicate(ComponentIdEquality{ SpatialConstants::CROSSSERVER_SENDER_ACK_ENDPOINT_COMPONENT_ID })
			->GetUnderlying();
	Schema_ComponentData* ReceiverData =
		Entity.Components.FindByPredicate(ComponentIdEquality{ SpatialConstants::CROSSSERVER_RECEIVER_ENDPOINT_COMPONENT_ID })
			->GetUnderlying();

	CrossServerEndpoints& NewEntry = CrossServerDataStore.FindOrAdd(EntityId);
	NewEntry.ACKedRPCs.Emplace(CrossServerEndpointACK(SenderACKData));
	NewEntry.ReceivedRPCs.Emplace(CrossServerEndpoint(ReceiverData));
}

void CrossServerRPCService::OnEndpointAuthorityGained(const Worker_EntityId EntityId, const Worker_ComponentId ComponentId)
{
	const EntityViewElement& Entity = SubView->GetView()[EntityId];
	const ComponentData* Data = Entity.Components.FindByPredicate([&](ComponentData& Data) {
		return Data.GetComponentId() == ComponentId;
	});

	check(Data != nullptr);

	switch (ComponentId)
	{
	case SpatialConstants::CROSSSERVER_SENDER_ENDPOINT_COMPONENT_ID:
	{
		CrossServerEndpoint SenderEndpoint(Data->GetUnderlying());
		CrossServer::WriterState& SenderState = CrossServerDataStore.FindChecked(EntityId).SenderState;
		SenderState.LastSentRPCId = SenderEndpoint.ReliableRPCBuffer.LastSentRPCId;
		for (int32 SlotIdx = 0; SlotIdx < SenderEndpoint.ReliableRPCBuffer.RingBuffer.Num(); ++SlotIdx)
		{
			const auto& Slot = SenderEndpoint.ReliableRPCBuffer.RingBuffer[SlotIdx];
			if (Slot.IsSet())
			{
				const TOptional<CrossServerRPCInfo>& TargetRef = SenderEndpoint.ReliableRPCBuffer.Counterpart[SlotIdx];
				check(TargetRef.IsSet());

				CrossServer::RPCKey RPCKey(EntityId, TargetRef.GetValue().RPCId);

				CrossServer::SentRPCEntry NewEntry;
				NewEntry.Target = RPCTarget(TargetRef.GetValue());
				NewEntry.SourceSlot = SlotIdx;

				SenderState.Mailbox.Add(RPCKey, NewEntry);
				SenderState.Alloc.Occupied[SlotIdx] = true;
			}
		}
		break;
	}
	case SpatialConstants::CROSSSERVER_RECEIVER_ACK_ENDPOINT_COMPONENT_ID:
	{
		CrossServerEndpointACK ReceiverACKEndpoint(Data->GetUnderlying());
		CrossServer::ReaderState& ReceiverACKState = CrossServerDataStore.FindChecked(EntityId).ReceiverACKState;

		uint32 numAcks = 0;
		for (int32 SlotIdx = 0; SlotIdx < ReceiverACKEndpoint.ACKArray.Num(); ++SlotIdx)
		{
			const TOptional<ACKItem>& ACK = ReceiverACKEndpoint.ACKArray[SlotIdx];
			if (ACK)
			{
				CrossServer::RPCSlots NewSlot;
				NewSlot.CounterpartEntity = ACK->Sender;
				NewSlot.ACKSlot = SlotIdx;

				ReceiverACKState.RPCSlots.Add(CrossServer::RPCKey(ACK->Sender, ACK->RPCId), NewSlot);
				ReceiverACKState.ACKAlloc.CommitSlot(SlotIdx);
			}
		}
		break;
	}
	default:
		checkNoEntry();
		break;
	}
}

void CrossServerRPCService::HandleRPC(const Worker_EntityId EntityId, const CrossServerEndpoint& Receiver)
{
	if (SubView->HasAuthority(EntityId, SpatialConstants::SERVER_AUTH_COMPONENT_SET_ID))
	{
		if (!CanExtractRPCDelegate.Execute(EntityId))
		{
			return;
		}
		ExtractCrossServerRPCs(EntityId, Receiver);
	}
}

bool CrossServerRPCService::IsCrossServerEndpoint(const Worker_ComponentId ComponentId)
{
	return ComponentId == SpatialConstants::CROSSSERVER_SENDER_ENDPOINT_COMPONENT_ID
		   || ComponentId == SpatialConstants::CROSSSERVER_SENDER_ACK_ENDPOINT_COMPONENT_ID
		   || ComponentId == SpatialConstants::CROSSSERVER_RECEIVER_ENDPOINT_COMPONENT_ID
		   || ComponentId == SpatialConstants::CROSSSERVER_RECEIVER_ACK_ENDPOINT_COMPONENT_ID;
}

void CrossServerRPCService::ExtractCrossServerRPCs(Worker_EntityId EndpointId, const CrossServerEndpoint& Receiver)
{
	// First, try to free ACK slots.
	CleanupACKsFor(EndpointId, Receiver);

	const RPCRingBuffer& Buffer = Receiver.ReliableRPCBuffer;

	CrossServerEndpoints& Endpoint = CrossServerDataStore.FindChecked(EndpointId);

	for (uint32 SlotIdx = 0; SlotIdx < RPCRingBufferUtils::GetRingBufferSize(ERPCType::CrossServer); ++SlotIdx)
	{
		const TOptional<RPCPayload>& Element = Buffer.RingBuffer[SlotIdx];
		if (Element.IsSet())
		{
			const TOptional<CrossServerRPCInfo>& Counterpart = Buffer.Counterpart[SlotIdx];
			if (ensure(Counterpart.IsSet()))
			{
				CrossServer::RPCKey RPCKey(Counterpart->Entity, Counterpart->RPCId);

				const bool bAlreadyQueued = Endpoint.ReceiverACKState.RPCSlots.Find(RPCKey) != nullptr;

				if (!bAlreadyQueued)
				{
					CrossServer::RPCSlots& NewSlots = Endpoint.ReceiverACKState.RPCSlots.Add(RPCKey);
					NewSlots.CounterpartSlot = SlotIdx;
					Endpoint.ReceiverSchedule.Add(RPCKey);
				}
			}
		}
	}

	while (!Endpoint.ReceiverSchedule.IsEmpty())
	{
		CrossServer::RPCKey RPC = Endpoint.ReceiverSchedule.Peek();
		CrossServer::RPCSlots& Slots = Endpoint.ReceiverACKState.RPCSlots.FindChecked(RPC);

		Endpoint.ReceiverSchedule.Extract();

		const RPCPayload& Payload = Buffer.RingBuffer[Slots.CounterpartSlot].GetValue();

		ExtractRPCCallback.Execute(FUnrealObjectRef(EndpointId, Payload.Offset), RPCSender(CrossServerRPCInfo(RPC.Get<0>(), RPC.Get<1>())),
								   Payload, Slots.CounterpartSlot);
	}
}

void CrossServerRPCService::WriteCrossServerACKFor(Worker_EntityId Receiver, const RPCSender& Sender)
{
	CrossServerEndpoints& Endpoint = CrossServerDataStore.FindChecked(Receiver);
	TOptional<uint32> ReservedSlot = Endpoint.ReceiverACKState.ACKAlloc.ReserveSlot();
	check(ReservedSlot.IsSet());
	uint32 SlotIdx = ReservedSlot.GetValue();

	ACKItem ACK;
	ACK.RPCId = Sender.RPCId;
	ACK.Sender = Sender.Entity;
	ACK.Result = static_cast<uint32>(CrossServer::Result::Success);

	EntityComponentId Pair(Receiver, SpatialConstants::CROSSSERVER_RECEIVER_ACK_ENDPOINT_COMPONENT_ID);

	Schema_ComponentUpdate* Update = RPCStore->GetOrCreateComponentUpdate(Pair);
	Schema_Object* UpdateObject = Schema_GetComponentUpdateFields(Update);

	Schema_Object* NewEntry = Schema_AddObject(UpdateObject, 1 + SlotIdx);
	ACK.WriteToSchema(NewEntry);

	CrossServer::RPCSlots& OccupiedSlot = Endpoint.ReceiverACKState.RPCSlots.FindChecked(CrossServer::RPCKey(Sender.Entity, Sender.RPCId));
	OccupiedSlot.ACKSlot = SlotIdx;
}

void CrossServerRPCService::UpdateSentRPCsACKs(Worker_EntityId SenderId, const CrossServerEndpointACK& ACKComponent)
{
	for (int32 SlotIdx = 0; SlotIdx < ACKComponent.ACKArray.Num(); ++SlotIdx)
	{
		if (ACKComponent.ACKArray[SlotIdx])
		{
			const ACKItem& ACK = ACKComponent.ACKArray[SlotIdx].GetValue();

			CrossServer::RPCKey RPCKey(ACK.Sender, ACK.RPCId);

			CrossServer::WriterState& SenderState = CrossServerDataStore.FindChecked(SenderId).SenderState;
			CrossServer::SentRPCEntry* SentRPC = SenderState.Mailbox.Find(RPCKey);
			if (SentRPC != nullptr)
			{
				SenderState.Alloc.FreeSlot(SentRPC->SourceSlot);
				SenderState.Mailbox.Remove(RPCKey);

				EntityComponentId Pair(ACK.Sender, SpatialConstants::CROSSSERVER_SENDER_ENDPOINT_COMPONENT_ID);
				RPCStore->GetOrCreateComponentUpdate(Pair);
			}
		}
	}
}

void CrossServerRPCService::CleanupACKsFor(Worker_EntityId EndpointId, const CrossServerEndpoint& Receiver)
{
	CrossServerEndpoints& Endpoint = CrossServerDataStore.FindChecked(EndpointId);
	CrossServer::ReaderState& State = Endpoint.ReceiverACKState;

	if (State.RPCSlots.Num() > 0)
	{
		CrossServer::ReadRPCMap ACKSToClear = State.RPCSlots;
		for (auto Iterator = ACKSToClear.CreateIterator(); Iterator; ++Iterator)
		{
			if (Iterator->Value.ACKSlot == -1)
			{
				Iterator.RemoveCurrent();
			}
		}

		if (ACKSToClear.Num() == 0)
		{
			return;
		}

		const RPCRingBuffer& Buffer = Receiver.ReliableRPCBuffer;

		for (uint32 Slot = 0; Slot < RPCRingBufferUtils::GetRingBufferSize(ERPCType::CrossServer); ++Slot)
		{
			const TOptional<RPCPayload>& Element = Buffer.RingBuffer[Slot];
			if (Element.IsSet())
			{
				const TOptional<CrossServerRPCInfo>& Counterpart = Buffer.Counterpart[Slot];
				Worker_EntityId CounterpartId = Counterpart.GetValue().Entity;
				uint64 RPCId = Counterpart.GetValue().RPCId;

				CrossServer::RPCKey RPCKey(CounterpartId, RPCId);
				ACKSToClear.Remove(RPCKey);
			}
		}

		EntityComponentId Pair(EndpointId, SpatialConstants::CROSSSERVER_RECEIVER_ACK_ENDPOINT_COMPONENT_ID);

		for (auto const& SlotToClear : ACKSToClear)
		{
			uint32 SlotIdx = SlotToClear.Value.ACKSlot;
			State.RPCSlots.Remove(SlotToClear.Key);

			RPCStore->GetOrCreateComponentUpdate(Pair);

			State.ACKAlloc.FreeSlot(SlotIdx);
		}
	}
}

void CrossServerRPCService::FlushPendingClearedFields(TPair<EntityComponentId, PendingUpdate>& UpdateToSend)
{
	if (UpdateToSend.Key.ComponentId == SpatialConstants::CROSSSERVER_SENDER_ENDPOINT_COMPONENT_ID)
	{
		CrossServer::WriterState& SenderState = CrossServerDataStore.FindChecked(UpdateToSend.Key.EntityId).SenderState;
		RPCRingBufferDescriptor Descriptor = RPCRingBufferUtils::GetRingBufferDescriptor(ERPCType::CrossServer);

		SenderState.Alloc.ForeachClearedSlot([&](uint32 ToClear) {
			uint32 Field = Descriptor.GetRingBufferElementFieldId(ERPCType::CrossServer, ToClear + 1);

			Schema_AddComponentUpdateClearedField(UpdateToSend.Value.Update, Field);
			Schema_AddComponentUpdateClearedField(UpdateToSend.Value.Update, Field + 1);
		});
	}

	if (UpdateToSend.Key.ComponentId == SpatialConstants::CROSSSERVER_RECEIVER_ACK_ENDPOINT_COMPONENT_ID)
	{
		CrossServer::SlotAlloc& SlotAlloc = CrossServerDataStore.FindChecked(UpdateToSend.Key.EntityId).ReceiverACKState.ACKAlloc;

		SlotAlloc.ForeachClearedSlot([&](uint32 ToClear) {
			Schema_AddComponentUpdateClearedField(UpdateToSend.Value.Update, 1 + ToClear);
		});
	}
}

} // namespace SpatialGDK
