// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "LoadBalancing/PartitionManager.h"
#include "EngineClasses/SpatialVirtualWorkerTranslator.h"
#include "LoadBalancing/LBDataStorage.h"
#include "LoadBalancing/PartitionManager.h"
#include "Schema/ServerWorker.h"
#include "Schema/StandardLibrary.h"
#include "SpatialView/SpatialOSWorker.h"
#include "Utils/ComponentFactory.h"
#include "Utils/InterestFactory.h"

#include "Algo/Find.h"

DEFINE_LOG_CATEGORY(LogSpatialPartitionManager);

namespace SpatialGDK
{
struct FPartitionInternalState
{
	Worker_PartitionId Id = 0;
	QueryConstraint LBConstraint;

	TOptional<Worker_RequestId> CreationRequest;
	TOptional<Worker_RequestId> AssignmentRequest;

	FString DisplayName;
	void* UserData = nullptr;
	bool bAcked = false;
	bool bInterestDirty = true;
	bool bPartitionCreated = false;

	FLBWorkerHandle UserAssignment;
	FLBWorkerHandle RequestedAssignment;
	FLBWorkerHandle CurrentAssignment;
};

struct FLBWorkerInternalState
{
	Worker_EntityId SystemWorkerId;
	Worker_EntityId ServerWorkerId;

	FName WorkerType;
	FString FullWorkerName;
};

FLBWorkerDesc::FLBWorkerDesc(FName InWorkerType)
	: WorkerType(InWorkerType)
{
}
FLBWorkerDesc::~FLBWorkerDesc() = default;

FPartitionDesc::~FPartitionDesc() = default;

namespace
{
CommandRequest CreateClaimPartitionRequest(Worker_PartitionId Partition)
{
	Worker_CommandRequest ClaimRequestData = Worker::CreateClaimPartitionRequest(Partition);
	return CommandRequest(OwningCommandRequestPtr(ClaimRequestData.schema_type), ClaimRequestData.component_id,
						  ClaimRequestData.command_index);
}
} // namespace

struct FPartitionManager::Impl
{
	Impl(ViewCoordinator& Coordinator, TUniquePtr<InterestFactory>&& InInterestF)
		: WorkerView(Coordinator.CreateSubView(SpatialConstants::SERVER_WORKER_COMPONENT_ID, SpatialGDK::FSubView::NoFilter,
											   SpatialGDK::FSubView::NoDispatcherCallbacks))
		, SystemWorkerView(Coordinator.CreateSubView(SpatialConstants::WORKER_COMPONENT_ID, SpatialGDK::FSubView::NoFilter,
													 SpatialGDK::FSubView::NoDispatcherCallbacks))
		, PartitionView(Coordinator.CreateSubView(SpatialConstants::PARTITION_ACK_COMPONENT_ID, SpatialGDK::FSubView::NoFilter,
												  SpatialGDK::FSubView::NoDispatcherCallbacks))
		, InterestF(MoveTemp(InInterestF))
		, WorkersDispatcher(WorkerView)
		, SystemWorkersDispatcher(SystemWorkerView)
	{
		WorkersDispatcher.DataStorages.Add(&WorkersData);
		SystemWorkersDispatcher.DataStorages.Add(&SystemWorkersData);
	}

	void AdvanceView(ISpatialOSWorker& Connection)
	{
		const TArray<Worker_Op>& Messages = Connection.GetWorkerMessages();

		for (const auto& Message : Messages)
		{
			switch (Message.op_type)
			{
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
					if (Partition && Partitions.Contains(*Partition))
					{
						(*Partition)->State->CreationRequest.Reset();
						(*Partition)->State->bPartitionCreated = true;

						PartitionCreationRequests.Remove(Op.request_id);
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
						CurPartitionId = FirstPartitionId;
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
						UE_LOG(LogSpatialPartitionManager, Error, TEXT("Claim Strategy partition failed : %s"), UTF8_TO_TCHAR(Op.message));
					}
					StrategyWorkerRequest.Reset();
				}
				for (auto Partition : Partitions)
				{
					FPartitionInternalState& State = *Partition->State;
					if (State.AssignmentRequest.IsSet() && State.AssignmentRequest.GetValue() == Op.request_id)
					{
						if (Op.status_code != WORKER_STATUS_CODE_SUCCESS)
						{
							UE_LOG(LogSpatialPartitionManager, Error, TEXT("Claim partition %llu failed : %s"), State.Id,
								   UTF8_TO_TCHAR(Op.message));
						}
						State.CurrentAssignment = State.RequestedAssignment;
						State.AssignmentRequest.Reset();
					}
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
				const EntityViewElement& PartitionElement = PartitionView.GetView().FindChecked(Delta.EntityId);
				for (const auto& Component : PartitionElement.Components)
				{
					if (Component.GetComponentId() == SpatialConstants::PARTITION_ACK_COMPONENT_ID)
					{
						// Right now, we wait for an answer from the worker to which the partition has been delegated to
						// This could help with preventing loss of authority if a worker is delegated an actor
						// before it knows it has been delegated the partition which has auth over it
						// There could be other ways to prevent that, which would make the partition_ack component useless.
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

		WorkersDispatcher.Advance();
		SystemWorkersDispatcher.Advance();

		TSet<Worker_EntityId_Key> WorkersToInspect = WorkersData.GetModifiedEntities();
		const TSet<Worker_EntityId_Key>& SystemWorkerModified = SystemWorkersData.GetModifiedEntities();
		if (SystemWorkerModified.Num() != 0)
		{
			for (const auto& Entry : WorkersData.GetObjects())
			{
				if (SystemWorkerModified.Contains(Entry.Value.SystemEntityId))
				{
					WorkersToInspect.Add(Entry.Key);
				}
			}
		}

		for (Worker_EntityId UpdatedWorker : WorkersToInspect)
		{
			const ServerWorker& ServerWorkerData = WorkersData.GetObjects().FindChecked(UpdatedWorker);
			if (ServerWorkerData.bReadyToBeginPlay)
			{
				FLBWorkerHandle* AlreadyConnectedWorker =
					Algo::FindByPredicate(ConnectedWorkers, [&UpdatedWorker](const FLBWorkerHandle& Connected) {
						return Connected->State->ServerWorkerId == UpdatedWorker;
					});
				if (AlreadyConnectedWorker == nullptr)
				{
					if (SystemWorkersData.GetObjects().Contains(ServerWorkerData.SystemEntityId))
					{
						WorkerConnected(UpdatedWorker);
					}
				}
			}
		}
		WorkersData.ClearModified();
		SystemWorkersData.ClearModified();
	}

	void WorkerConnected(Worker_EntityId ServerWorkerEntityId)
	{
		const ServerWorker& ServerWorkerData = WorkersData.GetObjects().FindChecked(ServerWorkerEntityId);
		Worker_EntityId SysEntityId = ServerWorkerData.SystemEntityId;
		const Worker& SystemWorkerData = SystemWorkersData.GetObjects().FindChecked(SysEntityId);

		FLBWorkerHandle ConnectedWorker = MakeShared<FLBWorkerDesc>(FName(SystemWorkerData.WorkerType));
		ConnectedWorker->State = MakeUnique<FLBWorkerInternalState>();
		ConnectedWorker->State->ServerWorkerId = ServerWorkerEntityId;
		ConnectedWorker->State->SystemWorkerId = SysEntityId;
		ConnectedWorker->State->WorkerType = ConnectedWorker->WorkerType;
		ConnectedWorker->State->FullWorkerName = SystemWorkerData.WorkerId;

		ConnectedWorkers.Add(ConnectedWorker);
		ConnectedWorkersThisFrame.Add(ConnectedWorker);
	}

	void Flush(ISpatialOSWorker& Connection)
	{
		// Wait for the initial entityId reservation query to land
		if (FirstPartitionId == 0)
		{
			return;
		}

		// Ask for more ids if we run out.
		if (CurPartitionId == FirstPartitionId + k_PartitionsReserveRange)
		{
			if (!PartitionReserveRequest.IsSet())
			{
				PartitionReserveRequest = Connection.SendReserveEntityIdsRequest(Impl::k_PartitionsReserveRange, RETRY_UNTIL_COMPLETE);
			}
			return;
		}

		for (auto& PartitionEntry : Partitions)
		{
			FPartitionInternalState& PartitionState = *PartitionEntry->State;
			if (!PartitionState.bPartitionCreated)
			{
				if (!PartitionState.CreationRequest)
				{
					PartitionState.Id = CurPartitionId++;
					IdToPartitionsMap.Add(PartitionState.Id, PartitionEntry);

					TArray<ComponentData> Components =
						CreatePartitionEntityComponents(PartitionState.DisplayName, PartitionState.Id, PartitionState.LBConstraint);

					Worker_EntityId PartitionEntity = PartitionState.Id;
					const Worker_RequestId RequestId =
						Connection.SendCreateEntityRequest(MoveTemp(Components), PartitionEntity, SpatialGDK::RETRY_UNTIL_COMPLETE);
					PartitionState.CreationRequest = RequestId;
					PartitionCreationRequests.Add(RequestId, PartitionEntry);
				}
				continue;
			}

			if (PartitionState.UserAssignment != PartitionState.CurrentAssignment)
			{
				if (!PartitionState.AssignmentRequest)
				{
					if (ConnectedWorkers.Find(PartitionState.UserAssignment) != nullptr)
					{
						Worker_EntityId SystemWorkerEntityId = PartitionState.UserAssignment->State->SystemWorkerId;

						PartitionState.RequestedAssignment = PartitionState.UserAssignment;
						PartitionState.AssignmentRequest = Connection.SendEntityCommandRequest(
							SystemWorkerEntityId, CreateClaimPartitionRequest(PartitionState.Id), SpatialGDK::RETRY_UNTIL_COMPLETE, {});
					}
				}
				continue;
			}
		}
	}

	void SetPartitionReady(Worker_EntityId EntityId)
	{
		FPartitionHandle* Partition = IdToPartitionsMap.Find(EntityId);
		if (Partition)
		{
			FPartitionInternalState& PartitionState = *(*Partition)->State;
			PartitionState.bAcked = true;
		}
	}

	TArray<ComponentData> CreatePartitionEntityComponents(const FString& PartitionName, const Worker_EntityId EntityId,
														  const QueryConstraint& LBConstraint)
	{
		AuthorityDelegationMap DelegationMap;
		DelegationMap.Add(SpatialConstants::GDK_KNOWN_ENTITY_AUTH_COMPONENT_SET_ID, SpatialConstants::INITIAL_STRATEGY_PARTITION_ENTITY_ID);
		DelegationMap.Add(SpatialConstants::PARTITION_WORKER_AUTH_COMPONENT_SET_ID, EntityId);

		TArray<Worker_ComponentData> Components;
		Components.Add(Position().CreateComponentData());
		Components.Add(Persistence().CreateComponentData());
		Components.Add(Metadata(PartitionName).CreateComponentData());

		Components.Add(InterestF->CreatePartitionInterest(LBConstraint, false).CreateComponentData());

		Components.Add(AuthorityDelegation(DelegationMap).CreateComponentData());
		Components.Add(ComponentFactory::CreateEmptyComponentData(SpatialConstants::PARTITION_SHADOW_COMPONENT_ID));
		Components.Add(ComponentFactory::CreateEmptyComponentData(SpatialConstants::PARTITION_ACK_COMPONENT_ID));
		Components.Add(ComponentFactory::CreateEmptyComponentData(SpatialConstants::GDK_KNOWN_ENTITY_TAG_COMPONENT_ID));
		Components.Add(ComponentFactory::CreateEmptyComponentData(SpatialConstants::PARTITION_AUTH_TAG_COMPONENT_ID));

		TArray<ComponentData> Components_Owning;
		for (auto& Component : Components)
		{
			Components_Owning.Add(ComponentData(OwningComponentDataPtr(Component.schema_type), Component.component_id));
		}

		return Components_Owning;
	}

	static constexpr uint32 k_PartitionsReserveRange = 1024;

	const FSubView& WorkerView;
	const FSubView& SystemWorkerView;
	const FSubView& PartitionView;

	TUniquePtr<InterestFactory> InterestF;

	Worker_EntityId StrategyWorkerEntityId = 0;
	TOptional<Worker_RequestId> StrategyWorkerRequest;

	// +++ Partition management data +++
	Worker_EntityId FirstPartitionId = 0;
	Worker_EntityId CurPartitionId = 0;

	TMap<Worker_RequestId_Key, FPartitionHandle> PartitionCreationRequests;
	TOptional<Worker_RequestId> PartitionReserveRequest = 0;

	TMap<Worker_EntityId_Key, FPartitionHandle> IdToPartitionsMap;
	TSet<FPartitionHandle> Partitions;
	// --- Partition management data ---

	// +++ Server worker management data +++
	TLBDataStorage<ServerWorker> WorkersData;
	TLBDataStorage<Worker> SystemWorkersData;
	FLBDataCollection WorkersDispatcher;
	FLBDataCollection SystemWorkersDispatcher;

	TSet<FLBWorkerHandle> ConnectedWorkers;
	TArray<FLBWorkerHandle> ConnectedWorkersThisFrame;
	TArray<FLBWorkerHandle> DisconnectedWorkersThisFrame;
	// --- Server worker management data ---
};

FPartitionManager::~FPartitionManager() = default;

FPartitionManager::FPartitionManager(Worker_EntityId InStrategyWorkerEntityId, ViewCoordinator& Coordinator,
									 TUniquePtr<InterestFactory>&& InterestF)
	: m_Impl(MakeUnique<Impl>(Coordinator, MoveTemp(InterestF)))
{
	m_Impl->StrategyWorkerEntityId = InStrategyWorkerEntityId;
}

void FPartitionManager::Init(ISpatialOSWorker& Connection)
{
	m_Impl->StrategyWorkerRequest = Connection.SendEntityCommandRequest(
		m_Impl->StrategyWorkerEntityId, CreateClaimPartitionRequest(SpatialConstants::INITIAL_STRATEGY_PARTITION_ENTITY_ID),
		SpatialGDK::RETRY_UNTIL_COMPLETE, {});

	m_Impl->PartitionReserveRequest = Connection.SendReserveEntityIdsRequest(Impl::k_PartitionsReserveRange, RETRY_UNTIL_COMPLETE);
}

bool FPartitionManager::IsReady()
{
	return !m_Impl->StrategyWorkerRequest.IsSet();
}

TOptional<Worker_PartitionId> FPartitionManager::GetPartitionId(FPartitionHandle Partition)
{
	if (!m_Impl->Partitions.Contains(Partition))
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

FPartitionHandle FPartitionManager::CreatePartition(FString DisplayName, void* UserData, const QueryConstraint& Interest)
{
	FPartitionHandle NewPartition = MakeShared<FPartitionDesc>();
	NewPartition->State = MakeUnique<FPartitionInternalState>();
	NewPartition->State->UserData = UserData;
	NewPartition->State->LBConstraint = Interest;
	NewPartition->State->DisplayName = MoveTemp(DisplayName);

	m_Impl->Partitions.Add(NewPartition);

	return NewPartition;
}

void FPartitionManager::SetPartitionInterest(FPartitionHandle Partition, const QueryConstraint& NewInterest)
{
	if (!m_Impl->Partitions.Contains(Partition))
	{
		return;
	}
	FPartitionInternalState& PartitionState = *Partition->State;
	PartitionState.LBConstraint = NewInterest;
	PartitionState.bInterestDirty = true;
}

void FPartitionManager::AssignPartitionTo(FPartitionHandle Partition, FLBWorkerHandle Worker)
{
	if (!m_Impl->Partitions.Contains(Partition))
	{
		return;
	}

	FPartitionInternalState& PartitionState = *Partition->State;
	PartitionState.UserAssignment = Worker;
}

void FPartitionManager::SetPartitionMetadata(FPartitionHandle Partition)
{
	if (!m_Impl->Partitions.Contains(Partition))
	{
		return;
	}
	// Later
}

void FPartitionManager::AdvanceView(ISpatialOSWorker& Connection)
{
	m_Impl->AdvanceView(Connection);
}

void FPartitionManager::Flush(ISpatialOSWorker& Connection)
{
	m_Impl->Flush(Connection);
}

TArray<FLBWorkerHandle> FPartitionManager::GetConnectedWorkers()
{
	TArray<FLBWorkerHandle> ConnectedWorkers;
	Swap(ConnectedWorkers, m_Impl->ConnectedWorkersThisFrame);
	return ConnectedWorkers;
}

TArray<FLBWorkerHandle> FPartitionManager::GetDisconnectedWorkers()
{
	TArray<FLBWorkerHandle> DisconnectedWorkers;
	Swap(DisconnectedWorkers, m_Impl->DisconnectedWorkersThisFrame);
	return DisconnectedWorkers;
}

Worker_EntityId FPartitionManager::GetServerWorkerEntityIdForWorker(FLBWorkerHandle Worker)
{
	if (!m_Impl->ConnectedWorkers.Contains(Worker))
	{
		return SpatialConstants::INVALID_ENTITY_ID;
	}

	return Worker->State->ServerWorkerId;
}

} // namespace SpatialGDK
