// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "Schema/Interest.h"

#include "CoreMinimal.h"
#include "Misc/Optional.h"
#include "IpConnection.h"
#include "Runtime/Launch/Resources/Version.h"

#include <WorkerSDK/improbable/c_worker.h>

#include "SpatialNetConnection.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogSpatialNetConnection, Log, All);

UCLASS(transient)
class SPATIALGDK_API USpatialNetConnection : public UIpConnection
{
	GENERATED_BODY()
public:
	USpatialNetConnection(const FObjectInitializer& ObjectInitializer);

	// Begin NetConnection Interface
	virtual void BeginDestroy() override;

	virtual void InitBase(UNetDriver* InDriver, class FSocket* InSocket, const FURL& InURL, EConnectionState InState, int32 InMaxPacket = 0, int32 InPacketOverhead = 0) override;
	virtual void LowLevelSend(void* Data, int32 CountBits, FOutPacketTraits& Traits) override;
	virtual bool ClientHasInitializedLevelFor(const AActor* TestActor) const override;
	virtual int32 IsNetReady(bool Saturate) override;

	/** Called by PlayerController to tell connection about client level visibility change */
#if ENGINE_MINOR_VERSION <= 23
	virtual void UpdateLevelVisibility(const FName& PackageName, bool bIsVisible) override;
#else
	virtual void UpdateLevelVisibility(const struct FUpdateLevelVisibilityLevelInfo& LevelVisibility) override;
#endif

	virtual void FlushDormancy(class AActor* Actor) override;

	virtual bool IsReplayConnection() const override { return false; }

	// These functions don't make a lot of sense in a SpatialOS implementation.
	virtual FString LowLevelGetRemoteAddress(bool bAppendPort = false) override { return TEXT(""); }
	virtual FString LowLevelDescribe() override { return TEXT(""); }
	virtual FString RemoteAddressToString() override { return TEXT(""); }
	virtual void CleanUp() override;
	///////
	// End NetConnection Interface

	void InitHeartbeat(class FTimerManager* InTimerManager, Worker_EntityId InPlayerControllerEntity);
	void SetHeartbeatTimeoutTimer();
	void SetHeartbeatEventTimer();

	void DisableHeartbeat();

	void OnHeartbeat();

	void ClientNotifyClientHasQuit();

	UPROPERTY()
	bool bReliableSpatialConnection;

	// Only used on the server for client connections.
	FString ConnectionOwningWorkerId;
	Worker_EntityId ConnectingClientSystemEntityId;

	class FTimerManager* TimerManager;

	// Player lifecycle
	Worker_EntityId PlayerControllerEntity;
	FTimerHandle HeartbeatTimer;
};
