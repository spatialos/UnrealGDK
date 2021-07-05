#include "SpatialStartupHandler.h"

#include "Algo/Copy.h"
#include "Algo/MinElement.h"
#include "EngineClasses/SpatialNetDriver.h"
#include "EngineClasses/SpatialPackageMapClient.h"
#include "EngineClasses/SpatialVirtualWorkerTranslator.h"
#include "SpatialView/EntityComponentTypes.h"
#include "Utils/EntityFactory.h"
#include "Utils/InterestFactory.h"

DEFINE_LOG_CATEGORY_STATIC(LogSpatialStartupHandler, Log, All);

namespace SpatialGDK
{
FSpatialStartupHandler::FSpatialStartupHandler(USpatialNetDriver& InNetDriver, const FInitialSetup& InSetup)
	: Setup(InSetup)
	, NetDriver(&InNetDriver)
{
}

bool FSpatialStartupHandler::TryFinishStartup()
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
			Stage = EStage::TryClaimingGSMEntityAuthority;
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
			Stage = EStage::FillWorkerTranslationState;
		}
	}

	if (Stage == EStage::FillWorkerTranslationState)
	{
		if (!bHasCalledPartitionEntityCreate)
		{
			bHasCalledPartitionEntityCreate = true;
			TMap<Worker_EntityId_Key, const ComponentData*> WorkerComponents;
			Algo::Transform(WorkerEntityIds, WorkerComponents, [this](const Worker_EntityId WorkerEntityId) {
				return TPair<Worker_EntityId_Key, const ComponentData*>{
					WorkerEntityId, GetCoordinator().GetView()[WorkerEntityId].Components.FindByPredicate(
										ComponentIdEquality{ SpatialConstants::SERVER_WORKER_COMPONENT_ID })
				};
			});

			UE_LOG(LogSpatialStartupHandler, Log, TEXT("Spawning partition entities for %d virtual workers"),
				   Setup.ExpectedServerWorkersCount);
			for (VirtualWorkerId GeneratedVirtualWorkerId = 1;
				 GeneratedVirtualWorkerId <= static_cast<VirtualWorkerId>(Setup.ExpectedServerWorkersCount); ++GeneratedVirtualWorkerId)
			{
				const Worker_EntityId PartitionEntityId = NetDriver->PackageMap->AllocateNewEntityId();
				UE_LOG(LogSpatialStartupHandler, Log, TEXT("- Virtual Worker: %d. Entity: %lld. "), GeneratedVirtualWorkerId,
					   PartitionEntityId);
				SpawnPartitionEntity(PartitionEntityId, GeneratedVirtualWorkerId);
				WorkersToPartitions.Emplace(GeneratedVirtualWorkerId,
											SpatialVirtualWorkerTranslator::WorkerInformation{
												TEXT("DUMMY"), WorkerEntityIds[GeneratedVirtualWorkerId - 1], PartitionEntityId });
			}
		}

		EntityHandler.ProcessOps(GetOps());

		if (PartitionsToCreate.Num() == 0)
		{
			// All partitions created.
			ComponentUpdate Update(SpatialConstants::VIRTUAL_WORKER_TRANSLATION_COMPONENT_ID);
			for (const auto& Entry : WorkersToPartitions)
			{
				Schema_Object* EntryObject = Schema_AddObject(Update.GetFields(), SpatialConstants::VIRTUAL_WORKER_TRANSLATION_MAPPING_ID);
				Schema_AddUint32(EntryObject, SpatialConstants::MAPPING_VIRTUAL_WORKER_ID, Entry.Key);
				AddStringToSchema(EntryObject, SpatialConstants::MAPPING_PHYSICAL_WORKER_NAME_ID, Entry.Value.WorkerName);
				Schema_AddEntityId(EntryObject, SpatialConstants::MAPPING_SERVER_WORKER_ENTITY_ID, Entry.Value.ServerWorkerEntityId);
				Schema_AddEntityId(EntryObject, SpatialConstants::MAPPING_PARTITION_ID, Entry.Value.PartitionEntityId);
			}
			GetCoordinator().SendComponentUpdate(SpatialConstants::INITIAL_VIRTUAL_WORKER_TRANSLATOR_ENTITY_ID, MoveTemp(Update));
			Stage = EStage::GetVirtualWorkerTranslationState;
		}
	}

	if (Stage == EStage::GetVirtualWorkerTranslationState)
	{
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
		const EntityViewElement* AssignedPartitionEntity = GetCoordinator().GetView().Find(LocalPartitionId);
		if (AssignedPartitionEntity != nullptr)
		{
			// NOTE: Consider if waiting for partition should be a separate step from sending ReadyToBeginPlay.
			ComponentUpdate MarkAsReady(SpatialConstants::SERVER_WORKER_COMPONENT_ID);
			Schema_AddBool(MarkAsReady.GetFields(), SpatialConstants::SERVER_WORKER_READY_TO_BEGIN_PLAY_ID, true);
			GetCoordinator().SendComponentUpdate(WorkerEntityId, MoveTemp(MarkAsReady));

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
			Stage = EStage::Finished;
		}
	}

	return Stage == EStage::Finished;
}

void FSpatialStartupHandler::SpawnPartitionEntity(Worker_EntityId PartitionEntityId, VirtualWorkerId VirtualWorkerId)
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

	CreateEntityDelegate OnCreateWorkerEntityResponse;
	OnCreateWorkerEntityResponse.BindLambda([this, PartitionEntityId](const Worker_CreateEntityResponseOp& Op) {
		PartitionsToCreate.Remove(PartitionEntityId);
	});

	EntityHandler.AddRequest(RequestId, MoveTemp(OnCreateWorkerEntityResponse));
}

bool FSpatialStartupHandler::TryClaimingStartupPartition()
{
	UGlobalStateManager* GlobalStateManager = &GetGSM();

	// Perform a naive leader election where we wait for the correct number of server workers to be present in the deployment, and then
	// whichever server has the lowest server worker entity ID becomes the leader and claims the snapshot partition.
	const Worker_EntityId LocalServerWorkerEntityId = WorkerEntityId;

	check(LocalServerWorkerEntityId != SpatialConstants::INVALID_ENTITY_ID);

	const Worker_EntityId_Key* LowestEntityId = Algo::MinElement(WorkerEntityIds);

	check(LowestEntityId != nullptr);

	if (LocalServerWorkerEntityId == *LowestEntityId)
	{
		UE_LOG(LogSpatialStartupHandler, Log, TEXT("MaybeClaimSnapshotPartition claiming snapshot partition"));
		GlobalStateManager->ClaimSnapshotPartition();
		return true;
	}
	UE_LOG(LogSpatialStartupHandler, Log, TEXT("Not claiming snapshot partition"));
	return false;
}

ViewCoordinator& FSpatialStartupHandler::GetCoordinator()
{
	return NetDriver->Connection->GetCoordinator();
}

const ViewCoordinator& FSpatialStartupHandler::GetCoordinator() const
{
	return NetDriver->Connection->GetCoordinator();
}

const TArray<Worker_Op>& FSpatialStartupHandler::GetOps() const
{
	return GetCoordinator().GetWorkerMessages();
}

UGlobalStateManager& FSpatialStartupHandler::GetGSM()
{
	return *NetDriver->GlobalStateManager;
}
} // namespace SpatialGDK
