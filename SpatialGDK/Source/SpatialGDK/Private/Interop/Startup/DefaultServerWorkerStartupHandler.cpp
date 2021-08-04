#include "Interop/Startup/DefaultServerWorkerStartupHandler.h"

#include "EngineClasses/SpatialNetDriver.h"

#include "Interop/Startup/AssignedPartitionStartupSteps.h"
#include "Interop/Startup/ElectGsmAuthWorkerStep.h"
#include "Interop/Startup/HandleSkeletonEntityCreationStep.h"
#include "Interop/Startup/PartitionCreationStartupSteps.h"
#include "Interop/Startup/ServerWorkerCreationStartupSteps.h"
#include "Interop/Startup/StartPlayStartupSteps.h"

namespace SpatialGDK
{
FSpatialServerStartupHandler::FSpatialServerStartupHandler(USpatialNetDriver& InNetDriver, const FInitialSetup& InSetup)
	: NetDriver(&InNetDriver)
	, Setup(InSetup)
	, State(MakeShared<FServerWorkerStartupContext>())
	, Executor(CreateSteps())
{
}

FSpatialServerStartupHandler::~FSpatialServerStartupHandler() = default;

bool FSpatialServerStartupHandler::TryFinishStartup()
{
	return Executor.TryFinish();
}

ISpatialOSWorker& FSpatialServerStartupHandler::GetWorkerInterface()
{
	return NetDriver->Connection->GetCoordinator();
}

UGlobalStateManager& FSpatialServerStartupHandler::GetGSM()
{
	return *NetDriver->GlobalStateManager;
}

FString FSpatialServerStartupHandler::GetStartupStateDescription() const
{
	return Executor.Describe();
}

TArray<TUniquePtr<FStartupStep>> FSpatialServerStartupHandler::CreateSteps()
{
	TArray<TUniquePtr<FStartupStep>> Steps;
	Steps.Emplace(MakeUnique<FCreateServerWorkerStep>(State, *NetDriver, GetWorkerInterface()));
	Steps.Emplace(MakeUnique<FWaitForServerWorkersStep>(State, Setup, GetWorkerInterface()));
	Steps.Emplace(MakeUnique<FWaitForGsmEntityStep>(GetWorkerInterface()));
	Steps.Emplace(MakeUnique<FDeriveDeploymentStartupStateStep>(State, GetWorkerInterface(), GetGSM()));
	Steps.Emplace(MakeUnique<FElectGsmAuthWorkerStep>(State, GetGSM()));
	Steps.Emplace(MakeUnique<FAuthCreateAndAssignPartitions>(State, Setup, *NetDriver, GetWorkerInterface()));
	Steps.Emplace(MakeUnique<FGetAssignedPartitionStep>(State, GetWorkerInterface(), *NetDriver));
	if (GetDefault<USpatialGDKSettings>()->bEnableSkeletonEntityCreation)
	{
		Steps.Emplace(MakeUnique<FCreateSkeletonEntities>(*NetDriver));
	}
	Steps.Emplace(MakeUnique<FHandleBeginPlayStep>(State, *NetDriver, GetWorkerInterface()));
	return Steps;
}
} // namespace SpatialGDK
