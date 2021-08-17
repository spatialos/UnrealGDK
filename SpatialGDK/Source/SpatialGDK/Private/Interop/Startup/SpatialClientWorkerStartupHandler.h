#pragma once

#include "Containers/Array.h"
#include "Misc/Optional.h"

#include "EngineClasses/SpatialVirtualWorkerTranslator.h"
#include "Interop/EntityQueryHandler.h"
#include "SpatialCommonTypes.h"

#include "Interop/Startup/SpatialStartupCommon.h"

class USpatialNetDriver;

namespace SpatialGDK
{
class ViewCoordinator;

class FSpatialClientStartupHandler final
{
public:
	explicit FSpatialClientStartupHandler(USpatialNetDriver& InNetDriver, UGameInstance& InGameInstance);
	bool TryFinishStartup();
	FString GetStartupStateDescription() const;

private:
	ViewCoordinator& GetCoordinator();
	const ViewCoordinator& GetCoordinator() const;

	USpatialNetDriver* NetDriver;
	TWeakObjectPtr<UGameInstance> GameInstance;

	TArray<TUniquePtr<FStartupStep>> CreateSteps();

	FStartupExecutor Executor;
};

} // namespace SpatialGDK
