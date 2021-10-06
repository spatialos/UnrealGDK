#pragma once

#include "Containers/Array.h"
#include "DefaultServerWorkerStartupHandler.h"
#include "Templates/SharedPointer.h"
#include "Templates/UniquePtr.h"

#include "Interop/Startup/ServerWorkerStartupCommon.h"
#include "Interop/Startup/SpatialStartupCommon.h"

class USpatialNetDriver;
class ISpatialOSWorker;

namespace SpatialGDK
{
class FAuthCreateAndAssignPartitions : public FStartupStep
{
public:
	FAuthCreateAndAssignPartitions(TSharedRef<const FServerWorkerStartupContext> InState, const FInitialSetup& InSetup,
								   USpatialNetDriver& InNetDriver, ISpatialOSWorker& InConnection);

	virtual bool TryFinish() override;
	virtual FString Describe() const override { return Executor.Describe(); }

private:
	TArray<TUniquePtr<FStartupStep>> CreateSteps();

	struct FPartitionCreationSharedState;

	class FDiscoverExistingPartitionsStep;
	class FCreateNecessaryPartitionsStep;
	class FWaitForPartitionVisibilityStep;
	class FAssignPartitionsToWorkersStep;

	TSharedRef<const FServerWorkerStartupContext> State;
	FInitialSetup Setup;
	USpatialNetDriver* NetDriver;
	ISpatialOSWorker* Connection;

	TSharedRef<FPartitionCreationSharedState> SharedState;

	FStartupExecutor Executor;
};
} // namespace SpatialGDK
