#include "Interop/Startup/ServerWorkerCreationStartupSteps.h"

#include "Interop/GlobalStateManager.h"
#include "SpatialView/EntityComponentTypes.h"

namespace SpatialGDK
{
FCreateServerWorkerStep::FCreateServerWorkerStep(TSharedRef<FServerWorkerStartupContext> InState, USpatialNetDriver& InNetDriver,
												 ISpatialOSWorker& InConnection)
	: State(InState)
	, NetDriver(&InNetDriver)
	, Connection(&InConnection)
{
}

void FCreateServerWorkerStep::Start()
{
	WorkerEntityCreator.Emplace(*NetDriver, *Connection);
}

bool FCreateServerWorkerStep::TryFinish()
{
	WorkerEntityCreator->ProcessOps(Connection->GetWorkerMessages());
	if (WorkerEntityCreator->IsFinished())
	{
		State->WorkerEntityId = WorkerEntityCreator->GetWorkerEntityId();
		return true;
	}
	return false;
}

FWaitForServerWorkersStep::FWaitForServerWorkersStep(TSharedRef<FServerWorkerStartupContext> InState, FInitialSetup InSetup,
													 ISpatialOSWorker& InConnection)
	: State(InState)
	, Setup(InSetup)
	, Connection(&InConnection)
{
}

bool FWaitForServerWorkersStep::TryFinish()
{
	TArray<Worker_EntityId> WorkerEntityIds;

	for (const auto& EntityData : Connection->GetView())
	{
		const ComponentData* WorkerComponent =
			EntityData.Value.Components.FindByPredicate(ComponentIdEquality{ SpatialConstants::SERVER_WORKER_COMPONENT_ID });
		if (WorkerComponent != nullptr)
		{
			WorkerEntityIds.Emplace(EntityData.Key);
		}
	}
	if (WorkerEntityIds.Num() >= Setup.ExpectedServerWorkersCount)
	{
		ensureMsgf(WorkerEntityIds.Num() == Setup.ExpectedServerWorkersCount,
				   TEXT("There should never be more server workers connected than we expect. Expected %d, actually have %d"),
				   Setup.ExpectedServerWorkersCount, WorkerEntityIds.Num());
		State->WorkerEntityIds = WorkerEntityIds;
		return true;
	}
	return false;
}

FWaitForGsmEntityStep::FWaitForGsmEntityStep(ISpatialOSWorker& InConnection)
	: Connection(&InConnection)
{
}

bool FWaitForGsmEntityStep::TryFinish()
{
	return Connection->HasEntity(SpatialConstants::INITIAL_GLOBAL_STATE_MANAGER_ENTITY_ID);
}

FDeriveDeploymentStartupStateStep::FDeriveDeploymentStartupStateStep(TSharedRef<FServerWorkerStartupContext> InState,
																	 ISpatialOSWorker& InConnection,
																	 UGlobalStateManager& InGlobalStateManager)
	: State(InState)
	, Connection(&InConnection)
	, GlobalStateManager(&InGlobalStateManager)
{
}

bool FDeriveDeploymentStartupStateStep::TryFinish()
{
	const EntityViewElement& GlobalStateManagerEntity =
		Connection->GetView().FindChecked(SpatialConstants::INITIAL_GLOBAL_STATE_MANAGER_ENTITY_ID);
	const ComponentData* StartupActorManagerData =
		GlobalStateManagerEntity.Components.FindByPredicate(ComponentIdEquality{ SpatialConstants::STARTUP_ACTOR_MANAGER_COMPONENT_ID });
	if (StartupActorManagerData != nullptr)
	{
		if (Schema_GetBoolCount(StartupActorManagerData->GetFields(), SpatialConstants::STARTUP_ACTOR_MANAGER_CAN_BEGIN_PLAY_ID) != 0)
		{
			// StartupActorManager's CAN_BEGIN_PLAY is raised after startup is finished, and is false in the initial snapshot.
			// If it's true at this point, then this worker is either recovering or loading from a non-initial snapshot.
			const bool bIsRecoveringOrSnapshot =
				GetBoolFromSchema(StartupActorManagerData->GetFields(), SpatialConstants::STARTUP_ACTOR_MANAGER_CAN_BEGIN_PLAY_ID);

			State->bIsRecoveringOrSnapshot = bIsRecoveringOrSnapshot;

			// We should only call BeginPlay on actors if this is a fresh deployment; otherwise, we assume that
			// BeginPlay has already been called on them before.
			GlobalStateManager->bCanSpawnWithAuthority = !bIsRecoveringOrSnapshot;

			return true;
		}
	}
	return false;
}

} // namespace SpatialGDK
