#include "Interop/Startup/DefaultServerWorkerStartupHandler.h"

#include "Algo/AllOf.h"
#include "Algo/MinElement.h"
#include "Algo/Transform.h"
#include "EngineClasses/SpatialNetDriver.h"
#include "EngineClasses/SpatialPackageMapClient.h"
#include "Interop/GlobalStateManager.h"
#include "LoadBalancing/AbstractLBStrategy.h"
#include "Schema/ServerWorker.h"
#include "Utils/EntityFactory.h"
#include "Utils/InterestFactory.h"
#include "Utils/SpatialStatics.h"

DEFINE_LOG_CATEGORY_STATIC(LogSpatialStartupHandler, Log, All);

namespace SpatialGDK
{
FSpatialServerStartupHandler::FSpatialServerStartupHandler(USpatialNetDriver& InNetDriver, const FInitialSetup& InSetup)
	: Setup(InSetup)
	, NetDriver(&InNetDriver)
{
}

bool FSpatialServerStartupHandler::TryFinishStartup()
{
	if (Stage == EStage::CreateWorkerEntity)
	{
		if (!bCalledCreateEntity)
		{
			bCalledCreateEntity = true;
			WorkerEntityCreator.Emplace(*NetDriver, *NetDriver->Connection);
		}
		WorkerEntityCreator->ProcessOps(GetOps());
		if (WorkerEntityCreator->IsFinished())
		{
			WorkerEntityId = WorkerEntityCreator->GetWorkerEntityId();
			Stage = EStage::WaitForWorkerEntities;
		}
	}

	if (Stage == EStage::WaitForWorkerEntities)
	{
		TMap<Worker_EntityId_Key, const ComponentData*> WorkerComponents;

		for (const auto& EntityData : GetCoordinator().GetView())
		{
			const ComponentData* WorkerComponent =
				EntityData.Value.Components.FindByPredicate(ComponentIdEquality{ SpatialConstants::SERVER_WORKER_COMPONENT_ID });
			if (WorkerComponent != nullptr)
			{
				WorkerComponents.Emplace(EntityData.Key, WorkerComponent);
			}
		}
		if (WorkerComponents.Num() >= Setup.ExpectedServerWorkersCount)
		{
			ensureMsgf(WorkerComponents.Num() == Setup.ExpectedServerWorkersCount,
					   TEXT("There should never be more server workers connected than we expect. Expected %d, actually have %d"),
					   Setup.ExpectedServerWorkersCount, WorkerComponents.Num());
			WorkerComponents.GetKeys(WorkerEntityIds);
			Stage = EStage::WaitForGSMEntity;
		}
	}

	if (Stage == EStage::WaitForGSMEntity)
	{
		const EntityViewElement* GlobalStateManagerEntity = GetCoordinator().GetView().Find(GetGSM().GlobalStateManagerEntityId);
		if (GlobalStateManagerEntity != nullptr)
		{
			Stage = EStage::DeriveDeploymentRecoveryState;
		}
	}

	if (Stage == EStage::DeriveDeploymentRecoveryState)
	{
		const EntityViewElement& GlobalStateManagerEntity = GetCoordinator().GetView().FindChecked(GetGSM().GlobalStateManagerEntityId);
		const ComponentData* StartupActorManagerData = GlobalStateManagerEntity.Components.FindByPredicate(
			ComponentIdEquality{ SpatialConstants::STARTUP_ACTOR_MANAGER_COMPONENT_ID });
		if (StartupActorManagerData != nullptr)
		{
			if (Schema_GetBoolCount(StartupActorManagerData->GetFields(), SpatialConstants::STARTUP_ACTOR_MANAGER_CAN_BEGIN_PLAY_ID) != 0)
			{
				// StartupActorManager's CAN_BEGIN_PLAY is raised after startup is finished, and is false in the initial snapshot.
				// If it's true at this point, then this worker is either recovering or loading from a non-initial snapshot.
				bIsRecoveringOrSnapshot =
					GetBoolFromSchema(StartupActorManagerData->GetFields(), SpatialConstants::STARTUP_ACTOR_MANAGER_CAN_BEGIN_PLAY_ID);

				// We should only call BeginPlay on actors if this is a fresh deployment; otherwise, we assume that
				// BeginPlay has already been called on them before.
				NetDriver->GlobalStateManager->bCanSpawnWithAuthority = !bIsRecoveringOrSnapshot;

				Stage = EStage::TryClaimingGSMEntityAuthority;
			}
		}
	}

	if (Stage == EStage::TryClaimingGSMEntityAuthority)
	{
		const bool bDidClaimStartupPartition = TryClaimingStartupPartition();
		if (bDidClaimStartupPartition)
		{
			Stage = EStage::WaitForGSMEntityAuthority;
		}
		else
		{
			Stage = EStage::GetVirtualWorkerTranslationState;
			bHasGSMAuth = false;
		}
	}

	if (Stage == EStage::WaitForGSMEntityAuthority)
	{
		const EntityViewElement& GlobalStateManagerEntity = GetCoordinator().GetView().FindChecked(GetGSM().GlobalStateManagerEntityId);
		if (GlobalStateManagerEntity.Authority.Contains(SpatialConstants::GDK_KNOWN_ENTITY_AUTH_COMPONENT_SET_ID))
		{
			bHasGSMAuth = true;
			GetGSM().SetDeploymentState();
			Stage = EStage::GsmAuthFillWorkerTranslationState;
		}
	}

	if (Stage == EStage::GsmAuthFillWorkerTranslationState)
	{
		PartitionQueryHandler.ProcessOps(GetOps());

		if (!bStartedWorkerPartitionQuery)
		{
			bStartedWorkerPartitionQuery = true;

			Worker_Constraint WorkerPartitionConstraint;
			WorkerPartitionConstraint.constraint.component_constraint.component_id = SpatialConstants::WORKER_PARTITION_TAG_COMPONENT_ID;
			WorkerPartitionConstraint.constraint_type = Worker_ConstraintType::WORKER_CONSTRAINT_TYPE_COMPONENT;

			Worker_EntityQuery PartitionsQuery{};

			PartitionsQuery.constraint = WorkerPartitionConstraint;

			static TArray<Worker_ComponentId> ComponentsToQuery{
				SpatialConstants::WORKER_PARTITION_TAG_COMPONENT_ID,
			};

			PartitionsQuery.snapshot_result_type_component_ids = ComponentsToQuery.GetData();
			PartitionsQuery.snapshot_result_type_component_id_count = ComponentsToQuery.Num();

			const Worker_RequestId QueryId = GetCoordinator().SendEntityQueryRequest(EntityQuery(PartitionsQuery), RETRY_UNTIL_COMPLETE);

			PartitionQueryHandler.AddRequest(QueryId, [this](const Worker_EntityQueryResponseOp& QueryResponse) {
				if (ensure(QueryResponse.status_code == WORKER_STATUS_CODE_SUCCESS))
				{
					const int32 WorkerPartitionsCount = QueryResponse.result_count;

					TArray<Worker_EntityId> DiscoveredPartitionEntityIds;
					DiscoveredPartitionEntityIds.Reserve(WorkerPartitionsCount);

					for (int32 WorkerPartitionIndex = 0; WorkerPartitionIndex < WorkerPartitionsCount; ++WorkerPartitionIndex)
					{
						DiscoveredPartitionEntityIds.Emplace(QueryResponse.results[WorkerPartitionIndex].entity_id);
					}

					const bool bShouldCreatePartitions = DiscoveredPartitionEntityIds.Num() < Setup.ExpectedServerWorkersCount;

					WorkerPartitions.Append(MoveTemp(DiscoveredPartitionEntityIds));

					Stage = bShouldCreatePartitions ? EStage::GsmAuthCreatePartitions : EStage::GsmAuthAssignPartitionsToVirtualWorkers;
				}
			});
		}
	}

	if (Stage == EStage::GsmAuthCreatePartitions)
	{
		const int32 PartitionsToSpawn = Setup.ExpectedServerWorkersCount - WorkerPartitions.Num();
		UE_LOG(LogSpatialStartupHandler, Log, TEXT("Spawning partition entities for %d virtual workers (need to spawn %d)"),
			   Setup.ExpectedServerWorkersCount, PartitionsToSpawn);

		TArray<Worker_EntityId> NewPartitionEntityIds;
		NewPartitionEntityIds.Reserve(PartitionsToSpawn);
		for (int32 PartitionIndex = 0; PartitionIndex < PartitionsToSpawn; ++PartitionIndex)
		{
			const VirtualWorkerId AssumedPartitionWorkerId = 1 + WorkerPartitions.Num() + PartitionIndex;
			const Worker_EntityId PartitionEntityId = NetDriver->PackageMap->AllocateNewEntityId();
			UE_LOG(LogSpatialStartupHandler, Log, TEXT("- Virtual Worker: %d. Entity: %lld. "), AssumedPartitionWorkerId,
				   PartitionEntityId);
			SpawnPartitionEntity(PartitionEntityId, AssumedPartitionWorkerId);
			NewPartitionEntityIds.Emplace(PartitionEntityId);
		}

		WorkerPartitions.Append(MoveTemp(NewPartitionEntityIds));

		Stage = EStage::GsmAuthWaitForPartitionsVisibility;
	}

	if (Stage == EStage::GsmAuthWaitForPartitionsVisibility)
	{
		const bool bAreAllPartitionsInView =
			Algo::AllOf(WorkerPartitions, [View = &GetCoordinator().GetView()](const Worker_EntityId& PartitionEntityId) {
				return View->Contains(PartitionEntityId);
			});

		if (bAreAllPartitionsInView)
		{
			Stage = EStage::GsmAuthAssignPartitionsToVirtualWorkers;
		}
	}

	if (Stage == EStage::GsmAuthAssignPartitionsToVirtualWorkers)
	{
		const EntityViewElement* VirtualWorkerTranslatorEntity =
			GetCoordinator().GetView().Find(SpatialConstants::INITIAL_VIRTUAL_WORKER_TRANSLATOR_ENTITY_ID);

		if (VirtualWorkerTranslatorEntity == nullptr)
		{
			return false;
		}

		const ComponentData* VirtualWorkerTranslatorComponent = VirtualWorkerTranslatorEntity->Components.FindByPredicate(
			ComponentIdEquality{ SpatialConstants::VIRTUAL_WORKER_TRANSLATION_COMPONENT_ID });
		if (VirtualWorkerTranslatorComponent == nullptr)
		{
			return false;
		}

		TArray<const ComponentData*> WorkerComponents;
		Algo::Transform(WorkerEntityIds, WorkerComponents, [this](const Worker_EntityId EntityId) {
			return GetCoordinator().GetView()[EntityId].Components.FindByPredicate(
				ComponentIdEquality{ SpatialConstants::SERVER_WORKER_COMPONENT_ID });
		});

		ComponentUpdate Update(SpatialConstants::VIRTUAL_WORKER_TRANSLATION_COMPONENT_ID);

		for (int32 WorkerIndex = 0; WorkerIndex < Setup.ExpectedServerWorkersCount; ++WorkerIndex)
		{
			const ComponentData* ServerWorkerData = WorkerComponents[WorkerIndex];

			const ServerWorker WorkerData(ServerWorkerData->GetUnderlying());

			const Worker_EntityId ServerWorkerEntityId = WorkerEntityIds[WorkerIndex];
			const Worker_EntityId PartitionEntityId = WorkerPartitions[WorkerIndex];

			const VirtualWorkerId WorkerId = 1 + WorkerIndex;

			// Assign the worker's partition to its system entity.
			ClaimHandler.ClaimPartition(GetCoordinator(), WorkerData.SystemEntityId, PartitionEntityId);

			// Reflect the partition assignment in the translator object.
			Schema_Object* EntryObject = Schema_AddObject(Update.GetFields(), SpatialConstants::VIRTUAL_WORKER_TRANSLATION_MAPPING_ID);
			Schema_AddUint32(EntryObject, SpatialConstants::MAPPING_VIRTUAL_WORKER_ID, WorkerId);
			AddStringToSchema(EntryObject, SpatialConstants::MAPPING_PHYSICAL_WORKER_NAME_ID, WorkerData.WorkerName);
			Schema_AddEntityId(EntryObject, SpatialConstants::MAPPING_SERVER_WORKER_ENTITY_ID, ServerWorkerEntityId);
			Schema_AddEntityId(EntryObject, SpatialConstants::MAPPING_PARTITION_ID, PartitionEntityId);
		}

		GetCoordinator().SendComponentUpdate(SpatialConstants::INITIAL_VIRTUAL_WORKER_TRANSLATOR_ENTITY_ID, MoveTemp(Update));

		Stage = EStage::GetVirtualWorkerTranslationState;
	}

	if (Stage == EStage::GetVirtualWorkerTranslationState)
	{
		const ComponentData* DeploymentMapComponent =
			GetCoordinator().GetView()[SpatialConstants::INITIAL_GLOBAL_STATE_MANAGER_ENTITY_ID].Components.FindByPredicate(
				ComponentIdEquality{ SpatialConstants::DEPLOYMENT_MAP_COMPONENT_ID });
		if (ensure(DeploymentMapComponent != nullptr))
		{
			const int32 ExistingSessionId =
				Schema_GetInt32(DeploymentMapComponent->GetFields(), SpatialConstants::DEPLOYMENT_MAP_SESSION_ID);

			GetGSM().ApplySessionId(ExistingSessionId);
		}

		const EntityViewElement* VirtualWorkerTranslatorEntity =
			GetCoordinator().GetView().Find(SpatialConstants::INITIAL_VIRTUAL_WORKER_TRANSLATOR_ENTITY_ID);
		if (VirtualWorkerTranslatorEntity != nullptr)
		{
			const ComponentData* WorkerTranslatorComponentData = VirtualWorkerTranslatorEntity->Components.FindByPredicate(
				ComponentIdEquality{ SpatialConstants::VIRTUAL_WORKER_TRANSLATION_COMPONENT_ID });
			if (WorkerTranslatorComponentData != nullptr)
			{
				// We've received worker translation data.
				TMap<VirtualWorkerId, SpatialVirtualWorkerTranslator::WorkerInformation> VirtualWorkerMapping;
				SpatialVirtualWorkerTranslator::ApplyMappingFromSchema(VirtualWorkerMapping, *WorkerTranslatorComponentData->GetFields());
				for (const auto& Mapping : VirtualWorkerMapping)
				{
					if (Mapping.Value.ServerWorkerEntityId == WorkerEntityId)
					{
						// We've received a virtual worker mapping that mentions our worker entity ID.
						LocalVirtualWorkerId = Mapping.Key;
						LocalPartitionId = Mapping.Value.PartitionEntityId;
						Stage = EStage::WaitForAssignedPartition;
					}
				}
			}
		}
	}

	if (Stage == EStage::WaitForAssignedPartition)
	{
		const EntityViewElement* AssignedPartitionEntity = GetCoordinator().GetView().Find(*LocalPartitionId);
		if (AssignedPartitionEntity != nullptr)
		{
			const EntityViewElement& VirtualWorkerTranslatorEntity =
				GetCoordinator().GetView().FindChecked(SpatialConstants::INITIAL_VIRTUAL_WORKER_TRANSLATOR_ENTITY_ID);
			const ComponentData* WorkerTranslatorComponentData = VirtualWorkerTranslatorEntity.Components.FindByPredicate(
				ComponentIdEquality{ SpatialConstants::VIRTUAL_WORKER_TRANSLATION_COMPONENT_ID });
			if (ensure(WorkerTranslatorComponentData != nullptr))
			{
				NetDriver->VirtualWorkerTranslator->ApplyMappingFromSchema(WorkerTranslatorComponentData->GetFields());

				// NOTE: Consider if waiting for partition should be a separate step from sending ReadyToBeginPlay.
				ComponentUpdate MarkAsReady(SpatialConstants::SERVER_WORKER_COMPONENT_ID);
				Schema_AddBool(MarkAsReady.GetFields(), SpatialConstants::SERVER_WORKER_READY_TO_BEGIN_PLAY_ID, true);
				GetCoordinator().SendComponentUpdate(*WorkerEntityId, MoveTemp(MarkAsReady));

				if (GetDefault<USpatialGDKSettings>()->bEnableSkeletonEntityCreation)
				{
					SkeletonEntityStep.Emplace(*NetDriver);
					Stage = EStage::CreateSkeletonEntities;
				}
				else
				{
					Stage = bHasGSMAuth ? EStage::GsmAuthDispatchGSMStartPlay : EStage::GsmNonAuthWaitForGSMStartPlay;
				}
			}
		}
	}

	if (Stage == EStage::CreateSkeletonEntities)
	{
		if (SkeletonEntityStep->TryFinish())
		{
			Stage = bHasGSMAuth ? EStage::GsmAuthDispatchGSMStartPlay : EStage::GsmNonAuthWaitForGSMStartPlay;
		}
	}

	if (Stage == EStage::GsmAuthDispatchGSMStartPlay)
	{
		const bool bAreAllWorkerEntitiesReady = Algo::AllOf(WorkerEntityIds, [this](const Worker_EntityId ServerWorkerEntityId) -> bool {
			const EntityViewElement& ServerWorkerEntity = GetCoordinator().GetView().FindChecked(ServerWorkerEntityId);
			const ComponentData* ServerWorkerComponent =
				ServerWorkerEntity.Components.FindByPredicate(ComponentIdEquality{ SpatialConstants::SERVER_WORKER_COMPONENT_ID });
			return GetBoolFromSchema(ServerWorkerComponent->GetFields(), SpatialConstants::SERVER_WORKER_READY_TO_BEGIN_PLAY_ID);
		});

		if (bAreAllWorkerEntitiesReady)
		{
			GetGSM().SendCanBeginPlayUpdate(true);

			NetDriver->CreateAndInitializeCoreClassesAfterStartup();

			GetGSM().TriggerBeginPlay();

			GetGSM().SetAcceptingPlayers(true);

			Stage = EStage::Finished;
		}
	}

	if (Stage == EStage::GsmNonAuthWaitForGSMStartPlay)
	{
		const EntityViewElement& GlobalStateManagerEntity = GetCoordinator().GetView().FindChecked(GetGSM().GlobalStateManagerEntityId);
		const ComponentData* StartupActorManagerData = GlobalStateManagerEntity.Components.FindByPredicate(
			ComponentIdEquality{ SpatialConstants::STARTUP_ACTOR_MANAGER_COMPONENT_ID });
		if (ensure(StartupActorManagerData != nullptr)
			&& Schema_GetBool(StartupActorManagerData->GetFields(), SpatialConstants::STARTUP_ACTOR_MANAGER_CAN_BEGIN_PLAY_ID))
		{
			NetDriver->CreateAndInitializeCoreClassesAfterStartup();

			GetGSM().TriggerBeginPlay();

			Stage = EStage::Finished;
		}
	}

	return Stage == EStage::Finished;
}

void FSpatialServerStartupHandler::SpawnPartitionEntity(Worker_EntityId PartitionEntityId, VirtualWorkerId VirtualWorkerId)
{
	const bool bRunStrategyWorker = USpatialStatics::IsStrategyWorkerEnabled();
	UAbstractLBStrategy* LoadBalanceStrategy = NetDriver->LoadBalanceStrategy;
	const bool bDirectAssignment = bRunStrategyWorker && !LoadBalanceStrategy->IsStrategyWorkerAware();

	UnrealServerInterestFactory* InterestF = nullptr;
	if (!bRunStrategyWorker || bDirectAssignment)
	{
		InterestF = NetDriver->InterestFactory.Get();
	}

	QueryConstraint LBConstraint = LoadBalanceStrategy->GetWorkerInterestQueryConstraint(VirtualWorkerId);

	TArray<FWorkerComponentData> Components = EntityFactory::CreatePartitionEntityComponents(
		TEXT("WorkerPartition"), PartitionEntityId, InterestF, LBConstraint, VirtualWorkerId, NetDriver->DebugCtx != nullptr);

	TArray<ComponentData> PartitionComponentsToCreate;
	Algo::Transform(Components, PartitionComponentsToCreate, [](const FWorkerComponentData& Component) {
		return ComponentData(OwningComponentDataPtr(Component.schema_type), Component.component_id);
	});

	const Worker_RequestId RequestId =
		GetCoordinator().SendCreateEntityRequest(MoveTemp(PartitionComponentsToCreate), PartitionEntityId, RETRY_UNTIL_COMPLETE);

	PartitionsToCreate.Emplace(PartitionEntityId);

	FCreateEntityDelegate OnCreateWorkerEntityResponse = ([this, PartitionEntityId](const Worker_CreateEntityResponseOp& Op) {
		PartitionsToCreate.Remove(PartitionEntityId);
	});

	EntityHandler.AddRequest(RequestId, MoveTemp(OnCreateWorkerEntityResponse));
}

bool FSpatialServerStartupHandler::TryClaimingStartupPartition()
{
	UGlobalStateManager* GlobalStateManager = &GetGSM();

	// Perform a naive leader election where we wait for the correct number of server workers to be present in the deployment, and then
	// whichever server has the lowest server worker entity ID becomes the leader and claims the snapshot partition.
	check(WorkerEntityId);

	const Worker_EntityId_Key* LowestEntityId = Algo::MinElement(WorkerEntityIds);

	check(LowestEntityId != nullptr);

	if (WorkerEntityId == *LowestEntityId)
	{
		UE_LOG(LogSpatialStartupHandler, Log, TEXT("MaybeClaimSnapshotPartition claiming snapshot partition"));
		GlobalStateManager->ClaimSnapshotPartition();
		return true;
	}
	UE_LOG(LogSpatialStartupHandler, Log, TEXT("Not claiming snapshot partition"));
	return false;
}

ViewCoordinator& FSpatialServerStartupHandler::GetCoordinator()
{
	return NetDriver->Connection->GetCoordinator();
}

const ViewCoordinator& FSpatialServerStartupHandler::GetCoordinator() const
{
	return NetDriver->Connection->GetCoordinator();
}

const TArray<Worker_Op>& FSpatialServerStartupHandler::GetOps() const
{
	return GetCoordinator().GetWorkerMessages();
}

UGlobalStateManager& FSpatialServerStartupHandler::GetGSM()
{
	return *NetDriver->GlobalStateManager;
}

FString FSpatialServerStartupHandler::GetStartupStateDescription() const
{
	switch (Stage)
	{
	case EStage::CreateWorkerEntity:
		return TEXT("Creating a worker entity");
	case EStage::WaitForWorkerEntities:
		return TEXT("Waiting for all worker entities to be visible");
	case EStage::WaitForGSMEntity:
		return TEXT("Waiting for GSM entity to become visible");
	case EStage::DeriveDeploymentRecoveryState:
		return TEXT("Deriving deployment recovery state");
	case EStage::TryClaimingGSMEntityAuthority:
		return TEXT("Electing GSM auth worker");
	case EStage::WaitForGSMEntityAuthority:
		return TEXT("Waiting to receive GSM entity authority");
	case EStage::GsmAuthFillWorkerTranslationState:
		return TEXT("Filling virtual worker translation state");
	case EStage::GsmAuthCreatePartitions:
		return TEXT("Creating partitions for workers");
	case EStage::GsmAuthWaitForPartitionsVisibility:
		return TEXT("Waiting for partitions to be created");
	case EStage::GsmAuthAssignPartitionsToVirtualWorkers:
		return TEXT("Assigning worker partitions to virtual workers");
	case EStage::GetVirtualWorkerTranslationState:
		return TEXT("Waiting for virtual worker ID to physical worker and worker partition mapping");
	case EStage::WaitForAssignedPartition:
		return TEXT("Waiting to see the assgned partition");
	case EStage::CreateSkeletonEntities:
		return TEXT("Creating skeleton entities");
	case EStage::GsmAuthDispatchGSMStartPlay:
		return TEXT("Waiting for all workers to confirm receipt of worker partitions");
	case EStage::GsmNonAuthWaitForGSMStartPlay:
		return TEXT("Waiting for the GSM worker to allow start play");
	case EStage::Finished:
		return TEXT("Finished");
	default:
		checkNoEntry();
	}
	return TEXT("Invalid state");
}

} // namespace SpatialGDK
