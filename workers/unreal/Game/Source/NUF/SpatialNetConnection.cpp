// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "SpatialNetConnection.h"

USpatialNetConnection::USpatialNetConnection(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	InternalAck = 1;
}
