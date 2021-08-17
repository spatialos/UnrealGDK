#pragma once

#include "Templates/SharedPointer.h"

#include "Interop/Startup/ServerWorkerStartupCommon.h"
#include "Interop/Startup/SpatialStartupCommon.h"

class USpatialNetDriver;
class UGlobalStateManager;

namespace SpatialGDK
{
class ISpatialOSWorker;

class FHandleBeginPlayStep : public FStartupStep
{
public:
	FHandleBeginPlayStep(TSharedRef<const FServerWorkerStartupContext> InState, USpatialNetDriver& InNetDriver,
						 ISpatialOSWorker& InConnection);

	virtual bool TryFinish() override;

private:
	UGlobalStateManager& GetGSM();

	TSharedRef<const FServerWorkerStartupContext> State;
	USpatialNetDriver* NetDriver;
	ISpatialOSWorker* Connection;
};
} // namespace SpatialGDK
