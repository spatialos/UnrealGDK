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
	RPCRingBufferDescriptor Descriptor = RPCRingBufferUtils::GetRingBufferDescriptor(Type);
	EntityRPCType EntityType = EntityRPCType(EntityId, Type);

	if (Descriptor.bShouldQueueOverflowed && OverflowedRPCs.Contains(EntityType))
	{
		// Already has queued RPCs of this type, queue until those are pushed.
		AddOverflowedRPC(EntityType, Payload);
		return EPushRPCResult::QueueOverflowed;
	}

	EPushRPCResult Result = PushRPCInternal(EntityId, Type, Payload);

	if (Result == EPushRPCResult::QueueOverflowed)
	{
		AddOverflowedRPC(EntityType, Payload);
	}

	return Result;
}

EPushRPCResult SpatialRPCService::PushRPCInternal(Worker_EntityId EntityId, ERPCType Type, RPCPayload Payload)
{
	RPCRingBufferDescriptor Descriptor = RPCRingBufferUtils::GetRingBufferDescriptor(Type);

	EntityComponentId EntityComponent = EntityComponentId(EntityId, Descriptor.RingBufferComponentId);
	EntityRPCType EntityType = EntityRPCType(EntityId, Type);

	Schema_Object* EndpointObject;
	uint64 LastAckedRPCId;
	if (View->HasComponent(EntityId, Descriptor.RingBufferComponentId))
	{
		check(View->HasAuthority(EntityId, Descriptor.RingBufferComponentId));

		Schema_ComponentUpdate** ComponentUpdatePtr = PendingComponentUpdatesToSend.Find(EntityComponent);
		if (ComponentUpdatePtr == nullptr)
		{
			ComponentUpdatePtr = &PendingComponentUpdatesToSend.Add(EntityComponent, Schema_CreateComponentUpdate());
		}
		EndpointObject = Schema_GetComponentUpdateFields(*ComponentUpdatePtr);

		if (Type == ERPCType::NetMulticast)
		{
			// Assume all multicast RPCs are auto-acked.
			LastAckedRPCId = LastSentRPCIds[EntityType];
		}
		else
		{
			// We shouldn't have authority over the component that has the acks.
			if (View->HasAuthority(EntityId, Descriptor.AckComponentId))
			{
				return EPushRPCResult::AckAuthority;
			}

			LastAckedRPCId = GetAckFromView(EntityId, Type);
		}
	}
	else
	{
		// If the entity isn't in the view, we assume this RPC was called before
		// CreateEntityRequest, so we put it into a component data object.
		Schema_ComponentData** ComponentDataPtr = PendingRPCsOnEntityCreation.Find(EntityComponent);
		if (ComponentDataPtr == nullptr)
		{
			ComponentDataPtr = &PendingRPCsOnEntityCreation.Add(EntityComponent, Schema_CreateComponentData());
		}
		EndpointObject = Schema_GetComponentDataFields(*ComponentDataPtr);

		LastAckedRPCId = 0;
	}

	if (!LastSentRPCIds.Contains(EntityType))
	{
		// This is a hack to cover the case where we created an actor and pushing RPCs on it before the entity is created.
		// Since we add things to LastSentRPCIds when we gain authority over the component, add 0 for the new entity.
		check(!View->HasComponent(EntityId, Descriptor.RingBufferComponentId));
		LastSentRPCIds.Add(EntityType, 0);
	}
	uint64 NewRPCId = LastSentRPCIds[EntityType] + 1;

	// Check capacity.
	if (Descriptor.HasCapacity(LastAckedRPCId, NewRPCId))
	{
		RPCRingBufferUtils::WriteRPCToSchema(EndpointObject, Descriptor, NewRPCId, MoveTemp(Payload));

		LastSentRPCIds[EntityType] = NewRPCId;
	}
	else
	{
		// Overflowed
		if (Descriptor.bShouldQueueOverflowed)
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

EPushRPCResult SpatialRPCService::PushOverflowedRPCs()
{
	bool bOverflowedRPCsRemaining = false;

	for (auto It = OverflowedRPCs.CreateIterator(); It; ++It)
	{
		Worker_EntityId EntityId = It.Key().EntityId;
		ERPCType Type = It.Key().Type;
		TArray<RPCPayload>& OverflowedRPCArray = It.Value();

		int NumProcessed = 0;
		for (RPCPayload& Payload : OverflowedRPCArray)
		{
			EPushRPCResult Result = PushRPCInternal(EntityId, Type, Payload);
			if (Result == EPushRPCResult::Success)
			{
				NumProcessed++;
			}
			else if (Result == EPushRPCResult::QueueOverflowed)
			{
				bOverflowedRPCsRemaining = true;
				break;
			}
			else
			{
				// Shouldn't happen, let the caller log.
				return Result;
			}
		}

		if (NumProcessed == OverflowedRPCArray.Num())
		{
			It.RemoveCurrent();
		}
		else
		{
			OverflowedRPCArray.RemoveAt(0, NumProcessed);
		}
	}

	return bOverflowedRPCsRemaining ? EPushRPCResult::QueueOverflowed : EPushRPCResult::Success;
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
		UpdateToSend.Update.component_id = It.Key.ComponentId;
		UpdateToSend.Update.schema_type = It.Value;
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
		LastSentRPCIds.Add(EntityRPCType(EntityId, ERPCType::NetMulticast), Component->MulticastRPCBuffer.LastSentRPCId);

		// Duplicate the initial multicast RPCs?
		break;
	}
	default:
		checkNoEntry();
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
		break;
	}
	case SpatialConstants::SERVER_ENDPOINT_COMPONENT_ID:
	{
		LastAckedRPCIds.Remove(EntityRPCType(EntityId, ERPCType::ServerReliable));
		LastAckedRPCIds.Remove(EntityRPCType(EntityId, ERPCType::ServerUnreliable));
		LastSentRPCIds.Remove(EntityRPCType(EntityId, ERPCType::ClientReliable));
		LastSentRPCIds.Remove(EntityRPCType(EntityId, ERPCType::ClientUnreliable));
		break;
	}
	case SpatialConstants::MULTICAST_RPCS_COMPONENT_ID:
	{
		LastSentRPCIds.Remove(EntityRPCType(EntityId, ERPCType::NetMulticast));
		break;
	}
	default:
		checkNoEntry();
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

	RPCRingBufferDescriptor Descriptor = RPCRingBufferUtils::GetRingBufferDescriptor(Type);

	if (Buffer.LastSentRPCId >= LastSeenRPCId)
	{
		for (uint64 RPCId = LastSeenRPCId + 1; RPCId <= Buffer.LastSentRPCId; RPCId++)
		{
			uint32 ElementIndex = Descriptor.GetRingBufferElementIndex(RPCId);
			if (Buffer.RingBuffer[ElementIndex].IsSet())
			{
				bool bKeepExtracting = ExtractRPCCallback.Execute(EntityId, Type, Buffer.RingBuffer[ElementIndex].GetValue());
				if (!bKeepExtracting)
				{
					break;
				}
			}
		}
	}
	else
	{
		// received RPCs out of order?
	}

	if (Type == ERPCType::NetMulticast)
	{
		LastSeenMulticastRPCIds[EntityId] = Buffer.LastSentRPCId;
	}
	else
	{
		LastAckedRPCIds[EntityTypePair] = Buffer.LastSentRPCId;
		EntityComponentId EntityComponentPair = EntityComponentId(EntityId, Descriptor.AckComponentId);
		if (View->HasComponent(EntityId, Descriptor.AckComponentId))
		{
			Schema_ComponentUpdate** ComponentUpdatePtr = PendingComponentUpdatesToSend.Find(EntityComponentPair);
			if (ComponentUpdatePtr == nullptr)
			{
				ComponentUpdatePtr = &PendingComponentUpdatesToSend.Add(EntityComponentPair, Schema_CreateComponentUpdate());
			}
			Schema_Object* EndpointObject = Schema_GetComponentUpdateFields(*ComponentUpdatePtr);

			RPCRingBufferUtils::WriteAckToSchema(EndpointObject, Descriptor, Buffer.LastSentRPCId);
		}
		else
		{
			// Return some error code?
		}
	}
}

void SpatialRPCService::AddOverflowedRPC(EntityRPCType EntityType, RPCPayload Payload)
{
	OverflowedRPCs.FindOrAdd(EntityType).Add(Payload);
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
	static RPCRingBuffer DummyBuffer;
	return DummyBuffer;
}

} // namespace SpatialGDK

