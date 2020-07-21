// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "Interop/Connection/SpatialWorkerConnection.h"

using namespace SpatialGDK;

void USpatialWorkerConnection::SetEventTracer(SpatialEventTracer* EventTracerIn)
{
	EventTracer = EventTracerIn;
}

SpatialSpanId USpatialWorkerConnection::CreateActiveSpan()
{
	return EventTracer->CreateActiveSpan();
}

void USpatialWorkerConnection::TraceEvent(const SpatialGDK::SpatialGDKEvent& Event)
{
	EventTracer->TraceEvent(Event);
}

