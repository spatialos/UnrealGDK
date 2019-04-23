// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "Schema/Component.h"
#include "SpatialConstants.h"
#include "Utils/SchemaUtils.h"

#include <WorkerSDK/improbable/c_schema.h>
#include <WorkerSDK/improbable/c_worker.h>

namespace improbable
{

struct RPCPayload
{
	//RPCPayload() = default;
	RPCPayload() = delete;

	RPCPayload(uint32 InOffset, uint32 InIndex) : Offset(InOffset), Index(InIndex)
	{
	}

	RPCPayload(const Schema_Object* RPCObject)
	{
		Offset = Schema_GetUint32(RPCObject, SpatialConstants::UNREAL_RPC_PAYLOAD_OFFSET_ID);
		Index = Schema_GetUint32(RPCObject, SpatialConstants::UNREAL_RPC_PAYLOAD_RPC_INDEX_ID);
		PayloadData = improbable::GetBytesFromSchema(RPCObject, SpatialConstants::UNREAL_RPC_PAYLOAD_RPC_PAYLOAD_ID);
	}

	void WriteToSchemaObject(Schema_Object* RPCObject, FBitWriter& Writer) const
	{
		Schema_AddUint32(RPCObject, SpatialConstants::UNREAL_RPC_PAYLOAD_OFFSET_ID, Offset);
		Schema_AddUint32(RPCObject, SpatialConstants::UNREAL_RPC_PAYLOAD_RPC_INDEX_ID, Index);
		improbable::AddBytesToSchema(RPCObject, SpatialConstants::UNREAL_RPC_PAYLOAD_RPC_PAYLOAD_ID, Writer);
	}

	uint32 Offset;
	uint32 Index;
	// TODO: Copy later?
	TArray<uint8> PayloadData;
};

struct RPCsOnEntityCreation : Component
{
	static const Worker_ComponentId ComponentId = SpatialConstants::RPC_ON_ENTITY_CREATION_ID;

	RPCsOnEntityCreation() = default;

	RPCsOnEntityCreation(const Worker_ComponentData& Data)
	{
		Schema_Object* ComponentObject = Schema_GetComponentDataFields(Data.schema_type);

		RPCs.Add(RPCPayload(ComponentObject));
	}

	Worker_ComponentData CreateRPCPayloadData()
	{
		Worker_ComponentData Data = {};
		Data.component_id = ComponentId;
		Data.schema_type = Schema_CreateComponentData(ComponentId);
		Schema_Object* ComponentObject = Schema_GetComponentDataFields(Data.schema_type);

		for (const auto& elem : RPCs)
		{
			FBitWriter DefaultWriter;
			elem.WriteToSchemaObject(ComponentObject, DefaultWriter);
			break;
		}

		return Data;
	}

	TArray<RPCPayload> RPCs;
};

/*
inline void AddRPCToSchema(Schema_Object* Object, Schema_FieldId Id, RPCPayload Data, FBitWriter& Writer)
{
	//Schema_Object* RPCObject = Schema_AddObject(Object, Id);
	Schema_Object* RPCObject = Object;

	Schema_AddUint32(RPCObject, SpatialConstants::UNREAL_RPC_PAYLOAD_OFFSET_ID, Data.Offset);
	Schema_AddUint32(RPCObject, SpatialConstants::UNREAL_RPC_PAYLOAD_RPC_INDEX_ID, Data.Index);
	AddBytesToSchema(RPCObject, SpatialConstants::UNREAL_RPC_PAYLOAD_RPC_PAYLOAD_ID, Writer);
}

inline RPCPayload GetRPCFromSchema(Schema_Object* Object, Schema_FieldId Id)
{
	RPCPayload RPC;

	//Schema_Object* RPCObject = Schema_GetObject(Object, Id);
	Schema_Object* RPCObject = Object;

	RPC.Offset = Schema_GetUint32(RPCObject, SpatialConstants::UNREAL_RPC_PAYLOAD_OFFSET_ID);
	RPC.Index = Schema_GetUint32(RPCObject, SpatialConstants::UNREAL_RPC_PAYLOAD_RPC_INDEX_ID);
	RPC.PayloadData = GetBytesFromSchema(RPCObject, SpatialConstants::UNREAL_RPC_PAYLOAD_RPC_PAYLOAD_ID);

	return RPC;
}
*/
/*
struct RPCData : Component
{
	static const Worker_ComponentId ComponentId = SpatialConstants::RPC_ON_ENTITY_CREATION_ID;

	RPCData() = default;

	RPCData(AActor* Actor)
	{
		const USceneComponent* RootComponent = Actor->GetRootComponent();

		Location = RootComponent ? FRepMovement::RebaseOntoZeroOrigin(Actor->GetActorLocation(), Actor) : FVector::ZeroVector;
		Rotation = RootComponent ? Actor->GetActorRotation() : FRotator::ZeroRotator;
		Scale = RootComponent ? Actor->GetActorScale() : FVector::OneVector;
		Velocity = RootComponent ? Actor->GetVelocity() : FVector::ZeroVector;
	}

	RPCData(const Worker_ComponentData& Data)
	{
		Schema_Object* ComponentObject = Schema_GetComponentDataFields(Data.schema_type);

		Location = GetVectorFromSchema(ComponentObject, 1);
		Rotation = GetRotatorFromSchema(ComponentObject, 2);
		Scale = GetVectorFromSchema(ComponentObject, 3);
		Velocity = GetVectorFromSchema(ComponentObject, 4);
	}

	Worker_ComponentData CreateRPCDataData()
	{
		Worker_ComponentData Data = {};
		Data.component_id = ComponentId;
		Data.schema_type = Schema_CreateComponentData(ComponentId);
		Schema_Object* ComponentObject = Schema_GetComponentDataFields(Data.schema_type);

		AddVectorToSchema(ComponentObject, 1, Location);
		AddRotatorToSchema(ComponentObject, 2, Rotation);
		AddVectorToSchema(ComponentObject, 3, Scale);
		AddVectorToSchema(ComponentObject, 4, Velocity);

		return Data;
	}

	FVector Location;
	FRotator Rotation;
	FVector Scale;
	FVector Velocity;
};
*/

}
