// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "Schema/RPCPayload.h"
#include "SpatialCommonTypes.h"

namespace SpatialGDK
{
struct FCrossServerRPCParams
{
	FCrossServerRPCParams(const FUnrealObjectRef& InObjectRef, const Worker_RequestId_Key InRequestId, RPCPayload&& InPayload,
						  const TOptional<Trace_SpanId>& InSpanId)
		: ObjectRef(InObjectRef)
		, Payload(MoveTemp(InPayload))
		, RequestId(InRequestId)
		, Timestamp(FDateTime::Now())
		, SpanId(InSpanId)
	{
	}

	// Moveable, not copyable.
	FCrossServerRPCParams() = delete;
	FCrossServerRPCParams(const FCrossServerRPCParams&) = delete;
	FCrossServerRPCParams(FCrossServerRPCParams&&) = default;
	FCrossServerRPCParams& operator=(const FCrossServerRPCParams&) = delete;
	FCrossServerRPCParams& operator=(FCrossServerRPCParams&&) = default;
	~FCrossServerRPCParams() = default;

	FUnrealObjectRef ObjectRef;
	RPCPayload Payload;
	Worker_RequestId_Key RequestId;
	FDateTime Timestamp;
	const TOptional<Trace_SpanId> SpanId;
};
} // namespace SpatialGDK
