// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "Interop/SpatialStaticComponentView.h"

#include "Schema/AuthorityIntent.h"
#include "Schema/ClientEndpoint.h"
#include "Schema/ClientRPCEndpointLegacy.h"
#include "Schema/Component.h"
#include "Schema/Heartbeat.h"
#include "Schema/Interest.h"
#include "Schema/MulticastRPCs.h"
#include "Schema/RPCPayload.h"
#include "Schema/ServerEndpoint.h"
#include "Schema/ServerRPCEndpointLegacy.h"
#include "Schema/Singleton.h"
#include "Schema/SpawnData.h"

Worker_Authority USpatialStaticComponentView::GetAuthority(Worker_EntityId EntityId, Worker_ComponentId ComponentId) const
{
	if (const TMap<Worker_ComponentId, Worker_Authority>* ComponentAuthorityMap = EntityComponentAuthorityMap.Find(EntityId))
	{
		if (const Worker_Authority* Authority = ComponentAuthorityMap->Find(ComponentId))
		{
			return *Authority;
		}
	}

	return WORKER_AUTHORITY_NOT_AUTHORITATIVE;
}

// TODO UNR-640 - Need to fix for authority loss imminent
bool USpatialStaticComponentView::HasAuthority(Worker_EntityId EntityId, Worker_ComponentId ComponentId) const
{
	return GetAuthority(EntityId, ComponentId) == WORKER_AUTHORITY_AUTHORITATIVE;
}

bool USpatialStaticComponentView::HasComponent(Worker_EntityId EntityId, Worker_ComponentId ComponentId) const
{
	if (auto* EntityComponentStorage = EntityComponentMap.Find(EntityId))
	{
		return EntityComponentStorage->Contains(ComponentId);
	}

	return false;
}

void USpatialStaticComponentView::OnAddComponent(const Worker_AddComponentOp& Op)
{
	TUniquePtr<SpatialGDK::ComponentStorageBase> Data;
	FWorkerComponentData OpData = { Op.data };
	switch (Op.data.component_id)
	{
	case SpatialConstants::ENTITY_ACL_COMPONENT_ID:
		Data = MakeUnique<SpatialGDK::ComponentStorage<SpatialGDK::EntityAcl>>(OpData);
		break;
	case SpatialConstants::METADATA_COMPONENT_ID:
		Data = MakeUnique<SpatialGDK::ComponentStorage<SpatialGDK::Metadata>>(OpData);
		break;
	case SpatialConstants::POSITION_COMPONENT_ID:
		Data = MakeUnique<SpatialGDK::ComponentStorage<SpatialGDK::Position>>(OpData);
		break;
	case SpatialConstants::PERSISTENCE_COMPONENT_ID:
		Data = MakeUnique<SpatialGDK::ComponentStorage<SpatialGDK::Persistence>>(OpData);
		break;
	case SpatialConstants::SPAWN_DATA_COMPONENT_ID:
		Data = MakeUnique<SpatialGDK::ComponentStorage<SpatialGDK::SpawnData>>(OpData);
		break;
	case SpatialConstants::SINGLETON_COMPONENT_ID:
		Data = MakeUnique<SpatialGDK::ComponentStorage<SpatialGDK::Singleton>>(OpData);
		break;
	case SpatialConstants::UNREAL_METADATA_COMPONENT_ID:
		Data = MakeUnique<SpatialGDK::ComponentStorage<SpatialGDK::UnrealMetadata>>(OpData);
		break;
	case SpatialConstants::INTEREST_COMPONENT_ID:
		Data = MakeUnique<SpatialGDK::ComponentStorage<SpatialGDK::Interest>>(OpData);
		break;
	case SpatialConstants::HEARTBEAT_COMPONENT_ID:
		Data = MakeUnique<SpatialGDK::ComponentStorage<SpatialGDK::Heartbeat>>(OpData);
		break;
	case SpatialConstants::RPCS_ON_ENTITY_CREATION_ID:
		Data = MakeUnique<SpatialGDK::ComponentStorage<SpatialGDK::RPCsOnEntityCreation>>(OpData);
		break;
	case SpatialConstants::CLIENT_RPC_ENDPOINT_COMPONENT_ID_LEGACY:
		Data = MakeUnique<SpatialGDK::ComponentStorage<SpatialGDK::ClientRPCEndpointLegacy>>(OpData);
		break;
	case SpatialConstants::SERVER_RPC_ENDPOINT_COMPONENT_ID_LEGACY:
		Data = MakeUnique<SpatialGDK::ComponentStorage<SpatialGDK::ServerRPCEndpointLegacy>>(OpData);
		break;
	case SpatialConstants::AUTHORITY_INTENT_COMPONENT_ID:
		Data = MakeUnique<SpatialGDK::ComponentStorage<SpatialGDK::AuthorityIntent>>(OpData);
		break;
	case SpatialConstants::CLIENT_ENDPOINT_COMPONENT_ID:
		Data = MakeUnique<SpatialGDK::ComponentStorage<SpatialGDK::ClientEndpoint>>(OpData);
		break;
	case SpatialConstants::SERVER_ENDPOINT_COMPONENT_ID:
		Data = MakeUnique<SpatialGDK::ComponentStorage<SpatialGDK::ServerEndpoint>>(OpData);
		break;
	case SpatialConstants::MULTICAST_RPCS_COMPONENT_ID:
		Data = MakeUnique<SpatialGDK::ComponentStorage<SpatialGDK::MulticastRPCs>>(OpData);
		break;
	default:
		// Component is not hand written, but we still want to know the existence of it on this entity.
		Data = nullptr;
	}
	EntityComponentMap.FindOrAdd(Op.entity_id).FindOrAdd(Op.data.component_id) = std::move(Data);
}

void USpatialStaticComponentView::OnRemoveComponent(const Worker_RemoveComponentOp& Op)
{
	if (auto* ComponentMap = EntityComponentMap.Find(Op.entity_id))
	{
		ComponentMap->Remove(Op.component_id);
	}
}

void USpatialStaticComponentView::OnRemoveEntity(Worker_EntityId EntityId)
{
	EntityComponentMap.Remove(EntityId);
	EntityComponentAuthorityMap.Remove(EntityId);
}

void USpatialStaticComponentView::OnComponentUpdate(const Worker_ComponentUpdateOp& Op)
{
	SpatialGDK::Component* Component = nullptr;

	switch (Op.update.component_id)
	{
	case SpatialConstants::ENTITY_ACL_COMPONENT_ID:
		Component = GetComponentData<SpatialGDK::EntityAcl>(Op.entity_id);
		break;
	case SpatialConstants::POSITION_COMPONENT_ID:
		Component = GetComponentData<SpatialGDK::Position>(Op.entity_id);
		break;
	case SpatialConstants::CLIENT_RPC_ENDPOINT_COMPONENT_ID_LEGACY:
		Component = GetComponentData<SpatialGDK::ClientRPCEndpointLegacy>(Op.entity_id);
		break;
	case SpatialConstants::SERVER_RPC_ENDPOINT_COMPONENT_ID_LEGACY:
		Component = GetComponentData<SpatialGDK::ServerRPCEndpointLegacy>(Op.entity_id);
		break;
	case SpatialConstants::AUTHORITY_INTENT_COMPONENT_ID:
		Component = GetComponentData<SpatialGDK::AuthorityIntent>(Op.entity_id);
		break;
	case SpatialConstants::CLIENT_ENDPOINT_COMPONENT_ID:
		Component = GetComponentData<SpatialGDK::ClientEndpoint>(Op.entity_id);
		break;
	case SpatialConstants::SERVER_ENDPOINT_COMPONENT_ID:
		Component = GetComponentData<SpatialGDK::ServerEndpoint>(Op.entity_id);
		break;
	case SpatialConstants::MULTICAST_RPCS_COMPONENT_ID:
		Component = GetComponentData<SpatialGDK::MulticastRPCs>(Op.entity_id);
		break;
	default:
		return;
	}

	if (Component)
	{
		Component->ApplyComponentUpdate(FWorkerComponentUpdate{ Op.update });
	}
}

void USpatialStaticComponentView::OnAuthorityChange(const Worker_AuthorityChangeOp& Op)
{
	EntityComponentAuthorityMap.FindOrAdd(Op.entity_id).FindOrAdd(Op.component_id) = (Worker_Authority)Op.authority;
}
