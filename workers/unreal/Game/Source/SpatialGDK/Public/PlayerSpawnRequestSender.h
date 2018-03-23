// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "improbable/unreal/spawner.h"
#include "LogMacros.h"
#include <improbable/worker.h>
#include <improbable/view.h>

namespace worker 
{ 
class Connection;
class View;
}  // ::worker

class USpatialOS;
struct FURL;
class FTimerManager;

DECLARE_LOG_CATEGORY_EXTERN(LogSpatialOSPlayerSpawner, Log, All);

/**
 * Sends PlayerSpawn command requests from a newly connected client.
 * Handles retries, in the event of a recoverable failure, by waiting and trying again.
 */
class SPATIALGDK_API FPlayerSpawnRequestSender
{
public:
	FPlayerSpawnRequestSender();
	~FPlayerSpawnRequestSender();

	void RequestPlayer(USpatialOS* InSpatialOS, FTimerManager* InTimerManager, const FURL& Url);

private:
	using SpawnPlayerCommand = improbable::unreal::PlayerSpawner::Commands::SpawnPlayer;

	void SendPlayerSpawnRequest();
	void HandlePlayerSpawnResponse(const worker::CommandResponseOp<SpawnPlayerCommand>& Op);

	FTimerManager* TimerManager;

	worker::Connection* Connection;
	worker::View* View;

	TOptional<uint32> ResponseCallbackKey;
	improbable::unreal::SpawnPlayerRequest Request;

	uint32 NumberOfAttempts;
};
