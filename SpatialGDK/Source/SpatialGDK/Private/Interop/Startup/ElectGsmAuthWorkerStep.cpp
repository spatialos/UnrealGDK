#include "Interop/Startup/ElectGsmAuthWorkerStep.h"

#include "Algo/MinElement.h"
#include "Interop/GlobalStateManager.h"
#include "Interop/Startup/ServerWorkerStartupCommon.h"

namespace SpatialGDK
{
FElectGsmAuthWorkerStep::FElectGsmAuthWorkerStep(TSharedRef<FServerWorkerStartupContext> InState, UGlobalStateManager& InGlobalStateManager)
	: State(InState)
	, GlobalStateManager(&InGlobalStateManager)
{
	StepName = TEXT("Electing GSM auth worker");
}

void FElectGsmAuthWorkerStep::Start()
{
	const bool bDidClaimStartupPartition = TryClaimingPartition();
	bShouldHaveGsmAuthority = bDidClaimStartupPartition;
	if (!bDidClaimStartupPartition)
	{
		State->bHasGSMAuth = false;
	}
}

bool FElectGsmAuthWorkerStep::TryFinish()
{
	if (!bShouldHaveGsmAuthority.GetValue())
	{
		return true;
	}

	if (GlobalStateManager->HasAuthority())
	{
		State->bHasGSMAuth = true;
		GlobalStateManager->SetDeploymentState();
		return true;
	}

	return false;
}

bool FElectGsmAuthWorkerStep::TryClaimingPartition()
{
	// Perform a naive leader election where we wait for the correct number of server workers to be
	// present in the deployment, and then whichever server has the lowest server worker entity ID
	// becomes the leader and claims the snapshot partition.
	check(State->WorkerEntityId);

	const Worker_EntityId* LowestEntityId = Algo::MinElement(State->WorkerEntityIds);

	check(LowestEntityId != nullptr);

	if (State->WorkerEntityId == *LowestEntityId)
	{
		UE_LOG(LogSpatialStartupHandler, Log, TEXT("MaybeClaimSnapshotPartition claiming snapshot partition"));
		GlobalStateManager->ClaimSnapshotPartition();
		return true;
	}
	UE_LOG(LogSpatialStartupHandler, Log, TEXT("Not claiming snapshot partition"));
	return false;
}

FString FElectGsmAuthWorkerStep::Describe() const
{
	const FString ElectionState = [this] {
		if (ensure(bShouldHaveGsmAuthority.IsSet()))
		{
			return *bShouldHaveGsmAuthority ? TEXT("Elected as GSM auth worker, waiting for authority") : TEXT("GSM nonauth worker");
		}
		return TEXT("Waiting for election");
	}();

	return FString::Printf(TEXT("GSM auth worker election: %s"), *ElectionState);
}
} // namespace SpatialGDK
