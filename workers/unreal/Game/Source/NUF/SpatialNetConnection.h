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
	void InitBase(UNetDriver* InDriver, class FSocket* InSocket, const FURL& InURL, EConnectionState InState, int32 InMaxPacket = 0, int32 InPacketOverhead = 0) override;
};
