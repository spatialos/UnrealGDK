// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "Interop/RPCs/ClientServerRPCService.h"

#include "EngineClasses/SpatialPackageMapClient.h"
#include "Interop/SpatialStaticComponentView.h"
#include "Schema/ClientEndpoint.h"
#include "Schema/ServerEndpoint.h"
#include "SpatialConstants.h"

DEFINE_LOG_CATEGORY(LogClientServerRPCService);

namespace SpatialGDK
{
ClientServerRPCService::ClientServerRPCService(const ExtractRPCDelegate InExtractRPCCallback, const FSubView& InSubView,
											   USpatialNetDriver* InNetDriver, FRPCStore& InRPCStore)
	: ExtractRPCCallback(InExtractRPCCallback)
	, SubView(&InSubView)
	, NetDriver(InNetDriver)
	, RPCStore(&InRPCStore)
{
}

void ClientServerRPCService::AdvanceView()
{
	const FSubViewDelta& SubViewDelta = SubView->GetViewDelta();
	for (const EntityDelta& Delta : SubViewDelta.EntityDeltas)
	{
		switch (Delta.Type)
		{
		case EntityDelta::UPDATE:
		{
			for (const ComponentChange& Change : Delta.ComponentUpdates)
			{
				if (IsClientOrServerEndpoint(Change.ComponentId))
				{
					ApplyComponentUpdate(Delta.EntityId, Change.ComponentId, Change.Update);
				}
			}
			break;
		}
		case EntityDelta::ADD:
			PopulateDataStore(Delta.EntityId);
			SetEntityData(Delta.EntityId);
			break;
		case EntityDelta::REMOVE:
			ClientServerDataStore.Remove(Delta.EntityId);
			break;
		case EntityDelta::TEMPORARILY_REMOVED:
			ClientServerDataStore.Remove(Delta.EntityId);
			PopulateDataStore(Delta.EntityId);
			SetEntityData(Delta.EntityId);
			break;
		default:
			break;
		}
	}
}

void ClientServerRPCService::ProcessChanges()
{
	const FSubViewDelta& SubViewDelta = SubView->GetViewDelta();
	for (const EntityDelta& Delta : SubViewDelta.EntityDeltas)
	{
		switch (Delta.Type)
		{
		case EntityDelta::UPDATE:
		{
			for (const ComponentChange& Change : Delta.ComponentUpdates)
			{
				ComponentUpdate(Delta.EntityId, Change.ComponentId, Change.Update);
			}
			break;
		}
		case EntityDelta::ADD:
			EntityAdded(Delta.EntityId);
			break;
		case EntityDelta::TEMPORARILY_REMOVED:
			EntityAdded(Delta.EntityId);
			break;
		default:
			break;
		}
	}
}

bool ClientServerRPCService::ContainsOverflowedRPC(const EntityRPCType& EntityRPC) const
{
	return OverflowedRPCs.Contains(EntityRPC);
}

TMap<EntityRPCType, TArray<PendingRPCPayload>>& ClientServerRPCService::GetOverflowedRPCs()
{
	return OverflowedRPCs;
}

void ClientServerRPCService::AddOverflowedRPC(const EntityRPCType EntityType, PendingRPCPayload&& Payload)
{
	OverflowedRPCs.FindOrAdd(EntityType).Add(MoveTemp(Payload));
}

void ClientServerRPCService::IncrementAckedRPCID(const Worker_EntityId EntityId, const ERPCType Type)
{
	const EntityRPCType EntityTypePair = EntityRPCType(EntityId, Type);
	uint64* LastAckedRPCId = LastAckedRPCIds.Find(EntityTypePair);
	if (LastAckedRPCId == nullptr)
	{
		UE_LOG(LogClientServerRPCService, Warning,
			   TEXT("ClientServerRPCService::IncrementAckedRPCID: Could not find last acked RPC id. Entity: %lld, RPC type: %s"), EntityId,
			   *SpatialConstants::RPCTypeToString(Type));
		return;
	}

	++(*LastAckedRPCId);

	const EntityComponentId EntityComponentPair = { EntityId, RPCRingBufferUtils::GetAckComponentId(Type) };
	Schema_Object* EndpointObject = Schema_GetComponentUpdateFields(RPCStore->GetOrCreateComponentUpdate(EntityComponentPair));

	RPCRingBufferUtils::WriteAckToSchema(EndpointObject, Type, *LastAckedRPCId);
}

uint64 ClientServerRPCService::GetAckFromView(const Worker_EntityId EntityId, const ERPCType Type)
{
	switch (Type)
	{
	case ERPCType::ClientReliable:
		return ClientServerDataStore[EntityId].Client.ReliableRPCAck;
	case ERPCType::ClientUnreliable:
		return ClientServerDataStore[EntityId].Client.UnreliableRPCAck;
	case ERPCType::ServerReliable:
		return ClientServerDataStore[EntityId].Server.ReliableRPCAck;
	case ERPCType::ServerUnreliable:
		return ClientServerDataStore[EntityId].Server.UnreliableRPCAck;
	default:
		checkNoEntry();
		return 0;
	}
}

void ClientServerRPCService::SetEntityData(Worker_EntityId EntityId)
{
	for (const Worker_ComponentId ComponentId : SubView->GetView()[EntityId].Authority)
	{
		OnEndpointAuthorityGained(EntityId, ComponentId);
	}
}

void ClientServerRPCService::EntityAdded(const Worker_EntityId EntityId)
{
	for (const Worker_ComponentId ComponentId : SubView->GetView()[EntityId].Authority)
	{
		if (!IsClientOrServerEndpoint(ComponentId))
		{
			continue;
		}
		ExtractRPCsForEntity(EntityId, ComponentId == SpatialConstants::CLIENT_ENDPOINT_COMPONENT_ID
										   ? SpatialConstants::SERVER_ENDPOINT_COMPONENT_ID
										   : SpatialConstants::CLIENT_ENDPOINT_COMPONENT_ID);
	}
}

void ClientServerRPCService::ComponentUpdate(const Worker_EntityId EntityId, const Worker_ComponentId ComponentId,
											 Schema_ComponentUpdate* Update)
{
	if (!IsClientOrServerEndpoint(ComponentId))
	{
		return;
	}
	HandleRPC(EntityId, ComponentId);
}

void ClientServerRPCService::PopulateDataStore(const Worker_EntityId EntityId)
{
	const EntityViewElement& Entity = SubView->GetView()[EntityId];
	const ClientEndpoint Client = ClientEndpoint(
		Entity.Components.FindByPredicate(ComponentIdEquality{ SpatialConstants::CLIENT_ENDPOINT_COMPONENT_ID })->GetUnderlying());
	const ServerEndpoint Server = ServerEndpoint(
		Entity.Components.FindByPredicate(ComponentIdEquality{ SpatialConstants::SERVER_ENDPOINT_COMPONENT_ID })->GetUnderlying());
	ClientServerDataStore.Emplace(EntityId, ClientServerEndpoints{ Client, Server });
}

void ClientServerRPCService::ApplyComponentUpdate(const Worker_EntityId EntityId, const Worker_ComponentId ComponentId,
												  Schema_ComponentUpdate* Update)
{
	switch (ComponentId)
	{
	case SpatialConstants::CLIENT_ENDPOINT_COMPONENT_ID:
		ClientServerDataStore[EntityId].Client.ApplyComponentUpdate(Update);
		break;
	case SpatialConstants::SERVER_ENDPOINT_COMPONENT_ID:
		ClientServerDataStore[EntityId].Server.ApplyComponentUpdate(Update);
		break;
	default:
		break;
	}
}

void ClientServerRPCService::OnEndpointAuthorityGained(const Worker_EntityId EntityId, const Worker_ComponentId ComponentId)
{
	switch (ComponentId)
	{
	case SpatialConstants::CLIENT_AUTH_COMPONENT_SET_ID:
	{
		const ClientEndpoint& Endpoint = ClientServerDataStore[EntityId].Client;
		LastSeenRPCIds.Add(EntityRPCType(EntityId, ERPCType::ClientReliable), Endpoint.ReliableRPCAck);
		LastSeenRPCIds.Add(EntityRPCType(EntityId, ERPCType::ClientUnreliable), Endpoint.UnreliableRPCAck);
		LastAckedRPCIds.Add(EntityRPCType(EntityId, ERPCType::ClientReliable), Endpoint.ReliableRPCAck);
		LastAckedRPCIds.Add(EntityRPCType(EntityId, ERPCType::ClientUnreliable), Endpoint.UnreliableRPCAck);
		RPCStore->LastSentRPCIds.Add(EntityRPCType(EntityId, ERPCType::ServerReliable), Endpoint.ReliableRPCBuffer.LastSentRPCId);
		RPCStore->LastSentRPCIds.Add(EntityRPCType(EntityId, ERPCType::ServerUnreliable), Endpoint.UnreliableRPCBuffer.LastSentRPCId);
		break;
	}
	case SpatialConstants::SERVER_AUTH_COMPONENT_SET_ID:
	{
		const ServerEndpoint& Endpoint = ClientServerDataStore[EntityId].Server;
		LastSeenRPCIds.Add(EntityRPCType(EntityId, ERPCType::ServerReliable), Endpoint.ReliableRPCAck);
		LastSeenRPCIds.Add(EntityRPCType(EntityId, ERPCType::ServerUnreliable), Endpoint.UnreliableRPCAck);
		LastAckedRPCIds.Add(EntityRPCType(EntityId, ERPCType::ServerReliable), Endpoint.ReliableRPCAck);
		LastAckedRPCIds.Add(EntityRPCType(EntityId, ERPCType::ServerUnreliable), Endpoint.UnreliableRPCAck);
		RPCStore->LastSentRPCIds.Add(EntityRPCType(EntityId, ERPCType::ClientReliable), Endpoint.ReliableRPCBuffer.LastSentRPCId);
		RPCStore->LastSentRPCIds.Add(EntityRPCType(EntityId, ERPCType::ClientUnreliable), Endpoint.UnreliableRPCBuffer.LastSentRPCId);
		break;
	}
	default:
		checkNoEntry();
		break;
	}
}

void ClientServerRPCService::OnEndpointAuthorityLost(const Worker_EntityId EntityId, const Worker_ComponentId ComponentId)
{
	switch (ComponentId)
	{
	case SpatialConstants::CLIENT_ENDPOINT_COMPONENT_ID:
	{
		LastSeenRPCIds.Remove(EntityRPCType(EntityId, ERPCType::ClientReliable));
		LastSeenRPCIds.Remove(EntityRPCType(EntityId, ERPCType::ClientUnreliable));
		LastAckedRPCIds.Remove(EntityRPCType(EntityId, ERPCType::ClientReliable));
		LastAckedRPCIds.Remove(EntityRPCType(EntityId, ERPCType::ClientUnreliable));
		RPCStore->LastSentRPCIds.Remove(EntityRPCType(EntityId, ERPCType::ServerReliable));
		RPCStore->LastSentRPCIds.Remove(EntityRPCType(EntityId, ERPCType::ServerUnreliable));
		ClearOverflowedRPCs(EntityId);
		break;
	}
	case SpatialConstants::SERVER_ENDPOINT_COMPONENT_ID:
	{
		LastSeenRPCIds.Remove(EntityRPCType(EntityId, ERPCType::ServerReliable));
		LastSeenRPCIds.Remove(EntityRPCType(EntityId, ERPCType::ServerUnreliable));
		LastAckedRPCIds.Remove(EntityRPCType(EntityId, ERPCType::ServerReliable));
		LastAckedRPCIds.Remove(EntityRPCType(EntityId, ERPCType::ServerUnreliable));
		RPCStore->LastSentRPCIds.Remove(EntityRPCType(EntityId, ERPCType::ClientReliable));
		RPCStore->LastSentRPCIds.Remove(EntityRPCType(EntityId, ERPCType::ClientUnreliable));
		ClearOverflowedRPCs(EntityId);
		break;
	}
	default:
		checkNoEntry();
		break;
	}
}

void ClientServerRPCService::ClearOverflowedRPCs(const Worker_EntityId EntityId)
{
	for (uint8 RPCType = static_cast<uint8>(ERPCType::ClientReliable); RPCType <= static_cast<uint8>(ERPCType::NetMulticast); RPCType++)
	{
		OverflowedRPCs.Remove(EntityRPCType(EntityId, static_cast<ERPCType>(RPCType)));
	}
}

void ClientServerRPCService::HandleRPC(const Worker_EntityId EntityId, const Worker_ComponentId ComponentId)
{
	// When migrating an Actor to another worker, we preemptively change the role to SimulatedProxy when updating authority intent.
	// This can happen while this worker still has ServerEndpoint authority, and attempting to process a server RPC causes the engine
	// to print errors if the role isn't Authority. Instead, we exit here, and the RPC will be processed by the server that receives
	// authority.
	const bool bIsServerRpc = ComponentId == SpatialConstants::CLIENT_ENDPOINT_COMPONENT_ID;
	if (bIsServerRpc && SubView->HasAuthority(EntityId, SpatialConstants::SERVER_AUTH_COMPONENT_SET_ID))
	{
		const TWeakObjectPtr<UObject> ActorReceivingRPC = NetDriver->PackageMap->GetObjectFromEntityId(EntityId);
		if (!ActorReceivingRPC.IsValid())
		{
			UE_LOG(LogClientServerRPCService, Log,
				   TEXT("Entity receiving ring buffer RPC does not exist in PackageMap, possibly due to corresponding actor getting "
						"destroyed. Entity: %lld, Component: %d"),
				   EntityId, ComponentId);
			return;
		}

		const bool bActorRoleIsSimulatedProxy = Cast<AActor>(ActorReceivingRPC.Get())->Role == ROLE_SimulatedProxy;
		if (bActorRoleIsSimulatedProxy)
		{
			UE_LOG(LogClientServerRPCService, Verbose,
				   TEXT("Will not process server RPC, Actor role changed to SimulatedProxy. This happens on migration. Entity: %lld"),
				   EntityId);
			return;
		}
	}
	ExtractRPCsForEntity(EntityId, ComponentId);
}

void ClientServerRPCService::ExtractRPCsForEntity(const Worker_EntityId EntityId, const Worker_ComponentId ComponentId)
{
	switch (ComponentId)
	{
	case SpatialConstants::CLIENT_ENDPOINT_COMPONENT_ID:
		ExtractRPCsForType(EntityId, ERPCType::ServerReliable);
		ExtractRPCsForType(EntityId, ERPCType::ServerUnreliable);
		break;
	case SpatialConstants::SERVER_ENDPOINT_COMPONENT_ID:
		ExtractRPCsForType(EntityId, ERPCType::ClientReliable);
		ExtractRPCsForType(EntityId, ERPCType::ClientUnreliable);
		break;
	default:
		checkNoEntry();
		break;
	}
}

void ClientServerRPCService::ExtractRPCsForType(const Worker_EntityId EntityId, const ERPCType Type)
{
	const EntityRPCType EntityTypePair = EntityRPCType(EntityId, Type);

	if (!LastSeenRPCIds.Contains(EntityTypePair))
	{
		UE_LOG(LogClientServerRPCService, Warning,
			   TEXT("Tried to extract RPCs but no entry in Last Seen Map! This can happen after server travel. Entity: %lld, type: %s"),
			   EntityId, *SpatialConstants::RPCTypeToString(Type));
		return;
	}
	const uint64 LastSeenRPCId = LastSeenRPCIds[EntityTypePair];

	const RPCRingBuffer& Buffer = GetBufferFromView(EntityId, Type);

	uint64 LastProcessedRPCId = LastSeenRPCId;
	if (Buffer.LastSentRPCId >= LastSeenRPCId)
	{
		uint64 FirstRPCIdToRead = LastSeenRPCId + 1;

		const uint32 BufferSize = RPCRingBufferUtils::GetRingBufferSize(Type);
		if (Buffer.LastSentRPCId > LastSeenRPCId + BufferSize)
		{
			UE_LOG(LogClientServerRPCService, Warning,
				   TEXT("ClientServerRPCService::ExtractRPCsForType: RPCs were overwritten without being processed! Entity: %lld, RPC "
						"type: %s, "
						"last seen RPC ID: %d, last sent ID: %d, buffer size: %d"),
				   EntityId, *SpatialConstants::RPCTypeToString(Type), LastSeenRPCId, Buffer.LastSentRPCId, BufferSize);
			FirstRPCIdToRead = Buffer.LastSentRPCId - BufferSize + 1;
		}

		for (uint64 RPCId = FirstRPCIdToRead; RPCId <= Buffer.LastSentRPCId; RPCId++)
		{
			const TOptional<RPCPayload>& Element = Buffer.GetRingBufferElement(RPCId);
			if (Element.IsSet())
			{
				ExtractRPCCallback.Execute(FUnrealObjectRef(EntityId, Element.GetValue().Offset), Element.GetValue(), RPCId);
				LastProcessedRPCId = RPCId;
			}
			else
			{
				UE_LOG(
					LogClientServerRPCService, Warning,
					TEXT("ClientServerRPCService::ExtractRPCsForType: Ring buffer element empty. Entity: %lld, RPC type: %s, empty element "
						 "RPC id: %d"),
					EntityId, *SpatialConstants::RPCTypeToString(Type), RPCId);
			}
		}
	}
	else
	{
		UE_LOG(
			LogClientServerRPCService, Warning,
			TEXT("ClientServerRPCService::ExtractRPCsForType: Last sent RPC has smaller ID than last seen RPC. Entity: %lld, RPC type: %s, "
				 "last sent ID: %d, last seen ID: %d"),
			EntityId, *SpatialConstants::RPCTypeToString(Type), Buffer.LastSentRPCId, LastSeenRPCId);
	}

	if (LastProcessedRPCId > LastSeenRPCId)
	{
		LastSeenRPCIds[EntityTypePair] = LastProcessedRPCId;
	}
}

const RPCRingBuffer& ClientServerRPCService::GetBufferFromView(const Worker_EntityId EntityId, const ERPCType Type)
{
	switch (Type)
	{
	// Server sends Client RPCs, so ClientReliable & ClientUnreliable buffers live on ServerEndpoint.
	case ERPCType::ClientReliable:
		return ClientServerDataStore[EntityId].Server.ReliableRPCBuffer;
	case ERPCType::ClientUnreliable:
		return ClientServerDataStore[EntityId].Server.UnreliableRPCBuffer;

	// Client sends Server RPCs, so ServerReliable & ServerUnreliable buffers live on ClientEndpoint.
	case ERPCType::ServerReliable:
		return ClientServerDataStore[EntityId].Client.ReliableRPCBuffer;
	case ERPCType::ServerUnreliable:
		return ClientServerDataStore[EntityId].Client.UnreliableRPCBuffer;
	default:
		checkNoEntry();
		static const RPCRingBuffer DummyBuffer(ERPCType::Invalid);
		return DummyBuffer;
	}
}

bool ClientServerRPCService::IsClientOrServerEndpoint(const Worker_ComponentId ComponentId)
{
	return ComponentId == SpatialConstants::CLIENT_ENDPOINT_COMPONENT_ID || ComponentId == SpatialConstants::SERVER_ENDPOINT_COMPONENT_ID;
}
} // namespace SpatialGDK
