// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "IpConnection.h"

#include "SpatialNetConnection.generated.h"

UCLASS(transient)
class SPATIALGDK_API USpatialNetConnection : public UIpConnection
{
	GENERATED_BODY()
public:
	USpatialNetConnection(const FObjectInitializer& ObjectInitializer);

	virtual void InitBase(UNetDriver* InDriver, class FSocket* InSocket, const FURL& InURL, EConnectionState InState, int32 InMaxPacket = 0, int32 InPacketOverhead = 0) override;
	virtual void InitRemoteConnection(UNetDriver* InDriver, class FSocket* InSocket, const FURL& InURL, const class FInternetAddr& InRemoteAddr, EConnectionState InState, int32 InMaxPacket = 0, int32 InPacketOverhead = 0) override;
	virtual void InitLocalConnection(UNetDriver* InDriver, class FSocket* InSocket, const FURL& InURL, EConnectionState InState, int32 InMaxPacket = 0, int32 InPacketOverhead = 0) override;
	virtual void LowLevelSend(void* Data, int32 CountBytes, int32 CountBits) override;
	virtual bool ClientHasInitializedLevelFor(const AActor* TestActor) const override;
	virtual void Tick() override;
	virtual int32 IsNetReady(bool Saturate) override;

	// These functions don't make a lot of sense in a SpatialOS implementation.
	virtual FString LowLevelGetRemoteAddress(bool bAppendPort = false) override { return TEXT(""); }
	virtual FString LowLevelDescribe() override { return TEXT(""); }
	virtual FString RemoteAddressToString() override { return TEXT(""); }
	///////

	UPROPERTY()
	bool bReliableSpatialConnection;

	UPROPERTY()
	FString WorkerAttribute;
};
