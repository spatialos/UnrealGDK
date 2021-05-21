// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "LoadBalancing/PartitionManager.h"
#include "EngineClasses/SpatialVirtualWorkerTranslator.h"
#include "Interop/Connection/SpatialOSWorkerInterface.h"
#include "LoadBalancing/PartitionManager.h"
#include "Schema/ServerWorker.h"
#include "Schema/StandardLibrary.h"
#include "Utils/ComponentFactory.h"
#include "Utils/InterestFactory.h"

DEFINE_LOG_CATEGORY(LogSpatialPartitionManager);

namespace SpatialGDK
{
struct FPartitionInternalState
{
	Worker_PartitionId Id = 0;
	QueryConstraint LBConstraint;

	TOptional<Worker_RequestId> CreationRequest;

	void* UserData = nullptr;
	bool bAcked = false;
	bool bInterestDirty = true;
	bool bPartitionCreated = false;

	VirtualWorkerId Assignment;
	bool bAssignmentDirty = true;
};

struct FPartitionManager::Impl
{
	Impl(ViewCoordinator& Coordinator, SpatialVirtualWorkerTranslator& InTranslator, TUniquePtr<InterestFactory>&& InInterestF)
		: WorkerView(Coordinator.CreateSubView(SpatialConstants::SERVER_WORKER_COMPONENT_ID, SpatialGDK::FSubView::NoFilter,
											   SpatialGDK::FSubView::NoDispatcherCallbacks))
		, SystemWorkerView(Coordinator.CreateSubView(SpatialConstants::WORKER_COMPONENT_ID, SpatialGDK::FSubView::NoFilter,
													 SpatialGDK::FSubView::NoDispatcherCallbacks))
		, PartitionView(Coordinator.CreateSubView(SpatialConstants::PARTITION_ACK_COMPONENT_ID, SpatialGDK::FSubView::NoFilter,
												  SpatialGDK::FSubView::NoDispatcherCallbacks))
		, Translator(InTranslator)
		, InterestF(MoveTemp(InInterestF))
	{
	}

	void AdvanceView(SpatialOSWorkerInterface* Connection)
	{
		const TArray<Worker_Op>& Messages = Connection->GetWorkerMessages();

		for (const auto& Message : Messages)
		{
			switch (Message.op_type)
			{
			case WORKER_OP_TYPE_ENTITY_QUERY_RESPONSE:
			{
				const Worker_EntityQueryResponseOp& Op = Message.op.entity_query_response;
				if (WorkerTranslationRequest.IsSet() && Op.request_id == WorkerTranslationRequest.GetValue())
				{
					if (Op.status_code == WORKER_STATUS_CODE_SUCCESS)
					{
						for (uint32_t i = 0; i < Op.results[0].component_count; i++)
						{
							Worker_ComponentData Data = Op.results[0].components[i];
							if (Data.component_id == SpatialConstants::VIRTUAL_WORKER_TRANSLATION_COMPONENT_ID)
							{
								Schema_Object* ComponentObject = Schema_GetComponentDataFields(Data.schema_type);
								Translator.ApplyVirtualWorkerManagerData(ComponentObject);
								bTranslatorIsReady = true;
								for (int32 VirtualWorker = 0; VirtualWorker < ExpectedWorkers; ++VirtualWorker)
								{
									if (Translator.GetServerWorkerEntityForVirtualWorker(VirtualWorker + 1)
										== SpatialConstants::INVALID_ENTITY_ID)
									{
										bTranslatorIsReady = false;
									}
								}
							}
						}
					}
				}
				bTranslationQueryInFlight = false;
			}

			case WORKER_OP_TYPE_CREATE_ENTITY_RESPONSE:
			{
				const Worker_CreateEntityResponseOp& Op = Message.op.create_entity_response;
				if (Op.status_code != WORKER_STATUS_CODE_SUCCESS)
				{
					UE_LOG(LogSpatialPartitionManager, Error, TEXT("Partition entity creation failed: \"%s\". Entity: %lld."),
						   UTF8_TO_TCHAR(Op.message), Op.entity_id);
				}
				else
				{
					FPartitionHandle* Partition = PartitionCreationRequests.Find(Op.request_id);
					if (Partition && PartitionsMap.Contains(*Partition))
					{
						(*Partition)->State->CreationRequest.Reset();
						(*Partition)->State->bPartitionCreated = true;

						PartitionCreationRequests.Remove(Op.request_id);
						if (PartitionCreationRequests.Num() == 0)
						{
							bStrategyPartitionsCreated = true;
						}
					}
				}

				break;
			}
			case WORKER_OP_TYPE_RESERVE_ENTITY_IDS_RESPONSE:
			{
				const Worker_ReserveEntityIdsResponseOp& Op = Message.op.reserve_entity_ids_response;

				if (PartitionReserveRequest.IsSet() && PartitionReserveRequest.GetValue() == Op.request_id)
				{
					PartitionReserveRequest.Reset();
					if (Op.status_code != WORKER_STATUS_CODE_SUCCESS)
					{
						UE_LOG(LogSpatialPartitionManager, Error, TEXT("Reserve partition Id failed : %s"), UTF8_TO_TCHAR(Op.message));
					}
					else
					{
						FirstPartitionId = Op.first_entity_id;
					}
				}
				break;
			}
			case WORKER_OP_TYPE_COMMAND_RESPONSE:
			{
				const Worker_CommandResponseOp& Op = Message.op.command_response;
				if (StrategyWorkerRequest.IsSet() && Op.request_id == StrategyWorkerRequest.GetValue())
				{
					if (Op.status_code != WORKER_STATUS_CODE_SUCCESS)
					{
						UE_LOG(LogSpatialPartitionManager, Error, TEXT("Claim partition failed : %s"), UTF8_TO_TCHAR(Op.message));
					}
					StrategyWorkerRequest.Reset();
				}
			}
			break;
			}
		}

		for (const EntityDelta& Delta : PartitionView.GetViewDelta().EntityDeltas)
		{
			switch (Delta.Type)
			{
			case EntityDelta::UPDATE:
			{
				for (const auto& CompleteUpdate : Delta.ComponentsRefreshed)
				{
					if (CompleteUpdate.ComponentId == SpatialConstants::PARTITION_ACK_COMPONENT_ID)
					{
						Schema_Object* Object = Schema_GetComponentDataFields(CompleteUpdate.CompleteUpdate.Data);
						if (Schema_GetUint64(Object, 1) != 0)
						{
							SetPartitionReady(Delta.EntityId);
						}
					}
				}
				for (const auto& Update : Delta.ComponentUpdates)
				{
					if (Update.ComponentId == SpatialConstants::PARTITION_ACK_COMPONENT_ID)
					{
						Schema_Object* Object = Schema_GetComponentUpdateFields(Update.Update);
						if (Schema_GetUint64(Object, 1) != 0)
						{
							SetPartitionReady(Delta.EntityId);
						}
					}
				}
			}
			break;
			case EntityDelta::ADD:
			{
				const EntityViewElement& WorkerDesc = WorkerView.GetView().FindChecked(Delta.EntityId);
				for (const auto& Component : WorkerDesc.Components)
				{
					if (Component.GetComponentId() == SpatialConstants::PARTITION_ACK_COMPONENT_ID)
					{
						Schema_Object* Object = Schema_GetComponentDataFields(Component.GetUnderlying());
						if (Schema_GetUint64(Object, 1) != 0)
						{
							SetPartitionReady(Delta.EntityId);
						}
					}
				}
			}
			break;
			case EntityDelta::REMOVE:
			{
			}
			case EntityDelta::TEMPORARILY_REMOVED:
			{
			}
			break;
			default:
				break;
			}
		}

		for (const EntityDelta& Delta : WorkerView.GetViewDelta().EntityDeltas)
		{
			switch (Delta.Type)
			{
			case EntityDelta::UPDATE:
			{
				ServerWorker& ServerWorkerData = Workers.FindChecked(Delta.EntityId);
				bool bWasReady = ServerWorkerData.bReadyToBeginPlay;
				for (const auto& CompleteUpdate : Delta.ComponentsRefreshed)
				{
					if (CompleteUpdate.ComponentId == SpatialConstants::SERVER_WORKER_COMPONENT_ID)
					{
						ServerWorkerData = ServerWorker(CompleteUpdate.Data);
					}
				}
				for (const auto& Update : Delta.ComponentUpdates)
				{
					if (Update.ComponentId == SpatialConstants::SERVER_WORKER_COMPONENT_ID)
					{
						ServerWorkerData.ApplyComponentUpdate(Update.Update);
					}
				}
				if (!bWasReady && ServerWorkerData.bReadyToBeginPlay)
				{
					++NumWorkerReady;
				}
			}
			break;
			case EntityDelta::ADD:
			{
				const EntityViewElement& WorkerDesc = WorkerView.GetView().FindChecked(Delta.EntityId);
				for (const auto& Component : WorkerDesc.Components)
				{
					if (Component.GetComponentId() == SpatialConstants::SERVER_WORKER_COMPONENT_ID)
					{
						ServerWorker& ServerWorkerData = Workers.Add(Delta.EntityId, ServerWorker(Component.GetUnderlying()));
						if (ServerWorkerData.bReadyToBeginPlay)
						{
							NumWorkerReady++;
						}
					}
				}
			}
			break;
			case EntityDelta::REMOVE:
			{
			}
			case EntityDelta::TEMPORARILY_REMOVED:
			{
			}
			break;
			default:
				break;
			}
		}

		for (const EntityDelta& Delta : SystemWorkerView.GetViewDelta().EntityDeltas)
		{
			switch (Delta.Type)
			{
			case EntityDelta::ADD:
			{
				const EntityViewElement& WorkerDesc = WorkerView.GetView().FindChecked(Delta.EntityId);
				for (const auto& Component : WorkerDesc.Components)
				{
					if (Component.GetComponentId() == SpatialConstants::WORKER_COMPONENT_ID)
					{
						Worker_ComponentData temp;
						temp.component_id = SpatialConstants::WORKER_COMPONENT_ID;
						temp.schema_type = Component.GetUnderlying();
						SystemWorkers.Add(Delta.EntityId, Worker(temp));
					}
				}
			}
			break;
			case EntityDelta::REMOVE:
			{
				SystemWorkers.Remove(Delta.EntityId);
				// TODO : Disconnect
			}
			case EntityDelta::TEMPORARILY_REMOVED:
			{
			}
			break;
			default:
				break;
			}
		}
	}

	void Flush(SpatialOSWorkerInterface* Connection)
	{
		if (NumWorkerReady < ExpectedWorkers)
		{
			return;
		}

		if (!bTranslatorIsReady)
		{
			QueryTranslation(Connection);
			return;
		}

		for (int32_t WorkerId = 1; WorkerId <= ExpectedWorkers; ++WorkerId)
		{
			if (ConnectedWorkers.Num() <= (WorkerId - 1))
			{
				Worker_EntityId ServerWorkerEntity = Translator.GetServerWorkerEntityForVirtualWorker(WorkerId);
				const ServerWorker& ServerWorkerData = Workers.FindChecked(ServerWorkerEntity);
				Worker_EntityId SysEntityId = ServerWorkerData.SystemEntityId;
				const Worker& SystemWorkerData = SystemWorkers.FindChecked(SysEntityId);

				FLBWorker ConnectedWorker;
				ConnectedWorker.WorkerId = WorkerId;
				ConnectedWorker.WorkerType = FName(SystemWorkerData.WorkerType);

				ConnectedWorkers.Add(ConnectedWorker);
				ConnectedWorkersThisFrame.Add(ConnectedWorker);
			}
		}

		if (PartitionsMap.Num() < ExpectedWorkers)
		{
			return;
		}

		if (FirstPartitionId == 0)
		{
			return;
		}

		for (auto& PartitionEntry : PartitionsMap)
		{
			FPartitionInternalState& PartitionState = *PartitionEntry.Key->State;
			if (!PartitionState.bPartitionCreated && !PartitionState.CreationRequest)
			{
				PartitionState.Id = FirstPartitionId + (PartitionState.Assignment - 1);

				TArray<FWorkerComponentData> Components = CreatePartitionEntityComponents(
					TEXT("StrategyPartition"), PartitionState.Id, PartitionEntry.Value, PartitionState.LBConstraint);

				const Worker_RequestId RequestId =
					Connection->SendCreateEntityRequest(MoveTemp(Components), &PartitionState.Id, SpatialGDK::RETRY_UNTIL_COMPLETE);
				PartitionState.CreationRequest = RequestId;
				PartitionCreationRequests.Add(RequestId, PartitionEntry.Key);
			}
		}

		CheckPartitionDistributed(Connection);
		if (!bPartitionsDistributed)
		{
			return;
		}
	}

	void SetPartitionReady(Worker_EntityId EntityId)
	{
		for (auto& Partition : PartitionsMap)
		{
			FPartitionInternalState& PartitionState = *Partition.Key->State;
			if (PartitionState.Id == EntityId)
			{
				PartitionState.bAcked = true;
				return;
			}
		}
	}

	void CheckPartitionDistributed(SpatialOSWorkerInterface* Connection)
	{
		if (bPartitionsDistributed)
		{
			return;
		}

		if (!bStrategyPartitionsCreated)
		{
			return;
		}

		for (auto& Partition : PartitionsMap)
		{
			FPartitionInternalState& PartitionState = *Partition.Key->State;
			Worker_EntityId ServerWorkerEntity = Translator.GetServerWorkerEntityForVirtualWorker(PartitionState.Assignment);

			const ServerWorker& ServerWorkerData = Workers.FindChecked(ServerWorkerEntity);

			Worker_CommandRequest ClaimRequest = Worker::CreateClaimPartitionRequest(PartitionState.Id);
			PartitionState.CreationRequest =
				Connection->SendCommandRequest(ServerWorkerData.SystemEntityId, &ClaimRequest, SpatialGDK::RETRY_UNTIL_COMPLETE, {});
		}

		bPartitionsDistributed = true;
	}

	void QueryTranslation(SpatialOSWorkerInterface* Connection)
	{
		if (bTranslationQueryInFlight)
		{
			return;
		}

		// Build a constraint for the Virtual Worker Translation.
		Worker_ComponentConstraint TranslationComponentConstraint;
		TranslationComponentConstraint.component_id = SpatialConstants::VIRTUAL_WORKER_TRANSLATION_COMPONENT_ID;

		Worker_Constraint TranslationConstraint;
		TranslationConstraint.constraint_type = WORKER_CONSTRAINT_TYPE_COMPONENT;
		TranslationConstraint.constraint.component_constraint = TranslationComponentConstraint;

		Worker_EntityQuery TranslationQuery{};
		TranslationQuery.constraint = TranslationConstraint;

		WorkerTranslationRequest = Connection->SendEntityQueryRequest(&TranslationQuery, RETRY_UNTIL_COMPLETE);
		bTranslationQueryInFlight = true;
	}

	TArray<FWorkerComponentData> CreatePartitionEntityComponents(FString const& PartitionName, const Worker_EntityId EntityId, int32 Idx,
																 const QueryConstraint& LBConstraint)
	{
		AuthorityDelegationMap DelegationMap;
		DelegationMap.Add(SpatialConstants::GDK_KNOWN_ENTITY_AUTH_COMPONENT_SET_ID, SpatialConstants::INITIAL_STRATEGY_PARTITION_ENTITY_ID);
		DelegationMap.Add(SpatialConstants::PARTITION_WORKER_AUTH_COMPONENT_SET_ID, EntityId);

		TArray<FWorkerComponentData> Components;
		Components.Add(Position().CreateComponentData());
		Components.Add(Persistence().CreateComponentData());
		Components.Add(Metadata(FString::Format(TEXT("{0}:{1}"), { *PartitionName, Idx })).CreateComponentData());

		Components.Add(InterestF->CreatePartitionInterest(LBConstraint, false).CreateComponentData());

		Components.Add(AuthorityDelegation(DelegationMap).CreateComponentData());
		Components.Add(ComponentFactory::CreateEmptyComponentData(SpatialConstants::PARTITION_SHADOW_COMPONENT_ID));
		Components.Add(ComponentFactory::CreateEmptyComponentData(SpatialConstants::PARTITION_ACK_COMPONENT_ID));
		Components.Add(ComponentFactory::CreateEmptyComponentData(SpatialConstants::GDK_KNOWN_ENTITY_TAG_COMPONENT_ID));
		Components.Add(ComponentFactory::CreateEmptyComponentData(SpatialConstants::PARTITION_AUTH_TAG_COMPONENT_ID));

		return Components;
	}

	const FSubView& WorkerView;
	const FSubView& SystemWorkerView;
	const FSubView& PartitionView;
	SpatialVirtualWorkerTranslator& Translator;
	TUniquePtr<InterestFactory> InterestF;

	Worker_EntityId StrategyWorkerEntityId = 0;
	TOptional<Worker_RequestId> StrategyWorkerRequest;

	TMap<Worker_RequestId, FPartitionHandle> PartitionCreationRequests;
	TOptional<Worker_RequestId> PartitionReserveRequest = 0;

	TMap<FPartitionHandle, uint32> PartitionsMap;

	bool bStrategyPartitionsCreated = false;

	TMap<Worker_EntityId, ServerWorker> Workers;
	TMap<Worker_EntityId, Worker> SystemWorkers;
	int32 NumWorkerReady = 0;

	Worker_EntityId FirstPartitionId = 0;
	int32 ExpectedWorkers = 0;

	int32 PartitionCounter = 0;

	bool bPartitionsDistributed = false;

	TOptional<Worker_RequestId> WorkerTranslationRequest;
	bool bTranslationQueryInFlight = false;
	bool bTranslatorIsReady = false;

	TArray<FLBWorker> ConnectedWorkers;
	TArray<FLBWorker> ConnectedWorkersThisFrame;
	TArray<FLBWorker> DisconnectedWorkersThisFrame;
};

FPartitionManager::~FPartitionManager() = default;

FPartitionManager::FPartitionManager(Worker_EntityId InStrategyWorkerEntityId, ViewCoordinator& Coordinator,
									 SpatialVirtualWorkerTranslator& InTranslator, TUniquePtr<InterestFactory>&& InterestF)
	: m_Impl(MakeUnique<Impl>(Coordinator, InTranslator, MoveTemp(InterestF)))
{
	m_Impl->StrategyWorkerEntityId = InStrategyWorkerEntityId;
}

void FPartitionManager::Init(SpatialOSWorkerInterface* Connection, uint32 ExpectedWorkers)
{
	Worker_CommandRequest ClaimRequest = Worker::CreateClaimPartitionRequest(SpatialConstants::INITIAL_STRATEGY_PARTITION_ENTITY_ID);
	m_Impl->StrategyWorkerRequest =
		Connection->SendCommandRequest(m_Impl->StrategyWorkerEntityId, &ClaimRequest, SpatialGDK::RETRY_UNTIL_COMPLETE, {});
	m_Impl->ExpectedWorkers = ExpectedWorkers;

	m_Impl->PartitionReserveRequest = Connection->SendReserveEntityIdsRequest(ExpectedWorkers, RETRY_UNTIL_COMPLETE);
}

bool FPartitionManager::IsReady()
{
	return m_Impl->bPartitionsDistributed;
}

TOptional<Worker_PartitionId> FPartitionManager::GetPartitionId(FPartitionHandle Partition)
{
	if (!m_Impl->PartitionsMap.Contains(Partition))
	{
		return {};
	}
	FPartitionInternalState& PartitionState = *Partition->State;
	if (!PartitionState.bAcked)
	{
		return {};
	}
	return PartitionState.Id;
}

FPartitionHandle FPartitionManager::CreatePartition(void* UserData, const QueryConstraint& Interest)
{
	if (m_Impl->PartitionsMap.Num() >= m_Impl->ExpectedWorkers)
	{
		// That'll be for later. Right now, still mix implementations.
		checkNoEntry();
		return FPartitionHandle();
	}

	FPartitionHandle NewPartition = MakeShared<FPartitionDesc>();
	NewPartition->State = MakeUnique<FPartitionInternalState>();
	NewPartition->State->UserData = UserData;
	NewPartition->State->LBConstraint = Interest;

	// Right now, still matching creation ordering with assignment;
	NewPartition->State->Assignment = m_Impl->PartitionsMap.Num() + 1;

	m_Impl->PartitionsMap.Add(NewPartition, m_Impl->PartitionCounter++);

	return NewPartition;
}

void FPartitionManager::SetPartitionInterest(FPartitionHandle Partition, const QueryConstraint& NewInterest)
{
	if (!m_Impl->PartitionsMap.Contains(Partition))
	{
		return;
	}
	FPartitionInternalState& PartitionState = *Partition->State;
	PartitionState.LBConstraint = NewInterest;
	PartitionState.bInterestDirty = true;
}

void FPartitionManager::AssignPartitionTo(FPartitionHandle Partition, VirtualWorkerId Worker)
{
	if (!m_Impl->PartitionsMap.Contains(Partition))
	{
		return;
	}

	// Later
}

void FPartitionManager::SetPartitionMetadata(FPartitionHandle Partition)
{
	if (!m_Impl->PartitionsMap.Contains(Partition))
	{
		return;
	}
	// Later
}

void FPartitionManager::AdvanceView(SpatialOSWorkerInterface* Connection)
{
	m_Impl->AdvanceView(Connection);
}

void FPartitionManager::Flush(SpatialOSWorkerInterface* Connection)
{
	m_Impl->Flush(Connection);
}

TArray<FLBWorker> FPartitionManager::GetConnectedWorkers()
{
	TArray<FLBWorker> ConnectedWorkers;
	Swap(ConnectedWorkers, m_Impl->ConnectedWorkersThisFrame);
	return MoveTemp(ConnectedWorkers);
}

TArray<FLBWorker> FPartitionManager::GetDisconnectedWorkers()
{
	TArray<FLBWorker> DisconnectedWorkers;
	Swap(DisconnectedWorkers, m_Impl->DisconnectedWorkersThisFrame);
	return MoveTemp(DisconnectedWorkers);
}

} // namespace SpatialGDK
