// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "Interop/RPCs/MulticastRPCService.h"

#include "EngineClasses/SpatialPackageMapClient.h"
#include "Interop/RPCs/SpatialRPCService.h"
#include "Schema/MulticastRPCs.h"
#include "SpatialConstants.h"
#include "SpatialView/EntityComponentTypes.h"
#include "Utils/RepLayoutUtils.h"

DEFINE_LOG_CATEGORY(LogMulticastRPCService);

namespace SpatialGDK
{
MulticastRPCService::MulticastRPCService(const ExtractRPCDelegate InExtractRPCCallback, const FSubView& InSubView, FRPCStore& InRPCStore)
	: ExtractRPCCallback(InExtractRPCCallback)
	, SubView(&InSubView)
	, RPCStore(&InRPCStore)
{
}

void MulticastRPCService::AdvanceView()
{
	const FSubViewDelta& SubViewDelta = SubView->GetViewDelta();
	for (const EntityDelta& Delta : SubViewDelta.EntityDeltas)
	{
		switch (Delta.Type)
		{
		case EntityDelta::UPDATE:
		{
			for (const AuthorityChange& Change : Delta.AuthorityLostTemporarily)
			{
				// We process auth lost temporarily twice. Once before updates and once after, so as not
				// to process updates that we received while we think we are still authoritiative.
				AuthorityLost(Delta.EntityId, Change.ComponentSetId);
			}
			for (const AuthorityChange& Change : Delta.AuthorityLost)
			{
				AuthorityLost(Delta.EntityId, Change.ComponentSetId);
			}
			for (const ComponentChange& Change : Delta.ComponentUpdates)
			{
				if (Change.ComponentId == SpatialConstants::MULTICAST_RPCS_COMPONENT_ID)
				{
					ApplyComponentUpdate(Delta.EntityId, Change.Update);
				}
				else if (Change.ComponentId > SpatialConstants::MULTICAST_RPCS_COMPONENT_ID)
				{
					break;
				}
			}
			for (const ComponentChange& Change : Delta.ComponentsRefreshed)
			{
				if (Change.ComponentId == SpatialConstants::MULTICAST_RPCS_COMPONENT_ID)
				{
					PopulateDataStore(Delta.EntityId);
				}
				else if (Change.ComponentId > SpatialConstants::MULTICAST_RPCS_COMPONENT_ID)
				{
					break;
				}
			}
			for (const AuthorityChange& Change : Delta.AuthorityGained)
			{
				AuthorityGained(Delta.EntityId, Change.ComponentSetId);
			}
			for (const AuthorityChange& Change : Delta.AuthorityLostTemporarily)
			{
				// Updates that we could have received while we weren't authoritative have now been processed.
				// Regain authority.
				AuthorityGained(Delta.EntityId, Change.ComponentSetId);
			}
			break;
		}
		case EntityDelta::ADD:
			PopulateDataStore(Delta.EntityId);
			break;
		case EntityDelta::REMOVE:
			OnRemoveMulticastRPCComponentForEntity(Delta.EntityId);
			MulticastDataStore.Remove(Delta.EntityId);
			break;
		case EntityDelta::TEMPORARILY_REMOVED:
			OnRemoveMulticastRPCComponentForEntity(Delta.EntityId);
			MulticastDataStore.Remove(Delta.EntityId);
			PopulateDataStore(Delta.EntityId);
			break;
		default:
			break;
		}
	}
}

void MulticastRPCService::ProcessChanges()
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
				ComponentUpdate(Delta.EntityId, Change.ComponentId);
			}
			for (const ComponentChange& Change : Delta.ComponentsRefreshed)
			{
				ComponentUpdate(Delta.EntityId, Change.ComponentId);
			}
			break;
		}
		case EntityDelta::ADD:
			EntityAdded(Delta.EntityId);
			break;
		case EntityDelta::TEMPORARILY_REMOVED:
			EntityRefresh(Delta.EntityId);
			break;
		default:
			break;
		}
	}
}

void MulticastRPCService::EntityAdded(const Worker_EntityId EntityId)
{
	OnCheckoutMulticastRPCComponentOnEntity(EntityId);
	for (const Worker_ComponentId ComponentId : SubView->GetView()[EntityId].Authority)
	{
		AuthorityGained(EntityId, ComponentId);
	}
}

void MulticastRPCService::EntityRefresh(Worker_EntityId EntityId)
{
	if (SubView->HasAuthority(EntityId, SpatialConstants::SERVER_AUTH_COMPONENT_SET_ID))
	{
		const MulticastRPCs& Component = MulticastDataStore[EntityId];

		// Update last seen and last sent ids to latest component data
		LastSeenMulticastRPCIds.Add(EntityId, Component.MulticastRPCBuffer.LastSentRPCId);
		RPCStore->LastSentRPCIds.Add(EntityRPCType(EntityId, ERPCType::NetMulticast), Component.MulticastRPCBuffer.LastSentRPCId);
	}

	// If this is a non-auth refresh, process any new RPC updates. This is a no-op for the auth worker.
	ExtractRPCs(EntityId);
}

void MulticastRPCService::ComponentUpdate(const Worker_EntityId EntityId, const Worker_ComponentId ComponentId)
{
	if (ComponentId != SpatialConstants::MULTICAST_RPCS_COMPONENT_ID)
	{
		return;
	}
	ExtractRPCs(EntityId);
}

void MulticastRPCService::AuthorityGained(const Worker_EntityId EntityId, const Worker_ComponentSetId ComponentSetId)
{
	if (ComponentSetId != SpatialConstants::SERVER_AUTH_COMPONENT_SET_ID)
	{
		return;
	}
	OnEndpointAuthorityGained(EntityId, ComponentSetId);
}

void MulticastRPCService::AuthorityLost(const Worker_EntityId EntityId, const Worker_ComponentSetId ComponentSetId)
{
	if (ComponentSetId != SpatialConstants::SERVER_AUTH_COMPONENT_SET_ID)
	{
		return;
	}
	OnEndpointAuthorityLost(EntityId, ComponentSetId);
}

void MulticastRPCService::PopulateDataStore(const Worker_EntityId EntityId)
{
	MulticastDataStore.Emplace(
		EntityId, MulticastRPCs(SubView->GetView()[EntityId]
									.Components.FindByPredicate(ComponentIdEquality{ SpatialConstants::MULTICAST_RPCS_COMPONENT_ID })
									->GetUnderlying()));
}

void MulticastRPCService::ApplyComponentUpdate(const Worker_EntityId EntityId, Schema_ComponentUpdate* Update)
{
	MulticastDataStore[EntityId].ApplyComponentUpdate(Update);
}

void MulticastRPCService::OnCheckoutMulticastRPCComponentOnEntity(const Worker_EntityId EntityId)
{
	const MulticastRPCs& Component = MulticastDataStore[EntityId];

	// When checking out entity, ignore multicast RPCs that are already on the component.
	LastSeenMulticastRPCIds.Add(EntityId, Component.MulticastRPCBuffer.LastSentRPCId);
}

void MulticastRPCService::OnRemoveMulticastRPCComponentForEntity(const Worker_EntityId EntityId)
{
	LastSeenMulticastRPCIds.Remove(EntityId);
}

void MulticastRPCService::OnEndpointAuthorityGained(const Worker_EntityId EntityId, const Worker_ComponentSetId ComponentSetId)
{
	const MulticastRPCs& Component = MulticastDataStore[EntityId];

	if (Component.MulticastRPCBuffer.LastSentRPCId == 0 && Component.InitiallyPresentMulticastRPCsCount > 0)
	{
		// Update last sent ID to the number of initially present RPCs so the clients who check out this entity
		// as it's created can process the initial multicast RPCs.
		RPCStore->LastSentRPCIds.Add(EntityRPCType(EntityId, ERPCType::NetMulticast), Component.InitiallyPresentMulticastRPCsCount);

		const RPCRingBufferDescriptor Descriptor = RPCRingBufferUtils::GetRingBufferDescriptor(ERPCType::NetMulticast);
		Schema_Object* SchemaObject = Schema_GetComponentUpdateFields(
			RPCStore->GetOrCreateComponentUpdate(EntityComponentId{ EntityId, SpatialConstants::MULTICAST_RPCS_COMPONENT_ID }));
		Schema_AddUint64(SchemaObject, Descriptor.LastSentRPCFieldId, Component.InitiallyPresentMulticastRPCsCount);
	}
	else
	{
		RPCStore->LastSentRPCIds.Add(EntityRPCType(EntityId, ERPCType::NetMulticast), Component.MulticastRPCBuffer.LastSentRPCId);
	}
}

void MulticastRPCService::OnEndpointAuthorityLost(const Worker_EntityId EntityId, const Worker_ComponentSetId ComponentSetId)
{
	// Set last seen to last sent, so we don't process own RPCs after crossing the boundary.
	LastSeenMulticastRPCIds.Add(EntityId, RPCStore->LastSentRPCIds[EntityRPCType(EntityId, ERPCType::NetMulticast)]);
	RPCStore->LastSentRPCIds.Remove(EntityRPCType(EntityId, ERPCType::NetMulticast));
}

void MulticastRPCService::ExtractRPCs(const Worker_EntityId EntityId)
{
	if (!LastSeenMulticastRPCIds.Contains(EntityId))
	{
		UE_LOG(LogMulticastRPCService, Warning,
			   TEXT("Tried to extract RPCs but no entry in Last Seen Map! This can happen after server travel. Entity: %lld, type: "
					"Multicast"),
			   EntityId);
		return;
	}
	const uint64 LastSeenRPCId = LastSeenMulticastRPCIds[EntityId];

	const RPCRingBuffer& Buffer = MulticastDataStore[EntityId].MulticastRPCBuffer;

	uint64 LastProcessedRPCId = LastSeenRPCId;
	if (Buffer.LastSentRPCId >= LastSeenRPCId)
	{
		uint64 FirstRPCIdToRead = LastSeenRPCId + 1;

		const uint32 BufferSize = RPCRingBufferUtils::GetRingBufferSize(ERPCType::NetMulticast);
		if (Buffer.LastSentRPCId > LastSeenRPCId + BufferSize)
		{
			UE_LOG(
				LogMulticastRPCService, Warning,
				TEXT("MulticastRPCService::ExtractRPCsForType: RPCs were overwritten without being processed! Entity: %lld, RPC type: %s, "
					 "last seen RPC ID: %d, last sent ID: %d, buffer size: %d"),
				EntityId, *SpatialConstants::RPCTypeToString(ERPCType::NetMulticast), LastSeenRPCId, Buffer.LastSentRPCId, BufferSize);
			FirstRPCIdToRead = Buffer.LastSentRPCId - BufferSize + 1;
		}

		for (uint64 RPCId = FirstRPCIdToRead; RPCId <= Buffer.LastSentRPCId; RPCId++)
		{
			const TOptional<RPCPayload>& Element = Buffer.GetRingBufferElement(RPCId);
			if (Element.IsSet())
			{
				ExtractRPCCallback.Execute(FUnrealObjectRef(EntityId, Element.GetValue().Offset), RPCSender(), Element.GetValue(), RPCId);
				LastProcessedRPCId = RPCId;
			}
			else
			{
				UE_LOG(LogMulticastRPCService, Warning,
					   TEXT("MulticastRPCService::ExtractRPCsForType: Ring buffer element empty. Entity: %lld, RPC type: %s, empty element "
							"RPC id: %d"),
					   EntityId, *SpatialConstants::RPCTypeToString(ERPCType::NetMulticast), RPCId);
			}
		}
	}
	else
	{
		UE_LOG(LogMulticastRPCService, Warning,
			   TEXT("MulticastRPCService::ExtractRPCsForType: Last sent RPC has smaller ID than last seen RPC. Entity: %lld, RPC type: %s, "
					"last sent ID: %d, last seen ID: %d"),
			   EntityId, *SpatialConstants::RPCTypeToString(ERPCType::NetMulticast), Buffer.LastSentRPCId, LastSeenRPCId);
	}

	if (LastProcessedRPCId > LastSeenRPCId)
	{
		LastSeenMulticastRPCIds[EntityId] = LastProcessedRPCId;
	}
}
} // namespace SpatialGDK
