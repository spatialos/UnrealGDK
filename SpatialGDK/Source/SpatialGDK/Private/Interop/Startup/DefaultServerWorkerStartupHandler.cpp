#include "Interop/Startup/DefaultServerWorkerStartupHandler.h"

#include "Algo/AllOf.h"
#include "Algo/MinElement.h"
#include "Algo/Transform.h"
#include "EngineClasses/SpatialNetDriver.h"
#include "EngineClasses/SpatialPackageMapClient.h"
#include "HAL/MallocStomp.h"
#include "Interop/GlobalStateManager.h"
#include "LoadBalancing/AbstractLBStrategy.h"
#include "Schema/ServerWorker.h"
#include "Utils/EntityFactory.h"
#include "Utils/InterestFactory.h"
#include "Utils/SpatialStatics.h"

DEFINE_LOG_CATEGORY_STATIC(LogSpatialStartupHandler, Log, All);

namespace SpatialGDK
{
FStartupExecutor::FStartupExecutor(TArray<FStartupStep> InSteps)
	: Steps(MoveTemp(InSteps))
{
	if (Steps.Num() > 0)
	{
		Steps[0].OnStepStarted();
	}
}

bool FStartupExecutor::TryFinishStartup()
{
	while (Steps.Num() > 0)
	{
		const FStartupStep& CurrentStep = Steps[0];
		if (CurrentStep.TryFinishStep())
		{
			Steps.RemoveAt(0);
			if (Steps.Num() > 0)
			{
				Steps[0].OnStepStarted();
			}
		}
		else
		{
			return false;
		}
	}

	return true;
}

template <typename TStep>
struct TStartupStep : public TStep
{
	explicit TStartupStep(TStep Step)
		: TStep(MoveTemp(Step))
	{
	}

	operator FStartupStep() &&
	{
		auto UniqueStep = MakeUnique<TStartupStep<TStep>>(*this);
		auto RawStep = UniqueStep.Get();
		return FStartupStep{ [Step = MoveTemp(UniqueStep)] {
								Step->OnStart();
							},
							 [RawStep] {
								 return RawStep->TryFinish();
							 } };
	}
};

TArray<FStartupStep> FSpatialServerStartupHandler::CreateSteps(USpatialNetDriver& InNetDriver, const FInitialSetup& InSetup)
{
	struct FCreateServerWorkerEntityStep
	{
		void OnStart() { EntityCreator.Emplace(*NetDriver, *NetDriver->Connection); }

		bool TryFinish()
		{
			EntityCreator->ProcessOps(NetDriver->Connection->GetWorkerMessages());
			if (EntityCreator->IsFinished())
			{
				*WorkerEntityIdAddress = EntityCreator->GetWorkerEntityId();
				return true;
			}
			return false;
		}

		USpatialNetDriver* NetDriver;
		TOptional<Worker_EntityId>* WorkerEntityIdAddress;
		TOptional<ServerWorkerEntityCreator> EntityCreator;
	};

	struct FWaitForServerWorkerEntitiesStep
	{
		void OnStart() {}

		bool TryFinish()
		{
			TMap<Worker_EntityId_Key, const ComponentData*> WorkerComponents;

			for (const auto& EntityData : Worker->GetView())
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
				WorkerComponents.GetKeys(*WorkerEntityIdsAddress);
				return true;
			}
			return false;
		}

		ISpatialOSWorker* Worker;
		FInitialSetup Setup;

		TArray<Worker_EntityId>* WorkerEntityIdsAddress;
	};

	struct FWaitForGsmEntityStep
	{
		void OnStart() {}
		bool TryFinish() { return Worker->HasEntity(SpatialConstants::INITIAL_GLOBAL_STATE_MANAGER_ENTITY_ID); }

		ISpatialOSWorker* Worker;
	};

	struct FDeriveDeploymentRecoveryStateStep
	{
		void OnStart() {}
		bool TryFinish()
		{
			const EntityViewElement& GlobalStateManagerEntity =
				Worker->GetView().FindChecked(SpatialConstants::INITIAL_GLOBAL_STATE_MANAGER_ENTITY_ID);
			const ComponentData* StartupActorManagerData = GlobalStateManagerEntity.Components.FindByPredicate(
				ComponentIdEquality{ SpatialConstants::STARTUP_ACTOR_MANAGER_COMPONENT_ID });
			if (StartupActorManagerData != nullptr)
			{
				if (Schema_GetBoolCount(StartupActorManagerData->GetFields(), SpatialConstants::STARTUP_ACTOR_MANAGER_CAN_BEGIN_PLAY_ID)
					!= 0)
				{
					// StartupActorManager's CAN_BEGIN_PLAY is raised after startup is finished, and is false in the initial snapshot.
					// If it's true at this point, then this worker is either recovering or loading from a non-initial snapshot.
					const bool bIsRecoveringOrSnapshot =
						GetBoolFromSchema(StartupActorManagerData->GetFields(), SpatialConstants::STARTUP_ACTOR_MANAGER_CAN_BEGIN_PLAY_ID);

					*bIsRecoveringOrSnapshotPtr = bIsRecoveringOrSnapshot;

					// We should only call BeginPlay on actors if this is a fresh deployment; otherwise, we assume that
					// BeginPlay has already been called on them before.
					NetDriver->GlobalStateManager->bCanSpawnWithAuthority = !bIsRecoveringOrSnapshot;

					return true;
				}
			}
			return false;
		}

		USpatialNetDriver* NetDriver;
		ISpatialOSWorker* Worker;
		bool* bIsRecoveringOrSnapshotPtr;
	};

	FStartupStep Dummy{ [] {},
						[] {
							return true;
						} };
	FStartupStep SetNextLegacyStep{ [] {},
									[StagePtr = &Stage] {
										*StagePtr = EStage::TryClaimingGSMEntityAuthority;
										return true;
									} };

	TArray<FStartupStep> Steps;
	Steps.Emplace(MoveTemp(Dummy));

	Steps.Emplace(TStartupStep<FCreateServerWorkerEntityStep>({ &InNetDriver, &WorkerEntityId }));
	Steps.Emplace(TStartupStep<FWaitForServerWorkerEntitiesStep>({ &InNetDriver.Connection->GetCoordinator(), InSetup, &WorkerEntityIds }));
	Steps.Emplace(TStartupStep<FWaitForGsmEntityStep>({ &InNetDriver.Connection->GetCoordinator() }));
	Steps.Emplace(TStartupStep<FDeriveDeploymentRecoveryStateStep>(
		{ &InNetDriver, &InNetDriver.Connection->GetCoordinator(), &bIsRecoveringOrSnapshot }));

	Steps.Emplace(MoveTemp(SetNextLegacyStep));
	return MoveTemp(Steps);
}

FSpatialServerStartupHandler::FSpatialServerStartupHandler(USpatialNetDriver& InNetDriver, const FInitialSetup& InSetup)
	: Executor(CreateSteps(InNetDriver, InSetup))
	, Setup(InSetup)
	, NetDriver(&InNetDriver)
{
}

bool FSpatialServerStartupHandler::TryFinishStartup()
{
	if (!Executor.TryFinishStartup())
	{
		return false;
	}

	if (Stage == EStage::WaitForWorkerEntities) {}

	if (Stage == EStage::WaitForGSMEntity) {}

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

				Stage = bNeedToCreatePartitions ? EStage::GsmAuthCreatePartitions : EStage::GsmAuthWaitForPartitionsVisibility;
			}
		}
	}

	if (Stage == EStage::GsmAuthCreatePartitions)
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

		Stage = EStage::GsmAuthWaitForPartitionsVisibility;
	}

	if (Stage == EStage::GsmAuthWaitForPartitionsVisibility)
	{
		const bool bAreAllPartitionsInView = Algo::AllOf(
			WorkersToPartitions,
			[View = &GetCoordinator().GetView()](const TPair<VirtualWorkerId, SpatialVirtualWorkerTranslator::WorkerInformation>& Worker) {
				return View->Contains(Worker.Value.PartitionEntityId);
			});

		if (bAreAllPartitionsInView)
		{
			Stage = EStage::GsmAuthAssignPartitionsToVirtualWorkers;
		}
	}

	if (Stage == EStage::GsmAuthAssignPartitionsToVirtualWorkers)
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
