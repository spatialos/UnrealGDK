// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"

#include <WorkerSDK/improbable/c_worker.h>


class USpatialNetConnection;
class USpatialNetDriver;

class ULatencyManager
{
public:
	ULatencyManager(USpatialNetConnection* InConnection);

	void Enable(Worker_EntityId InPlayerControllerEntity);
	void Disable();

private:
	void SendPingOrPong(Worker_ComponentId ComponentId);

	Worker_EntityId PlayerControllerEntity;
	float LastPingSent;

	USpatialNetConnection* NetConnection;
	USpatialNetDriver* NetDriver;
};
