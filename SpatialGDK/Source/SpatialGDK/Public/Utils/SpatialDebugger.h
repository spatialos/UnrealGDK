// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Containers/Queue.h"
#include "Engine/Canvas.h"
#include "GameFramework/Info.h"

#include <WorkerSDK/improbable/c_worker.h>

#include "SpatialDebugger.generated.h"

class APawn;
class APlayerController;
class USpatialNetDriver;

UCLASS(SpatialType=Singleton)
class SPATIALGDK_API ASpatialDebugger :
	public AInfo
{
	GENERATED_UCLASS_BODY()

public:
	void OnEntityAdded(const Worker_EntityId EntityId);
	void OnEntityRemoved(const Worker_EntityId EntityId);

	virtual void Tick(float DeltaSeconds) override;
	virtual void BeginPlay() override;

private:

	static const int VIRTUAL_WORKER_MAX_COUNT = 4;
	static const int POSITION_HASH_BUCKETS = 1024;

	enum EIcon
	{
		ICON_AUTH,
		ICON_AUTH_INTENT,
		ICON_LOCK_OPEN,
		ICON_LOCK_CLOSED,
		ICON_MAX
	};

	enum ELockStatus
	{
		LOCKSTATUS_OPEN,
		LOCKSTATUS_CLOSED,
		LOCKSTATUS_MAX
	};

	// This Mapping is maintained independently on each client, and corresponds to each client's view on the SpatialOS world
	TMap<int64, TWeakObjectPtr<AActor>> EntityActorMapping;

	FDelegateHandle DrawDebugDelegateHandle;
	void DrawDebug(class UCanvas* Canvas, APlayerController* Controller);

	APawn* LocalPawn;
	APlayerController *LocalPlayerController;

	float MaxRange;

	bool bShowAuth;
	bool bShowAuthIntent;
	bool bShowLock;
	bool bShowEntityId;

	void LoadIcons();

	const FString IconFilenames[ICON_MAX] =
	{
		FString(TEXT("/SpatialGDK/SpatialDebugger/Icons/Auth.Auth")),
		FString(TEXT("/SpatialGDK/SpatialDebugger/Icons/AuthIntent.AuthIntent")),
		FString(TEXT("/SpatialGDK/SpatialDebugger/Icons/LockOpenSmall.LockOpenSmall")),
		FString(TEXT("/SpatialGDK/SpatialDebugger/Icons/LockClosedSmall.LockClosedSmall"))
	};

	FCanvasIcon Icons[ICON_MAX];

	FColor WorkerColors[VIRTUAL_WORKER_MAX_COUNT] =
	{
		FColor::Red,
		FColor::Green,
		FColor::Blue,
		FColor::Yellow
	};

	FColor LockColors[LOCKSTATUS_MAX] =
	{
		FColor::Green,
		FColor::Yellow
	};

	
	FString GetVirtualWorkerId(const Worker_EntityId EntityId) const;
	static int32 VirtualWorkerIdToInt(const FString& VirtualWorkerId);
	static int32 HashPosition(const FVector& Position);
};
