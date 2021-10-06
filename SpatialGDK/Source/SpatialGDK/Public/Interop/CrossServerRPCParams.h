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
						  const FSpatialGDKSpanId& InSpanId)
		: ObjectRef(InObjectRef)
		, Payload(MoveTemp(InPayload))
		, RequestId(InRequestId)
		, Timestamp(FDateTime::Now())
		, SpanId(InSpanId)
	{
	}

	FCrossServerRPCParams() = delete;
	~FCrossServerRPCParams() = default;
	// Moveable, not copyable.
	FCrossServerRPCParams(const FCrossServerRPCParams&) = delete;
	FCrossServerRPCParams(FCrossServerRPCParams&&) = default;
	FCrossServerRPCParams& operator=(FCrossServerRPCParams&&) = default;

	FUnrealObjectRef ObjectRef;
	RPCPayload Payload;
	Worker_RequestId_Key RequestId;
	FDateTime Timestamp;
	FSpatialGDKSpanId SpanId;
};
} // namespace SpatialGDK
