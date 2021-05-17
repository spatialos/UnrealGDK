// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "Interop/Connection/SpatialGDKSpanId.h"
#include "Schema/RPCPayload.h"
#include "SpatialConstants.h"
#include "SpatialView/EntityComponentId.h"

DECLARE_DELEGATE_RetVal_OneParam(bool, ActorCanExtractRPCDelegate, Worker_EntityId);
DECLARE_DELEGATE_FourParams(ExtractRPCDelegate, const FUnrealObjectRef&, const SpatialGDK::RPCSender&, SpatialGDK::RPCPayload,
							TOptional<uint64>);

namespace SpatialGDK
{
struct EntityRPCType
{
	EntityRPCType(Worker_EntityId EntityId, ERPCType Type)
		: EntityId(EntityId)
		, Type(Type)
	{
	}

	Worker_EntityId EntityId;
	ERPCType Type;

	friend bool operator==(const EntityRPCType& Lhs, const EntityRPCType& Rhs)
	{
		return Lhs.EntityId == Rhs.EntityId && Lhs.Type == Rhs.Type;
	}

	friend uint32 GetTypeHash(EntityRPCType Value)
	{
		return HashCombine(::GetTypeHash(static_cast<int64>(Value.EntityId)), ::GetTypeHash(static_cast<uint32>(Value.Type)));
	}
};

enum class EPushRPCResult : uint8
{
	Success,

	QueueOverflowed,
	DropOverflowed,
	HasAckAuthority,
	NoRingBufferAuthority,
	EntityBeingCreated
};

struct PendingUpdate
{
	PendingUpdate(Schema_ComponentUpdate* InUpdate)
		: Update(InUpdate)
	{
	}

	Schema_ComponentUpdate* Update;
	TArray<FSpatialGDKSpanId> SpanIds;
};

struct PendingRPCPayload
{
	PendingRPCPayload(const RPCPayload& InPayload, const FSpatialGDKSpanId& SpanId)
		: Payload(InPayload)
		, SpanId(SpanId)
	{
	}

	RPCPayload Payload;
	FSpatialGDKSpanId SpanId;
};

struct FRPCStore
{
	Schema_ComponentUpdate* GetOrCreateComponentUpdate(EntityComponentId EntityComponentIdPair, const FSpatialGDKSpanId& SpanId = {});
	Schema_ComponentData* GetOrCreateComponentData(EntityComponentId EntityComponentIdPair);
	void AddSpanIdForComponentUpdate(EntityComponentId EntityComponentIdPair, const FSpatialGDKSpanId& SpanId);

	TMap<EntityRPCType, uint64> LastSentRPCIds;
	TMap<EntityComponentId, PendingUpdate> PendingComponentUpdatesToSend;
	TMap<EntityComponentId, Schema_ComponentData*> PendingRPCsOnEntityCreation;
};
} // namespace SpatialGDK
