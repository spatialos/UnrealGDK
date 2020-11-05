// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"

#include <WorkerSDK/improbable/c_trace.h>
#include <WorkerSDK/improbable/c_schema.h>

namespace SpatialGDK
{
struct EventTraceUniqueId
{
	uint8 InternalBytes[16] = { 0 }; // This size may need to be adjusted in the future. Usage should not be direct.

	FString GetString() const;

	bool IsValid() const
	{
		for(auto& Byte : InternalBytes)
		{
			if (Byte != 0)
			{
				return true;
			}
		}
		return false;
	}

	static EventTraceUniqueId ReadFromSchemaObject(Schema_Object* Obj, Schema_FieldId FieldId);
	static void WriteToSchemaObject(const EventTraceUniqueId& Id, Schema_Object* Obj, Schema_FieldId FieldId);
	static EventTraceUniqueId GenerateUnique(const Trace_SpanId& SpanId);
};
} // namespace SpatialGDK
