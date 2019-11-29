// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "Interop/SpatialRPCService.h"

#include "Interop/SpatialStaticComponentView.h"
#include "Schema/ClientEndpoint.h"
#include "Schema/MulticastRPCs.h"
#include "Schema/ServerEndpoint.h"

DEFINE_LOG_CATEGORY(LogSpatialRPCService);

namespace SpatialGDK
{

SpatialRPCService::SpatialRPCService(SpatialRPCService::ExtractRPCCallbackType ExtractRPCCallback, const USpatialStaticComponentView* View)
	: ExtractRPCCallback(ExtractRPCCallback)
	, View(View)
{
}

void SpatialRPCService::PushRPC(Worker_EntityId EntityId, ERPCType Type, RPCPayload Payload)
{
	RPCRingBufferDescriptor Descriptor = RPCRingBufferUtils::GetRingBufferDescriptor(Type);

	EntityComponentId EntityComponent = EntityComponentId(EntityId, Descriptor.RingBufferComponentId);
	EntityRPCType EntityType = EntityRPCType(EntityId, Type);

	if (Descriptor.bShouldQueueOverflowed && OverflowedRPCs.Contains(EntityComponent))
	{
		// Log and add to overflowed.
		AddOverflowedRPC(EntityComponent, MoveTemp(Payload));
		return;
	}

	Schema_Object* EndpointObject;
	uint64 LastAckedRPCId;
	if (View->HasComponent(EntityId, Descriptor.RingBufferComponentId))
	{
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
				UE_LOG(LogSpatialRPCService, Error, TEXT("SpatialRPCService::PushRPC: Has authority on ack component when sending RPC. Entity: %lld, component: %d"), EntityId, Descriptor.AckComponentId);
				return;
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

	uint64 NewRPCId = LastSentRPCIds[EntityType] + 1;

	// Check capacity.
	if (Descriptor.HasCapacity(LastAckedRPCId, NewRPCId))
	{
		/*Schema_Object* RPCObject = Schema_AddObject(EndpointObject, Descriptor.GetRingBufferElementFieldId(NewRPCId));
		RPCPayload::WriteToSchemaObject(RPCObject, Payload.Offset, Payload.Index, Payload.PayloadData.GetData(), Payload.PayloadData.Num());

		Schema_ClearField(EndpointObject, Descriptor.LastSentFieldId);
		Schema_AddUint64(EndpointObject, Descriptor.LastSentFieldId, NewRPCId);*/

		RPCRingBufferUtils::WriteRPCToSchema(EndpointObject, Descriptor, NewRPCId, MoveTemp(Payload));

		LastSentRPCIds[EntityType] = NewRPCId;
	}
	else
	{
		//UE_LOG(LogSpatialRPCService, Warning, TEXT("SpatialRPCService::PushRPC: Queuing RPC for object: %s %s"), *TargetObject->GetPathName(), *Function->GetName());

		// Overflowed
		if (Descriptor.bShouldQueueOverflowed)
		{
			AddOverflowedRPC(EntityComponent, MoveTemp(Payload));
		}
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
		ExtractRPCsForType(EntityId, ERPCType::ServerReliable);
		ExtractRPCsForType(EntityId, ERPCType::ServerUnreliable);
		break;
	case SpatialConstants::SERVER_ENDPOINT_COMPONENT_ID:
		ExtractRPCsForType(EntityId, ERPCType::ServerReliable);
		ExtractRPCsForType(EntityId, ERPCType::ServerUnreliable);
		break;
	case SpatialConstants::MULTICAST_RPCS_COMPONENT_ID:
		ExtractRPCsForType(EntityId, ERPCType::NetMulticast);
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

	RPCRingBufferDescriptor Descriptor = RPCRingBufferUtils::GetRingBufferDescriptor(Type);

	if (Buffer.LastSentRPCId >= LastSeenRPCId)
	{
		for (uint64 RPCId = LastSeenRPCId + 1; RPCId <= Buffer.LastSentRPCId; RPCId++)
		{
			uint32 ElementIndex = Descriptor.GetRingBufferElementIndex(RPCId);
			if (Buffer.RingBuffer[ElementIndex].IsSet())
			{
				ExtractRPCCallback(EntityId, Type, Buffer.RingBuffer[ElementIndex].GetValue());
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

void SpatialRPCService::AddOverflowedRPC(EntityComponentId EntityComponent, RPCPayload Payload)
{
	OverflowedRPCs.FindOrAdd(EntityComponent).Add(Payload);
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

