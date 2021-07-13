#pragma once

#include "EngineClasses/SpatialVirtualWorkerTranslator.h"
#include "Interop/Connection/SpatialWorkerConnection.h"
#include "Interop/GlobalStateManager.h"
#include "SkeletonEntityCreationStep.h"
#include "SpatialView/ViewCoordinator.h"

class USpatialNetDriver;

namespace SpatialGDK
{
class FSpatialServerStartupHandler
{
public:
	struct FInitialSetup
	{
		int32 ExpectedServerWorkersCount;
	};
	explicit FSpatialServerStartupHandler(USpatialNetDriver& InNetDriver, const FInitialSetup& InSetup);
	bool TryFinishStartup();
	void SpawnPartitionEntity(Worker_EntityId PartitionEntityId, VirtualWorkerId VirtualWorkerId);

private:
	bool TryClaimingStartupPartition();

	bool bCalledCreateEntity = false;
	TOptional<ServerWorkerEntityCreator> ServerWorkerEntityCreator1;
	Worker_EntityId WorkerEntityId;

	TArray<Worker_EntityId_Key> WorkerEntityIds;

	bool bHasGSMAuth = false;

	bool bIsRecoveringOrSnapshot = false;

	bool bHasCalledPartitionEntityCreate = false;
	FCreateEntityHandler EntityHandler;
	TArray<Worker_PartitionId> WorkerPartitions;

	TMap<VirtualWorkerId, SpatialVirtualWorkerTranslator::WorkerInformation> WorkersToPartitions;
	TSet<Worker_PartitionId> PartitionsToCreate;

	VirtualWorkerId LocalVirtualWorkerId;
	Worker_PartitionId LocalPartitionId;

	FClaimPartitionHandler ClaimHandler;

	TOptional<FSkeletonEntityCreationStartupStep> SkeletonEntityStep;

	ViewCoordinator& GetCoordinator();
	const ViewCoordinator& GetCoordinator() const;
	const TArray<Worker_Op>& GetOps() const;
	UGlobalStateManager& GetGSM();

	enum class EStage : uint8
	{
		CreateWorkerEntity,
		WaitForWorkerEntities,
		WaitForGSMEntity,
		DeriveDeploymentRecoveryState,
		TryClaimingGSMEntityAuthority,
		WaitForGSMEntityAuthority,

		FillWorkerTranslationState,
		AuthCreatePartitions,
		WaitForAuthPartitionsCreated,
		WaitForAuthPartitionsVisibility,
		AssignPartitionsToVirtualWorkers,

		GetVirtualWorkerTranslationState,
		WaitForAssignedPartition,

		CreateSkeletonEntities,

		DispatchGSMStartPlay,
		WaitForGSMStartPlay,

		Finished,
		Initial = CreateWorkerEntity,
	};

	EStage Stage = EStage::Initial;

	FInitialSetup Setup;

	USpatialNetDriver* NetDriver;
};

class FSpatialClientStartupHandler
{
public:
	struct FInitialSetup
	{
		int32 ExpectedServerWorkersCount;
	};
	explicit FSpatialClientStartupHandler(USpatialNetDriver& InNetDriver, UGameInstance& InGameInstance, const FInitialSetup& InSetup);
	virtual ~FSpatialClientStartupHandler();
	bool TryFinishStartup();
	void QueryGSM();
	ViewCoordinator& GetCoordinator();
	const ViewCoordinator& GetCoordinator() const;
	const TArray<Worker_Op>& GetOps() const;

private:
	bool bQueriedGSM = false;
	bool bStartedRetrying = false;
	FTimerHandle GSMQueryRetryTimer;
	FEntityQueryHandler QueryHandler;
	struct FDeploymentMapData
	{
		FString DeploymentMapURL;

		bool bAcceptingPlayers;

		int32 DeploymentSessionId;

		uint32 SchemaHash;
	};
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

	FInitialSetup Setup;

	EStage Stage = EStage::Initial;

	USpatialNetDriver* NetDriver;
	TWeakObjectPtr<UGameInstance> GameInstance;
};

} // namespace SpatialGDK
