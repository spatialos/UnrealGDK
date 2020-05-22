// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "SpatialView/CommandResponse.h"
#include "SpatialView/CommandRequest.h"
#include "SpatialView/EntityQuery.h"
#include "Misc/Optional.h"
#include "Containers/UnrealString.h"
#include <improbable/c_worker.h>


namespace SpatialGDK
{

struct ReserveEntityIdsRequest
{
	Worker_RequestId RequestId;
	uint32 NumberOfEntityIds;
	TOptional<uint32> TimeoutMillis;
};

struct CreateEntityRequest
{
	Worker_RequestId RequestId;
	TArray<ComponentData> EntityComponents;
	TOptional<Worker_EntityId> EntityId;
	TOptional<uint32> TimeoutMillis;
};

struct DeleteEntityRequest
{
	Worker_RequestId RequestId;
	Worker_EntityId EntityId;
	TOptional<uint32> TimeoutMillis;
};

struct EntityQueryRequest
{
	Worker_RequestId RequestId;
	EntityQuery Query;
	TOptional<uint32> TimeoutMillis;
};

struct EntityCommandRequest
{
	Worker_EntityId EntityId;
	Worker_RequestId RequestId;
	CommandRequest Request;
	TOptional<uint32> TimeoutMillis;
};

struct EntityCommandResponse
{
	Worker_RequestId RequestId;
	CommandResponse Response;
};

struct EntityCommandFailure
{
	Worker_RequestId RequestId;
	FString Message;
};

}  // namespace SpatialGDK
