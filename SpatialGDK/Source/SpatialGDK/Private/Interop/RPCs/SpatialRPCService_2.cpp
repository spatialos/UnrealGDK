// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "Interop/RPCs/SpatialRPCService_2.h"
#include "Interop/RPCs/SpatialRPCService_2_Queues.h"
#include "Interop/RPCs/SpatialRPCService_2_Writers.h"

#include "EngineClasses/SpatialNetDriver.h"

#include "SpatialGDKSettings.h"

DEFINE_LOG_CATEGORY(LogSpatialRPCService_2);

namespace SpatialGDK
{
SpatialRPCService_2::SpatialRPCService_2(const FSubView& InActorAuthSubView,
										 const FSubView& InActorNonAuthSubView
										 /*, USpatialLatencyTracer* InSpatialLatencyTracer, SpatialEventTracer* InEventTracer*/
										 ,
										 USpatialNetDriver* InNetDriver)
	: AuthSubView(&InActorAuthSubView)
	, ActorSubView(&InActorAuthSubView)
{
	IncomingRPCs.BindProcessingFunction(FProcessRPCDelegate::CreateRaw(this, &SpatialRPCService_2::ApplyRPC));

	const USpatialGDKSettings* Settings = GetDefault<USpatialGDKSettings>();

	if (InNetDriver->IsServer())
	{
		{
			auto RPCType = ERPCType::ClientReliable;
			auto RPCDesc = RPCRingBufferUtils::GetRingBufferDescriptor(RPCType);

			AddRPCQueue(ClientReliable, SpatialConstants::SERVER_AUTH_COMPONENT_SET_ID,
						MakeUnique<MonotonicRingBufferWithACKWriter>(RPCRingBufferUtils::GetRingBufferComponentId(RPCType),
																	 RPCDesc.LastSentRPCFieldId, RPCDesc.SchemaFieldStart,
																	 RPCDesc.RingBufferSize, RPCRingBufferUtils::GetAckComponentId(RPCType),
																	 RPCRingBufferUtils::GetAckFieldId(RPCType)),
						MakeUnique<RPCLocalOverflowQueue>());
		}

		{
			auto RPCType = ERPCType::ClientUnreliable;
			auto RPCDesc = RPCRingBufferUtils::GetRingBufferDescriptor(RPCType);

			AddRPCQueue(ClientUnreliable, SpatialConstants::SERVER_AUTH_COMPONENT_SET_ID,
						MakeUnique<MonotonicRingBufferWithACKWriter>(RPCRingBufferUtils::GetRingBufferComponentId(RPCType),
																	 RPCDesc.LastSentRPCFieldId, RPCDesc.SchemaFieldStart,
																	 RPCDesc.RingBufferSize, RPCRingBufferUtils::GetAckComponentId(RPCType),
																	 RPCRingBufferUtils::GetAckFieldId(RPCType)),
						MakeUnique<RPCFixedCapacityQueue>(RPCDesc.RingBufferSize));
		}

		{
			auto RPCType = ERPCType::NetMulticast;
			auto RPCDesc = RPCRingBufferUtils::GetRingBufferDescriptor(RPCType);

			AddRPCQueue(Multicast, SpatialConstants::SERVER_AUTH_COMPONENT_SET_ID,
						MakeUnique<MonotonicRingBufferWriter>(RPCRingBufferUtils::GetRingBufferComponentId(RPCType),
															  RPCDesc.LastSentRPCFieldId, RPCDesc.SchemaFieldStart, RPCDesc.RingBufferSize),
						MakeUnique<RPCFixedCapacityQueue>(RPCDesc.RingBufferSize));
		}
	}
	else
	{
		{
			auto RPCType = ERPCType::ServerReliable;
			auto RPCDesc = RPCRingBufferUtils::GetRingBufferDescriptor(RPCType);

			AddRPCQueue(ServerReliable, SpatialConstants::CLIENT_AUTH_COMPONENT_SET_ID,
						MakeUnique<MonotonicRingBufferWithACKWriter>(RPCRingBufferUtils::GetRingBufferComponentId(RPCType),
																	 RPCDesc.LastSentRPCFieldId, RPCDesc.SchemaFieldStart,
																	 RPCDesc.RingBufferSize, RPCRingBufferUtils::GetAckComponentId(RPCType),
																	 RPCRingBufferUtils::GetAckFieldId(RPCType)),
						MakeUnique<RPCLocalOverflowQueue>());
		}

		{
			auto RPCType = ERPCType::ServerUnreliable;
			auto RPCDesc = RPCRingBufferUtils::GetRingBufferDescriptor(RPCType);

			AddRPCQueue(ServerUnreliable, SpatialConstants::CLIENT_AUTH_COMPONENT_SET_ID,
						MakeUnique<MonotonicRingBufferWithACKWriter>(RPCRingBufferUtils::GetRingBufferComponentId(RPCType),
																	 RPCDesc.LastSentRPCFieldId, RPCDesc.SchemaFieldStart,
																	 RPCDesc.RingBufferSize, RPCRingBufferUtils::GetAckComponentId(RPCType),
																	 RPCRingBufferUtils::GetAckFieldId(RPCType)),
						MakeUnique<RPCFixedCapacityQueue>(RPCDesc.RingBufferSize));
		}

		{
			Worker_ComponentId MovementChannelComp = 0;
			Schema_FieldId RPCField = 1;
			Schema_FieldId CountField = 2;
			uint32_t BufferSize = 1;

			AddRPCQueue(DedicatedMovementQueue, SpatialConstants::CLIENT_AUTH_COMPONENT_SET_ID,
						MakeUnique<MonotonicRingBufferWriter>(MovementChannelComp, CountField, RPCField, BufferSize),
						MakeUnique<RPCMostRecentQueue>(BufferSize));
		}
	}
}

void SpatialRPCService_2::AddRPCQueue(uint32 QueueName, Worker_ComponentSetId Authority, TUniquePtr<RPCWriter> Writer,
									  TUniquePtr<RPCQueue> Queue)
{
	RPCDescription NewDesc;
	NewDesc.Authority = Authority;
	NewDesc.Writer = MoveTemp(Writer);
	NewDesc.Queue = MoveTemp(Queue);

	Queues.Add(QueueName, MoveTemp(NewDesc));
}

void SpatialRPCService_2::AdvanceView()
{
	const FSubViewDelta* ViewDeltas[] = { &AuthSubView->GetViewDelta(), &ActorSubView->GetViewDelta() };
	for (const FSubViewDelta* ViewDelta : ViewDeltas)
		for (const EntityDelta& Delta : ViewDelta->EntityDeltas)
		{
			switch (Delta.Type)
			{
			case EntityDelta::UPDATE:
			{
				for (const ComponentChange& Change : Delta.ComponentUpdates)
				{
					for (auto& QueueEntry : Queues)
					{
						QueueEntry.Value.Writer->OnUpdate(Delta.EntityId, Change);
					}
				}
				for (const ComponentChange& Change : Delta.ComponentsRefreshed)
				{
					for (auto& QueueEntry : Queues)
					{
						QueueEntry.Value.Writer->OnCompleteUpdate(Delta.EntityId, Change);
					}
				}

				for (const AuthorityChange& Change : Delta.AuthorityLost)
				{
					for (auto& QueueEntry : Queues)
					{
						if (QueueEntry.Value.Authority == Change.ComponentSetId)
						{
							QueueEntry.Value.Queue->OnAuthLost(Delta.EntityId);
							QueueEntry.Value.Writer->OnAuthLost(Delta.EntityId);
						}
					}
				}
				for (const AuthorityChange& Change : Delta.AuthorityLostTemporarily)
				{
					for (auto& QueueEntry : Queues)
					{
						if (QueueEntry.Value.Authority == Change.ComponentSetId)
						{
							QueueEntry.Value.Queue->OnAuthLost(Delta.EntityId);
							QueueEntry.Value.Writer->OnAuthLost(Delta.EntityId);
						}
					}
				}
				for (const AuthorityChange& Change : Delta.AuthorityGained)
				{
					for (auto& QueueEntry : Queues)
					{
						if (QueueEntry.Value.Authority == Change.ComponentSetId)
						{
							const EntityViewElement& ViewElement = AuthSubView->GetView().FindChecked(Delta.EntityId);
							QueueEntry.Value.Queue->OnAuthGained(Delta.EntityId, ViewElement);
							QueueEntry.Value.Writer->OnAuthGained(Delta.EntityId, ViewElement);
						}
					}
				}
				for (const AuthorityChange& Change : Delta.AuthorityLostTemporarily)
				{
					for (auto& QueueEntry : Queues)
					{
						if (QueueEntry.Value.Authority == Change.ComponentSetId)
						{
							const EntityViewElement& ViewElement = AuthSubView->GetView().FindChecked(Delta.EntityId);
							QueueEntry.Value.Queue->OnAuthGained(Delta.EntityId, ViewElement);
							QueueEntry.Value.Writer->OnAuthGained(Delta.EntityId, ViewElement);
						}
					}
				}

				break;
			}
			case EntityDelta::ADD:
			{
				const EntityViewElement& ViewElement = AuthSubView->GetView().FindChecked(Delta.EntityId);
				for (auto& QueueEntry : Queues)
				{
					if (ViewElement.Authority.Find(QueueEntry.Value.Authority))
					{
						QueueEntry.Value.Queue->OnAuthGained(Delta.EntityId, ViewElement);
						QueueEntry.Value.Writer->OnAuthGained(Delta.EntityId, ViewElement);
					}
				}
			}
			break;
			case EntityDelta::REMOVE:
				for (auto& QueueEntry : Queues)
				{
					QueueEntry.Value.Queue->OnAuthLost(Delta.EntityId);
					QueueEntry.Value.Writer->OnAuthLost(Delta.EntityId);
				}
				break;
			case EntityDelta::TEMPORARILY_REMOVED:
			{
				const EntityViewElement& ViewElement = AuthSubView->GetView().FindChecked(Delta.EntityId);
				for (auto& QueueEntry : Queues)
				{
					QueueEntry.Value.Queue->OnAuthLost(Delta.EntityId);
					QueueEntry.Value.Writer->OnAuthLost(Delta.EntityId);
					if (ViewElement.Authority.Find(QueueEntry.Value.Authority))
					{
						QueueEntry.Value.Queue->OnAuthGained(Delta.EntityId, ViewElement);
						QueueEntry.Value.Writer->OnAuthGained(Delta.EntityId, ViewElement);
					}
				}
			}
			break;
			default:
				break;
			}
		}
}

void SpatialRPCService_2::ProcessChanges(const float NetDriverTime) {}

void SpatialRPCService_2::ProcessIncomingRPCs() {}

void SpatialRPCService_2::ProcessOrQueueIncomingRPC(const FUnrealObjectRef& InTargetObjectRef, RPCPayload InPayload,
													TOptional<uint64> RPCIdForLinearEventTrace)
{
}

FRPCErrorInfo SpatialRPCService_2::ApplyRPC(const FPendingRPCParams& Params)
{
	return FRPCErrorInfo();
}

FRPCErrorInfo SpatialRPCService_2::ApplyRPCInternal(UObject* TargetObject, UFunction* Function, const FPendingRPCParams& PendingRPCParams)
{
	return FRPCErrorInfo();
}

void SpatialRPCService_2::PushRPC(uint32 QueueName, Worker_EntityId Entity, TArray<uint8> Data)
{
	RPCDescription& SelectedQueue = Queues.FindChecked(QueueName);
	SelectedQueue.Queue->Push(Entity, MoveTemp(Data));
}

TArray<SpatialRPCService_2::UpdateToSend> SpatialRPCService_2::GetRPCsAndAcksToSend()
{
	TArray<UpdateToSend> Updates;
	for (auto& QueueEntry : Queues)
	{
		RPCDescription& Queue = QueueEntry.Value;
		RPCQueue::FlushCallback CreateUpdate = [&](Worker_EntityId Entity, const TArray<uint8>* RPCData, uint32 NumRPC) {
			UpdateToSend Update;
			Update.EntityId = Entity;
			Update.Update.component_id = Queue.Writer->GetComponentToWriteTo();
			Update.Update.schema_type = Schema_CreateComponentUpdate();
			Schema_Object* Object = Schema_GetComponentUpdateFields(Update.Update.schema_type);
			for (uint32 i = 0; i < NumRPC; ++i)
			{
				Queue.Writer->Write(Entity, Object, RPCData[i].GetData(), RPCData[i].Num());
			}
			Updates.Add(Update);
		};
		Queue.Queue->FlushAll(CreateUpdate);
	}

	return Updates;
}

TArray<FWorkerComponentData> SpatialRPCService_2::GetRPCComponentsOnEntityCreation(Worker_EntityId EntityId)
{
	TArray<FWorkerComponentData> CreationData;
	for (auto& QueueEntry : Queues)
	{
		RPCDescription& Queue = QueueEntry.Value;
		RPCQueue::FlushCallback CreateData = [&](Worker_EntityId Entity, const TArray<uint8>* RPCData, uint32 NumRPC) {
			FWorkerComponentData Data;
			Data.component_id = Queue.Writer->GetComponentToWriteTo();
			Data.schema_type = Schema_CreateComponentData();
			Schema_Object* Object = Schema_GetComponentDataFields(Data.schema_type);
			for (uint32 i = 0; i < NumRPC; ++i)
			{
				Queue.Writer->Write(Entity, Object, RPCData[i].GetData(), RPCData[i].Num());
			}
			CreationData.Add(Data);
		};
		Queue.Queue->Flush(EntityId, CreateData);
	}
	return CreationData;
}

void SpatialRPCService_2::ClearPendingRPCs(Worker_EntityId EntityId) {}

} // namespace SpatialGDK
