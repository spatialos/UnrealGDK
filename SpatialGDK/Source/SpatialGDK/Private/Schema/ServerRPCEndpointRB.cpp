// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "Schema/ServerRPCEndpointRB.h"

#include "Schema/ClientRPCEndpointRB.h"
#include "Schema/RPCPayload.h"
#include "SpatialGDKSettings.h"
#include "Utils/SchemaUtils.h"

namespace SpatialGDK
{

ServerRPCEndpointRB::ServerRPCEndpointRB()
	: ReliableRPCs(GetDefault<USpatialGDKSettings>()->GetRPCRingBufferSize(SCHEMA_ClientReliableRPC), 1)
	, UnreliableRPCs(GetDefault<USpatialGDKSettings>()->GetRPCRingBufferSize(SCHEMA_ClientUnreliableRPC), ReliableRPCs.SchemaFieldStart + ReliableRPCs.RingBufferSize + 1)
	, LastExecutedReliableRPCFieldId(UnreliableRPCs.SchemaFieldStart + UnreliableRPCs.RingBufferSize + 1)
	, LastExecutedUnreliableRPCFieldId(LastExecutedReliableRPCFieldId + 1)
{
}

ServerRPCEndpointRB::ServerRPCEndpointRB(const Worker_ComponentData& Data)
	: ServerRPCEndpointRB()
{
	Schema_Object* EndpointObject = Schema_GetComponentDataFields(Data.schema_type);
	ReliableRPCs.ReadFromSchema(EndpointObject);
	UnreliableRPCs.ReadFromSchema(EndpointObject);

	LastExecutedReliableRPC = Schema_GetUint64(EndpointObject, LastExecutedReliableRPCFieldId);
	LastExecutedUnreliableRPC = Schema_GetUint64(EndpointObject, LastExecutedUnreliableRPCFieldId);
}

void ServerRPCEndpointRB::ApplyComponentUpdate(const Worker_ComponentUpdate& Update)
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
}

Worker_ComponentData ServerRPCEndpointRB::CreateRPCEndpointData(const QueuedRPCMap* RPCMap)
{
	Worker_ComponentData Data{};
	Data.component_id = ComponentId;
	Data.schema_type = Schema_CreateComponentData();
	Schema_Object* ComponentObject = Schema_GetComponentDataFields(Data.schema_type);

	if (RPCMap != nullptr)
	{
		if (const TArray<RPCPayload>* ReliableRPCArray = RPCMap->Find(SCHEMA_ClientReliableRPC))
		{
			if (ReliableRPCArray->Num() > ReliableRPCs.GetCapacity(0))
			{
				UE_LOG(LogTemp, Warning, TEXT("Whoops, too many reliable client RPCs on creation"));
				// What do!
			}
			ReliableRPCs.WriteToSchema(ComponentObject, *ReliableRPCArray);
		}
		if (const TArray<RPCPayload>* UnreliableRPCArray = RPCMap->Find(SCHEMA_ClientUnreliableRPC))
		{
			UnreliableRPCs.WriteToSchema(ComponentObject, *UnreliableRPCArray);
		}
	}

	return Data;
}

Worker_ComponentUpdate ServerRPCEndpointRB::CreateRPCEndpointUpdate(const QueuedRPCMap* RPCMap, const TSet<ESchemaComponentType>* RPCTypesExecuted, const ClientRPCEndpointRB* ClientEndpoint)
{
	Worker_ComponentUpdate Update{};
	Update.component_id = ComponentId;
	Update.schema_type = Schema_CreateComponentUpdate();
	Schema_Object* ComponentObject = Schema_GetComponentUpdateFields(Update.schema_type);

	if (RPCMap != nullptr)
	{
		if (const TArray<RPCPayload>* ReliableRPCArray = RPCMap->Find(SCHEMA_ClientReliableRPC))
		{
			if (ReliableRPCArray->Num() > ReliableRPCs.GetCapacity(ClientEndpoint ? ClientEndpoint->LastExecutedReliableRPC : 0))
			{
				UE_LOG(LogTemp, Warning, TEXT("Whoops, too many reliable client RPCs for update"));
				// What do!
			}
			ReliableRPCs.WriteToSchema(ComponentObject, *ReliableRPCArray);
		}
		if (const TArray<RPCPayload>* UnreliableRPCArray = RPCMap->Find(SCHEMA_ClientUnreliableRPC))
		{
			UnreliableRPCs.WriteToSchema(ComponentObject, *UnreliableRPCArray);
		}
	}

	if (RPCTypesExecuted != nullptr)
	{
		if (RPCTypesExecuted->Contains(SCHEMA_ServerReliableRPC))
		{
			Schema_AddUint64(ComponentObject, LastExecutedReliableRPCFieldId, LastExecutedReliableRPC);
		}
		if (RPCTypesExecuted->Contains(SCHEMA_ServerUnreliableRPC))
		{
			Schema_AddUint64(ComponentObject, LastExecutedUnreliableRPCFieldId, LastExecutedUnreliableRPC);
		}
	}

	return Update;
}

TArray<RPCPayload> ServerRPCEndpointRB::RetrieveNewRPCs(ClientRPCEndpointRB& ClientEndpoint, TSet<ESchemaComponentType>& RPCTypesRetrieved)
{
	TArray<RPCPayload> RPCs;

	if (ReliableRPCs.LastSentRPCId > ClientEndpoint.LastExecutedReliableRPC)
	{
		ReliableRPCs.GetRPCsSince(ClientEndpoint.LastExecutedReliableRPC, RPCs);

		RPCTypesRetrieved.Add(SCHEMA_ClientReliableRPC);
		ClientEndpoint.LastExecutedReliableRPC = ReliableRPCs.LastSentRPCId;
	}
	if (UnreliableRPCs.LastSentRPCId > ClientEndpoint.LastExecutedUnreliableRPC)
	{
		UnreliableRPCs.GetRPCsSince(ClientEndpoint.LastExecutedUnreliableRPC, RPCs);

		RPCTypesRetrieved.Add(SCHEMA_ClientUnreliableRPC);
		ClientEndpoint.LastExecutedUnreliableRPC = UnreliableRPCs.LastSentRPCId;
	}

	return RPCs;
}

bool ServerRPCEndpointRB::HasNewRPCs(const ClientRPCEndpointRB& ClientEndpoint) const
{
	return ReliableRPCs.LastSentRPCId > ClientEndpoint.LastExecutedReliableRPC ||
		UnreliableRPCs.LastSentRPCId > ClientEndpoint.LastExecutedUnreliableRPC;
}

} // namespace SpatialGDK
