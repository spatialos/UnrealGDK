// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "IpConnection.h"
#include "SpatialNetConnection.generated.h"

/**
 * 
 */
UCLASS(transient, config = Engine)
class NUF_API USpatialNetConnection : public UIpConnection
{
	GENERATED_BODY()
public:
	USpatialNetConnection(const FObjectInitializer& ObjectInitializer);

	virtual void InitBase(UNetDriver* InDriver, class FSocket* InSocket, const FURL& InURL, EConnectionState InState, int32 InMaxPacket = 0, int32 InPacketOverhead = 0) override;
	virtual void InitRemoteConnection(UNetDriver* InDriver, class FSocket* InSocket, const FURL& InURL, const class FInternetAddr& InRemoteAddr, EConnectionState InState, int32 InMaxPacket = 0, int32 InPacketOverhead = 0) override;
	virtual void InitLocalConnection(UNetDriver* InDriver, class FSocket* InSocket, const FURL& InURL, EConnectionState InState, int32 InMaxPacket = 0, int32 InPacketOverhead = 0) override;
	virtual bool ClientHasInitializedLevelFor(const UObject* TestObject) const override;
	virtual void LowLevelSend(void* Data, int32 CountBytes, int32 CountBits) override;

	UPROPERTY()
	bool bReliableSpatialConnection;
};
