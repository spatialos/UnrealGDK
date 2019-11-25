// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "GameFramework/OnlineReplStructs.h"
#include "UObject/NoExportTypes.h"

#include <WorkerSDK/improbable/c_worker.h>
#include <WorkerSDK/improbable/c_schema.h>

#include "SpatialPlayerSpawner.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogSpatialPlayerSpawner, Log, All);

class FTimerManager;
class USpatialNetDriver;

UCLASS()
class SPATIALGDK_API USpatialPlayerSpawner : public UObject
{
	GENERATED_BODY()

public:

	void Init(USpatialNetDriver* NetDriver, FTimerManager* TimerManager, uint32 SchemaHash);

	// Server
	void ReceivePlayerSpawnRequest(Schema_Object* Payload, const char* CallerAttribute, Worker_RequestId RequestId);

	// Client
	void SendPlayerSpawnRequest();
	void ReceivePlayerSpawnResponse(const Worker_CommandResponseOp& Op);

private:
	void ObtainPlayerParams(struct FURL& LoginURL, FUniqueNetIdRepl& OutUniqueId, FName& OutOnlinePlatformName);

	UPROPERTY()
	USpatialNetDriver* NetDriver;

	FTimerManager* TimerManager;
	int NumberOfAttempts;

	TSet<FString> WorkersWithPlayersSpawned;
	uint32 SchemaHash;
};
