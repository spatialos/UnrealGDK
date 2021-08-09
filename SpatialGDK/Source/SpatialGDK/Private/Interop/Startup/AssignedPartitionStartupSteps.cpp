#include "Interop/Startup/AssignedPartitionStartupSteps.h"

#include "EngineClasses/SpatialNetDriver.h"
#include "EngineClasses/SpatialVirtualWorkerTranslator.h"
#include "Interop/GlobalStateManager.h"
#include "SpatialView/EntityComponentTypes.h"

namespace SpatialGDK
{
class FReadGsmDeploymentMapState : public FStartupStep
{
public:
	FReadGsmDeploymentMapState(ISpatialOSWorker& InConnection, UGlobalStateManager& InGlobalStateManager);

	virtual bool TryFinish() override;

private:
	ISpatialOSWorker* Connection;
	UGlobalStateManager* GlobalStateManager;
};

class FReadVirtualWorkerTranslatorStep : public FStartupStep
{
public:
	FReadVirtualWorkerTranslatorStep(TSharedRef<FServerWorkerStartupContext> InState,
									 TSharedRef<FAssignedPartitionSharedContext> InLocalPartitionIdPtr, ISpatialOSWorker& InConnection,
									 UGlobalStateManager& InGlobalStateManager);

	virtual bool TryFinish() override;

private:
	TSharedRef<const FServerWorkerStartupContext> State;
	TSharedRef<FAssignedPartitionSharedContext> LocalPartitionIdPtr;

	ISpatialOSWorker* Connection;
	UGlobalStateManager* GlobalStateManager;
};

class FWaitForAssignedPartition : public FStartupStep
{
public:
	FWaitForAssignedPartition(TSharedRef<FServerWorkerStartupContext> InState,
							  TSharedRef<const FAssignedPartitionSharedContext> InLocalPartitionIdPtr, USpatialNetDriver& InNetDriver,
							  ISpatialOSWorker& InConnection);

	virtual bool TryFinish() override;

private:
	TSharedRef<const FServerWorkerStartupContext> State;
	TSharedRef<const FAssignedPartitionSharedContext> LocalPartitionIdPtr;

	USpatialNetDriver* NetDriver;
	ISpatialOSWorker* Connection;
};

FGetAssignedPartitionStep::FGetAssignedPartitionStep(TSharedRef<FServerWorkerStartupContext> InState, ISpatialOSWorker& InConnection,
													 USpatialNetDriver& InNetDriver)
	: State(InState)
	, Connection(&InConnection)
	, NetDriver(&InNetDriver)
	, PartitionDiscoveryContext(MakeShared<FAssignedPartitionSharedContext>())
	, Executor(CreateSteps())
{
}

TArray<TUniquePtr<FStartupStep>> FGetAssignedPartitionStep::CreateSteps()
{
	TArray<TUniquePtr<FStartupStep>> Steps;

	Steps.Emplace(MakeUnique<FReadGsmDeploymentMapState>(*Connection, *NetDriver->GlobalStateManager));
	Steps.Emplace(
		MakeUnique<FReadVirtualWorkerTranslatorStep>(State, PartitionDiscoveryContext, *Connection, *NetDriver->GlobalStateManager));
	Steps.Emplace(MakeUnique<FWaitForAssignedPartition>(State, PartitionDiscoveryContext, *NetDriver, *Connection));

	return Steps;
}

FReadGsmDeploymentMapState::FReadGsmDeploymentMapState(ISpatialOSWorker& InConnection, UGlobalStateManager& InGlobalStateManager)
	: Connection(&InConnection)
	, GlobalStateManager(&InGlobalStateManager)
{
}

bool FReadGsmDeploymentMapState::TryFinish()
{
	const ComponentData* DeploymentMapComponent =
		Connection->GetView()[SpatialConstants::INITIAL_GLOBAL_STATE_MANAGER_ENTITY_ID].Components.FindByPredicate(
			ComponentIdEquality{ SpatialConstants::DEPLOYMENT_MAP_COMPONENT_ID });
	if (DeploymentMapComponent == nullptr)
	{
		return false;
	}
	FDeploymentMapData MapData;
	if (FDeploymentMapData::TryRead(*DeploymentMapComponent->GetFields(), MapData))
	{
		GlobalStateManager->ApplySessionId(MapData.DeploymentSessionId);

		return true;
	}
	return false;
}

FReadVirtualWorkerTranslatorStep::FReadVirtualWorkerTranslatorStep(TSharedRef<FServerWorkerStartupContext> InState,
																   TSharedRef<FAssignedPartitionSharedContext> InLocalPartitionIdPtr,
																   ISpatialOSWorker& InConnection,
																   UGlobalStateManager& InGlobalStateManager)
	: State(InState)
	, LocalPartitionIdPtr(InLocalPartitionIdPtr)
	, Connection(&InConnection)
	, GlobalStateManager(&InGlobalStateManager)
{
	StepName = TEXT("Retrieving VirtualWorkerTranslator assigned partition");
}

bool FReadVirtualWorkerTranslatorStep::TryFinish()
{
	const EntityViewElement* VirtualWorkerTranslatorEntity =
		Connection->GetView().Find(SpatialConstants::INITIAL_VIRTUAL_WORKER_TRANSLATOR_ENTITY_ID);
	if (VirtualWorkerTranslatorEntity == nullptr)
	{
		return false;
	}

	const ComponentData* WorkerTranslatorComponentData = VirtualWorkerTranslatorEntity->Components.FindByPredicate(
		ComponentIdEquality{ SpatialConstants::VIRTUAL_WORKER_TRANSLATION_COMPONENT_ID });
	if (WorkerTranslatorComponentData == nullptr)
	{
		return false;
	}

	// We've received worker translation data.
	TMap<VirtualWorkerId, SpatialVirtualWorkerTranslator::WorkerInformation> VirtualWorkerMapping;
	SpatialVirtualWorkerTranslator::ApplyMappingFromSchema(VirtualWorkerMapping, *WorkerTranslatorComponentData->GetFields());
	for (const auto& Mapping : VirtualWorkerMapping)
	{
		if (Mapping.Value.ServerWorkerEntityId == State->WorkerEntityId)
		{
			// We've received a virtual worker mapping that mentions our worker entity ID.
			LocalPartitionIdPtr->LocalPartitionId = Mapping.Value.PartitionEntityId;
			return true;
		}
	}

	return false;
}

FWaitForAssignedPartition::FWaitForAssignedPartition(TSharedRef<FServerWorkerStartupContext> InState,
													 TSharedRef<const FAssignedPartitionSharedContext> InLocalPartitionIdPtr,
													 USpatialNetDriver& InNetDriver, ISpatialOSWorker& InConnection)
	: State(InState)
	, LocalPartitionIdPtr(InLocalPartitionIdPtr)
	, NetDriver(&InNetDriver)
	, Connection(&InConnection)
{
	StepName = TEXT("Waiting for assigned partition to come into view");
}

bool FWaitForAssignedPartition::TryFinish()
{
	const EntityViewElement* AssignedPartitionEntity = Connection->GetView().Find(*LocalPartitionIdPtr->LocalPartitionId);
	if (AssignedPartitionEntity == nullptr)
	{
		return false;
	}

	const EntityViewElement& VirtualWorkerTranslatorEntity =
		Connection->GetView().FindChecked(SpatialConstants::INITIAL_VIRTUAL_WORKER_TRANSLATOR_ENTITY_ID);
	const ComponentData* WorkerTranslatorComponentData = VirtualWorkerTranslatorEntity.Components.FindByPredicate(
		ComponentIdEquality{ SpatialConstants::VIRTUAL_WORKER_TRANSLATION_COMPONENT_ID });
	if (ensure(WorkerTranslatorComponentData != nullptr))
	{
		NetDriver->VirtualWorkerTranslator->ApplyMappingFromSchema(WorkerTranslatorComponentData->GetFields());

		ComponentUpdate MarkAsReady(SpatialConstants::SERVER_WORKER_COMPONENT_ID);
		Schema_AddBool(MarkAsReady.GetFields(), SpatialConstants::SERVER_WORKER_READY_TO_BEGIN_PLAY_ID, true);
		Connection->SendComponentUpdate(*State->WorkerEntityId, MoveTemp(MarkAsReady));
		return true;
	}

	return false;
}
} // namespace SpatialGDK
