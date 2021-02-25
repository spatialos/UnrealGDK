// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "Interop/CrossServerRPCHandler.h"
#include "EngineClasses/SpatialLoadBalanceEnforcer.h"
#include "EngineClasses/SpatialNetBitWriter.h"
#include "Interop/Connection/SpatialGDKSpanId.h"
#include "Interop/RPCs/SpatialRPCService.h"
#include "Interop/SpatialClassInfoManager.h"
#include "Utils/RPCContainer.h"
#include "Utils/RepDataUtils.h"

#include "CoreMinimal.h"

#include "SpatialSender.generated.h"

class USpatialActorChannel;
class SpatialDispatcher;
class USpatialNetDriver;
class USpatialPackageMapClient;
class USpatialReceiver;
class USpatialStaticComponentView;
class USpatialClassInfoManager;
class USpatialWorkerConnection;

namespace SpatialGDK
{
class SpatialEventTracer;
}

// TODO: Clear TMap entries when USpatialActorChannel gets deleted - UNR:100
// care for actor getting deleted before actor channel
using FChannelObjectPair = TPair<TWeakObjectPtr<USpatialActorChannel>, TWeakObjectPtr<UObject>>;
using FUpdatesQueuedUntilAuthority = TMap<Worker_EntityId_Key, TArray<FWorkerComponentUpdate>>;

UCLASS()
class SPATIALGDK_API USpatialSender : public UObject
{
	GENERATED_BODY()

public:
	void Init(USpatialNetDriver* InNetDriver, FTimerManager* InTimerManager, SpatialGDK::SpatialRPCService* InRPCService,
			  SpatialGDK::SpatialEventTracer* InEventTracer);

	void SendCommandResponse(Worker_RequestId RequestId, Worker_CommandResponse& Response, const FSpatialGDKSpanId& CauseSpanId);
	void SendEmptyCommandResponse(Worker_ComponentId ComponentId, Schema_FieldId CommandIndex, Worker_RequestId RequestId,
								  const FSpatialGDKSpanId& CauseSpanId);
	void SendCommandFailure(Worker_RequestId RequestId, const FString& Message, const FSpatialGDKSpanId& CauseSpanId);

	UPROPERTY()
	USpatialWorkerConnection* Connection;

	UPROPERTY()
	USpatialReceiver* Receiver;

	UPROPERTY()
	USpatialPackageMapClient* PackageMap;

	UPROPERTY()
	USpatialClassInfoManager* ClassInfoManager;

	FTimerManager* TimerManager;

	SpatialGDK::SpatialRPCService* RPCService;

	TArray<TSharedRef<FReliableRPCForRetry>> RetryRPCs;

	SpatialGDK::SpatialEventTracer* EventTracer;
};
