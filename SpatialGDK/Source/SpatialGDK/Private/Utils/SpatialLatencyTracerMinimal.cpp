#include "Utils/SpatialLatencyTracerMinimal.h"
#include "SpatialConstants.h"
#include "Utils/SpatialLatencyTracer.h"

// We can't use the types directly because of a mismatch between the legacy headers included by SpatialLatencyTracer
static_assert(sizeof(uint32) == sizeof(Schema_FieldId), "Expected size match");
static_assert(sizeof(int32) == sizeof(TraceKey), "Expected size match");

int32 FSpatialLatencyTracerMinimal::ReadTraceFromSchemaObject(worker::c::Schema_Object* Obj, uint32 FieldId)
{
#if TRACE_LIB_ACTIVE
	if (USpatialLatencyTracer* Tracer = USpatialLatencyTracer::GetTracer(nullptr))
	{
		return Tracer->ReadTraceFromSchemaObject(Obj, (Schema_FieldId)FieldId);
	}
#endif
	return static_cast<int32>(InvalidTraceKey);
}

void FSpatialLatencyTracerMinimal::WriteTraceToSchemaObject(int32 Key, worker::c::Schema_Object* Obj, uint32 FieldId)
{
#if TRACE_LIB_ACTIVE
	if (USpatialLatencyTracer* Tracer = USpatialLatencyTracer::GetTracer(nullptr))
	{
		Tracer->WriteTraceToSchemaObject(static_cast<TraceKey>(Key), Obj, (Schema_FieldId)SpatialConstants::UNREAL_RPC_PAYLOAD_TRACE_ID);
	}
#endif
}
