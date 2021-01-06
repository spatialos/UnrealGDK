#include "Interop/SpatialRoutingSystem.h"
#include "Interop/Connection/SpatialOSWorkerInterface.h"
#include "Schema/ServerWorker.h"
#include "Utils/InterestFactory.h"

DEFINE_LOG_CATEGORY(LogSpatialRoutingSystem);

namespace SpatialGDK
{
void SpatialRoutingSystem::ProcessUpdate(Worker_EntityId Entity, const ComponentChange& Change, RoutingComponents& Components)
{
	switch (Change.ComponentId)
	{
	case SpatialConstants::CROSSSERVER_SENDER_ENDPOINT_COMPONENT_ID:
		switch (Change.Type)
		{
		case ComponentChange::COMPLETE_UPDATE:
		{
			Components.Sender.Emplace(CrossServerEndpoint(Change.CompleteUpdate.Data));
			break;
		}
		case ComponentChange::UPDATE:
		{
			Components.Sender->ApplyComponentUpdate(Change.Update);
			break;
		}
		}
		OnSenderChanged(Entity, Components);
		break;
	case SpatialConstants::CROSSSERVER_SENDER_ACK_ENDPOINT_COMPONENT_ID:
	case SpatialConstants::CROSSSERVER_RECEIVER_ENDPOINT_COMPONENT_ID:
		check(Change.Type == ComponentChange::COMPLETE_UPDATE);
		break;
	case SpatialConstants::CROSSSERVER_RECEIVER_ACK_ENDPOINT_COMPONENT_ID:
		switch (Change.Type)
		{
		case ComponentChange::COMPLETE_UPDATE:
		{
			Components.ReceiverACK.Emplace(CrossServerEndpointACK(Change.CompleteUpdate.Data));
			break;
		}

		case ComponentChange::UPDATE:
		{
			Components.ReceiverACK->ApplyComponentUpdate(Change.Update);
			break;
		}
		}
		OnReceiverACKChanged(Entity, Components);
		break;
	default:
		checkNoEntry();
		break;
	}
}

void SpatialRoutingSystem::OnSenderChanged(Worker_EntityId SenderId, RoutingComponents& Components)
{
	TMap<CrossServer::RPCKey, Worker_EntityId> ReceiverAbsent;
	CrossServer::ReadRPCMap SlotsToClear = Components.SenderACKState.RPCSlots;

	const RPCRingBuffer& Buffer = Components.Sender->ReliableRPCBuffer;

	// Scan the buffer for freed slots and new RPCs to schedule
	for (uint32 SlotIdx = 0; SlotIdx < RPCRingBufferUtils::GetRingBufferSize(ERPCType::CrossServer); ++SlotIdx)
	{
		const TOptional<RPCPayload>& Element = Buffer.RingBuffer[SlotIdx];
		if (Element.IsSet())
		{
			const TOptional<CrossServerRPCInfo>& Counterpart = Buffer.Counterpart[SlotIdx];

			Worker_EntityId Receiver = Counterpart->Entity;
			uint64 RPCId = Counterpart->RPCId;
			CrossServer::RPCKey RPCKey(SenderId, RPCId);
			SlotsToClear.Remove(RPCKey);

			if (RoutingComponents* ReceiverComps = RoutingWorkerView.Find(Receiver))
			{
				if (ReceiverComps->ReceiverState.Mailbox.Find(RPCKey) == nullptr)
				{
					CrossServer::SentRPCEntry Entry;
					Entry.Target = RPCTarget(Counterpart.GetValue());
					Entry.SourceSlot = SlotIdx;

					ReceiverComps->ReceiverState.Mailbox.Add(RPCKey, Entry);
					ReceiverComps->ReceiverSchedule.Add(RPCKey);
					ReceiversToInspect.Add(Receiver);
				}
			}
			else
			{
				ReceiverAbsent.Add(RPCKey, Receiver);
			}
		}
	}

	for (auto const& SlotToClear : SlotsToClear)
	{
		const CrossServer::RPCSlots& Slots = SlotToClear.Value;
		{
			EntityComponentId SenderPair(SenderId, SpatialConstants::CROSSSERVER_SENDER_ACK_ENDPOINT_COMPONENT_ID);
			Components.SenderACKState.ACKAlloc.FreeSlot(Slots.ACKSlot);

			GetOrCreateComponentUpdate(SenderPair);
		}

		if (RoutingComponents* ReceiverComps = RoutingWorkerView.Find(Slots.CounterpartEntity))
		{
			ClearReceiverSlot(Slots.CounterpartEntity, SlotToClear.Key, *ReceiverComps);
		}

		Components.SenderACKState.RPCSlots.Remove(SlotToClear.Key);
	}

	// Absent receiver's ACK are written after freeing slots, to ensure capacity is available.
	for (auto RPC : ReceiverAbsent)
	{
		auto& Slots = Components.SenderACKState.RPCSlots.FindOrAdd(RPC.Key);
		if (Slots.ACKSlot < 0)
		{
			Slots.CounterpartEntity = RPC.Value;
			WriteACKToSender(RPC.Key, Components, CrossServer::Result::TargetUnknown);
		}
	}
}

void SpatialRoutingSystem::ClearReceiverSlot(Worker_EntityId Receiver, CrossServer::RPCKey RPCKey, RoutingComponents& ReceiverComponents)
{
	CrossServer::SentRPCEntry* SentRPC = ReceiverComponents.ReceiverState.Mailbox.Find(RPCKey);
	check(SentRPC != nullptr);

	EntityComponentId ReceiverPair(Receiver, SpatialConstants::CROSSSERVER_RECEIVER_ENDPOINT_COMPONENT_ID);

	check(SentRPC->DestinationSlot.IsSet());
	uint32 SlotIdx = SentRPC->DestinationSlot.GetValue();

	ReceiverComponents.ReceiverState.Mailbox.Remove(RPCKey);
	ReceiverComponents.ReceiverState.Alloc.FreeSlot(SlotIdx);

	GetOrCreateComponentUpdate(ReceiverPair);
	if (!ReceiverComponents.ReceiverSchedule.IsEmpty())
	{
		ReceiversToInspect.Add(ReceiverPair.Get<0>());
	}
}

void SpatialRoutingSystem::TransferRPCsToReceiver(Worker_EntityId ReceiverId, RoutingComponents& Components)
{
	if (RoutingComponents* ReceiverComps = RoutingWorkerView.Find(ReceiverId))
	{
		while (!ReceiverComps->ReceiverSchedule.IsEmpty())
		{
			TOptional<uint32> FreeSlot = ReceiverComps->ReceiverState.Alloc.PeekFreeSlot();
			if (!FreeSlot)
			{
				return;
			}
			CrossServer::RPCKey RPCToSend = ReceiverComps->ReceiverSchedule.Extract();
			CrossServer::SentRPCEntry& SentRPC = ReceiverComps->ReceiverState.Mailbox.FindChecked(RPCToSend);

			check(!SentRPC.DestinationSlot.IsSet());

			Worker_EntityId SenderId = RPCToSend.Get<0>();
			uint64 RPCId = RPCToSend.Get<1>();

			RoutingComponents* SenderComps = RoutingWorkerView.Find(SenderId);

			if (!SenderComps)
			{
				ReceiverComps->ReceiverState.Mailbox.Remove(RPCToSend);
				// Sender disappeared before we could deliver the RPC.
				// This should eventually become impossible by tombstoning actors instead of erasing their entities.
				continue;
			}

			const TOptional<RPCPayload>& Element = SenderComps->Sender->ReliableRPCBuffer.RingBuffer[SentRPC.SourceSlot];
			check(Element.IsSet());

			Schema_ComponentUpdate* ReceiverUpdate =
				GetOrCreateComponentUpdate(EntityComponentId(ReceiverId, SpatialConstants::CROSSSERVER_RECEIVER_ENDPOINT_COMPONENT_ID));
			Schema_Object* EndpointObject = Schema_GetComponentUpdateFields(ReceiverUpdate);

			const uint32 SlotIdx = FreeSlot.GetValue();
			CrossServer::WritePayloadAndCounterpart(EndpointObject, Element.GetValue(), CrossServerRPCInfo(SenderId, RPCId), SlotIdx);

			SentRPC.DestinationSlot = SlotIdx;
			ReceiverComps->ReceiverState.Alloc.CommitSlot(SlotIdx);

			// Remember which slot should be freed when the sender removes its entry.
			CrossServer::RPCSlots NewSlot;
			NewSlot.CounterpartEntity = ReceiverId;
			NewSlot.CounterpartSlot = SlotIdx;

			SenderComps->SenderACKState.RPCSlots.Add(RPCToSend, NewSlot);
		}
	}
}

void SpatialRoutingSystem::WriteACKToSender(CrossServer::RPCKey RPCKey, RoutingComponents& SenderComponents, CrossServer::Result Result)
{
	CrossServer::RPCSlots* Slots = SenderComponents.SenderACKState.RPCSlots.Find(RPCKey);
	check(Slots != nullptr);

	// Check if the slot has already been taken care of.
	if (Slots->ACKSlot < 0)
	{
		if (TOptional<uint32_t> ReservedSlot = SenderComponents.SenderACKState.ACKAlloc.ReserveSlot())
		{
			Slots->ACKSlot = ReservedSlot.GetValue();
			EntityComponentId SenderPair(RPCKey.Get<0>(), SpatialConstants::CROSSSERVER_SENDER_ACK_ENDPOINT_COMPONENT_ID);
			Schema_ComponentUpdate* Update = GetOrCreateComponentUpdate(SenderPair);
			Schema_Object* UpdateObject = Schema_GetComponentUpdateFields(Update);

			ACKItem SenderACK;
			SenderACK.Sender = RPCKey.Get<0>();
			SenderACK.RPCId = RPCKey.Get<1>();
			SenderACK.Result = static_cast<uint32>(Result);

			Schema_Object* NewEntry = Schema_AddObject(UpdateObject, 1 + Slots->ACKSlot);
			SenderACK.WriteToSchema(NewEntry);
		}
		else
		{
			checkNoEntry();
			// Out of free slots, should not be possible.
			UE_LOG(LogTemp, Log, TEXT("Out of sender ACK slot"));
		}
	}
}

void SpatialRoutingSystem::OnReceiverACKChanged(Worker_EntityId EntityId, RoutingComponents& Components)
{
	CrossServerEndpointACK& ReceiverACK = Components.ReceiverACK.GetValue();
	for (int32 SlotIdx = 0; SlotIdx < ReceiverACK.ACKArray.Num(); ++SlotIdx)
	{
		if (ReceiverACK.ACKArray[SlotIdx])
		{
			const ACKItem& ReceiverACKItem = ReceiverACK.ACKArray[SlotIdx].GetValue();

			CrossServer::RPCKey RPCKey(ReceiverACKItem.Sender, ReceiverACKItem.RPCId);
			CrossServer::SentRPCEntry* SentRPC = Components.ReceiverState.Mailbox.Find(RPCKey);
			if (SentRPC == nullptr)
			{
				continue;
			}

			RoutingComponents* SenderComponents = RoutingWorkerView.Find(ReceiverACKItem.Sender);
			if (SenderComponents != nullptr)
			{
				WriteACKToSender(RPCKey, *SenderComponents, CrossServer::Result::Success);
			}
			else
			{
				// Sender disappeared, clear Receiver slot.
				ClearReceiverSlot(EntityId, RPCKey, Components);
			}
		}
	}
}

Schema_ComponentUpdate* SpatialRoutingSystem::GetOrCreateComponentUpdate(
	TPair<Worker_EntityId_Key, Worker_ComponentId> EntityComponentIdPair)
{
	check(EntityComponentIdPair.Key != 0);
	Schema_ComponentUpdate** ComponentUpdatePtr = PendingComponentUpdatesToSend.Find(EntityComponentIdPair);
	if (ComponentUpdatePtr == nullptr)
	{
		ComponentUpdatePtr = &PendingComponentUpdatesToSend.Add(EntityComponentIdPair, Schema_CreateComponentUpdate());
	}
	return *ComponentUpdatePtr;
}

void SpatialRoutingSystem::Advance(SpatialOSWorkerInterface* Connection)
{
	// Messages are inspected to take care of the Worker entity creation.
	const TArray<Worker_Op>& Messages = Connection->GetWorkerMessages();
	for (const auto& Message : Messages)
	{
		switch (Message.op_type)
		{
		case WORKER_OP_TYPE_RESERVE_ENTITY_IDS_RESPONSE:
		{
			const Worker_ReserveEntityIdsResponseOp& Op = Message.op.reserve_entity_ids_response;
			if (Op.request_id == RoutingWorkerRequest)
			{
				if (Op.first_entity_id == SpatialConstants::INVALID_ENTITY_ID)
				{
					UE_LOG(LogSpatialRoutingSystem, Error, TEXT("Reserve entity failed : %s"), UTF8_TO_TCHAR(Op.message));
					RoutingWorkerRequest = 0;
				}
				else
				{
					RoutingPartition = Message.op.reserve_entity_ids_response.first_entity_id;
					CreateRoutingPartition(Connection);
				}
			}
			break;
		}
		case WORKER_OP_TYPE_CREATE_ENTITY_RESPONSE:
		{
			const Worker_CreateEntityResponseOp& Op = Message.op.create_entity_response;
			if (Op.request_id == RoutingWorkerRequest)
			{
				if (Op.entity_id == SpatialConstants::INVALID_ENTITY_ID)
				{
					UE_LOG(LogSpatialRoutingSystem, Error, TEXT("Create entity failed : %s"), UTF8_TO_TCHAR(Op.message));
				}
				RoutingWorkerRequest = 0;

				Worker_CommandRequest ClaimRequest = Worker::CreateClaimPartitionRequest(RoutingPartition);
				RoutingWorkerRequest =
					Connection->SendCommandRequest(RoutingWorkerSystemEntityId, &ClaimRequest, SpatialGDK::RETRY_UNTIL_COMPLETE, {});
			}
			break;
		}
		case WORKER_OP_TYPE_COMMAND_RESPONSE:
		{
			const Worker_CommandResponseOp& Op = Message.op.command_response;
			if (Op.request_id == RoutingWorkerRequest)
			{
				if (Op.status_code != WORKER_STATUS_CODE_SUCCESS)
				{
					UE_LOG(LogSpatialRoutingSystem, Error, TEXT("Claim partition failed : %s"), UTF8_TO_TCHAR(Op.message));
				}
				RoutingWorkerRequest = 0;
			}
		}
		break;
		}
	}

	const FSubViewDelta& SubViewDelta = SubView.GetViewDelta();
	for (const EntityDelta& Delta : SubViewDelta.EntityDeltas)
	{
		switch (Delta.Type)
		{
		case EntityDelta::UPDATE:
		{
			RoutingComponents& Components = RoutingWorkerView.FindChecked(Delta.EntityId);
			for (const ComponentChange& Change : Delta.ComponentUpdates)
			{
				ProcessUpdate(Delta.EntityId, Change, Components);
			}

			for (const ComponentChange& Change : Delta.ComponentsRefreshed)
			{
				ProcessUpdate(Delta.EntityId, Change, Components);
			}
		}
		break;
		case EntityDelta::ADD:
		{
			const EntityViewElement& EntityView = SubView.GetView().FindChecked(Delta.EntityId);
			RoutingComponents& Components = RoutingWorkerView.Add(Delta.EntityId);

			for (const auto& ComponentDesc : EntityView.Components)
			{
				switch (ComponentDesc.GetComponentId())
				{
				case SpatialConstants::CROSSSERVER_SENDER_ENDPOINT_COMPONENT_ID:
					Components.Sender = CrossServerEndpoint(ComponentDesc.GetUnderlying());
					// TODO : Should inspect the component if we were reloading a snapshot
					break;
				case SpatialConstants::CROSSSERVER_SENDER_ACK_ENDPOINT_COMPONENT_ID:
				{
					CrossServerEndpointACK TempView(ComponentDesc.GetUnderlying());
					for (int32 SlotIdx = 0; SlotIdx < TempView.ACKArray.Num(); ++SlotIdx)
					{
						const TOptional<ACKItem>& Slot = TempView.ACKArray[SlotIdx];
						if (Slot.IsSet())
						{
							CrossServer::RPCSlots& Slots =
								Components.SenderACKState.RPCSlots.FindOrAdd(CrossServer::RPCKey(Slot->Sender, Slot->RPCId));
							Slots.ACKSlot = SlotIdx;
							Components.SenderACKState.ACKAlloc.CommitSlot(SlotIdx);
						}
					}
				}
				break;
				case SpatialConstants::CROSSSERVER_RECEIVER_ENDPOINT_COMPONENT_ID:
				{
					CrossServerEndpoint TempView(ComponentDesc.GetUnderlying());
					for (int32 SlotIdx = 0; SlotIdx < TempView.ReliableRPCBuffer.RingBuffer.Num(); ++SlotIdx)
					{
						const auto& Slot = TempView.ReliableRPCBuffer.RingBuffer[SlotIdx];
						if (Slot.IsSet())
						{
							const TOptional<CrossServerRPCInfo>& SenderBackRef = TempView.ReliableRPCBuffer.Counterpart[SlotIdx];
							check(SenderBackRef.IsSet());

							CrossServer::RPCKey RPCKey(SenderBackRef->Entity, SenderBackRef->RPCId);
							CrossServer::SentRPCEntry NewEntry;
							NewEntry.DestinationSlot = SlotIdx;
							NewEntry.Target = RPCTarget(SenderBackRef.GetValue());

							Components.ReceiverState.Mailbox.Add(RPCKey, NewEntry);
							Components.ReceiverState.Alloc.CommitSlot(SlotIdx);

							RoutingComponents& SenderComponents = RoutingWorkerView.FindOrAdd(NewEntry.Target.Entity);
							// If we were reloading a snapshot, have to check that the sender still exists before just creating a new entry.
							CrossServer::RPCSlots& Slots = SenderComponents.SenderACKState.RPCSlots.FindOrAdd(RPCKey);
							Slots.CounterpartEntity = Delta.EntityId;
							Slots.CounterpartSlot = SlotIdx;
						}
					}
				}
				break;
				case SpatialConstants::CROSSSERVER_RECEIVER_ACK_ENDPOINT_COMPONENT_ID:
					Components.ReceiverACK = CrossServerEndpointACK(ComponentDesc.GetUnderlying());
					// TODO : Should inspect the component if we were reloading a snapshot
					break;
				}
			}
		}
		break;
		case EntityDelta::REMOVE:
		case EntityDelta::TEMPORARILY_REMOVED:
		{
			ReceiversToInspect.Remove(Delta.EntityId);
			PendingComponentUpdatesToSend.Remove(
				EntityComponentId(Delta.EntityId, SpatialConstants::CROSSSERVER_RECEIVER_ENDPOINT_COMPONENT_ID));
			PendingComponentUpdatesToSend.Remove(
				EntityComponentId(Delta.EntityId, SpatialConstants::CROSSSERVER_SENDER_ACK_ENDPOINT_COMPONENT_ID));

			if (RoutingComponents* Components = RoutingWorkerView.Find(Delta.EntityId))
			{
				for (auto MailboxItem : Components->ReceiverState.Mailbox)
				{
					CrossServer::SentRPCEntry& Entry = MailboxItem.Value;
					if (Entry.DestinationSlot)
					{
						RoutingComponents* SenderComponents = RoutingWorkerView.Find(MailboxItem.Key.Get<0>());
						if (SenderComponents != nullptr)
						{
							WriteACKToSender(MailboxItem.Key, *SenderComponents, CrossServer::Result::TargetDestroyed);
						}
					}
				}

				for (auto Slots : Components->SenderACKState.RPCSlots)
				{
					Worker_EntityId Receiver = Slots.Value.CounterpartEntity;
					if (Receiver != SpatialConstants::INVALID_ENTITY_ID && Slots.Value.ACKSlot != -1)
					{
						// The receiver would be waiting for an update from the sender.
						RoutingComponents* ReceiverComponents = RoutingWorkerView.Find(Receiver);
						if (ReceiverComponents)
						{
							ClearReceiverSlot(Receiver, Slots.Key, *ReceiverComponents);
						}
					}
				}

				RoutingWorkerView.Remove(Delta.EntityId);
			}
		}
		break;
		default:
			break;
		}
	}

	for (auto Receiver : ReceiversToInspect)
	{
		TransferRPCsToReceiver(Receiver, RoutingWorkerView.FindChecked(Receiver));
	}
	ReceiversToInspect.Empty();
}

void SpatialRoutingSystem::Flush(SpatialOSWorkerInterface* Connection)
{
	for (auto& Entry : PendingComponentUpdatesToSend)
	{
		Worker_EntityId Entity = Entry.Key.Get<0>();
		Worker_ComponentId CompId = Entry.Key.Get<1>();

		if (RoutingComponents* Components = RoutingWorkerView.Find(Entity))
		{
			if (CompId == SpatialConstants::CROSSSERVER_RECEIVER_ENDPOINT_COMPONENT_ID)
			{
				RPCRingBufferDescriptor Descriptor = RPCRingBufferUtils::GetRingBufferDescriptor(ERPCType::CrossServer);

				Components->ReceiverState.Alloc.ForeachClearedSlot([&](uint32_t ToClear) {
					uint32 Field = Descriptor.GetRingBufferElementFieldId(ERPCType::CrossServer, ToClear + 1);

					Schema_AddComponentUpdateClearedField(Entry.Value, Field);
					Schema_AddComponentUpdateClearedField(Entry.Value, Field + 1);
				});
			}

			if (CompId == SpatialConstants::CROSSSERVER_SENDER_ACK_ENDPOINT_COMPONENT_ID)
			{
				Components->SenderACKState.ACKAlloc.ForeachClearedSlot([&](uint32_t ToClear) {
					Schema_AddComponentUpdateClearedField(Entry.Value, ToClear + 1);
				});
			}
		}
		FWorkerComponentUpdate Update;
		Update.component_id = CompId;
		Update.schema_type = Entry.Value;
		Connection->SendComponentUpdate(Entity, &Update);
	}
	PendingComponentUpdatesToSend.Empty();
}

void SpatialRoutingSystem::Init(SpatialOSWorkerInterface* Connection)
{
	RoutingWorkerRequest = Connection->SendReserveEntityIdsRequest(1, SpatialGDK::RETRY_UNTIL_COMPLETE);
}

void SpatialRoutingSystem::CreateRoutingPartition(SpatialOSWorkerInterface* Connection)
{
	AuthorityDelegationMap Map;
	Map.Add(SpatialConstants::GDK_KNOWN_ENTITY_AUTH_COMPONENT_SET_ID, RoutingPartition);

	TArray<FWorkerComponentData> Components;
	Components.Add(Position().CreatePositionData());
	Components.Add(Metadata(FString(TEXT("RoutingPartition"))).CreateMetadataData());
	Components.Add(AuthorityDelegation(Map).CreateAuthorityDelegationData());
	Components.Add(InterestFactory::CreateRoutingWorkerInterest().CreateInterestData());

	RoutingWorkerRequest = Connection->SendCreateEntityRequest(Components, &RoutingPartition, SpatialGDK::RETRY_UNTIL_COMPLETE);
}

void SpatialRoutingSystem::Destroy(SpatialOSWorkerInterface* Connection)
{
	Connection->SendDeleteEntityRequest(RoutingPartition, SpatialGDK::RETRY_UNTIL_COMPLETE);
}

} // namespace SpatialGDK
