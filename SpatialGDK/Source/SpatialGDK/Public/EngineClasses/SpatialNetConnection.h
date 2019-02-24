// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "IpConnection.h"

#include <WorkerSDK/improbable/c_worker.h>

#include "SpatialNetConnection.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogSpatialNetConnection, Log, All);

UCLASS(transient)
class SPATIALGDK_API USpatialNetConnection : public UIpConnection
{
	GENERATED_BODY()
public:
	USpatialNetConnection(const FObjectInitializer& ObjectInitializer);

	virtual void BeginDestroy() override;

	virtual void InitBase(UNetDriver* InDriver, class FSocket* InSocket, const FURL& InURL, EConnectionState InState, int32 InMaxPacket = 0, int32 InPacketOverhead = 0) override;
	virtual void LowLevelSend(void* Data, int32 CountBytes, int32 CountBits) override;
	virtual bool ClientHasInitializedLevelFor(const AActor* TestActor) const override;
	virtual void Tick() override;
	virtual int32 IsNetReady(bool Saturate) override;

	// These functions don't make a lot of sense in a SpatialOS implementation.
	virtual FString LowLevelGetRemoteAddress(bool bAppendPort = false) override { return TEXT(""); }
	virtual FString LowLevelDescribe() override { return TEXT(""); }
	virtual FString RemoteAddressToString() override { return TEXT(""); }
	///////

	void InitHeartbeat(class FTimerManager* InTimerManager, Worker_EntityId InPlayerControllerEntity);
	void SetHeartbeatTimeoutTimer();
	void SetHeartbeatEventTimer();

	void DisableHeartbeat();

	void OnHeartbeat();

	UPROPERTY()
	bool bReliableSpatialConnection;

	UPROPERTY()
	FString WorkerAttribute;

	class FTimerManager* TimerManager;

	// Player lifecycle
	Worker_EntityId PlayerControllerEntity;
	FTimerHandle HeartbeatTimer;
};
