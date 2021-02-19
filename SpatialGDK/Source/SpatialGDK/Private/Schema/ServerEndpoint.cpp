// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "Schema/ServerEndpoint.h"

#include "Schema/LoadBalancingStuff.h"

namespace SpatialGDK
{
ServerEndpoint::ServerEndpoint(Schema_ComponentData* Data)
	: ReliableRPCBuffer(ERPCType::ClientReliable)
	, UnreliableRPCBuffer(ERPCType::ClientUnreliable)
{
	ReadFromSchema(Schema_GetComponentDataFields(Data));
}

void ServerEndpoint::ApplyComponentUpdate(Schema_ComponentUpdate* Update)
{
	ReadFromSchema(Schema_GetComponentUpdateFields(Update));
}

void ServerEndpoint::ReadFromSchema(Schema_Object* SchemaObject)
{
	RPCRingBufferUtils::ReadBufferFromSchema(SchemaObject, ReliableRPCBuffer);
	RPCRingBufferUtils::ReadBufferFromSchema(SchemaObject, UnreliableRPCBuffer);
	RPCRingBufferUtils::ReadAckFromSchema(SchemaObject, ERPCType::ServerReliable, ReliableRPCAck);
	RPCRingBufferUtils::ReadAckFromSchema(SchemaObject, ERPCType::ServerUnreliable, UnreliableRPCAck);
	RPCRingBufferUtils::ReadAckFromSchema(SchemaObject, ERPCType::ServerAlwaysWrite, AlwaysWriteRPCAck);
}

} // namespace SpatialGDK

namespace SpatialGDK
{
LoadBalancingStuff::LoadBalancingStuff(const Worker_ComponentData& Data)
	: LoadBalancingStuff(*Data.schema_type)
{
}

LoadBalancingStuff::LoadBalancingStuff(Schema_ComponentData& Data)
{
	Schema_Object* ComponentObject = Schema_GetComponentDataFields(&Data);

	ActorGroupId = Schema_GetUint32(ComponentObject, 1);

	ActorSetId = Schema_GetUint32(ComponentObject, 2);
}

Worker_ComponentData LoadBalancingStuff::CreateComponentData() const
{
	Worker_ComponentData Data = {};
	Data.component_id = ComponentId;
	Data.schema_type = Schema_CreateComponentData();
	Schema_Object* ComponentObject = Schema_GetComponentDataFields(Data.schema_type);

	Schema_AddUint32(ComponentObject, ActorGroupId, 1);
	Schema_AddUint32(ComponentObject, ActorSetId, 2);

	return Data;
}

Worker_ComponentUpdate LoadBalancingStuff::CreateLoadBalancingStuffUpdate() const
{
	Worker_ComponentUpdate Update = {};
	Update.component_id = ComponentId;
	Update.schema_type = Schema_CreateComponentUpdate();
	Schema_Object* ComponentObject = Schema_GetComponentUpdateFields(Update.schema_type);

	Schema_AddUint32(ComponentObject, ActorGroupId, 1);
	Schema_AddUint32(ComponentObject, ActorSetId, 2);

	return Update;
}

void LoadBalancingStuff::ApplyComponentUpdate(const Worker_ComponentUpdate& Update)
{
	Schema_Object* ComponentObject = Schema_GetComponentUpdateFields(Update.schema_type);

	ActorGroupId = Schema_GetUint32(ComponentObject, 1);

	ActorSetId = Schema_GetUint32(ComponentObject, 2);
}

} // namespace SpatialGDK
