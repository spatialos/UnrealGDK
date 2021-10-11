#include "Interop/Startup/PartitionCreationStartupSteps.h"

#include "Algo/AllOf.h"
#include "Algo/MinElement.h"
#include "EngineClasses/SpatialNetDriver.h"
#include "EngineClasses/SpatialPackageMapClient.h"
#include "Interop/GlobalStateManager.h"
#include "LoadBalancing/AbstractLBStrategy.h"
#include "Schema/ServerWorker.h"
#include "SpatialView/EntityComponentTypes.h"
#include "Utils/EntityFactory.h"
#include "Utils/InterestFactory.h"
#include "Utils/SpatialStatics.h"

namespace SpatialGDK
{
class FAuthCreateAndAssignPartitions::FCreateNecessaryPartitionsStep : public FStartupStep
{
public:
	FCreateNecessaryPartitionsStep(TSharedRef<FPartitionCreationSharedState> InSharedState, FInitialSetup InSetup,
								   USpatialNetDriver& InNetDriver, ISpatialOSWorker& InConnection)
		: SharedState(InSharedState)
		, Setup(InSetup)
		, NetDriver(&InNetDriver)
		, Connection(&InConnection)
	{
		StepName = TEXT("Creating worker partitions");
	}

	virtual void Start() override;

	void SpawnPartitionEntity(Worker_EntityId PartitionEntityId, VirtualWorkerId VirtualWorkerId);

private:
	TSharedRef<FPartitionCreationSharedState> SharedState;

	FInitialSetup Setup;
	USpatialNetDriver* NetDriver;
	ISpatialOSWorker* Connection;
};

class FAuthCreateAndAssignPartitions::FWaitForPartitionVisibilityStep : public FStartupStep
{
public:
	FWaitForPartitionVisibilityStep(TSharedRef<const FPartitionCreationSharedState> SharedState, ISpatialOSWorker& InConnection)
		: SharedState(SharedState)
		, Connection(&InConnection)
	{
		StepName = TEXT("Waiting for worker partitions to be visible");
	}

	virtual bool TryFinish() override;

private:
	TSharedRef<const FPartitionCreationSharedState> SharedState;
	ISpatialOSWorker* Connection;
};

class FAuthCreateAndAssignPartitions::FAssignPartitionsToWorkersStep : public FStartupStep
{
public:
	FAssignPartitionsToWorkersStep(TSharedRef<const FServerWorkerStartupContext> InState,
								   TSharedRef<const FPartitionCreationSharedState> InSharedState, FInitialSetup InSetup,
								   ISpatialOSWorker& InConnection)
		: State(InState)
		, SharedState(InSharedState)
		, Setup(InSetup)
		, Connection(&InConnection)
	{
		StepName = TEXT("Assigning worker partitions to workers");
	}

	virtual bool TryFinish() override;

private:
	TSharedRef<const FServerWorkerStartupContext> State;
	TSharedRef<const FPartitionCreationSharedState> SharedState;
	FInitialSetup Setup;

	ISpatialOSWorker* Connection;

	FClaimPartitionHandler ClaimHandler;
};

FAuthCreateAndAssignPartitions::FAuthCreateAndAssignPartitions(TSharedRef<const FServerWorkerStartupContext> InState,
															   const FInitialSetup& InSetup, USpatialNetDriver& InNetDriver,
															   ISpatialOSWorker& InConnection)
	: State(InState)
	, Setup(InSetup)
	, NetDriver(&InNetDriver)
	, Connection(&InConnection)
	, SharedState(MakeShared<FPartitionCreationSharedState>())
	, Executor(CreateSteps())
{
}

TArray<TUniquePtr<FStartupStep>> FAuthCreateAndAssignPartitions::CreateSteps()
{
	TArray<TUniquePtr<FStartupStep>> Steps;

	Steps.Emplace(MakeUnique<FDiscoverExistingPartitionsStep>(SharedState, *Connection, SpatialConstants::WORKER_PARTITION_TAG_COMPONENT_ID,
															  TArray<Worker_ComponentId>()));
	Steps.Emplace(MakeUnique<FCreateNecessaryPartitionsStep>(SharedState, Setup, *NetDriver, *Connection));
	Steps.Emplace(MakeUnique<FWaitForPartitionVisibilityStep>(SharedState, *Connection));
	Steps.Emplace(MakeUnique<FAssignPartitionsToWorkersStep>(State, SharedState, Setup, *Connection));

	return Steps;
}

bool FAuthCreateAndAssignPartitions::TryFinish()
{
	if (!State->bHasGSMAuth)
	{
		return true;
	}
	return Executor.TryFinish();
}

void FDiscoverExistingPartitionsStep::Start()
{
	Worker_Constraint WorkerPartitionConstraint;
	WorkerPartitionConstraint.constraint.component_constraint.component_id = PartitionComponentTag;
	WorkerPartitionConstraint.constraint_type = Worker_ConstraintType::WORKER_CONSTRAINT_TYPE_COMPONENT;

	Worker_EntityQuery PartitionsQuery{};

	PartitionsQuery.constraint = WorkerPartitionConstraint;

	PartitionsQuery.snapshot_result_type_component_ids = PartitionComponents.GetData();
	PartitionsQuery.snapshot_result_type_component_id_count = PartitionComponents.Num();

	const Worker_RequestId QueryId = Connection->SendEntityQueryRequest(EntityQuery(PartitionsQuery), RETRY_UNTIL_COMPLETE);

	PartitionQueryHandler.AddRequest(QueryId, [this](const Worker_EntityQueryResponseOp& QueryResponse) {
		OnPartitionQueryComplete(QueryResponse);
	});
}

void FDiscoverExistingPartitionsStep::OnPartitionQueryComplete(const Worker_EntityQueryResponseOp& QueryResponse)
{
	if (ensure(QueryResponse.status_code == WORKER_STATUS_CODE_SUCCESS))
	{
		const int32 WorkerPartitionsCount = QueryResponse.result_count;

		TMap<Worker_EntityId_Key, EntityViewElement> DiscoveredPartitionEntityIds;
		DiscoveredPartitionEntityIds.Reserve(WorkerPartitionsCount);

		for (int32 WorkerPartitionIndex = 0; WorkerPartitionIndex < WorkerPartitionsCount; ++WorkerPartitionIndex)
		{
			EntityViewElement Components;
			const Worker_Entity& PartitionEntity = QueryResponse.results[WorkerPartitionIndex];
			for (uint32 ComponentIdx = 0; ComponentIdx < PartitionEntity.component_count; ++ComponentIdx)
			{
				const Worker_ComponentData& WorkerData = PartitionEntity.components[ComponentIdx];
				ComponentData Data = ComponentData::CreateCopy(WorkerData.schema_type, WorkerData.component_id);
				Components.Components.Add(MoveTemp(Data));
			}
			DiscoveredPartitionEntityIds.Emplace(PartitionEntity.entity_id, MoveTemp(Components));
		}

		SharedState->DiscoveredPartitionEntityIds = MoveTemp(DiscoveredPartitionEntityIds);

		bQueryComplete = true;
	}
}

bool FDiscoverExistingPartitionsStep::TryFinish()
{
	PartitionQueryHandler.ProcessOps(Connection->GetWorkerMessages());
	return bQueryComplete;
}

void FAuthCreateAndAssignPartitions::FCreateNecessaryPartitionsStep::Start()
{
	if (SharedState->DiscoveredPartitionEntityIds.Num() >= Setup.ExpectedServerWorkersCount)
	{
		return;
	}

	const int32 PreexistingPartitionsCount = SharedState->DiscoveredPartitionEntityIds.Num();

	const int32 PartitionsToSpawn = Setup.ExpectedServerWorkersCount - PreexistingPartitionsCount;
	UE_LOG(LogSpatialStartupHandler, Log, TEXT("Spawning partition entities for %d virtual workers (need to spawn %d)"),
		   Setup.ExpectedServerWorkersCount, PartitionsToSpawn);

	TArray<Worker_EntityId> NewPartitionEntityIds;
	NewPartitionEntityIds.Reserve(PartitionsToSpawn);
	for (int32 PartitionIndex = 0; PartitionIndex < PartitionsToSpawn; ++PartitionIndex)
	{
		const VirtualWorkerId AssumedPartitionWorkerId = 1 + PreexistingPartitionsCount + PartitionIndex;
		const Worker_EntityId PartitionEntityId = NetDriver->PackageMap->AllocateNewEntityId();
		UE_LOG(LogSpatialStartupHandler, Log, TEXT("- Virtual Worker: %d. Entity: %lld. "), AssumedPartitionWorkerId, PartitionEntityId);
		SpawnPartitionEntity(PartitionEntityId, AssumedPartitionWorkerId);
		NewPartitionEntityIds.Emplace(PartitionEntityId);
	}

	SharedState->NewCreatedPartitionEntityIds = MoveTemp(NewPartitionEntityIds);
}

void FAuthCreateAndAssignPartitions::FCreateNecessaryPartitionsStep::SpawnPartitionEntity(Worker_EntityId PartitionEntityId,
																						  VirtualWorkerId VirtualWorkerId)
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

	Connection->SendCreateEntityRequest(MoveTemp(PartitionComponentsToCreate), PartitionEntityId, RETRY_UNTIL_COMPLETE);
}

bool FAuthCreateAndAssignPartitions::FWaitForPartitionVisibilityStep::TryFinish()
{
	const auto IsPartitionEntityVisible = [View = &Connection->GetView()](const Worker_EntityId& PartitionEntityId) {
		return View->Contains(PartitionEntityId);
	};
	TArray<Worker_EntityId_Key> PartitionEntities;
	SharedState->DiscoveredPartitionEntityIds.GetKeys(PartitionEntities);
	const bool bAreAllDiscoveredPartitionsInView = Algo::AllOf(PartitionEntities, IsPartitionEntityVisible);
	const bool bAreAllCreatedPartitionsInView = Algo::AllOf(SharedState->NewCreatedPartitionEntityIds, IsPartitionEntityVisible);

	return bAreAllDiscoveredPartitionsInView && bAreAllCreatedPartitionsInView;
}

bool FAuthCreateAndAssignPartitions::FAssignPartitionsToWorkersStep::TryFinish()
{
	const EntityViewElement* VirtualWorkerTranslatorEntity =
		Connection->GetView().Find(SpatialConstants::INITIAL_VIRTUAL_WORKER_TRANSLATOR_ENTITY_ID);

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
	Algo::Transform(State->WorkerEntityIds, WorkerComponents, [this](const Worker_EntityId EntityId) {
		return Connection->GetView()[EntityId].Components.FindByPredicate(
			ComponentIdEquality{ SpatialConstants::SERVER_WORKER_COMPONENT_ID });
	});

	TArray<Worker_PartitionId> WorkerPartitions;
	SharedState->DiscoveredPartitionEntityIds.GetKeys(WorkerPartitions);
	WorkerPartitions.Append(SharedState->NewCreatedPartitionEntityIds);

	ComponentUpdate Update(SpatialConstants::VIRTUAL_WORKER_TRANSLATION_COMPONENT_ID);

	for (int32 WorkerIndex = 0; WorkerIndex < Setup.ExpectedServerWorkersCount; ++WorkerIndex)
	{
		const ComponentData* ServerWorkerData = WorkerComponents[WorkerIndex];

		const ServerWorker WorkerData(ServerWorkerData->GetUnderlying());

		const Worker_EntityId ServerWorkerEntityId = State->WorkerEntityIds[WorkerIndex];
		const Worker_EntityId PartitionEntityId = WorkerPartitions[WorkerIndex];

		const VirtualWorkerId WorkerId = 1 + WorkerIndex;

		// Assign the worker's partition to its system entity.
		ClaimHandler.ClaimPartition(*Connection, WorkerData.SystemEntityId, PartitionEntityId);

		// Reflect the partition assignment in the translator object.
		Schema_Object* EntryObject = Schema_AddObject(Update.GetFields(), SpatialConstants::VIRTUAL_WORKER_TRANSLATION_MAPPING_ID);
		Schema_AddUint32(EntryObject, SpatialConstants::MAPPING_VIRTUAL_WORKER_ID, WorkerId);
		AddStringToSchema(EntryObject, SpatialConstants::MAPPING_PHYSICAL_WORKER_NAME_ID, WorkerData.WorkerName);
		Schema_AddEntityId(EntryObject, SpatialConstants::MAPPING_SERVER_WORKER_ENTITY_ID, ServerWorkerEntityId);
		Schema_AddEntityId(EntryObject, SpatialConstants::MAPPING_PARTITION_ID, PartitionEntityId);
	}

	Connection->SendComponentUpdate(SpatialConstants::INITIAL_VIRTUAL_WORKER_TRANSLATOR_ENTITY_ID, MoveTemp(Update));

	return true;
}
} // namespace SpatialGDK
