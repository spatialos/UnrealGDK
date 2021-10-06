#include "Interop/Startup/StartPlayStartupSteps.h"

#include "Algo/AllOf.h"
#include "EngineClasses/SpatialNetDriver.h"
#include "Interop/GlobalStateManager.h"
#include "Interop/SpatialPlayerSpawner.h"
#include "SpatialView/EntityComponentTypes.h"

namespace SpatialGDK
{
FHandleBeginPlayStep::FHandleBeginPlayStep(TSharedRef<const FServerWorkerStartupContext> InState, USpatialNetDriver& InNetDriver,
										   ISpatialOSWorker& InConnection)
	: State(InState)
	, NetDriver(&InNetDriver)
	, Connection(&InConnection)
{
}

bool FHandleBeginPlayStep::TryFinish()
{
	if (State->bHasGSMAuth)
	{
		const bool bAreAllWorkerEntitiesReady =
			Algo::AllOf(State->WorkerEntityIds, [this](const Worker_EntityId ServerWorkerEntityId) -> bool {
				const EntityViewElement& ServerWorkerEntity = Connection->GetView().FindChecked(ServerWorkerEntityId);
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

			NetDriver->PlayerSpawner->StartProcessingRequests();

			return true;
		}
	}
	else
	{
		const EntityViewElement& GlobalStateManagerEntity = Connection->GetView().FindChecked(GetGSM().GlobalStateManagerEntityId);
		const ComponentData* StartupActorManagerData = GlobalStateManagerEntity.Components.FindByPredicate(
			ComponentIdEquality{ SpatialConstants::STARTUP_ACTOR_MANAGER_COMPONENT_ID });
		if (ensure(StartupActorManagerData != nullptr)
			&& Schema_GetBool(StartupActorManagerData->GetFields(), SpatialConstants::STARTUP_ACTOR_MANAGER_CAN_BEGIN_PLAY_ID))
		{
			NetDriver->CreateAndInitializeCoreClassesAfterStartup();

			GetGSM().TriggerBeginPlay();

			NetDriver->PlayerSpawner->StartProcessingRequests();

			return true;
		}
	}

	return false;
}

UGlobalStateManager& FHandleBeginPlayStep::GetGSM()
{
	return *NetDriver->GlobalStateManager;
}
} // namespace SpatialGDK
