// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "Utils/SchemaUtils.h"

#include <WorkerSDK/improbable/c_schema.h>
#include <WorkerSDK/improbable/c_worker.h>

#include "CoreMinimal.h"

#include "TimerManager.h"
#include "HttpModule.h"
#include "Interfaces/IHttpRequest.h"
#include "Interfaces/IHttpResponse.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"

#include "SpatialPlatformCoordinator.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogSpatialPlatformCoordinator, Log, All)

class USpatialNetDriver;

UCLASS()
class SPATIALGDK_API USpatialPlatformCoordinator : public UObject
{
	GENERATED_BODY()

public:
	USpatialPlatformCoordinator();

	virtual void Init(UNetDriver* InDriver);

	void StartSendingHeartbeat();
	void SendReadyStatus();
	void StartPollingForGameserverStatus();
	void StartWatchingForGameserverStatus();

private:
	USpatialNetDriver*					Driver;
	FString								Url;

	FTimerHandle						HeartBeatTimerHandler;
	FTimerHandle						GameserverStatusTimerHandler;
};
