// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "Interop/Connection/SpatialWorkerConnection.h"

using namespace SpatialGDK;

void USpatialWorkerConnection::SetEventTracer(SpatialGDK::SpatialEventTracer* EventTracerIn)
{
	EventTracer = EventTracerIn;
}
