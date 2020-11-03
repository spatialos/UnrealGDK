// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"

#include <WorkerSDK/improbable/c_schema.h>

namespace SpatialGDK
{
struct EventTraceUniqueId
{
	uint8 internal_bytes[8] = { 0 }; // This size may need to be adjusted in the future. Usage should not be direct.

	FString GetString() const;

	bool IsValid() const
	{
		for (int i = 0; i < sizeof(internal_bytes); i++)
		{
			if (internal_bytes[i] != 0)
			{
				return true;
			}
		}
		return false;
	}

	static EventTraceUniqueId ReadFromSchemaObject(Schema_Object* Obj, Schema_FieldId FieldId);
	static void WriteToSchemaObject(EventTraceUniqueId Id, Schema_Object* Obj, Schema_FieldId FieldId);
	static EventTraceUniqueId GenerateUnique();
};
} // namespace SpatialGDK
