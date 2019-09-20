// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "Interop/SpatialRingBufferManager.h"

#include "EngineClasses/SpatialNetDriver.h"
#include "EngineClasses/SpatialPackageMapClient.h"
#include "Interop/Connection/SpatialWorkerConnection.h"
#include "Interop/SpatialStaticComponentView.h"
#include "Schema/ClientRPCEndpointRB.h"
#include "Schema/ServerRPCEndpointRB.h"
#include "Schema/MulticastEndpointRB.h"

using namespace SpatialGDK;

void USpatialRingBufferManager::Init(USpatialNetDriver* InNetDriver)
{
	NetDriver = InNetDriver;
	WorkerConnection = NetDriver->Connection;
	StaticComponentView = NetDriver->StaticComponentView;
	PackageMap = NetDriver->PackageMap;
}

void USpatialRingBufferManager::SendRPCs()
{
	TSet<Worker_EntityId_Key> EntityIds;

	for (auto It : RPCsToSendMap)
	{
		Worker_EntityId EntityId = It.Key;
		if (PackageMap->IsEntityIdPendingCreation(EntityId))
		{
			continue;
		}
		EntityIds.Add(EntityId);
	}

	for (auto It : LastHandledRPCMap)
	{
		Worker_EntityId EntityId = It.Key;
		if (PackageMap->IsEntityIdPendingCreation(EntityId))
		{
			UE_LOG(LogTemp, Warning, TEXT("Huh? Handled RPCs for entity that hasn't even been created. This shouldn't happen."));
			continue;
		}
		EntityIds.Add(EntityId);
	}

	for (Worker_EntityId EntityId : EntityIds)
	{
		QueuedRPCMap* RPCs = RPCsToSendMap.Find(EntityId);
		TSet<ESchemaComponentType>* RPCTypesExecuted = LastHandledRPCMap.Find(EntityId);

		if ((RPCs != nullptr && (RPCs->Contains(SCHEMA_ClientReliableRPC) || RPCs->Contains(SCHEMA_ClientUnreliableRPC))) ||
			(RPCTypesExecuted != nullptr && (RPCTypesExecuted->Contains(SCHEMA_ServerReliableRPC) || RPCTypesExecuted->Contains(SCHEMA_ServerUnreliableRPC))))
		{
			ClientRPCEndpointRB* ClientEndpoint = StaticComponentView->GetComponentData<ClientRPCEndpointRB>(EntityId);
			ServerRPCEndpointRB* ServerEndpoint = StaticComponentView->GetComponentData<ServerRPCEndpointRB>(EntityId);

			if (ClientEndpoint != nullptr && ServerEndpoint != nullptr)
			{
				WorkerConnection->SendComponentUpdate(EntityId, ServerEndpoint->CreateRPCEndpointUpdate(RPCs, RPCTypesExecuted, ClientEndpoint));
				UE_LOG(LogTemp, Warning, TEXT("Sending RPCs: %lld client %d %d server exe %d %d"), EntityId, ServerEndpoint->ReliableRPCs.LastSentRPCId, ServerEndpoint->UnreliableRPCs.LastSentRPCId, ServerEndpoint->LastExecutedReliableRPC, ServerEndpoint->LastExecutedUnreliableRPC);
			}
			else
			{
				UE_LOG(LogTemp, Warning, TEXT("No client/server endpoint"));
			}
		}
		if ((RPCs != nullptr && (RPCs->Contains(SCHEMA_ServerReliableRPC) || RPCs->Contains(SCHEMA_ServerUnreliableRPC))) ||
			(RPCTypesExecuted != nullptr && (RPCTypesExecuted->Contains(SCHEMA_ClientReliableRPC) || RPCTypesExecuted->Contains(SCHEMA_ClientUnreliableRPC))))
		{
			ClientRPCEndpointRB* ClientEndpoint = StaticComponentView->GetComponentData<ClientRPCEndpointRB>(EntityId);
			ServerRPCEndpointRB* ServerEndpoint = StaticComponentView->GetComponentData<ServerRPCEndpointRB>(EntityId);

			if (ClientEndpoint != nullptr && ServerEndpoint != nullptr)
			{
				WorkerConnection->SendComponentUpdate(EntityId, ClientEndpoint->CreateRPCEndpointUpdate(RPCs, RPCTypesExecuted, ServerEndpoint));
				UE_LOG(LogTemp, Warning, TEXT("Sending RPCs: %lld server %d %d client exe %d %d"), EntityId, ClientEndpoint->ReliableRPCs.LastSentRPCId, ClientEndpoint->UnreliableRPCs.LastSentRPCId, ClientEndpoint->LastExecutedReliableRPC, ClientEndpoint->LastExecutedUnreliableRPC);
			}
			else
			{
				UE_LOG(LogTemp, Warning, TEXT("No client/server endpoint"));
			}
		}
		if (RPCs != nullptr && RPCs->Contains(SCHEMA_NetMulticastRPC))
		{
			if (MulticastRPCEndpointRB* MulticastEndpoint = StaticComponentView->GetComponentData<MulticastRPCEndpointRB>(EntityId))
			{
				WorkerConnection->SendComponentUpdate(EntityId, MulticastEndpoint->CreateRPCEndpointUpdate(RPCs));
				UE_LOG(LogTemp, Warning, TEXT("Sending RPCs: %lld multi %d"), EntityId, MulticastEndpoint->MulticastRPCs.LastSentRPCId);
			}
			else
			{
				UE_LOG(LogTemp, Warning, TEXT("No multicast endpoint"));
			}
		}
	}

	// TODO: Leave reliable RPCs that overflowed.
	RPCsToSendMap.Empty();
	LastHandledRPCMap.Empty();
}

