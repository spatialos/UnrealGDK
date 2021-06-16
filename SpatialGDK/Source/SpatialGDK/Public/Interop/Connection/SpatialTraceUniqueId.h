// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"

#include <WorkerSDK/improbable/c_worker.h>

#include "Utils/GDKPropertyMacros.h"

class UFunction;

namespace SpatialGDK
{
class EventTraceUniqueId
{
	uint32 Hash = 0;
	EventTraceUniqueId(uint32 Hash)
		: Hash(Hash)
	{
	}

public:
	FString ToString() const;
	uint32 Get() const { return Hash; };

	bool IsValid() const { return Hash != 0; }

	static EventTraceUniqueId GenerateForRPC(FSpatialEntityId Entity, uint8 Type, uint64 RPCId);
	static EventTraceUniqueId GenerateForNamedRPC(FSpatialEntityId Entity, FName Name, uint64 RPCId);
	static EventTraceUniqueId GenerateForProperty(FSpatialEntityId Entity, const GDK_PROPERTY(Property) * Property);
	static EventTraceUniqueId GenerateForCrossServerRPC(FSpatialEntityId Entity, uint64 UniqueRequestId);
};
} // namespace SpatialGDK
