// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"

#include <WorkerSDK/improbable/c_schema.h>
#include <WorkerSDK/improbable/c_trace.h>

class UFunction;

namespace SpatialGDK
{
struct EventTraceUniqueId
{
	uint32 Hash{ 0 };
	EventTraceUniqueId(uint32 Hash)
		: Hash(Hash)
	{
	}

	FString ToString() const;

	bool IsValid() const { return Hash != 0; }

	static EventTraceUniqueId Generate(uint64 Entity, uint8 Type, uint64 Id);
};
} // namespace SpatialGDK
