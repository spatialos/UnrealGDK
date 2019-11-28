// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "Interop/SpatialRPCService.h"

#include "Interop/SpatialStaticComponentView.h"
#include "SpatialGDKSettings.h"

DEFINE_LOG_CATEGORY(LogSpatialRPCService);

namespace SpatialGDK
{
class ClientEndpoint;
class ServerEndpoint;
class MulticastEndpoint;

SpatialRPCService::SpatialRPCService(SpatialRPCService::ExtractRPCCallbackType ExtractRPCCallback, const USpatialStaticComponentView* View)
	: ExtractRPCCallback(ExtractRPCCallback)
	, View(View)
{
}

RingBufferDescriptor SpatialRPCService::GetRingBufferDescriptor(ERPCType Type)
{
	if (RingBufferDescriptor* Descriptor = RingBufferDescriptors.Find(Type))
	{
		return *Descriptor;
	}

	UE_LOG(LogSpatialRPCService, Error, TEXT(""));
	return RingBufferDescriptor();
}

void SpatialRPCService::InitRingBufferDescriptors()
{
	// Client endpoint looks like:
	//   [ Server Reliable RPC Ring Buffer + Last Sent ID, Server Unreliable RPC Ring Buffer + Last Sent ID, Client Reliable Ack, Client Unreliable Ack ]
	// Server endpoint:
	//   [ Client Reliable RPC Ring Buffer + Last Sent ID, Client Unreliable RPC Ring Buffer + Last Sent ID, Server Reliable Ack, Server Unreliable Ack ]
	// Multicast endpoint:
	//   [ Multicast RPC Ring Buffer + Last Sent ID ]
	//
	// I want the descriptors to be defined in this form, but have the opposite mapping: from RPC Type to components and field IDs, e.g.:
	//
	// Client Unreliable RPCs: {
	//   Ring Buffer Component: SERVER_ENDPOINT,
	//   Ring Buffer Field ID start: Client Reliable Last Sent ID Field ID + 1,
	//   Ring Buffer size: get from settings,
	//   Last Sent ID Field ID: Field ID start + Ring Buffer size,
	//   Ack Component: CLIENT_ENDPOINT,
	//   Ack Field ID: Client Reliable Ack Field ID + 1,
	//   Should queue overflowed: false
	// }
	const USpatialGDKSettings* SpatialGDKSettings = GetDefault<USpatialGDKSettings>();

	RingBufferDescriptors.Empty();

	RingBufferDescriptors.Add(ERPCType::ClientReliable, {
			ERPCType::ClientReliable,
			SpatialConstants::SERVER_RPC_ENDPOINT_COMPONENT_ID, // buffer comp
			SpatialGDKSettings->GetRPCRingBufferSize(ERPCType::ClientReliable), // buffer size
			1, // field start
			1 + SpatialGDKSettings->MaxRPCRingBufferSize, // last sent field id
			SpatialConstants::CLIENT_RPC_ENDPOINT_COMPONENT_ID, // ack comp
			SpatialGDKSettings->MaxRPCRingBufferSize * 2 + 2 + 1, // ack field id
			true // should queue overflowed
		});

	RingBufferDescriptors.Add(ERPCType::ClientUnreliable, {
			ERPCType::ClientUnreliable,
			SpatialConstants::SERVER_RPC_ENDPOINT_COMPONENT_ID, // buffer comp
			SpatialGDKSettings->GetRPCRingBufferSize(ERPCType::ClientUnreliable), // buffer size
			RingBufferDescriptors[ERPCType::ClientReliable].LastSentFieldId + 1, // field start
			RingBufferDescriptors[ERPCType::ClientReliable].LastSentFieldId + 1 + SpatialGDKSettings->MaxRPCRingBufferSize, // last sent field id
			SpatialConstants::CLIENT_RPC_ENDPOINT_COMPONENT_ID, // ack comp
			RingBufferDescriptors[ERPCType::ClientReliable].AckFieldId + 1, // ack field id
			false // should queue overflowed
		});

	RingBufferDescriptors.Add(ERPCType::ServerReliable, {
			ERPCType::ServerReliable,
			SpatialConstants::CLIENT_RPC_ENDPOINT_COMPONENT_ID, // buffer comp
			SpatialGDKSettings->GetRPCRingBufferSize(ERPCType::ServerReliable), // buffer size
			1, // field start
			1 + SpatialGDKSettings->MaxRPCRingBufferSize, // last sent field id
			SpatialConstants::SERVER_RPC_ENDPOINT_COMPONENT_ID, // ack comp
			SpatialGDKSettings->MaxRPCRingBufferSize * 2 + 2 + 1, // ack field id
			true // should queue overflowed
		});

	RingBufferDescriptors.Add(ERPCType::ServerUnreliable, {
			ERPCType::ServerUnreliable,
			SpatialConstants::CLIENT_RPC_ENDPOINT_COMPONENT_ID, // buffer comp
			SpatialGDKSettings->GetRPCRingBufferSize(ERPCType::ServerUnreliable), // buffer size
			RingBufferDescriptors[ERPCType::ServerReliable].LastSentFieldId + 1, // field start
			RingBufferDescriptors[ERPCType::ServerReliable].LastSentFieldId + 1 + SpatialGDKSettings->MaxRPCRingBufferSize, // last sent field id
			SpatialConstants::SERVER_RPC_ENDPOINT_COMPONENT_ID, // ack comp
			RingBufferDescriptors[ERPCType::ServerReliable].AckFieldId + 1, // ack field id
			false // should queue overflowed
		});

	RingBufferDescriptors.Add(ERPCType::NetMulticast, {
			ERPCType::NetMulticast,
			SpatialConstants::NETMULTICAST_RPCS_COMPONENT_ID, // buffer comp
			SpatialGDKSettings->GetRPCRingBufferSize(ERPCType::NetMulticast), // buffer size
			1, // field start
			1 + SpatialGDKSettings->MaxRPCRingBufferSize, // last sent field id
			SpatialConstants::INVALID_COMPONENT_ID, // ack comp
			0, // ack field id
			false // should queue overflowed
		});

	// TODO: Tests to make sure these numbers add up.
}

void SpatialRPCService::PushRPC(Worker_EntityId EntityId, ERPCType Type, RPCPayload Payload)
{
	RingBufferDescriptor Descriptor = GetRingBufferDescriptor(Type);

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

			LastAckedRPCId = GetAckFromView(EntityId, Descriptor);
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
		Schema_Object* RPCObject = Schema_AddObject(EndpointObject, Descriptor.GetRingBufferElementFieldId(NewRPCId));
		RPCPayload::WriteToSchemaObject(RPCObject, Payload.Offset, Payload.Index, Payload.PayloadData.GetData(), Payload.PayloadData.Num());

		Schema_ClearField(EndpointObject, Descriptor.LastSentFieldId);
		Schema_AddUint64(EndpointObject, Descriptor.LastSentFieldId, NewRPCId);

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
		SpatialConstants::CLIENT_RPC_ENDPOINT_COMPONENT_ID,
		SpatialConstants::SERVER_RPC_ENDPOINT_COMPONENT_ID,
		SpatialConstants::NETMULTICAST_RPCS_COMPONENT_ID
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

}

void SpatialRPCService::AddOverflowedRPC(EntityComponentId EntityComponent, RPCPayload Payload)
{
	OverflowedRPCs.FindOrAdd(EntityComponent).Add(Payload);
}

uint64 SpatialRPCService::GetAckFromView(Worker_EntityId EntityId, const RingBufferDescriptor& Descriptor)
{
	switch (Descriptor.AckComponentId)
	{
	case SpatialConstants::CLIENT_RPC_ENDPOINT_COMPONENT_ID:
		return View->GetComponentData<ClientEndpoint>(EntityId)->GetAck(Descriptor.Type);
	case SpatialConstants::SERVER_RPC_ENDPOINT_COMPONENT_ID:
		return View->GetComponentData<ServerEndpoint>(EntityId)->GetAck(Descriptor.Type);
	default:
		checkNoEntry();
		return 0;
	}
}

const RPCRingBuffer& SpatialRPCService::GetBufferFromView(Worker_EntityId EntityId, const RingBufferDescriptor& Descriptor)
{
	switch (Descriptor.AckComponentId)
	{
	case SpatialConstants::CLIENT_RPC_ENDPOINT_COMPONENT_ID:
		return View->GetComponentData<ClientEndpoint>(EntityId)->GetBuffer(Descriptor.Type);
	case SpatialConstants::SERVER_RPC_ENDPOINT_COMPONENT_ID:
		return View->GetComponentData<ServerEndpoint>(EntityId)->GetBuffer(Descriptor.Type);
	case SpatialConstants::NETMULTICAST_RPCS_COMPONENT_ID:
		return View->GetComponentData<MulticastEndpoint>(EntityId)->GetBuffer(Descriptor.Type);
	default:
		checkNoEntry();
		return RPCRingBuffer();
	}
}

} // namespace SpatialGDK

