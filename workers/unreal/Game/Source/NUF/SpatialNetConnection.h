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
};
