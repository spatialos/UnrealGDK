// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "SpatialConstants.h"
#include "Utils/SpatialLatencyTracer.h"
#include "CoreMinimal.h"

class FSpatialLatencyTracerMinimal
{
public:
	static TraceKey ReadTraceFromSchemaObject(worker::c::Schema_Object* Obj, Schema_FieldId FieldId)
	{
#if TRACE_LIB_ACTIVE
		if (USpatialLatencyTracer* Tracer = USpatialLatencyTracer::GetTracer(nullptr))
		{
			return Tracer->ReadTraceFromSchemaObject(Obj, static_cast<Schema_FieldId>(FieldId));
		}
#endif
		return InvalidTraceKey;
	}
	static void WriteTraceToSchemaObject(TraceKey Key, worker::c::Schema_Object* Obj, Schema_FieldId FieldId)
	{
#if TRACE_LIB_ACTIVE
		if (USpatialLatencyTracer* Tracer = USpatialLatencyTracer::GetTracer(nullptr))
		{
			Tracer->WriteTraceToSchemaObject(Key, Obj, FieldId);
		}
#endif
	}
};
