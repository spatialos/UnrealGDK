// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"

#include <improbable/c_worker.h>

#include "SpatialPlayerSpawner.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogSpatialPlayerSpawner, Log, All);

class USpatialNetDriver;

UCLASS()
class SPATIALGDK_API USpatialPlayerSpawner : public UObject
{
	GENERATED_BODY()
	
public:

	void Init(USpatialNetDriver* NetDriver, FTimerManager* TimerManager);

	// Server
	void ReceivePlayerSpawnRequest(FString URLString, const char* CallerWorkerId, Worker_RequestId RequestId);

	// Client
	void SendPlayerSpawnRequest();
	void ReceivePlayerSpawnResponse(Worker_CommandResponseOp& Op);

private:
	UPROPERTY()
	USpatialNetDriver* NetDriver;
	
	FTimerManager* TimerManager;
	int NumberOfAttempts;
};
