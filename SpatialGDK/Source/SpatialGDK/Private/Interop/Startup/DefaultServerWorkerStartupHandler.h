#pragma once

#include "Containers/UnrealString.h"
#include "Templates/SharedPointer.h"

#include "Interop/Startup/SpatialStartupCommon.h"

class USpatialNetDriver;
class UGlobalStateManager;

namespace SpatialGDK
{
class ISpatialOSWorker;

struct FServerWorkerStartupContext;

struct FInitialSetup
{
	int32 ExpectedServerWorkersCount;
};

class FSpatialServerStartupHandler final
{
public:
	explicit FSpatialServerStartupHandler(USpatialNetDriver& InNetDriver, const FInitialSetup& InSetup);
	~FSpatialServerStartupHandler();
	bool TryFinishStartup();
	FString GetStartupStateDescription() const;

private:
	ISpatialOSWorker& GetWorkerInterface();
	UGlobalStateManager& GetGSM();

	TArray<TUniquePtr<FStartupStep>> CreateSteps();

	USpatialNetDriver* NetDriver;

	FInitialSetup Setup;

	TSharedRef<FServerWorkerStartupContext> State;

	FStartupExecutor Executor;
};
} // namespace SpatialGDK
