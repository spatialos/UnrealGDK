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
	virtual ~FSpatialClientStartupHandler();
	bool TryFinishStartup();
	FString GetStartupStateDescription() const;

private:
	void QueryGSM();
	ViewCoordinator& GetCoordinator();
	const ViewCoordinator& GetCoordinator() const;
	const TArray<Worker_Op>& GetOps() const;

	bool bQueriedGSM = false;
	bool bStartedRetrying = false;
	TOptional<FString> ClientStartupExtraState;
	FTimerHandle GSMQueryRetryTimer;
	FEntityQueryHandler QueryHandler;

	TOptional<FDeploymentMapData> GSMData;
	static bool GetFromComponentData(const Worker_ComponentData& Component, FDeploymentMapData& OutData);

	struct FSnapshotData
	{
		uint64 SnapshotVersion;
	};
	TOptional<FSnapshotData> SnapshotData;
	static bool GetFromComponentData(const Worker_ComponentData& Component, FSnapshotData& OutData);

	bool bFinishedMapLoad = false;
	FDelegateHandle PostMapLoadedDelegateHandle;
	void OnMapLoaded(UWorld* LoadedWorld);

	enum class EStage : uint8
	{
		QueryGSM,
		WaitForMapLoad,
		SendPlayerSpawnRequest,
		Finished,
		Initial = QueryGSM,
	};

	EStage Stage = EStage::Initial;

	USpatialNetDriver* NetDriver;
	TWeakObjectPtr<UGameInstance> GameInstance;

	TArray<TUniquePtr<FStartupStep>> CreateSteps();

	FStartupExecutor Executor;
};

} // namespace SpatialGDK
