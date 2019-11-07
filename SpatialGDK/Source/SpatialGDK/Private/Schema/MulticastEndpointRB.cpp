// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "Schema/MulticastEndpointRB.h"

#include "Schema/RPCPayload.h"
#include "SpatialGDKSettings.h"
#include "Utils/SchemaUtils.h"

namespace SpatialGDK
{

MulticastRPCEndpointRB::MulticastRPCEndpointRB()
	: MulticastRPCs(GetDefault<USpatialGDKSettings>()->GetRPCRingBufferSize(SCHEMA_NetMulticastRPC), 1)
{
}

MulticastRPCEndpointRB::MulticastRPCEndpointRB(const Worker_ComponentData& Data)
	: MulticastRPCEndpointRB()
{
	Schema_Object* EndpointObject = Schema_GetComponentDataFields(Data.schema_type);
	MulticastRPCs.ReadFromSchema(EndpointObject);

	// Ignore the RPCs that came on checkout.
	LastExecutedMulticastRPC = MulticastRPCs.LastSentRPCId;
}

void MulticastRPCEndpointRB::ApplyComponentUpdate(const Worker_ComponentUpdate& Update)
{
	Schema_Object* EndpointObject = Schema_GetComponentUpdateFields(Update.schema_type);
	MulticastRPCs.ReadFromSchema(EndpointObject);
}

Worker_ComponentData MulticastRPCEndpointRB::CreateRPCEndpointData(const QueuedRPCMap* RPCMap)
{
	Worker_ComponentData Data{};
	Data.component_id = ComponentId;
	Data.schema_type = Schema_CreateComponentData();
	Schema_Object* ComponentObject = Schema_GetComponentDataFields(Data.schema_type);

	if (RPCMap != nullptr)
	{
		if (const TArray<RPCPayload>* MulticastRPCArray = RPCMap->Find(SCHEMA_NetMulticastRPC))
		{
			MulticastRPCs.WriteToSchema(ComponentObject, *MulticastRPCArray);
		}
	}

	return Data;
}

Worker_ComponentUpdate MulticastRPCEndpointRB::CreateRPCEndpointUpdate(const QueuedRPCMap* RPCMap)
{
	Worker_ComponentUpdate Update{};
	Update.component_id = ComponentId;
	Update.schema_type = Schema_CreateComponentUpdate();
	Schema_Object* ComponentObject = Schema_GetComponentUpdateFields(Update.schema_type);

	if (RPCMap != nullptr)
	{
		if (const TArray<RPCPayload>* MulticastRPCArray = RPCMap->Find(SCHEMA_NetMulticastRPC))
		{
			MulticastRPCs.WriteToSchema(ComponentObject, *MulticastRPCArray);
		}
	}

	return Update;
}

TArray<RPCPayload> MulticastRPCEndpointRB::RetrieveNewRPCs()
{
	TArray<RPCPayload> RPCs;

	if (MulticastRPCs.LastSentRPCId > LastExecutedMulticastRPC)
	{
		MulticastRPCs.GetRPCsSince(LastExecutedMulticastRPC, RPCs);

		LastExecutedMulticastRPC = MulticastRPCs.LastSentRPCId;
	}

	return RPCs;
}

} // namespace SpatialGDK
