// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "Schema/ClientRPCEndpointRB.h"

#include "Schema/ServerRPCEndpointRB.h"
#include "Schema/RPCPayload.h"
#include "SpatialGDKSettings.h"
#include "Utils/SchemaUtils.h"

namespace SpatialGDK
{

ClientRPCEndpointRB::ClientRPCEndpointRB(const Worker_ComponentData& Data)
	: ReliableRPCs(GetDefault<USpatialGDKSettings>()->GetRPCRingBufferSize(SCHEMA_ServerReliableRPC), 1)
	, UnreliableRPCs(GetDefault<USpatialGDKSettings>()->GetRPCRingBufferSize(SCHEMA_ServerUnreliableRPC), ReliableRPCs.SchemaFieldStart + ReliableRPCs.RingBufferSize + 1)
	, LastExecutedReliableRPCFieldId(UnreliableRPCs.SchemaFieldStart + UnreliableRPCs.RingBufferSize + 1)
	, LastExecutedUnreliableRPCFieldId(LastExecutedReliableRPCFieldId + 1)
	, LastExecutedMulticastRPCFieldId(LastExecutedUnreliableRPCFieldId + 1)
{
	Schema_Object* EndpointObject = Schema_GetComponentDataFields(Data.schema_type);
	ReliableRPCs.ReadFromSchema(EndpointObject);
	UnreliableRPCs.ReadFromSchema(EndpointObject);

	LastExecutedReliableRPC = Schema_GetUint64(EndpointObject, LastExecutedReliableRPCFieldId);
	LastExecutedUnreliableRPC = Schema_GetUint64(EndpointObject, LastExecutedUnreliableRPCFieldId);
	LastExecutedMulticastRPC = Schema_GetUint64(EndpointObject, LastExecutedMulticastRPCFieldId);
}

void ClientRPCEndpointRB::ApplyComponentUpdate(const Worker_ComponentUpdate& Update)
{
	Schema_Object* EndpointObject = Schema_GetComponentUpdateFields(Update.schema_type);
	ReliableRPCs.ReadFromSchema(EndpointObject);
	UnreliableRPCs.ReadFromSchema(EndpointObject);

	if (Schema_GetUint64Count(EndpointObject, LastExecutedReliableRPCFieldId) > 0)
	{
		LastExecutedReliableRPC = Schema_GetUint64(EndpointObject, LastExecutedReliableRPCFieldId);
	}
	if (Schema_GetUint64Count(EndpointObject, LastExecutedUnreliableRPCFieldId) > 0)
	{
		LastExecutedUnreliableRPC = Schema_GetUint64(EndpointObject, LastExecutedUnreliableRPCFieldId);
	}
	if (Schema_GetUint64Count(EndpointObject, LastExecutedMulticastRPCFieldId) > 0)
	{
		LastExecutedMulticastRPC = Schema_GetUint64(EndpointObject, LastExecutedMulticastRPCFieldId);
	}
}

Worker_ComponentData ClientRPCEndpointRB::CreateRPCEndpointData()
{
	Worker_ComponentData Data{};
	Data.component_id = ComponentId;
	Data.schema_type = Schema_CreateComponentData();
	Schema_Object* ComponentObject = Schema_GetComponentDataFields(Data.schema_type);

	return Data;
}

Worker_ComponentUpdate ClientRPCEndpointRB::CreateRPCEndpointUpdate(const QueuedRPCMap* RPCMap, const TSet<ESchemaComponentType>* RPCTypesExecuted, const ServerRPCEndpointRB* ServerEndpoint)
{
	Worker_ComponentUpdate Update{};
	Update.component_id = ComponentId;
	Update.schema_type = Schema_CreateComponentUpdate();
	Schema_Object* ComponentObject = Schema_GetComponentUpdateFields(Update.schema_type);

	if (RPCMap != nullptr)
	{
		if (const TArray<RPCPayload>* ReliableRPCArray = RPCMap->Find(SCHEMA_ServerReliableRPC))
		{
			if (ReliableRPCArray->Num() > ReliableRPCs.GetCapacity(ServerEndpoint ? ServerEndpoint->LastExecutedReliableRPC : 0))
			{
				UE_LOG(LogTemp, Warning, TEXT("Whoops, too many reliable server RPCs for update"));
				// What do!
			}
			ReliableRPCs.WriteToSchema(ComponentObject, *ReliableRPCArray);
		}
		if (const TArray<RPCPayload>* UnreliableRPCArray = RPCMap->Find(SCHEMA_ServerUnreliableRPC))
		{
			UnreliableRPCs.WriteToSchema(ComponentObject, *UnreliableRPCArray);
		}
	}

	if (RPCTypesExecuted != nullptr)
	{
		if (RPCTypesExecuted->Contains(SCHEMA_ClientReliableRPC))
		{
			Schema_AddUint64(ComponentObject, LastExecutedReliableRPCFieldId, LastExecutedReliableRPC);
		}
		if (RPCTypesExecuted->Contains(SCHEMA_ClientUnreliableRPC))
		{
			Schema_AddUint64(ComponentObject, LastExecutedUnreliableRPCFieldId, LastExecutedUnreliableRPC);
		}
	}

	return Update;
}

TArray<RPCPayload> ClientRPCEndpointRB::RetrieveNewRPCs(ServerRPCEndpointRB& ServerEndpoint, TSet<ESchemaComponentType>& RPCTypesRetrieved)
{
	TArray<RPCPayload> RPCs;

	if (ReliableRPCs.LastSentRPCId > ServerEndpoint.LastExecutedReliableRPC)
	{
		ReliableRPCs.GetRPCsSince(ServerEndpoint.LastExecutedReliableRPC, RPCs);

		RPCTypesRetrieved.Add(SCHEMA_ServerReliableRPC);
		ServerEndpoint.LastExecutedReliableRPC = ReliableRPCs.LastSentRPCId;
	}
	if (UnreliableRPCs.LastSentRPCId > ServerEndpoint.LastExecutedUnreliableRPC)
	{
		UnreliableRPCs.GetRPCsSince(ServerEndpoint.LastExecutedUnreliableRPC, RPCs);

		RPCTypesRetrieved.Add(SCHEMA_ServerUnreliableRPC);
		ServerEndpoint.LastExecutedUnreliableRPC = UnreliableRPCs.LastSentRPCId;
	}

	return RPCs;
}

bool ClientRPCEndpointRB::HasNewRPCs(const ServerRPCEndpointRB& ServerEndpoint) const
{
	return ReliableRPCs.LastSentRPCId > ServerEndpoint.LastExecutedReliableRPC ||
		UnreliableRPCs.LastSentRPCId > ServerEndpoint.LastExecutedUnreliableRPC;
}

} // namespace SpatialGDK
