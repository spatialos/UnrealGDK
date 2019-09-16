// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "Interop/SpatialRingBufferManager.h"

#include "EngineClasses/SpatialNetDriver.h"
#include "Interop/Connection/SpatialWorkerConnection.h"
#include "Interop/SpatialStaticComponentView.h"
#include "Schema/ClientRPCEndpointRB.h"
#include "Schema/ServerRPCEndpointRB.h"
#include "Schema/MulticastEndpointRB.h"

using namespace SpatialGDK;

void USpatialRingBufferManager::Init(USpatialNetDriver* InNetDriver)
{
	NetDriver = InNetDriver;
	WorkerConnection = InNetDriver->Connection;
	StaticComponentView = NetDriver->StaticComponentView;
}

void USpatialRingBufferManager::SendRPCs()
{
	for (auto It : RPCsToSendMap)
	{
		Worker_EntityId EntityId = It.Key;
		QueuedRPCMap& RPCs = It.Value;

		TSet<ESchemaComponentType>* RPCTypesExecuted = LastHandledRPCMap.Find(EntityId);

		if (RPCs.Contains(SCHEMA_ClientReliableRPC) || RPCs.Contains(SCHEMA_ClientUnreliableRPC) ||
			(RPCTypesExecuted != nullptr && (RPCTypesExecuted->Contains(SCHEMA_ServerReliableRPC) || RPCTypesExecuted->Contains(SCHEMA_ServerUnreliableRPC))))
		{
			ClientRPCEndpointRB* ClientEndpoint = StaticComponentView->GetComponentData<ClientRPCEndpointRB>(EntityId);
			ServerRPCEndpointRB* ServerEndpoint = StaticComponentView->GetComponentData<ServerRPCEndpointRB>(EntityId);

			if (ClientEndpoint != nullptr && ServerEndpoint != nullptr)
			{
				WorkerConnection->SendComponentUpdate(EntityId, ServerEndpoint->CreateRPCEndpointUpdate(&RPCs, RPCTypesExecuted, ClientEndpoint));
				UE_LOG(LogTemp, Warning, TEXT("Sending RPCs: %lld client %d %d server exe %d %d"), EntityId, ServerEndpoint->ReliableRPCs.LastSentRPCId, ServerEndpoint->UnreliableRPCs.LastSentRPCId, ServerEndpoint->LastExecutedReliableRPC, ServerEndpoint->LastExecutedUnreliableRPC);
			}
			else
			{
				UE_LOG(LogTemp, Warning, TEXT("No client/server endpoint"));
			}
		}
		if (RPCs.Contains(SCHEMA_ServerReliableRPC) || RPCs.Contains(SCHEMA_ServerUnreliableRPC) ||
			(RPCTypesExecuted != nullptr && (RPCTypesExecuted->Contains(SCHEMA_ClientReliableRPC) || RPCTypesExecuted->Contains(SCHEMA_ClientUnreliableRPC))))
		{
			ClientRPCEndpointRB* ClientEndpoint = StaticComponentView->GetComponentData<ClientRPCEndpointRB>(EntityId);
			ServerRPCEndpointRB* ServerEndpoint = StaticComponentView->GetComponentData<ServerRPCEndpointRB>(EntityId);

			if (ClientEndpoint != nullptr && ServerEndpoint != nullptr)
			{
				WorkerConnection->SendComponentUpdate(EntityId, ClientEndpoint->CreateRPCEndpointUpdate(&RPCs, RPCTypesExecuted, ServerEndpoint));
				UE_LOG(LogTemp, Warning, TEXT("Sending RPCs: %lld server %d %d client exe %d %d"), EntityId, ClientEndpoint->ReliableRPCs.LastSentRPCId, ClientEndpoint->UnreliableRPCs.LastSentRPCId, ClientEndpoint->LastExecutedReliableRPC, ClientEndpoint->LastExecutedUnreliableRPC);
			}
			else
			{
				UE_LOG(LogTemp, Warning, TEXT("No client/server endpoint"));
			}
		}
		if (RPCs.Contains(SCHEMA_NetMulticastRPC))
		{
			if (MulticastRPCEndpointRB* MulticastEndpoint = StaticComponentView->GetComponentData<MulticastRPCEndpointRB>(EntityId))
			{
				WorkerConnection->SendComponentUpdate(EntityId, MulticastEndpoint->CreateRPCEndpointUpdate(&RPCs));
				UE_LOG(LogTemp, Warning, TEXT("Sending RPCs: %lld multi %d"), EntityId, MulticastEndpoint->MulticastRPCs.LastSentRPCId);
			}
			else
			{
				UE_LOG(LogTemp, Warning, TEXT("No multicast endpoint"));
			}
		}
	}

	for (auto It : LastHandledRPCMap)
	{
		Worker_EntityId EntityId = It.Key;
		TSet<ESchemaComponentType>& RPCTypesExecuted = It.Value;

		if (RPCsToSendMap.Contains(EntityId))
		{
			continue;
		}

		if (RPCTypesExecuted.Contains(SCHEMA_ServerReliableRPC) || RPCTypesExecuted.Contains(SCHEMA_ServerUnreliableRPC))
		{
			if (ServerRPCEndpointRB* ServerEndpoint = StaticComponentView->GetComponentData<ServerRPCEndpointRB>(EntityId))
			{
				WorkerConnection->SendComponentUpdate(EntityId, ServerEndpoint->CreateRPCEndpointUpdate(nullptr, &RPCTypesExecuted, nullptr));
				UE_LOG(LogTemp, Warning, TEXT("Sending RPCs: %lld client - - server exe %d %d"), EntityId, ServerEndpoint->LastExecutedReliableRPC, ServerEndpoint->LastExecutedUnreliableRPC);
			}
			else
			{
				UE_LOG(LogTemp, Warning, TEXT("No server endpoint"));
			}
		}
		if (RPCTypesExecuted.Contains(SCHEMA_ClientReliableRPC) || RPCTypesExecuted.Contains(SCHEMA_ClientUnreliableRPC))
		{
			if (ClientRPCEndpointRB* ClientEndpoint = StaticComponentView->GetComponentData<ClientRPCEndpointRB>(EntityId))
			{
				WorkerConnection->SendComponentUpdate(EntityId, ClientEndpoint->CreateRPCEndpointUpdate(nullptr, &RPCTypesExecuted, nullptr));
				UE_LOG(LogTemp, Warning, TEXT("Sending RPCs: %lld server - - client exe %d %d"), EntityId, ClientEndpoint->LastExecutedReliableRPC, ClientEndpoint->LastExecutedUnreliableRPC);
			}
			else
			{
				UE_LOG(LogTemp, Warning, TEXT("No client endpoint"));
			}
		}
	}

	RPCsToSendMap.Empty();
	LastHandledRPCMap.Empty();
}

