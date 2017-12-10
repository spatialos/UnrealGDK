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
	virtual void InitBase(UNetDriver* InDriver, class FSocket* InSocket, const FURL& InURL, EConnectionState InState, int32 InMaxPacket = 0, int32 InPacketOverhead = 0) override;
	virtual void InitRemoteConnection(UNetDriver* InDriver, class FSocket* InSocket, const FURL& InURL, const class FInternetAddr& InRemoteAddr, EConnectionState InState, int32 InMaxPacket = 0, int32 InPacketOverhead = 0) override;
	virtual void InitLocalConnection(UNetDriver* InDriver, class FSocket* InSocket, const FURL& InURL, EConnectionState InState, int32 InMaxPacket = 0, int32 InPacketOverhead = 0) override;
	virtual void LowLevelSend(void* Data, int32 CountBytes, int32 CountBits) override;
	virtual bool ClientHasInitializedLevelFor(const UObject* TestObject) const override;

	//NUF: we might not need this in the end if we end up relying on bInternalAck.
	UPROPERTY()
	bool bFakeSpatialClient;

protected:

	//TODO-giray: Remove once we don't need to have an IP connection.
	// Currently we have regular Unreal connections that transmit through a per-client socket, along with the new "Spatial" connections.
	// We will eventually remove the old type of connection, but until then this flag will let us special case certain behavior (such as writing into a socket).
	UPROPERTY()
	bool bVanillaUnrealConnection;	
};
