// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "EngineUtils.h"
#include "UObject/NoExportTypes.h"

#include <WorkerSDK/improbable/c_schema.h>
#include <WorkerSDK/improbable/c_worker.h>

#include "EntityQueryHandler.h"
#include "Interop/EntityCommandHandler.h"
#include "Interop/SpatialCommandsHandler.h"
#include "Utils/SchemaUtils.h"

#include "GlobalStateManager.generated.h"

class USpatialNetDriver;
class USpatialActorChannel;
class USpatialStaticComponentView;
class USpatialSender;
class USpatialReceiver;

namespace SpatialGDK
{
class ViewCoordinator;
}

DECLARE_LOG_CATEGORY_EXTERN(LogGlobalStateManager, Log, All)

UCLASS()
class SPATIALGDK_API UGlobalStateManager : public UObject
{
	GENERATED_BODY()

public:
	void Init(USpatialNetDriver* InNetDriver);

	void ApplySessionId(int32 InSessionId);

	bool HasAuthority() const;

	void SetDeploymentState();
	void SetAcceptingPlayers(bool bAcceptingPlayers);
	void IncrementSessionID();

	void SendCanBeginPlayUpdate(const bool bInCanBeginPlay);

	void Advance();

	FORCEINLINE int32 GetSessionId() const { return DeploymentSessionId; }

	void ResetGSM();

	void BeginDestroy() override;

	void TriggerBeginPlay();

	void HandleActorBasedOnLoadBalancer(AActor* ActorIterator) const;

	Worker_EntityId GetLocalServerWorkerEntityId() const;
	void ClaimSnapshotPartition();

	Worker_EntityId GlobalStateManagerEntityId;

	// Startup Actor Manager Component
	bool bCanSpawnWithAuthority;
	bool bGSMReadyForPlay;

private:
	// Deployment Map Component
	int32 DeploymentSessionId = 0;

public:
#if WITH_EDITOR
	void OnPrePIEEnded(bool bValue);
	void ReceiveShutdownMultiProcessRequest();

	void OnReceiveShutdownCommand(const Worker_Op& Op, const Worker_CommandRequestOp& CommandRequestOp);

	void OnShutdownComponentUpdate(Schema_ComponentUpdate* Update);
	void ReceiveShutdownAdditionalServersEvent();
#endif // WITH_EDITOR

private:
	void SendSessionIdUpdate();

#if WITH_EDITOR
	void SendShutdownMultiProcessRequest();
	void SendShutdownAdditionalServersEvent();
#endif // WITH_EDITOR

private:
	UPROPERTY()
	USpatialNetDriver* NetDriver;

	SpatialGDK::ViewCoordinator* ViewCoordinator;

	SpatialGDK::FCommandsHandler CommandsHandler;

#if WITH_EDITOR
	SpatialGDK::EntityCommandRequestHandler RequestHandler;

	FDelegateHandle PrePIEEndedHandle;
#endif // WITH_EDITOR

	bool bTranslationQueryInFlight;
};
