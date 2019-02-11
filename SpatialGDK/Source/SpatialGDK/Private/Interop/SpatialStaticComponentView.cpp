// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "Interop/SpatialStaticComponentView.h"

#include "Schema/Component.h"
#include "Schema/Heartbeat.h"
#include "Schema/Interest.h"
#include "Schema/Singleton.h"
#include "Schema/SpawnData.h"

Worker_Authority USpatialStaticComponentView::GetAuthority(Worker_EntityId EntityId, Worker_ComponentId ComponentId)
{
	if (TMap<Worker_ComponentId, Worker_Authority>* ComponentAuthorityMap = EntityComponentAuthorityMap.Find(EntityId))
	{
		if (Worker_Authority* Authority = ComponentAuthorityMap->Find(ComponentId))
		{
			return *Authority;
		}
	}

	return WORKER_AUTHORITY_NOT_AUTHORITATIVE;
}

// TODO UNR-640 - Need to fix for authority loss imminent
bool USpatialStaticComponentView::HasAuthority(Worker_EntityId EntityId, Worker_ComponentId ComponentId)
{
	return GetAuthority(EntityId, ComponentId) == WORKER_AUTHORITY_AUTHORITATIVE;
}

void USpatialStaticComponentView::OnAddComponent(const Worker_AddComponentOp& Op)
{
	TUniquePtr<improbable::ComponentStorageBase> Data;
	switch (Op.data.component_id)
	{
	case SpatialConstants::ENTITY_ACL_COMPONENT_ID:
		Data = MakeUnique<improbable::ComponentStorage<improbable::EntityAcl>>(Op.data);
		break;
	case SpatialConstants::METADATA_COMPONENT_ID:
		Data = MakeUnique<improbable::ComponentStorage<improbable::Metadata>>(Op.data);
		break;
	case SpatialConstants::POSITION_COMPONENT_ID:
		Data = MakeUnique<improbable::ComponentStorage<improbable::Position>>(Op.data);
		break;
	case SpatialConstants::PERSISTENCE_COMPONENT_ID:
		Data = MakeUnique<improbable::ComponentStorage<improbable::Persistence>>(Op.data);
		break;
	case SpatialConstants::SPAWN_DATA_COMPONENT_ID:
		Data = MakeUnique<improbable::ComponentStorage<improbable::SpawnData>>(Op.data);
		break;
	case SpatialConstants::SINGLETON_COMPONENT_ID:
		Data = MakeUnique<improbable::ComponentStorage<improbable::Singleton>>(Op.data);
		break;
	case SpatialConstants::UNREAL_METADATA_COMPONENT_ID:
		Data = MakeUnique<improbable::ComponentStorage<improbable::UnrealMetadata>>(Op.data);
		break;
	case SpatialConstants::INTEREST_COMPONENT_ID:
		Data = MakeUnique<improbable::ComponentStorage<improbable::Interest>>(Op.data);
		break;
	case SpatialConstants::HEARTBEAT_COMPONENT_ID:
		Data = MakeUnique<improbable::ComponentStorage<improbable::Heartbeat>>(Op.data);
		break;
	default:
		return;
	}

	EntityComponentMap.FindOrAdd(Op.entity_id).FindOrAdd(Op.data.component_id) = std::move(Data);
}

void USpatialStaticComponentView::OnRemoveEntity(Worker_EntityId EntityId)
{
	EntityComponentMap.Remove(EntityId);
}

void USpatialStaticComponentView::OnComponentUpdate(const Worker_ComponentUpdateOp& Op)
{
	improbable::Component* Component = nullptr;

	switch (Op.update.component_id)
	{
	case SpatialConstants::ENTITY_ACL_COMPONENT_ID:
		Component = GetComponentData<improbable::EntityAcl>(Op.entity_id);
		break;
	case SpatialConstants::POSITION_COMPONENT_ID:
		Component = GetComponentData<improbable::Position>(Op.entity_id);
		break;
	default:
		return;
	}

	if (Component) {
		Component->ApplyComponentUpdate(Op.update);
	}
}

void USpatialStaticComponentView::OnAuthorityChange(const Worker_AuthorityChangeOp& Op)
{
	EntityComponentAuthorityMap.FindOrAdd(Op.entity_id).FindOrAdd(Op.component_id) = (Worker_Authority)Op.authority;
}
