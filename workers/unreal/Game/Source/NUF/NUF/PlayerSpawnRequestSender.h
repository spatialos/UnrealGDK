// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "Generated/Usr/improbable/spawner/spawner.h"
#include "LogMacros.h"
#include <worker.h>
#include <view.h>

namespace worker 
{ 
class worker::Connection;
class worker::View;
}  // ::worker

class USpatialOS;
struct FURL;
class FTimerManager;

DECLARE_LOG_CATEGORY_EXTERN(LogSpatialOSPlayerSpawner, Log, All);

/**
 * Sends PlayerSpawn command requests from a newly connected client.
 * Handles retries, in the event of a recoverable failure, by waiting and trying again.
 */
class NUF_API FPlayerSpawnRequestSender
{
public:
	FPlayerSpawnRequestSender();
	~FPlayerSpawnRequestSender();

	void RequestPlayer(USpatialOS* InSpatialOS, FTimerManager* InTimerManager, const FURL& Url);

private:
	using SpawnPlayerCommand = improbable::spawner::PlayerSpawner::Commands::SpawnPlayer;

	void SendPlayerSpawnRequest();
	void HandlePlayerSpawnResponse(const worker::CommandResponseOp<SpawnPlayerCommand>& Op);

	FTimerManager* TimerManager;

	worker::Connection* Connection;
	worker::View* View;

	TOptional<uint32> ResponseCallbackKey;
	improbable::spawner::SpawnPlayerRequest Request;

	uint32 NumberOfAttempts;
};
