// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "Interop/Connection/SpatialWorkerConnection.h"

#include <WorkerSDK/improbable/c_trace.h>

using namespace SpatialGDK;

void USpatialWorkerConnection::SetEventTracer(SpatialEventTracer* EventTracerIn)
{
	EventTracer = EventTracerIn;
}

SpatialSpanId USpatialWorkerConnection::CreateActiveSpan()
{
	return EventTracer->CreateActiveSpan();
}

