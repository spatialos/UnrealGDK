// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"

#include "Schema/RPCPayload.h"

#include <WorkerSDK/improbable/c_schema.h>
#include <WorkerSDK/improbable/c_worker.h>

DECLARE_LOG_CATEGORY_EXTERN(LogSpatialRPCService, Log, All);

class USpatialStaticComponentView;
struct RPCRingBuffer;

namespace SpatialGDK
{

struct EntityRPCType
{
	EntityRPCType(Worker_EntityId EntityId, ERPCType Type)
		: EntityId(EntityId)
		, Type(Type)
	{}

	Worker_EntityId EntityId;
	ERPCType Type;

	inline bool operator==(const EntityRPCType& Other)
	{
		return EntityId == Other.EntityId && Type == Other.Type;
	}

	friend inline uint32 GetTypeHash(EntityRPCType Value)
	{
		return ::GetTypeHash(static_cast<int64>(Value.EntityId)) + 977u * ::GetTypeHash(static_cast<uint32>(Value.Type));
	}
};

struct EntityComponentId
{
	EntityComponentId(Worker_EntityId EntityId, Worker_ComponentId ComponentId)
		: EntityId(EntityId)
		, ComponentId(ComponentId)
	{}

	Worker_EntityId EntityId;
	Worker_ComponentId ComponentId;

	inline bool operator==(const EntityComponentId& Other)
	{
		return EntityId == Other.EntityId && ComponentId == Other.ComponentId;
	}

	friend inline uint32 GetTypeHash(EntityComponentId Value)
	{
		return ::GetTypeHash(static_cast<int64>(Value.EntityId)) + 977u * ::GetTypeHash(static_cast<uint32>(Value.ComponentId));
	}
};

struct RingBufferDescriptor
{
	inline Schema_FieldId GetRingBufferElementFieldId(uint64 RPCId)
	{
		return SchemaFieldStart + ((RPCId - 1) % RingBufferSize);
	}

	inline bool HasCapacity(uint64 LastAckedRPCId, uint64 NewRPCId)
	{
		return LastAckedRPCId + RingBufferSize >= NewRPCId;
	}

	ERPCType Type;

	Worker_ComponentId RingBufferComponentId;
	uint32 RingBufferSize;
	Schema_FieldId SchemaFieldStart;
	Schema_FieldId LastSentFieldId;

	Worker_ComponentId AckComponentId;
	Schema_FieldId AckFieldId;

	bool bShouldQueueOverflowed;
};

class SpatialRPCService
{
public:
	using ExtractRPCCallbackType = TFunction<bool(Worker_EntityId, ERPCType, const RPCPayload&)>;

	SpatialRPCService(ExtractRPCCallbackType ExtractRPCCallback, const USpatialStaticComponentView* View);

	void PushRPC(Worker_EntityId EntityId, ERPCType Type, RPCPayload Payload);

	struct UpdateToSend
	{
		Worker_EntityId EntityId;
		Worker_ComponentUpdate Update;
	};
	TArray<UpdateToSend> GetRPCsAndAcksToSend();
	TArray<Worker_ComponentData> GetRPCComponentsOnEntityCreation(Worker_EntityId EntityId);

	// Will also store acked IDs locally.
	// Calls ExtractRPCCallback for each RPC it extracts from a given component. If the callback returns false,
	// stops retrieving RPCs.
	void ExtractRPCsForEntity(Worker_EntityId EntityId, Worker_ComponentId ComponentId);

	/*{
		switch (ComponentId)
		{
		case CLIENT_ENDPOINT:
			ExtractForType(ClientReliable);
			ExtractForType(ClientUnreliable);

		}
	}*/

private:
	void InitRingBufferDescriptors();
	RingBufferDescriptor GetRingBufferDescriptor(ERPCType Type);

	void ExtractRPCsForType(ERPCType Type);

	void AddOverflowedRPC(EntityComponentId EntityComponent, RPCPayload Payload);

	uint64 GetAckFromView(Worker_EntityId EntityId, const RingBufferDescriptor& Descriptor);
	const RPCRingBuffer& GetBufferFromView(Worker_EntityId EntityId, const RingBufferDescriptor& Descriptor);

private:
	ExtractRPCCallbackType ExtractRPCCallback;
	const USpatialStaticComponentView* View;

	TMap<ERPCType, RingBufferDescriptor> RingBufferDescriptors;

	// This is local, not written into schema.
	TMap<Worker_EntityId_Key, uint64> LastSeenMulticastRPCIds;

	// Stored here for things we have authority over.
	TMap<EntityRPCType, uint64> LastAckedRPCIds;
	TMap<EntityRPCType, uint64> LastSentRPCIds;

	TMap<EntityComponentId, Schema_ComponentData*> PendingRPCsOnEntityCreation;

	TMap<EntityComponentId, Schema_ComponentUpdate*> PendingComponentUpdatesToSend;
	TMap<EntityComponentId, TArray<RPCPayload>> OverflowedRPCs;
};

} // namespace SpatialGDK
