#include "Interop/Startup/DefaultServerWorkerStartupHandler.h"

#include "EngineClasses/SpatialNetDriver.h"
#include "EngineClasses/SpatialPackageMapClient.h"

#include "Interop/SkeletonEntityPopulator.h"
#include "Interop/Startup/AssignedPartitionStartupSteps.h"
#include "Interop/Startup/ElectGsmAuthWorkerStep.h"
#include "Interop/Startup/HandleSkeletonEntityCreationStep.h"
#include "Interop/Startup/PartitionCreationStartupSteps.h"
#include "Interop/Startup/ServerWorkerCreationStartupSteps.h"
#include "Interop/Startup/StartPlayStartupSteps.h"
#include "Interop/Startup/StrategyWorkerStartupSteps.h"

#include "Interop/GlobalStateManager.h"
#include "Utils/EntityFactory.h"

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
	const USpatialGDKSettings* Settings = GetDefault<USpatialGDKSettings>();

	TArray<TUniquePtr<FStartupStep>> Steps;
	Steps.Emplace(MakeUnique<FCreateServerWorkerStep>(State, *NetDriver, GetWorkerInterface()));
	Steps.Emplace(MakeUnique<FWaitForServerWorkersStep>(State, Setup, GetWorkerInterface()));
	Steps.Emplace(MakeUnique<FWaitForGsmEntityStep>(GetWorkerInterface()));
	Steps.Emplace(MakeUnique<FDeriveDeploymentStartupStateStep>(State, GetWorkerInterface(), GetGSM()));
	if (!Settings->bRunStrategyWorker)
	{
		Steps.Emplace(MakeUnique<FElectGsmAuthWorkerStep>(State, GetGSM()));
		Steps.Emplace(MakeUnique<FAuthCreateAndAssignPartitions>(State, Setup, *NetDriver, GetWorkerInterface()));
		Steps.Emplace(MakeUnique<FGetAssignedPartitionStep>(State, GetWorkerInterface(), *NetDriver));
	}
	else
	{
		Steps.Emplace(MakeUnique<FCreateStagingPartition>(State, *NetDriver, GetGSM()));
		Steps.Emplace(MakeUnique<FWaitForGSMAuthOrInitialManifest>(State, *NetDriver, GetGSM()));
	}
	if (Settings->bRunStrategyWorker || Settings->bEnableSkeletonEntityCreation)
	{
		Steps.Emplace(MakeUnique<FCreateSkeletonEntities>(*NetDriver));
	}
	Steps.Emplace(MakeUnique<FHandleBeginPlayStep>(State, *NetDriver, GetWorkerInterface()));
	return Steps;
}
} // namespace SpatialGDK
