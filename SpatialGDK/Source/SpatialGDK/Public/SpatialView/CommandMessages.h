// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once
#include "Misc/Optional.h"
#include "Containers/UnrealString.h"
#include <improbable/c_worker.h>

namespace SpatialGDK
{

struct CreateEntityRequest
{
	Worker_RequestId RequestId;
	// todo this should be a owning entity state type.
	Worker_ComponentData* EntityComponents;
	uint32 ComponentCount;
	TOptional<Worker_EntityId> EntityId;
	TOptional<uint32> TimeoutMillis;
};

struct CreateEntityResponse
{
	Worker_RequestId RequestId;
	Worker_StatusCode StatusCode;
	FString Message;
	Worker_EntityId EntityId;
};

}  // namespace SpatialGDK
