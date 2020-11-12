// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"

#include <WorkerSDK/improbable/c_schema.h>
#include <WorkerSDK/improbable/c_trace.h>

namespace SpatialGDK
{
struct EventTraceUniqueId
{
	uint64 Hash{ 0 };

	FString ToString() const;

	bool IsValid() const
	{
		return Hash != 0;
	}

	//static EventTraceUniqueId ReadTraceIDFromSchemaObject(Schema_Object* Obj, Schema_FieldId FieldId);
	//static void WriteTraceIDToSchemaObject(const EventTraceUniqueId& Id, Schema_Object* Obj, Schema_FieldId FieldId);
	//static EventTraceUniqueId GenerateUnique(const Trace_SpanId& SpanId);

	static EventTraceUniqueId GenerateUnique(uint64 Entity, uint64 Component, UFunction* Function);
};
} // namespace SpatialGDK
