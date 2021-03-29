// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "Utils/SchemaUtils.h"

#include <WorkerSDK/improbable/c_schema.h>
#include <WorkerSDK/improbable/c_worker.h>

#include "CoreMinimal.h"

#include "HttpModule.h"
#include "Interfaces/IHttpRequest.h"
#include "Interfaces/IHttpResponse.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"
#include "TimerManager.h"

#include "SpatialPlatformCoordinator.generated.h"

#define CHECK_PLATFORM_SWITCH(is_heartbeat)	\
FString strSwitch = FPlatformMisc::GetEnvironmentVariable(TEXT("bEnableSpatialPlatformCoordinator")).ToLower().TrimStartAndEnd();	\
if (strSwitch.IsEmpty()) { return; } \
if (is_heartbeat == true && strSwitch != "true") { return; }


DECLARE_LOG_CATEGORY_EXTERN(LogSpatialPlatformCoordinator, Log, All)

class USpatialNetDriver;

UCLASS()
class SPATIALGDK_API USpatialPlatformCoordinator : public UObject
{
	GENERATED_BODY()

public:
	USpatialPlatformCoordinator();
	virtual ~USpatialPlatformCoordinator();

	virtual void Init(UNetDriver* InDriver);

	void StartSendingHeartbeat();
	void SendReadyStatus();
	void StartPollingForGameserverStatus();
	void StartWatchingForGameserverStatus();
	void StartPollingForWorkerFlags();

private:
	USpatialNetDriver* Driver;
	FString Url;

	FTimerHandle HeartBeatTimerHandler;
	FTimerHandle GameserverStatusTimerHandler;
	FTimerHandle WorkerFlagsTimerHandler;

	TMap<FString, TSharedRef<IHttpRequest, ESPMode::ThreadSafe>> CachedRequests;

	FString HeartBeatRequestKey = "HeartBeatRequestKey";
	FString ReadyRequestKey = "ReadyRequestKey";
	FString GameserverRequestKey = "GameserverRequestKey";
	FString WorkerflagsRequestKey = "WorkerflagsRequestKey";
};
