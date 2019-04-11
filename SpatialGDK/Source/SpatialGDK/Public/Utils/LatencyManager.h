// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"

#include <WorkerSDK/improbable/c_worker.h>

#include "LatencyManager.generated.h"

class USpatialNetConnection;
class USpatialNetDriver;

UCLASS()
class SPATIALGDK_API ULatencyManager : public UObject
{
	GENERATED_BODY()
public:
	ULatencyManager(const FObjectInitializer& ObjectInitializer);

	void Enable(Worker_EntityId InPlayerControllerEntity);
	void Disable();

private:
	void SendPingOrPong(Worker_ComponentId ComponentId);

	Worker_EntityId PlayerControllerEntity;
	float LastPingSent;

	TWeakObjectPtr<USpatialNetConnection> NetConnection;
	TWeakObjectPtr<USpatialNetDriver> NetDriver;
};
