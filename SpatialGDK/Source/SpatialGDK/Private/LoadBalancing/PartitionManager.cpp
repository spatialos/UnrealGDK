// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "LoadBalancing/PartitionManager.h"

#include "EngineClasses/SpatialVirtualWorkerTranslator.h"
#include "Interop/SpatialCommandsHandler.h"
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
	TSet<Worker_ComponentId> CurrentMetadataComponents;
	TArray<ComponentData> PendingMetadataCreation;
	TArray<ComponentUpdate> PendingMetadataUpdates;

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
	Impl(const FSubView& InServerWorkerView, ViewCoordinator& Coordinator, TUniquePtr<InterestFactory>&& InInterestF)
		: WorkerView(InServerWorkerView)
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
		CommandsHandler.ProcessOps(Connection.GetWorkerMessages());

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

	bool WaitForPartitionIdAvailable(ISpatialOSWorker& Connection)
	{
		// Ask for more ids if we run out.
		if (FirstPartitionId == 0 || CurPartitionId >= FirstPartitionId + k_PartitionsReserveRange)
		{
			if (!PartitionReserveRequest.IsSet())
			{
				PartitionReserveRequest = Connection.SendReserveEntityIdsRequest(Impl::k_PartitionsReserveRange, RETRY_UNTIL_COMPLETE);
				CommandsHandler.AddRequest(*PartitionReserveRequest, [this](const Worker_ReserveEntityIdsResponseOp& Op) {
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
				});
			}
			return true;
		}
		return false;
	}

	void Flush(ISpatialOSWorker& Connection)
	{
		if (WaitForPartitionIdAvailable(Connection))
		{
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

					for (auto& InitialComponent : PartitionState.PendingMetadataCreation)
					{
						PartitionState.CurrentMetadataComponents.Add(InitialComponent.GetComponentId());
						Components.Add(MoveTemp(InitialComponent));
					}
					PartitionState.PendingMetadataCreation.Empty();

					Worker_EntityId PartitionEntity = PartitionState.Id;
					const Worker_RequestId RequestId =
						Connection.SendCreateEntityRequest(MoveTemp(Components), PartitionEntity, SpatialGDK::RETRY_UNTIL_COMPLETE);
					PartitionState.CreationRequest = RequestId;
					PartitionCreationRequests.Add(RequestId, PartitionEntry);
					CommandsHandler.AddRequest(RequestId, [this](const Worker_CreateEntityResponseOp& Op) {
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
					});
				}
				continue;
			}

			for (auto& NewComponent : PartitionState.PendingMetadataCreation)
			{
				PartitionState.CurrentMetadataComponents.Add(NewComponent.GetComponentId());
				Connection.SendAddComponent(PartitionState.Id, MoveTemp(NewComponent));
			}
			PartitionState.PendingMetadataCreation.Empty();

			for (auto& Update : PartitionState.PendingMetadataUpdates)
			{
				Connection.SendComponentUpdate(PartitionState.Id, MoveTemp(Update));
			}
			PartitionState.PendingMetadataUpdates.Empty();

			if (PartitionState.UserAssignment != PartitionState.CurrentAssignment)
			{
				if (!PartitionState.AssignmentRequest)
				{
					if (ConnectedWorkers.Find(PartitionState.UserAssignment) != nullptr)
					{
						Worker_EntityId SystemWorkerEntityId = PartitionState.UserAssignment->State->SystemWorkerId;

						PartitionState.RequestedAssignment = PartitionState.UserAssignment;
						PartitionState.AssignmentRequest = CommandsHandler.ClaimPartition(
							Connection, SystemWorkerEntityId, PartitionState.Id,
							[this, PartitionEntry](const Worker_CommandResponseOp& Op) {
								if (Partitions.Contains(PartitionEntry))
								{
									FPartitionInternalState& State = *PartitionEntry->State;
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
							});
					}
				}
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
		DelegationMap.Add(SpatialConstants::PARTITION_METADATA_AUTH_COMPONENT_SET_ID,
						  SpatialConstants::INITIAL_STRATEGY_PARTITION_ENTITY_ID);
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
	FCommandsHandler CommandsHandler;

	// +++ Partition management data +++
	Worker_EntityId FirstPartitionId = 0;
	Worker_EntityId CurPartitionId = 0;

	TMap<Worker_RequestId_Key, FPartitionHandle> PartitionCreationRequests;
	TOptional<Worker_RequestId> PartitionReserveRequest;

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

FPartitionManager::FPartitionManager(const FSubView& InServerWorkerView, Worker_EntityId InStrategyWorkerEntityId,
									 ViewCoordinator& Coordinator, TUniquePtr<InterestFactory>&& InterestF)
	: m_Impl(MakeUnique<Impl>(InServerWorkerView, Coordinator, MoveTemp(InterestF)))
{
	m_Impl->StrategyWorkerEntityId = InStrategyWorkerEntityId;
}

void FPartitionManager::Init(ISpatialOSWorker& Connection)
{
	m_Impl->StrategyWorkerRequest = m_Impl->CommandsHandler.ClaimPartition(
		Connection, m_Impl->StrategyWorkerEntityId, SpatialConstants::INITIAL_STRATEGY_PARTITION_ENTITY_ID,
		[this](const Worker_CommandResponseOp& Op) {
			if (Op.status_code != WORKER_STATUS_CODE_SUCCESS)
			{
				UE_LOG(LogSpatialPartitionManager, Error, TEXT("Claim Strategy partition failed : %s"), UTF8_TO_TCHAR(Op.message));
			}
			m_Impl->StrategyWorkerRequest.Reset();
		});

	m_Impl->WaitForPartitionIdAvailable(Connection);
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

FPartitionHandle FPartitionManager::CreatePartition(FString DisplayName, void* UserData, const QueryConstraint& Interest,
													TArray<ComponentData> MetaData)
{
	FPartitionHandle NewPartition = MakeShared<FPartitionDesc>();
	NewPartition->State = MakeUnique<FPartitionInternalState>();
	NewPartition->State->UserData = UserData;
	NewPartition->State->LBConstraint = Interest;
	NewPartition->State->DisplayName = MoveTemp(DisplayName);
	NewPartition->State->PendingMetadataCreation = MoveTemp(MetaData);

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

void FPartitionManager::UpdatePartitionMetadata(FPartitionHandle Partition, TArray<ComponentUpdate> Updates)
{
	if (!m_Impl->Partitions.Contains(Partition))
	{
		return;
	}

	FPartitionInternalState& PartitionState = *Partition->State;
	for (auto& Update : Updates)
	{
		// Disallow adding new components on the fly for now.
		if (ensureAlways(PartitionState.CurrentMetadataComponents.Contains(Update.GetComponentId())))
		{
			ComponentUpdate* PendingUpdate =
				PartitionState.PendingMetadataUpdates.FindByPredicate([&Update](const ComponentUpdate& QueuedUpdate) {
					return QueuedUpdate.GetComponentId() == Update.GetComponentId();
				});
			if (PendingUpdate != nullptr)
			{
				PendingUpdate->Merge(MoveTemp(Update));
			}
			else
			{
				PartitionState.PendingMetadataUpdates.Add(MoveTemp(Update));
			}
		}
	}
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

FLBWorkerHandle FPartitionManager::GetWorkerForServerWorkerEntity(Worker_EntityId EntityId)
{
	for (const auto& Worker : m_Impl->ConnectedWorkers)
	{
		if (Worker->State->ServerWorkerId == EntityId)
		{
			return Worker;
		}
	}

	return FLBWorkerHandle();
}

} // namespace SpatialGDK
