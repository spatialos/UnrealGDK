#pragma once

#include "Containers/Array.h"
#include "DefaultServerWorkerStartupHandler.h"
#include "Templates/SharedPointer.h"
#include "Templates/UniquePtr.h"

#include "Interop/EntityQueryHandler.h"
#include "Interop/Startup/ServerWorkerStartupCommon.h"
#include "Interop/Startup/SpatialStartupCommon.h"

class USpatialNetDriver;
class ISpatialOSWorker;

namespace SpatialGDK
{
struct FPartitionCreationSharedState : public TSharedFromThis<FPartitionCreationSharedState>
{
	TMap<Worker_EntityId_Key, EntityViewElement> DiscoveredPartitionEntityIds;
	TArray<Worker_EntityId> NewCreatedPartitionEntityIds;
};

class FDiscoverExistingPartitionsStep : public FStartupStep
{
public:
	FDiscoverExistingPartitionsStep(TSharedRef<FPartitionCreationSharedState> InSharedState, ISpatialOSWorker& InConnection,
									Worker_ComponentId ComponentToLookFor, TArray<Worker_ComponentId> ComponentsToRead)
		: SharedState(InSharedState)
		, Connection(&InConnection)
		, PartitionComponentTag(ComponentToLookFor)
		, PartitionComponents(MoveTemp(ComponentsToRead))
	{
		StepName = TEXT("Discovering existing worker partitions");
	}

	virtual void Start() override;

	void OnPartitionQueryComplete(const Worker_EntityQueryResponseOp& QueryResponse);

	virtual bool TryFinish() override;

private:
	TSharedRef<FPartitionCreationSharedState> SharedState;

	ISpatialOSWorker* Connection;

	FEntityQueryHandler PartitionQueryHandler;
	Worker_ComponentId PartitionComponentTag;
	TArray<Worker_ComponentId> PartitionComponents;
	bool bQueryComplete = false;
};

class FAuthCreateAndAssignPartitions : public FStartupStep
{
public:
	FAuthCreateAndAssignPartitions(TSharedRef<const FServerWorkerStartupContext> InState, const FInitialSetup& InSetup,
								   USpatialNetDriver& InNetDriver, ISpatialOSWorker& InConnection);

	virtual bool TryFinish() override;
	virtual FString Describe() const override { return Executor.Describe(); }

private:
	TArray<TUniquePtr<FStartupStep>> CreateSteps();

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
