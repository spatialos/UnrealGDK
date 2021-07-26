#pragma once

#include "EngineClasses/SpatialVirtualWorkerTranslator.h"
#include "Interop/ClaimPartitionHandler.h"
#include "Interop/Connection/SpatialWorkerConnection.h"
#include "Interop/CreateEntityHandler.h"
#include "Interop/SkeletonEntityCreationStep.h"
#include "SpatialCommonTypes.h"

class USpatialNetDriver;
class UGlobalStateManager;

namespace SpatialGDK
{
class ViewCoordinator;

struct FStartupStep
{
	using FOnStepStarted = TUniqueFunction<void()>;
	using FTryFinishStep = TUniqueFunction<bool()>;

	FOnStepStarted OnStepStarted;
	FTryFinishStep TryFinishStep;
};

using FStartupStep2 = TUniqueObj<FStartupStep>;
using FStartupSteps = TArray<FStartupStep2>;

class FStartupExecutor
{
public:
	explicit FStartupExecutor(FStartupSteps InSteps);
	bool TryFinishStartup();

private:
	FStartupSteps Steps;
};

class FSpatialServerStartupHandler final
{
public:
	struct FInitialSetup
	{
		int32 ExpectedServerWorkersCount;
	};
	explicit FSpatialServerStartupHandler(USpatialNetDriver& InNetDriver, const FInitialSetup& InSetup);
	~FSpatialServerStartupHandler();
	bool TryFinishStartup();
	FString GetStartupStateDescription() const;

private:
	ISpatialOSWorker& GetCoordinator();
	const ISpatialOSWorker& GetCoordinator() const;
	UGlobalStateManager& GetGSM();

	enum class EStage : uint8
	{
		CreateWorkerEntity,
		WaitForWorkerEntities,
		WaitForGSMEntity,
		DeriveDeploymentRecoveryState,
		TryClaimingGSMEntityAuthority,
		WaitForGSMEntityAuthority,

		GsmAuthFillWorkerTranslationState,
		GsmAuthCreatePartitions,
		GsmAuthWaitForPartitionsVisibility,
		GsmAuthAssignPartitionsToVirtualWorkers,

		GetVirtualWorkerTranslationState,
		WaitForAssignedPartition,

		CreateSkeletonEntities,

		GsmAuthDispatchGSMStartPlay,
		GsmNonAuthWaitForGSMStartPlay,

		Finished,
		Initial = CreateWorkerEntity,
	};

	TArray<TUniqueObj<FStartupStep>> CreateSteps(USpatialNetDriver& InNetDriver, const FInitialSetup& InSetup);

	EStage Stage = EStage::Initial;

	FInitialSetup Setup;
	struct FInternalState;
	TUniqueObj<FInternalState> State;

	USpatialNetDriver* NetDriver;

	FStartupExecutor Executor;
};
} // namespace SpatialGDK
