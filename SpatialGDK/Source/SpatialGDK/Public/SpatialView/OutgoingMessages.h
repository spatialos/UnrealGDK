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
	FRequestId RequestId;
	uint32 NumberOfEntityIds;
	TOptional<uint32> TimeoutMillis;
};

struct CreateEntityRequest
{
	FRequestId RequestId;
	TArray<ComponentData> EntityComponents;
	TOptional<FEntityId> EntityId;
	TOptional<uint32> TimeoutMillis;
	TOptional<Trace_SpanId> SpanId;
};

struct DeleteEntityRequest
{
	FRequestId RequestId;
	FEntityId EntityId;
	TOptional<uint32> TimeoutMillis;
	TOptional<Trace_SpanId> SpanId;
};

struct EntityQueryRequest
{
	FRequestId RequestId;
	EntityQuery Query;
	TOptional<uint32> TimeoutMillis;
};

struct EntityCommandRequest
{
	FEntityId EntityId;
	FRequestId RequestId;
	CommandRequest Request;
	TOptional<uint32> TimeoutMillis;
	TOptional<Trace_SpanId> SpanId;
};

struct EntityCommandResponse
{
	FRequestId RequestId;
	CommandResponse Response;
	TOptional<Trace_SpanId> SpanId;
};

struct EntityCommandFailure
{
	FRequestId RequestId;
	FString Message;
	TOptional<Trace_SpanId> SpanId;
};

struct LogMessage
{
	Worker_LogLevel Level;
	FName LoggerName;
	FString Message;
};

} // namespace SpatialGDK
