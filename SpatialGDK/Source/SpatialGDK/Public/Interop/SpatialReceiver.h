// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"

#include "EngineClasses/SpatialActorChannel.h"
#include "EngineClasses/SpatialNetDriver.h"
#include "EngineClasses/SpatialPackageMapClient.h"
#include "Interop/RPCs/SpatialRPCService.h"
#include "Interop/SpatialClassInfoManager.h"
#include "Interop/SpatialOSDispatcherInterface.h"
#include "Schema/UnrealObjectRef.h"
#include "SpatialCommonTypes.h"

#include <WorkerSDK/improbable/c_schema.h>
#include <WorkerSDK/improbable/c_worker.h>

#include "SpatialReceiver.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogSpatialReceiver, Log, All);

class USpatialNetConnection;
class USpatialSender;
class UGlobalStateManager;
class SpatialLoadBalanceEnforcer;

namespace SpatialGDK
{
class SpatialEventTracer;
} // namespace SpatialGDK

UCLASS()
class USpatialReceiver : public UObject, public SpatialOSDispatcherInterface
{
	GENERATED_BODY()

public:
	void Init(USpatialNetDriver* NetDriver, SpatialGDK::SpatialEventTracer* InEventTracer);

	virtual void AddPendingReliableRPC(Worker_RequestId RequestId, TSharedRef<struct FReliableRPCForRetry> ReliableRPC) override;

	void OnDisconnect(uint8 StatusCode, const FString& Reason);

private:
	UPROPERTY()
	USpatialNetDriver* NetDriver;

	UPROPERTY()
	USpatialSender* Sender;

	UPROPERTY()
	USpatialPackageMapClient* PackageMap;

	FReliableRPCMap PendingReliableRPCs;

	TMap<Worker_RequestId_Key, EntityQueryDelegate> EntityQueryDelegates;
	SpatialGDK::SpatialEventTracer* EventTracer;
};
