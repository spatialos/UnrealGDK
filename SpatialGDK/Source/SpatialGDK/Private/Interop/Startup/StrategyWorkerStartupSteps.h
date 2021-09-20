#pragma once

#include "Containers/UnrealString.h"
#include "Templates/SharedPointer.h"

#include "Interop/SpatialCommandsHandler.h"
#include "Interop/Startup/ServerWorkerStartupCommon.h"
#include "Interop/Startup/SpatialStartupCommon.h"

class USpatialNetDriver;
class UGlobalStateManager;

namespace SpatialGDK
{
class FWaitForGSMAuthOrInitialManifest : public FStartupStep
{
public:
	FWaitForGSMAuthOrInitialManifest(TSharedRef<FServerWorkerStartupContext> InState, USpatialNetDriver& InNetDriver,
									 UGlobalStateManager& InGlobalStateManager);

	virtual void Start() override;
	virtual bool TryFinish() override;

private:
	TSharedRef<FServerWorkerStartupContext> State;
	USpatialNetDriver& NetDriver;
	UGlobalStateManager& GlobalStateManager;
};

class FCreateStagingPartition : public FStartupStep
{
public:
	FCreateStagingPartition(TSharedRef<FServerWorkerStartupContext> InState, USpatialNetDriver& InNetDriver,
							UGlobalStateManager& InGlobalStateManager);

	virtual void Start() override;

	virtual bool TryFinish() override;

private:
	Worker_EntityId StagingPartitionId;
	TSharedRef<FServerWorkerStartupContext> State;
	USpatialNetDriver& NetDriver;
	UGlobalStateManager& GlobalStateManager;
	FCommandsHandler CommandsHandler;

	bool bPartitionClaimed = false;
};

} // namespace SpatialGDK
