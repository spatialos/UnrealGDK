// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "Connection/SpatialGDKSpanId.h"
#include "Schema/RPCPayload.h"
#include "SpatialCommonTypes.h"

namespace SpatialGDK
{
struct FCrossServerRPCParams
{
	FCrossServerRPCParams(const FUnrealObjectRef& InObjectRef, const Worker_RequestId_Key InRequestId, RPCPayload&& InPayload,
						  const uint32 InTimeoutMillis, const FSpatialGDKSpanId& InSpanId)
		: ObjectRef(InObjectRef)
		, Payload(MoveTemp(InPayload))
		, RequestId(InRequestId)
		, Timestamp(FDateTime::Now())
		, TimeoutMillis(InTimeoutMillis)
		, SpanId(InSpanId)
	{
	}

	// Moveable, not copyable.
	FCrossServerRPCParams() = delete;
	FCrossServerRPCParams(const FCrossServerRPCParams&) = delete;
	FCrossServerRPCParams(FCrossServerRPCParams&&) = default;
	FCrossServerRPCParams& operator=(const FCrossServerRPCParams&) = delete;
	FCrossServerRPCParams& operator=(FCrossServerRPCParams&&) = delete;
	~FCrossServerRPCParams() = default;

	FUnrealObjectRef ObjectRef;
	RPCPayload Payload;
	Worker_RequestId_Key RequestId;
	FDateTime Timestamp;
	uint32 TimeoutMillis;
	const FSpatialGDKSpanId SpanId;
};
} // namespace SpatialGDK
