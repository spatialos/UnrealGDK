// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"

#include <WorkerSDK/improbable/c_worker.h>

#include "Utils/GDKPropertyMacros.h"

class UFunction;

namespace SpatialGDK
{
struct EventTraceUniqueId
{
	uint32 Hash = 0;
	EventTraceUniqueId(uint32 Hash)
		: Hash(Hash)
	{
	}

	FString ToString() const;

	bool IsValid() const { return Hash != 0; }

	static EventTraceUniqueId GenerateForRPC(Worker_EntityId Entity, uint8 Type, uint64 RPCId);
	static EventTraceUniqueId GenerateForProperty(Worker_EntityId Entity, const GDK_PROPERTY(Property) * Property);
};
} // namespace SpatialGDK
