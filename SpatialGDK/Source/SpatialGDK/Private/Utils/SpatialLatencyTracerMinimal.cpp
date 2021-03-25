#include "Utils/SpatialLatencyTracerMinimal.h"
#include "Utils/SpatialLatencyTracer.h"
#include "SpatialConstants.h"

// We cannot use 
static_assert(sizeof(uint32) == sizeof(Schema_FieldId), "Expected size match");
static_assert(sizeof(int32) == sizeof(TraceKey), "Expected size match");

int32 FSpatialLatencyTracerMinimal::ReadTraceFromSchemaObject(Schema_Object* Obj, uint32 FieldId)
{
#if TRACE_LIB_ACTIVE
	if (USpatialLatencyTracer* Tracer = USpatialLatencyTracer::GetTracer(nullptr))
	{
		Trace = Tracer->ReadTraceFromSchemaObject(Obj, (Schema_FieldId)FieldId);
		return Trace;
	}
#endif
	return static_cast<int32>(InvalidTraceKey);
}

void FSpatialLatencyTracerMinimal::WriteTraceToSchemaObject(int32 Key, Schema_Object* Obj, uint32 FieldId)
{
#if TRACE_LIB_ACTIVE
	if (USpatialLatencyTracer* Tracer = USpatialLatencyTracer::GetTracer(nullptr))
	{
		Tracer->WriteTraceToSchemaObject(static_cast<TraceKey>(Trace), RPCObject, (Schema_FieldId)SpatialConstants::UNREAL_RPC_PAYLOAD_TRACE_ID);
	}
#endif
}
