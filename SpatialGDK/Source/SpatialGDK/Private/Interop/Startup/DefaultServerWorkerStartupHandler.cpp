#include "Interop/Startup/DefaultServerWorkerStartupHandler.h"

#include "Algo/MinElement.h"
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
			ServerWorkerEntityCreator1.Emplace(*NetDriver, *NetDriver->Connection);
		}
		ServerWorkerEntityCreator1->ProcessOps(GetOps());
		if (ServerWorkerEntityCreator1->IsFinished())
		{
			WorkerEntityId = ServerWorkerEntityCreator1->GetWorkerEntityId();
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
			// HACK
			GetGSM().SetDeploymentState();
			Stage = EStage::FillWorkerTranslationState;
		}
	}

	if (Stage == EStage::FillWorkerTranslationState)
	{
		const EntityViewElement* VirtualWorkerTranslatorEntity =
			GetCoordinator().GetView().Find(SpatialConstants::INITIAL_VIRTUAL_WORKER_TRANSLATOR_ENTITY_ID);
		if (VirtualWorkerTranslatorEntity != nullptr)
		{
			const ComponentData* VirtualWorkerTranslatorComponent = VirtualWorkerTranslatorEntity->Components.FindByPredicate(
				ComponentIdEquality{ SpatialConstants::VIRTUAL_WORKER_TRANSLATION_COMPONENT_ID });
			if (ensure(VirtualWorkerTranslatorComponent != nullptr))
			{
				SpatialVirtualWorkerTranslator::ApplyMappingFromSchema(WorkersToPartitions, *VirtualWorkerTranslatorComponent->GetFields());
				Algo::Transform(WorkersToPartitions, WorkerPartitions,
								[](const TPair<VirtualWorkerId, SpatialVirtualWorkerTranslator::WorkerInformation>& Pair) {
									return Pair.Value.PartitionEntityId;
								});
				const bool bNeedToCreatePartitions = WorkerPartitions.Num() < Setup.ExpectedServerWorkersCount;

				Stage = bNeedToCreatePartitions ? EStage::AuthCreatePartitions : EStage::WaitForAuthPartitionsVisibility;
			}
		}
	}

	if (Stage == EStage::AuthCreatePartitions)
	{
		UE_LOG(LogSpatialStartupHandler, Log, TEXT("Spawning partition entities for %d virtual workers"), Setup.ExpectedServerWorkersCount);
		for (VirtualWorkerId GeneratedVirtualWorkerId = 1;
			 GeneratedVirtualWorkerId <= static_cast<VirtualWorkerId>(Setup.ExpectedServerWorkersCount); ++GeneratedVirtualWorkerId)
		{
			if (WorkersToPartitions.Contains(GeneratedVirtualWorkerId))
			{
				// This virtual worker mapping already exists, no need to create a partition.
				continue;
			}

			PhysicalWorkerName WorkerName = TEXT("DUMMY");

			const ComponentData* ServerWorkerData =
				GetCoordinator().GetComponent(WorkerEntityIds[GeneratedVirtualWorkerId - 1], SpatialConstants::SERVER_WORKER_COMPONENT_ID);
			if (ensure(ServerWorkerData != nullptr))
			{
				WorkerName = GetStringFromSchema(ServerWorkerData->GetFields(), SpatialConstants::SERVER_WORKER_NAME_ID);
			}

			const Worker_EntityId PartitionEntityId = NetDriver->PackageMap->AllocateNewEntityId();
			UE_LOG(LogSpatialStartupHandler, Log, TEXT("- Virtual Worker: %d. Entity: %lld. "), GeneratedVirtualWorkerId,
				   PartitionEntityId);
			SpawnPartitionEntity(PartitionEntityId, GeneratedVirtualWorkerId);
			WorkersToPartitions.Emplace(GeneratedVirtualWorkerId,
										SpatialVirtualWorkerTranslator::WorkerInformation{
											WorkerName, WorkerEntityIds[GeneratedVirtualWorkerId - 1], PartitionEntityId });
		}

		Stage = EStage::WaitForAuthPartitionsCreated;
	}

	if (Stage == EStage::WaitForAuthPartitionsCreated)
	{
		EntityHandler.ProcessOps(GetOps());

		// This checks for whether all create requests finished, NOT whether the partitions are in view.
		const bool bWereAllPartitionsCreated = PartitionsToCreate.Num() == 0;

		if (bWereAllPartitionsCreated)
		{
			Stage = EStage::AssignPartitionsToVirtualWorkers;
		}
	}

	if (Stage == EStage::WaitForAuthPartitionsVisibility)
	{
		const bool bAreAllPartitionsInView = Algo::AllOf(
			WorkersToPartitions,
			[View = &GetCoordinator().GetView()](const TPair<VirtualWorkerId, SpatialVirtualWorkerTranslator::WorkerInformation>& Worker) {
				return View->Contains(Worker.Value.PartitionEntityId);
			});

		if (bAreAllPartitionsInView)
		{
			Stage = EStage::AssignPartitionsToVirtualWorkers;
		}
	}

	if (Stage == EStage::AssignPartitionsToVirtualWorkers)
	{
		TMap<Worker_EntityId_Key, const ComponentData*> WorkerComponents;
		Algo::Transform(WorkerEntityIds, WorkerComponents, [this](const Worker_EntityId EntityId) {
			return TPair<Worker_EntityId_Key, const ComponentData*>{
				EntityId, GetCoordinator().GetView()[EntityId].Components.FindByPredicate(
							  ComponentIdEquality{ SpatialConstants::SERVER_WORKER_COMPONENT_ID })
			};
		});

		ComponentUpdate Update(SpatialConstants::VIRTUAL_WORKER_TRANSLATION_COMPONENT_ID);
		for (const auto& Entry : WorkersToPartitions)
		{
			// Assign the worker's partition to its system entity.
			const ComponentData* ServerWorkerComponentData = WorkerComponents.FindChecked(Entry.Value.ServerWorkerEntityId);
			const ServerWorker ServerWorkerData(ServerWorkerComponentData->GetUnderlying());
			ClaimHandler.ClaimPartition(GetCoordinator(), ServerWorkerData.SystemEntityId, Entry.Value.PartitionEntityId);

			// Reflect the partition assignment in the translator object.
			Schema_Object* EntryObject = Schema_AddObject(Update.GetFields(), SpatialConstants::VIRTUAL_WORKER_TRANSLATION_MAPPING_ID);
			Schema_AddUint32(EntryObject, SpatialConstants::MAPPING_VIRTUAL_WORKER_ID, Entry.Key);
			AddStringToSchema(EntryObject, SpatialConstants::MAPPING_PHYSICAL_WORKER_NAME_ID, Entry.Value.WorkerName);
			Schema_AddEntityId(EntryObject, SpatialConstants::MAPPING_SERVER_WORKER_ENTITY_ID, Entry.Value.ServerWorkerEntityId);
			Schema_AddEntityId(EntryObject, SpatialConstants::MAPPING_PARTITION_ID, Entry.Value.PartitionEntityId);
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
					Stage = bHasGSMAuth ? EStage::DispatchGSMStartPlay : EStage::WaitForGSMStartPlay;
				}
			}
		}
	}

	if (Stage == EStage::CreateSkeletonEntities)
	{
		if (SkeletonEntityStep->TryFinish())
		{
			Stage = bHasGSMAuth ? EStage::DispatchGSMStartPlay : EStage::WaitForGSMStartPlay;
		}
	}

	if (Stage == EStage::DispatchGSMStartPlay)
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

	if (Stage == EStage::WaitForGSMStartPlay)
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

	SpatialGDK::QueryConstraint LBConstraint = LoadBalanceStrategy->GetWorkerInterestQueryConstraint(VirtualWorkerId);

	TArray<FWorkerComponentData> Components = EntityFactory::CreatePartitionEntityComponents(
		TEXT("WorkerPartition"), PartitionEntityId, InterestF, LBConstraint, VirtualWorkerId, NetDriver->DebugCtx != nullptr);
	TArray<ComponentData> PartitionComponentsToCreate;
	Algo::Transform(Components, PartitionComponentsToCreate, [](const FWorkerComponentData& Component) {
		return ComponentData(OwningComponentDataPtr(Component.schema_type), Component.component_id);
	});
	const Worker_RequestId RequestId = GetCoordinator().SendCreateEntityRequest(MoveTemp(PartitionComponentsToCreate), PartitionEntityId,
																				SpatialGDK::RETRY_UNTIL_COMPLETE);

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
} // namespace SpatialGDK
