// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "IpConnection.h"
#include "Runtime/Launch/Resources/Version.h"

#include "Schema/Interest.h"

#include <WorkerSDK/improbable/c_worker.h>

#include "LatencyManager.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogLatencyManager, Log, All);

class USpatialNetConnection;
class USpatialNetDriver;

UCLASS(transient)
class SPATIALGDK_API ULatencyManager : public UObject
{
	GENERATED_BODY()
public:
	ULatencyManager(const FObjectInitializer& ObjectInitializer);

	void Init(USpatialNetDriver* InDriver, USpatialNetConnection* InConnection);
	void Enable(FTimerManager* InTimerManager, Worker_EntityId InPlayerControllerEntity);
	void Disable();

private:
	void InitServerPing();
	void InitClientPong();

	void SendPingOrPong(uint32 PingId, Worker_ComponentId ComponentId);
	void RemoveExpiredPings();

	USpatialNetDriver* Driver;
	USpatialNetConnection* NetConnection;
	Worker_EntityId PlayerControllerEntity;

	class FTimerManager* TimerManager;
	FTimerHandle LatencyTimer;
	TMap<uint32, float> SentPingTimestamps;
	uint32 CurrentPingID;
};
