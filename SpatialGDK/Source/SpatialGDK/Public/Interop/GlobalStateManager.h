// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "Utils/SchemaUtils.h"

#include "CoreMinimal.h"
#include "EngineUtils.h"
#include "UObject/NoExportTypes.h"

#include <WorkerSDK/improbable/c_schema.h>
#include <WorkerSDK/improbable/c_worker.h>

#include "EntityQueryHandler.h"
#include "Interop/ClaimPartitionHandler.h"

#include "GlobalStateManager.generated.h"

class USpatialNetDriver;
class USpatialActorChannel;
class USpatialStaticComponentView;
class USpatialSender;
class USpatialReceiver;

DECLARE_LOG_CATEGORY_EXTERN(LogGlobalStateManager, Log, All)

UCLASS()
class SPATIALGDK_API UGlobalStateManager : public UObject
{
	GENERATED_BODY()

public:
	void Init(USpatialNetDriver* InNetDriver);

	void ApplyDeploymentMapData(Schema_ComponentData* Data);
	void ApplySnapshotVersionData(Schema_ComponentData* Data);
	void ApplyStartupActorManagerData(Schema_ComponentData* Data);

	void ApplyDeploymentMapUpdate(Schema_ComponentUpdate* Update);
	void ApplyStartupActorManagerUpdate(Schema_ComponentUpdate* Update);

	DECLARE_DELEGATE_OneParam(QueryDelegate, const Worker_EntityQueryResponseOp&);
	void QueryGSM(const QueryDelegate& Callback);
	static bool GetAcceptingPlayersAndSessionIdFromQueryResponse(const Worker_EntityQueryResponseOp& Op, bool& OutAcceptingPlayers,
																 int32& OutSessionId);
	void ApplyVirtualWorkerMappingFromQueryResponse(const Worker_EntityQueryResponseOp& Op) const;
	void ApplyDataFromQueryResponse(const Worker_EntityQueryResponseOp& Op);

	void QueryTranslation();

	void SetDeploymentState();
	void SetAcceptingPlayers(bool bAcceptingPlayers);
	void IncrementSessionID();

	void Advance();

	FORCEINLINE FString GetDeploymentMapURL() const { return DeploymentMapURL; }
	FORCEINLINE bool GetAcceptingPlayers() const { return bAcceptingPlayers; }
	FORCEINLINE int32 GetSessionId() const { return DeploymentSessionId; }
	FORCEINLINE uint32 GetSchemaHash() const { return SchemaHash; }
	FORCEINLINE uint64 GetSnapshotVersion() const { return SnapshotVersion; }

	void AuthorityChanged(const Worker_ComponentSetAuthorityChangeOp& AuthChangeOp);

	void ResetGSM();

	void BeginDestroy() override;

	void TrySendWorkerReadyToBeginPlay();
	void TriggerBeginPlay();
	bool GetCanBeginPlay() const;

	bool IsReady() const;

	void HandleActorBasedOnLoadBalancer(AActor* ActorIterator) const;

	Worker_EntityId GetLocalServerWorkerEntityId() const;
	void ClaimSnapshotPartition();

	Worker_EntityId GlobalStateManagerEntityId;

private:
	// Deployment Map Component
	FString DeploymentMapURL;
	bool bAcceptingPlayers;
	int32 DeploymentSessionId = 0;
	uint32 SchemaHash;
	uint64 SnapshotVersion = 0;

	// Startup Actor Manager Component
	bool bHasSentReadyForVirtualWorkerAssignment;
	bool bCanBeginPlay;
	bool bCanSpawnWithAuthority;

public:
#if WITH_EDITOR
	void OnPrePIEEnded(bool bValue);
	void ReceiveShutdownMultiProcessRequest();

	void OnShutdownComponentUpdate(Schema_ComponentUpdate* Update);
	void ReceiveShutdownAdditionalServersEvent();
#endif // WITH_EDITOR

private:
	void SetDeploymentMapURL(const FString& MapURL);
	void SendSessionIdUpdate();

	void SendCanBeginPlayUpdate(const bool bInCanBeginPlay);

#if WITH_EDITOR
	void SendShutdownMultiProcessRequest();
	void SendShutdownAdditionalServersEvent();
#endif // WITH_EDITOR

private:
	UPROPERTY()
	USpatialNetDriver* NetDriver;

	UPROPERTY()
	USpatialStaticComponentView* StaticComponentView;

	TUniquePtr<SpatialGDK::ClaimPartitionHandler> ClaimHandler;
	SpatialGDK::EntityQueryHandler QueryHandler;

	FDelegateHandle PrePIEEndedHandle;

	bool bTranslationQueryInFlight;
};
