// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "Containers/UnrealString.h"
#include "Misc/Optional.h"
#include "SpatialView/CommandRequest.h"
#include "SpatialView/CommandResponse.h"
#include "SpatialView/ComponentData.h"
#include "SpatialView/EntityQuery.h"
#include "UObject/NameTypes.h"
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
	TOptional<FSpatialEntityId> EntityId;
	TOptional<uint32> TimeoutMillis;
	FSpatialGDKSpanId SpanId;
};

struct DeleteEntityRequest
{
	Worker_RequestId RequestId;
	FSpatialEntityId EntityId;
	TOptional<uint32> TimeoutMillis;
	FSpatialGDKSpanId SpanId;
};

struct EntityQueryRequest
{
	Worker_RequestId RequestId;
	EntityQuery Query;
	TOptional<uint32> TimeoutMillis;
};

struct EntityCommandRequest
{
	FSpatialEntityId EntityId;
	Worker_RequestId RequestId;
	CommandRequest Request;
	TOptional<uint32> TimeoutMillis;
	FSpatialGDKSpanId SpanId;
};

struct EntityCommandResponse
{
	Worker_RequestId RequestId;
	CommandResponse Response;
	FSpatialGDKSpanId SpanId;
};

struct EntityCommandFailure
{
	Worker_RequestId RequestId;
	FString Message;
	FSpatialGDKSpanId SpanId;
};

struct LogMessage
{
	Worker_LogLevel Level;
	FName LoggerName;
	FString Message;
};

} // namespace SpatialGDK
