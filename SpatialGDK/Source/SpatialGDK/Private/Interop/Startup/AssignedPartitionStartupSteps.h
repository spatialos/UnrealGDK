#pragma once

#include "Containers/UnrealString.h"
#include "Templates/SharedPointer.h"
#include "Templates/UniquePtr.h"

#include "Interop/Startup/ServerWorkerStartupCommon.h"
#include "Interop/Startup/SpatialStartupCommon.h"
#include "SpatialCommonTypes.h"
#include "SpatialView/SpatialOSWorker.h"

class USpatialNetDriver;

namespace SpatialGDK
{
class ISpatialOSWorker;

struct FAssignedPartitionSharedContext : public TSharedFromThis<FAssignedPartitionSharedContext>
{
	TOptional<Worker_PartitionId> LocalPartitionId;
};

class FGetAssignedPartitionStep : public FStartupStep
{
public:
	FGetAssignedPartitionStep(TSharedRef<FServerWorkerStartupContext> InState, ISpatialOSWorker& InConnection,
							  USpatialNetDriver& InNetDriver);

	virtual bool TryFinish() override { return Executor.TryFinish(); }
	virtual FString Describe() const override { return Executor.Describe(); }

private:
	TArray<TUniquePtr<FStartupStep>> CreateSteps();

	TSharedRef<FServerWorkerStartupContext> State;
	ISpatialOSWorker* Connection;
	USpatialNetDriver* NetDriver;

	TSharedRef<FAssignedPartitionSharedContext> PartitionDiscoveryContext;

	FStartupExecutor Executor;
};
} // namespace SpatialGDK
