// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "Interop/SpatialRPCService.h"

#include "Interop/SpatialStaticComponentView.h"
#include "Schema/ClientEndpoint.h"
#include "Schema/MulticastRPCs.h"
#include "Schema/ServerEndpoint.h"

DEFINE_LOG_CATEGORY(LogSpatialRPCService);

namespace SpatialGDK
{

SpatialRPCService::SpatialRPCService(ExtractRPCDelegate ExtractRPCCallback, const USpatialStaticComponentView* View)
	: ExtractRPCCallback(ExtractRPCCallback)
	, View(View)
{
}

EPushRPCResult SpatialRPCService::PushRPC(Worker_EntityId EntityId, ERPCType Type, RPCPayload Payload)
{
	EntityRPCType EntityType = EntityRPCType(EntityId, Type);

	if (RPCRingBufferUtils::ShouldQueueOverflowed(Type) && OverflowedRPCs.Contains(EntityType))
	{
		// Already has queued RPCs of this type, queue until those are pushed.
		AddOverflowedRPC(EntityType, MoveTemp(Payload));
		return EPushRPCResult::QueueOverflowed;
	}

	EPushRPCResult Result = PushRPCInternal(EntityId, Type, MoveTemp(Payload));

	if (Result == EPushRPCResult::QueueOverflowed)
	{
		AddOverflowedRPC(EntityType, MoveTemp(Payload));
	}

	return Result;
}

EPushRPCResult SpatialRPCService::PushRPCInternal(Worker_EntityId EntityId, ERPCType Type, RPCPayload&& Payload)
{
	Worker_ComponentId RingBufferComponentId = RPCRingBufferUtils::GetRingBufferComponentId(Type);

	EntityComponentId EntityComponent = EntityComponentId(EntityId, RingBufferComponentId);
	EntityRPCType EntityType = EntityRPCType(EntityId, Type);

	Schema_Object* EndpointObject;
	uint64 LastAckedRPCId;
	if (View->HasComponent(EntityId, RingBufferComponentId))
	{
		if (!View->HasAuthority(EntityId, RingBufferComponentId))
		{
			return EPushRPCResult::NoRingBufferAuthority;
		}

		EndpointObject = Schema_GetComponentUpdateFields(GetOrCreateComponentUpdate(EntityComponent));

		if (Type == ERPCType::NetMulticast)
		{
			// Assume all multicast RPCs are auto-acked.
			LastAckedRPCId = LastSentRPCIds.FindRef(EntityType);
		}
		else
		{
			// We shouldn't have authority over the component that has the acks.
			if (View->HasAuthority(EntityId, RPCRingBufferUtils::GetAckComponentId(Type)))
			{
				return EPushRPCResult::HasAckAuthority;
			}

			LastAckedRPCId = GetAckFromView(EntityId, Type);
		}
	}
	else
	{
		// If the entity isn't in the view, we assume this RPC was called before
		// CreateEntityRequest, so we put it into a component data object.
		EndpointObject = Schema_GetComponentDataFields(GetOrCreateComponentData(EntityComponent));

		LastAckedRPCId = 0;
	}

	uint64 NewRPCId = LastSentRPCIds.FindRef(EntityType) + 1;

	// Check capacity.
	if (LastAckedRPCId + RPCRingBufferUtils::GetRingBufferSize(Type) >= NewRPCId)
	{
		RPCRingBufferUtils::WriteRPCToSchema(EndpointObject, Type, NewRPCId, Payload);

		LastSentRPCIds.Add(EntityType, NewRPCId);
	}
	else
	{
		// Overflowed
		if (RPCRingBufferUtils::ShouldQueueOverflowed(Type))
		{
			return EPushRPCResult::QueueOverflowed;
		}
		else
		{
			return EPushRPCResult::DropOverflowed;
		}
	}

	return EPushRPCResult::Success;
}

void SpatialRPCService::PushOverflowedRPCs()
{
	for (auto It = OverflowedRPCs.CreateIterator(); It; ++It)
	{
		Worker_EntityId EntityId = It.Key().EntityId;
		ERPCType Type = It.Key().Type;
		TArray<RPCPayload>& OverflowedRPCArray = It.Value();

		int NumProcessed = 0;
		bool bShouldDrop = false;
		for (RPCPayload& Payload : OverflowedRPCArray)
		{
			EPushRPCResult Result = PushRPCInternal(EntityId, Type, MoveTemp(Payload));

			switch (Result)
			{
			case EPushRPCResult::Success:
				NumProcessed++;
				break;
			case EPushRPCResult::DropOverflowed:
				checkf(false, TEXT("Shouldn't be able to drop on overflow for RPC type that was previously queued."));
				break;
			case EPushRPCResult::HasAckAuthority:
				UE_LOG(LogSpatialRPCService, Warning, TEXT("SpatialRPCService::PushOverflowedRPCs: Gained authority over ack component for RPC type that was overflowed. Entity: %lld, RPC type: %s"), EntityId, *SpatialConstants::RPCTypeToString(Type));
				bShouldDrop = true;
				break;
			case EPushRPCResult::NoRingBufferAuthority:
				UE_LOG(LogSpatialRPCService, Warning, TEXT("SpatialRPCService::PushOverflowedRPCs: Lost authority over ring buffer component for RPC type that was overflowed. Entity: %lld, RPC type: %s"), EntityId, *SpatialConstants::RPCTypeToString(Type));
				bShouldDrop = true;
				break;
			}

			// This includes the valid case of RPCs still overflowing (EPushRPCResult::QueueOverflowed), as well as the error cases.
			if (Result != EPushRPCResult::Success)
			{
				break;
			}
		}

		if (NumProcessed == OverflowedRPCArray.Num() || bShouldDrop)
		{
			It.RemoveCurrent();
		}
		else
		{
			OverflowedRPCArray.RemoveAt(0, NumProcessed);
		}
	}
}

void SpatialRPCService::ClearOverflowedRPCs(Worker_EntityId EntityId)
{
	for (uint8 RPCType = static_cast<uint8>(ERPCType::ClientReliable); RPCType <= static_cast<uint8>(ERPCType::NetMulticast); RPCType++)
	{
		OverflowedRPCs.Remove(EntityRPCType(EntityId, static_cast<ERPCType>(RPCType)));
	}
}

TArray<SpatialRPCService::UpdateToSend> SpatialRPCService::GetRPCsAndAcksToSend()
{
	TArray<SpatialRPCService::UpdateToSend> UpdatesToSend;

	for (auto& It : PendingComponentUpdatesToSend)
	{
		SpatialRPCService::UpdateToSend& UpdateToSend = UpdatesToSend.AddZeroed_GetRef();
		UpdateToSend.EntityId = It.Key.EntityId;
		UpdateToSend.UpdateObj.component_id = It.Key.ComponentId;
		UpdateToSend.UpdateObj.schema_type = It.Value;
	}

	PendingComponentUpdatesToSend.Empty();

	return UpdatesToSend;
}

TArray<Worker_ComponentData> SpatialRPCService::GetRPCComponentsOnEntityCreation(Worker_EntityId EntityId)
{
	static Worker_ComponentId EndpointComponentIds[] = {
		SpatialConstants::CLIENT_ENDPOINT_COMPONENT_ID,
		SpatialConstants::SERVER_ENDPOINT_COMPONENT_ID,
		SpatialConstants::MULTICAST_RPCS_COMPONENT_ID
	};

	TArray<Worker_ComponentData> Components;

	for (Worker_ComponentId EndpointComponentId : EndpointComponentIds)
	{
		EntityComponentId EntityComponent = EntityComponentId(EntityId, EndpointComponentId);

		Worker_ComponentData& Component = Components.AddZeroed_GetRef();
		Component.component_id = EndpointComponentId;
		if (Schema_ComponentData** ComponentData = PendingRPCsOnEntityCreation.Find(EntityComponent))
		{
			// When sending initial multicast RPCs, write the number of RPCs into a separate field instead of
			// last sent RPC ID field. When the server gains authority for the first time, it will copy the
			// value over to last sent RPC ID, so the clients that checked out the entity process the initial RPCs.
			if (EndpointComponentId == SpatialConstants::MULTICAST_RPCS_COMPONENT_ID)
			{
				RPCRingBufferUtils::MoveLastSentIdToInitiallyPresentCount(Schema_GetComponentDataFields(*ComponentData), LastSentRPCIds[EntityRPCType(EntityId, ERPCType::NetMulticast)]);
			}

			if (EndpointComponentId == SpatialConstants::CLIENT_ENDPOINT_COMPONENT_ID)
			{
				UE_LOG(LogSpatialRPCService, Error, TEXT("SpatialRPCService::GetRPCComponentsOnEntityCreation: Initial RPCs present on ClientEndpoint! EntityId: %lld"), EntityId);
			}

			Component.schema_type = *ComponentData;
			PendingRPCsOnEntityCreation.Remove(EntityComponent);
		}
		else
		{
			Component.schema_type = Schema_CreateComponentData();
		}
	}

	return Components;
}

void SpatialRPCService::ExtractRPCsForEntity(Worker_EntityId EntityId, Worker_ComponentId ComponentId)
{
	switch (ComponentId)
	{
	case SpatialConstants::CLIENT_ENDPOINT_COMPONENT_ID:
		if (View->HasAuthority(EntityId, SpatialConstants::SERVER_ENDPOINT_COMPONENT_ID))
		{
			ExtractRPCsForType(EntityId, ERPCType::ServerReliable);
			ExtractRPCsForType(EntityId, ERPCType::ServerUnreliable);
		}
		break;
	case SpatialConstants::SERVER_ENDPOINT_COMPONENT_ID:
		if (View->HasAuthority(EntityId, SpatialConstants::CLIENT_ENDPOINT_COMPONENT_ID))
		{
			ExtractRPCsForType(EntityId, ERPCType::ClientReliable);
			ExtractRPCsForType(EntityId, ERPCType::ClientUnreliable);
		}
		break;
	case SpatialConstants::MULTICAST_RPCS_COMPONENT_ID:
		ExtractRPCsForType(EntityId, ERPCType::NetMulticast);
		break;
	default:
		checkNoEntry();
		break;
	}
}

void SpatialRPCService::OnCheckoutEntity(Worker_EntityId EntityId)
{
	const MulticastRPCs* Component = View->GetComponentData<MulticastRPCs>(EntityId);
	// When checking out entity, ignore multicast RPCs that are already on the component.
	LastSeenMulticastRPCIds.Add(EntityId, Component->MulticastRPCBuffer.LastSentRPCId);
}

void SpatialRPCService::OnRemoveEntity(Worker_EntityId EntityId)
{
	LastSeenMulticastRPCIds.Remove(EntityId);
}

void SpatialRPCService::OnEndpointAuthorityGained(Worker_EntityId EntityId, Worker_ComponentId ComponentId)
{
	switch (ComponentId)
	{
	case SpatialConstants::CLIENT_ENDPOINT_COMPONENT_ID:
	{
		const ClientEndpoint* Endpoint = View->GetComponentData<ClientEndpoint>(EntityId);
		LastAckedRPCIds.Add(EntityRPCType(EntityId, ERPCType::ClientReliable), Endpoint->ReliableRPCAck);
		LastAckedRPCIds.Add(EntityRPCType(EntityId, ERPCType::ClientUnreliable), Endpoint->UnreliableRPCAck);
		LastSentRPCIds.Add(EntityRPCType(EntityId, ERPCType::ServerReliable), Endpoint->ReliableRPCBuffer.LastSentRPCId);
		LastSentRPCIds.Add(EntityRPCType(EntityId, ERPCType::ServerUnreliable), Endpoint->UnreliableRPCBuffer.LastSentRPCId);
		break;
	}
	case SpatialConstants::SERVER_ENDPOINT_COMPONENT_ID:
	{
		const ServerEndpoint* Endpoint = View->GetComponentData<ServerEndpoint>(EntityId);
		LastAckedRPCIds.Add(EntityRPCType(EntityId, ERPCType::ServerReliable), Endpoint->ReliableRPCAck);
		LastAckedRPCIds.Add(EntityRPCType(EntityId, ERPCType::ServerUnreliable), Endpoint->UnreliableRPCAck);
		LastSentRPCIds.Add(EntityRPCType(EntityId, ERPCType::ClientReliable), Endpoint->ReliableRPCBuffer.LastSentRPCId);
		LastSentRPCIds.Add(EntityRPCType(EntityId, ERPCType::ClientUnreliable), Endpoint->UnreliableRPCBuffer.LastSentRPCId);
		break;
	}
	case SpatialConstants::MULTICAST_RPCS_COMPONENT_ID:
	{
		const MulticastRPCs* Component = View->GetComponentData<MulticastRPCs>(EntityId);

		if (Component->MulticastRPCBuffer.LastSentRPCId == 0 && Component->InitiallyPresentMulticastRPCsCount > 0)
		{
			// Update last sent ID to the number of initially present RPCs so the clients who check out this entity
			// as it's created can process the initial multicast RPCs.
			LastSentRPCIds.Add(EntityRPCType(EntityId, ERPCType::NetMulticast), Component->InitiallyPresentMulticastRPCsCount);

			RPCRingBufferDescriptor Descriptor = RPCRingBufferUtils::GetRingBufferDescriptor(ERPCType::NetMulticast);
			Schema_Object* SchemaObject = Schema_GetComponentUpdateFields(GetOrCreateComponentUpdate(EntityComponentId(EntityId, ComponentId)));
			Schema_AddUint64(SchemaObject, Descriptor.LastSentRPCFieldId, Component->InitiallyPresentMulticastRPCsCount);
		}
		else
		{
			LastSentRPCIds.Add(EntityRPCType(EntityId, ERPCType::NetMulticast), Component->MulticastRPCBuffer.LastSentRPCId);
		}

		break;
	}
	default:
		checkNoEntry();
		break;
	}
}

void SpatialRPCService::OnEndpointAuthorityLost(Worker_EntityId EntityId, Worker_ComponentId ComponentId)
{
	switch (ComponentId)
	{
	case SpatialConstants::CLIENT_ENDPOINT_COMPONENT_ID:
	{
		LastAckedRPCIds.Remove(EntityRPCType(EntityId, ERPCType::ClientReliable));
		LastAckedRPCIds.Remove(EntityRPCType(EntityId, ERPCType::ClientUnreliable));
		LastSentRPCIds.Remove(EntityRPCType(EntityId, ERPCType::ServerReliable));
		LastSentRPCIds.Remove(EntityRPCType(EntityId, ERPCType::ServerUnreliable));
		ClearOverflowedRPCs(EntityId);
		break;
	}
	case SpatialConstants::SERVER_ENDPOINT_COMPONENT_ID:
	{
		LastAckedRPCIds.Remove(EntityRPCType(EntityId, ERPCType::ServerReliable));
		LastAckedRPCIds.Remove(EntityRPCType(EntityId, ERPCType::ServerUnreliable));
		LastSentRPCIds.Remove(EntityRPCType(EntityId, ERPCType::ClientReliable));
		LastSentRPCIds.Remove(EntityRPCType(EntityId, ERPCType::ClientUnreliable));
		ClearOverflowedRPCs(EntityId);
		break;
	}
	case SpatialConstants::MULTICAST_RPCS_COMPONENT_ID:
	{
		// Set last seen to last sent, so we don't process own RPCs after crossing the boundary.
		LastSeenMulticastRPCIds.Add(EntityId, LastSentRPCIds[EntityRPCType(EntityId, ERPCType::NetMulticast)]);
		LastSentRPCIds.Remove(EntityRPCType(EntityId, ERPCType::NetMulticast));
		break;
	}
	default:
		checkNoEntry();
		break;
	}
}

void SpatialRPCService::ExtractRPCsForType(Worker_EntityId EntityId, ERPCType Type)
{
	uint64 LastSeenRPCId;
	EntityRPCType EntityTypePair = EntityRPCType(EntityId, Type);

	if (Type == ERPCType::NetMulticast)
	{
		LastSeenRPCId = LastSeenMulticastRPCIds[EntityId];
	}
	else
	{
		LastSeenRPCId = LastAckedRPCIds[EntityTypePair];
	}

	const RPCRingBuffer& Buffer = GetBufferFromView(EntityId, Type);

	uint64 LastProcessedRPCId = LastSeenRPCId;
	if (Buffer.LastSentRPCId >= LastSeenRPCId)
	{
		uint64 FirstRPCIdToRead = LastSeenRPCId + 1;

		uint32 BufferSize = RPCRingBufferUtils::GetRingBufferSize(Type);
		if (Buffer.LastSentRPCId > LastSeenRPCId + BufferSize)
		{
			UE_LOG(LogSpatialRPCService, Warning, TEXT("SpatialRPCService::ExtractRPCsForType: RPCs were overwritten without being processed! Entity: %lld, RPC type: %s, last seen RPC ID: %d, last sent ID: %d, buffer size: %d"),
				EntityId, *SpatialConstants::RPCTypeToString(Type), LastSeenRPCId, Buffer.LastSentRPCId, BufferSize);
			FirstRPCIdToRead = Buffer.LastSentRPCId - BufferSize + 1;
		}

		for (uint64 RPCId = FirstRPCIdToRead; RPCId <= Buffer.LastSentRPCId; RPCId++)
		{
			const TOptional<RPCPayload>& Element = Buffer.GetRingBufferElement(RPCId);
			if (Element.IsSet())
			{
				bool bKeepExtracting = ExtractRPCCallback.Execute(EntityId, Type, Element.GetValue());
				if (!bKeepExtracting)
				{
					break;
				}
				LastProcessedRPCId = RPCId;
			}
			else
			{
				UE_LOG(LogSpatialRPCService, Warning, TEXT("SpatialRPCService::ExtractRPCsForType: Ring buffer element empty. Entity: %lld, RPC type: %s, empty element RPC id: %d"), EntityId, *SpatialConstants::RPCTypeToString(Type), RPCId);
			}
		}
	}
	else
	{
		UE_LOG(LogSpatialRPCService, Warning, TEXT("SpatialRPCService::ExtractRPCsForType: Last sent RPC has smaller ID than last seen RPC. Entity: %lld, RPC type: %s, last sent ID: %d, last seen ID: %d"),
			EntityId, *SpatialConstants::RPCTypeToString(Type), Buffer.LastSentRPCId, LastSeenRPCId);
	}

	if (LastProcessedRPCId > LastSeenRPCId)
	{
		if (Type == ERPCType::NetMulticast)
		{
			LastSeenMulticastRPCIds[EntityId] = LastProcessedRPCId;
		}
		else
		{
			LastAckedRPCIds[EntityTypePair] = LastProcessedRPCId;
			EntityComponentId EntityComponentPair = EntityComponentId(EntityId, RPCRingBufferUtils::GetAckComponentId(Type));

			Schema_Object* EndpointObject = Schema_GetComponentUpdateFields(GetOrCreateComponentUpdate(EntityComponentPair));

			RPCRingBufferUtils::WriteAckToSchema(EndpointObject, Type, LastProcessedRPCId);
		}
	}
}

void SpatialRPCService::AddOverflowedRPC(EntityRPCType EntityType, RPCPayload&& Payload)
{
	OverflowedRPCs.FindOrAdd(EntityType).Add(MoveTemp(Payload));
}

uint64 SpatialRPCService::GetAckFromView(Worker_EntityId EntityId, ERPCType Type)
{
	switch (Type)
	{
	case ERPCType::ClientReliable:
		return View->GetComponentData<ClientEndpoint>(EntityId)->ReliableRPCAck;
	case ERPCType::ClientUnreliable:
		return View->GetComponentData<ClientEndpoint>(EntityId)->UnreliableRPCAck;
	case ERPCType::ServerReliable:
		return View->GetComponentData<ServerEndpoint>(EntityId)->ReliableRPCAck;
	case ERPCType::ServerUnreliable:
		return View->GetComponentData<ServerEndpoint>(EntityId)->UnreliableRPCAck;
	}

	checkNoEntry();
	return 0;
}

const RPCRingBuffer& SpatialRPCService::GetBufferFromView(Worker_EntityId EntityId, ERPCType Type)
{
	switch (Type)
	{
	// Server sends Client RPCs, so ClientReliable & ClientUnreliable buffers live on ServerEndpoint.
	case ERPCType::ClientReliable:
		return View->GetComponentData<ServerEndpoint>(EntityId)->ReliableRPCBuffer;
	case ERPCType::ClientUnreliable:
		return View->GetComponentData<ServerEndpoint>(EntityId)->UnreliableRPCBuffer;

	// Client sends Server RPCs, so ServerReliable & ServerUnreliable buffers live on ClientEndpoint.
	case ERPCType::ServerReliable:
		return View->GetComponentData<ClientEndpoint>(EntityId)->ReliableRPCBuffer;
	case ERPCType::ServerUnreliable:
		return View->GetComponentData<ClientEndpoint>(EntityId)->UnreliableRPCBuffer;

	case ERPCType::NetMulticast:
		return View->GetComponentData<MulticastRPCs>(EntityId)->MulticastRPCBuffer;
	}

	checkNoEntry();
	static const RPCRingBuffer DummyBuffer(ERPCType::Invalid);
	return DummyBuffer;
}

Schema_ComponentUpdate* SpatialRPCService::GetOrCreateComponentUpdate(EntityComponentId EntityComponentIdPair)
{
	Schema_ComponentUpdate** ComponentUpdatePtr = PendingComponentUpdatesToSend.Find(EntityComponentIdPair);
	if (ComponentUpdatePtr == nullptr)
	{
		ComponentUpdatePtr = &PendingComponentUpdatesToSend.Add(EntityComponentIdPair, Schema_CreateComponentUpdate());
	}
	return *ComponentUpdatePtr;
}

Schema_ComponentData* SpatialRPCService::GetOrCreateComponentData(EntityComponentId EntityComponentIdPair)
{
	Schema_ComponentData** ComponentDataPtr = PendingRPCsOnEntityCreation.Find(EntityComponentIdPair);
	if (ComponentDataPtr == nullptr)
	{
		ComponentDataPtr = &PendingRPCsOnEntityCreation.Add(EntityComponentIdPair, Schema_CreateComponentData());
	}
	return *ComponentDataPtr;
}

} // namespace SpatialGDK

