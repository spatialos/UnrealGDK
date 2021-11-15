// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "LoadBalancing/PartitionManager.h"
#include "LoadBalancing/PartitionManagerImpl.h"

#include "EngineClasses/SpatialPartitionSystem.h"
#include "EngineClasses/SpatialVirtualWorkerTranslator.h"
#include "Schema/ChangeInterest.h"
#include "SpatialView/SpatialOSWorker.h"
#include "Utils/ComponentFactory.h"
#include "Utils/InterestFactory.h"

#include "Algo/Find.h"

DEFINE_LOG_CATEGORY(LogSpatialPartitionManager);

namespace SpatialGDK
{
TArray<TUniquePtr<FStartupStep>> CreatePartitionManagerStartupSequence(ISpatialOSWorker& Connection, FPartitionManager::Impl& PartitionMgr);

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

PartitionInterestUpdate::PartitionInterestUpdate()
{
	const USpatialGDKSettings* Settings = GetDefault<USpatialGDKSettings>();
	if (Settings->PartitionSystemClass)
	{
		USpatialPartitionSystem* PartitionSystem = Cast<USpatialPartitionSystem>(Settings->PartitionSystemClass->GetDefaultObject());
		TArray<FLBDataStorage*> DataStores = PartitionSystem->GetData();
		for (auto Store : DataStores)
		{
			PartitionMetaData.Append(Store->GetComponentsToWatch().Array());
		}
	}
}

void PartitionInterestUpdate::Clear()
{
	NewWorker.Empty();
	NewServerWorker.Empty();
	NewPartition.Empty();
	RemovedWorker.Empty();
	RemovedServerWorker.Empty();
	RemovedPartition.Empty();
}

bool PartitionInterestUpdate::HasChanged() const
{
	return NewPartition.Num() > 0 || NewWorker.Num() > 0 || NewServerWorker.Num() > 0 || RemovedPartition.Num() > 0
		   || RemovedWorker.Num() > 0 || RemovedServerWorker.Num() > 0;
}

FPartitionManager::Impl::Impl(const FSubView& InServerWorkerView, ViewCoordinator& Coordinator, InterestFactory& InInterestF)
	: WorkerView(InServerWorkerView)
	, SystemWorkerView(Coordinator.CreateSubView(SpatialConstants::WORKER_COMPONENT_ID, SpatialGDK::FSubView::NoFilter,
												 SpatialGDK::FSubView::NoDispatcherCallbacks))
	, PartitionView(Coordinator.CreateSubView(SpatialConstants::PARTITION_ACK_COMPONENT_ID, SpatialGDK::FSubView::NoFilter,
											  SpatialGDK::FSubView::NoDispatcherCallbacks))
	, InterestF(InInterestF)
	, WorkersDispatcher(WorkerView)
	, SystemWorkersDispatcher(SystemWorkerView)
{
	WorkersDispatcher.DataStorages.Add(&WorkersData);
	SystemWorkersDispatcher.DataStorages.Add(&SystemWorkersData);
	StrategyWorkerEntityId = Coordinator.GetWorkerSystemEntityId();

	const USpatialGDKSettings* Settings = GetDefault<USpatialGDKSettings>();
	InterestUpdate.bEnabled = Settings->bUserSpaceServerInterest;
}

void FPartitionManager::Impl::AdvanceView(ISpatialOSWorker& Connection)
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
			InterestUpdate.NewPartition.Add(Delta.EntityId);
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
			InterestUpdate.RemovedPartition.Add(Delta.EntityId);
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

	InterestUpdate.NewServerWorker.Append(WorkersDispatcher.EntitiesAdded.Array());
	InterestUpdate.RemovedServerWorker.Append(WorkersDispatcher.EntitiesRemoved.Array());
	InterestUpdate.NewWorker.Append(SystemWorkersDispatcher.EntitiesAdded.Array());
	InterestUpdate.RemovedWorker.Append(SystemWorkersDispatcher.EntitiesRemoved.Array());

	CheckWorkersConnected(WorkersData.GetModifiedEntities(), SystemWorkersData.GetModifiedEntities());

	WorkersData.ClearModified();
	SystemWorkersData.ClearModified();
}

void FPartitionManager::Impl::CheckWorkersConnected(TSet<Worker_EntityId_Key> WorkersToInspect,
													const TSet<Worker_EntityId_Key>& SystemWorkerModified)
{
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
		FLBWorkerHandle ConnectedWorker;
		if (auto FindRes = Algo::FindByPredicate(ConnectedWorkers, [&UpdatedWorker](const FLBWorkerHandle& Connected) {
				return Connected->State->ServerWorkerId == UpdatedWorker;
			}))
		{
			ConnectedWorker = *FindRes;
		}
		if (ConnectedWorker == nullptr)
		{
			if (SystemWorkersData.GetObjects().Contains(ServerWorkerData.SystemEntityId))
			{
				ConnectedWorker = WorkerConnected(UpdatedWorker);
			}
		}

		if (ConnectedWorker && !ConnectedWorker->State->bReadyToBeginPlay && ServerWorkerData.bReadyToBeginPlay)
		{
			ConnectedWorker->State->bReadyToBeginPlay = true;
			ConnectedWorkersThisFrame.Add(ConnectedWorker);
		}
	}
}

FLBWorkerHandle FPartitionManager::Impl::WorkerConnected(Worker_EntityId ServerWorkerEntityId)
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
	return ConnectedWorker;
}

bool FPartitionManager::Impl::WaitForPartitionIdAvailable(ISpatialOSWorker& Connection)
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

void FPartitionManager::Impl::Flush(ISpatialOSWorker& Connection)
{
	if (WaitForPartitionIdAvailable(Connection))
	{
		return;
	}

	FlushInterestUpdates(Connection);

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
						[this, PartitionEntry, &Connection, SystemWorkerEntityId](const Worker_CommandResponseOp& Op) {
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

			if (PartitionState.bInterestDirty && false)
			{
				if (Connection.GetView().Find(PartitionState.Id))
				{
					Worker_ComponentUpdate WorkerUpdate =
						InterestF.CreatePartitionInterest(PartitionState.LBConstraint, true).CreateInterestUpdate();
					ComponentUpdate Update(OwningComponentUpdatePtr(WorkerUpdate.schema_type), WorkerUpdate.component_id);
					Connection.SendComponentUpdate(PartitionState.Id, MoveTemp(Update));
					PartitionState.bInterestDirty = false;
				}
			}
		}
	}
}

void FPartitionManager::Impl::FlushInterestUpdates(ISpatialOSWorker& Connection)
{
	if (!InterestUpdate.bEnabled)
	{
		return;
	}

	for (auto Worker : ConnectedWorkers)
	{
		if (!Worker->State->bInterestInitialized)
		{
			ChangeInterestRequest Request;
			Request.SystemEntityId = Worker->State->SystemWorkerId;
			Request.bOverwrite = false;

			{
				// Initial snapshot interest query.
				ChangeInterestQuery SnapshotQuery;
				SnapshotQuery.Entities.Add(SpatialConstants::INITIAL_GLOBAL_STATE_MANAGER_ENTITY_ID);
				SnapshotQuery.Entities.Add(SpatialConstants::INITIAL_SPAWNER_ENTITY_ID);
				SnapshotQuery.Entities.Add(SpatialConstants::INITIAL_VIRTUAL_WORKER_TRANSLATOR_ENTITY_ID);
				SnapshotQuery.Components = InterestF.GetServerNonAuthInterestResultType().ComponentIds;
				SnapshotQuery.ComponentSets = InterestF.GetServerNonAuthInterestResultType().ComponentSetsIds;
				Request.QueriesToAdd.Add(SnapshotQuery);
			}

			if (PartitionView.GetCompleteEntities().Num() > 0)
			{
				ChangeInterestQuery PartitionQuery;
				PartitionQuery.Entities.Append(PartitionView.GetCompleteEntities());
				PartitionQuery.Components = InterestUpdate.PartitionMetaData;
				PartitionQuery.Components.Add(SpatialConstants::PARTITION_ACK_COMPONENT_ID);
				Request.QueriesToAdd.Add(MoveTemp(PartitionQuery));
			}
			if (SystemWorkersDispatcher.SubView.GetCompleteEntities().Num() > 0)
			{
				ChangeInterestQuery WorkerQuery;
				WorkerQuery.Entities.Append(SystemWorkersDispatcher.SubView.GetCompleteEntities());
				WorkerQuery.Components = { SpatialConstants::WORKER_COMPONENT_ID, SpatialConstants::SYSTEM_COMPONENT_ID };
				Request.QueriesToAdd.Add(MoveTemp(WorkerQuery));
			}
			if (WorkersDispatcher.SubView.GetCompleteEntities().Num() > 0)
			{
				ChangeInterestQuery ServerWorkerQuery;
				ServerWorkerQuery.Entities.Append(WorkersDispatcher.SubView.GetCompleteEntities());
				for (auto Entity : ServerWorkerQuery.Entities)
				{
					UE_LOG(LogTemp, Log, TEXT("Make worker %llu interested in %llu"), Request.SystemEntityId, Entity);
				}
				ServerWorkerQuery.Components = { SpatialConstants::SERVER_WORKER_COMPONENT_ID,
												 SpatialConstants::GDK_KNOWN_ENTITY_TAG_COMPONENT_ID };
				Request.QueriesToAdd.Add(MoveTemp(ServerWorkerQuery));
			}
			Request.DebugOutput();
			Request.SendRequest(Connection);

			Worker->State->bInterestInitialized = true;
		}
		else if (InterestUpdate.HasChanged())
		{
			ChangeInterestRequest Request;
			Request.SystemEntityId = Worker->State->SystemWorkerId;
			Request.bOverwrite = false;

			if (InterestUpdate.NewPartition.Num() > 0)
			{
				ChangeInterestQuery PartitionQuery;
				PartitionQuery.Entities.Append(InterestUpdate.NewPartition);
				PartitionQuery.Components = InterestUpdate.PartitionMetaData;
				PartitionQuery.Components.Add(SpatialConstants::PARTITION_ACK_COMPONENT_ID);
				Request.QueriesToAdd.Add(MoveTemp(PartitionQuery));
			}
			if (InterestUpdate.RemovedPartition.Num() > 0)
			{
				ChangeInterestQuery PartitionQuery;
				PartitionQuery.Entities.Append(InterestUpdate.NewPartition);
				PartitionQuery.Components = InterestUpdate.PartitionMetaData;
				PartitionQuery.Components.Add(SpatialConstants::PARTITION_ACK_COMPONENT_ID);
				Request.QueriesToRemove.Add(MoveTemp(PartitionQuery));
			}
			if (InterestUpdate.NewWorker.Num() > 0)
			{
				ChangeInterestQuery WorkerQuery;
				WorkerQuery.Entities.Append(InterestUpdate.NewWorker);
				WorkerQuery.Components = { SpatialConstants::WORKER_COMPONENT_ID, SpatialConstants::SYSTEM_COMPONENT_ID };
				Request.QueriesToAdd.Add(MoveTemp(WorkerQuery));
			}
			if (InterestUpdate.RemovedWorker.Num() > 0)
			{
				ChangeInterestQuery WorkerQuery;
				WorkerQuery.Entities.Append(InterestUpdate.RemovedWorker);
				WorkerQuery.Components = { SpatialConstants::WORKER_COMPONENT_ID, SpatialConstants::SYSTEM_COMPONENT_ID };
				Request.QueriesToRemove.Add(MoveTemp(WorkerQuery));
			}
			if (InterestUpdate.NewServerWorker.Num() > 0)
			{
				ChangeInterestQuery ServerWorkerQuery;
				ServerWorkerQuery.Entities.Append(InterestUpdate.NewServerWorker);
				ServerWorkerQuery.Components = { SpatialConstants::SERVER_WORKER_COMPONENT_ID,
												 SpatialConstants::GDK_KNOWN_ENTITY_TAG_COMPONENT_ID };
				Request.QueriesToAdd.Add(MoveTemp(ServerWorkerQuery));
			}
			if (InterestUpdate.RemovedServerWorker.Num() > 0)
			{
				ChangeInterestQuery ServerWorkerQuery;
				ServerWorkerQuery.Entities.Append(InterestUpdate.RemovedServerWorker);
				ServerWorkerQuery.Components = { SpatialConstants::SERVER_WORKER_COMPONENT_ID,
												 SpatialConstants::GDK_KNOWN_ENTITY_TAG_COMPONENT_ID };
				Request.QueriesToRemove.Add(MoveTemp(ServerWorkerQuery));
			}

			Request.DebugOutput();
			Request.SendRequest(Connection);
		}
	}

	InterestUpdate.Clear();
}

void FPartitionManager::Impl::SetPartitionReady(Worker_EntityId EntityId)
{
	FPartitionHandle* Partition = IdToPartitionsMap.Find(EntityId);
	if (Partition)
	{
		FPartitionInternalState& PartitionState = *(*Partition)->State;
		PartitionState.bAcked = true;
	}
}

TArray<ComponentData> FPartitionManager::Impl::CreatePartitionEntityComponents(const FString& PartitionName, const Worker_EntityId EntityId,
																			   const QueryConstraint& LBConstraint)
{
	AuthorityDelegationMap DelegationMap;
	DelegationMap.Add(SpatialConstants::PARTITION_METADATA_AUTH_COMPONENT_SET_ID, SpatialConstants::INITIAL_STRATEGY_PARTITION_ENTITY_ID);
	DelegationMap.Add(SpatialConstants::PARTITION_WORKER_AUTH_COMPONENT_SET_ID, EntityId);

	TArray<Worker_ComponentData> Components;
	Components.Add(Position().CreateComponentData());
	Components.Add(Persistence().CreateComponentData());
	Components.Add(Metadata(PartitionName).CreateComponentData());

	Components.Add(InterestF.CreatePartitionInterest(LBConstraint, false).CreateComponentData());

	Components.Add(AuthorityDelegation(DelegationMap).CreateComponentData());
	Components.Add(ComponentFactory::CreateEmptyComponentData(SpatialConstants::PARTITION_SHADOW_COMPONENT_ID));
	Components.Add(ComponentFactory::CreateEmptyComponentData(SpatialConstants::PARTITION_ACK_COMPONENT_ID));
	Components.Add(ComponentFactory::CreateEmptyComponentData(SpatialConstants::GDK_KNOWN_ENTITY_TAG_COMPONENT_ID));
	Components.Add(ComponentFactory::CreateEmptyComponentData(SpatialConstants::PARTITION_AUTH_TAG_COMPONENT_ID));
	Components.Add(ComponentFactory::CreateEmptyComponentData(SpatialConstants::LOADBALANCER_PARTITION_TAG_COMPONENT_ID));

	TArray<ComponentData> Components_Owning;
	for (auto& Component : Components)
	{
		Components_Owning.Add(ComponentData(OwningComponentDataPtr(Component.schema_type), Component.component_id));
	}

	return Components_Owning;
}

FPartitionManager::FPartitionManager(const FSubView& InServerWorkerView, ViewCoordinator& Coordinator, InterestFactory& InterestF)
	: m_Impl(MakeUnique<Impl>(InServerWorkerView, Coordinator, InterestF))
{
}

FPartitionManager::~FPartitionManager() = default;

void FPartitionManager::Init(ISpatialOSWorker& Connection)
{
	m_Impl->StartupExecutor.Emplace(CreatePartitionManagerStartupSequence(Connection, *m_Impl));
}

bool FPartitionManager::IsReady()
{
	if (m_Impl->StartupExecutor.IsSet() && m_Impl->StartupExecutor->TryFinish())
	{
		m_Impl->StartupExecutor.Reset();
	}
	return !m_Impl->StartupExecutor.IsSet();
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

FPartitionHandle FPartitionManager::CreatePartition(FString DisplayName, const QueryConstraint& Interest, TArray<ComponentData> MetaData)
{
	FPartitionHandle NewPartition = MakeShared<FPartitionDesc>();
	NewPartition->State = MakeUnique<FPartitionInternalState>();
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

Worker_EntityId FPartitionManager::GetSystemWorkerEntityIdForWorker(FLBWorkerHandle Worker)
{
	if (!m_Impl->ConnectedWorkers.Contains(Worker))
	{
		return SpatialConstants::INVALID_ENTITY_ID;
	}

	return Worker->State->SystemWorkerId;
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
