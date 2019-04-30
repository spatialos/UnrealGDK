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
	TUniquePtr<SpatialGDK::ComponentStorageBase> Data;
	switch (Op.data.component_id)
	{
	case SpatialConstants::ENTITY_ACL_COMPONENT_ID:
		Data = MakeUnique<SpatialGDK::ComponentStorage<SpatialGDK::EntityAcl>>(Op.data);
		break;
	case SpatialConstants::METADATA_COMPONENT_ID:
		Data = MakeUnique<SpatialGDK::ComponentStorage<SpatialGDK::Metadata>>(Op.data);
		break;
	case SpatialConstants::POSITION_COMPONENT_ID:
		Data = MakeUnique<SpatialGDK::ComponentStorage<SpatialGDK::Position>>(Op.data);
		break;
	case SpatialConstants::PERSISTENCE_COMPONENT_ID:
		Data = MakeUnique<SpatialGDK::ComponentStorage<SpatialGDK::Persistence>>(Op.data);
		break;
	case SpatialConstants::SPAWN_DATA_COMPONENT_ID:
		Data = MakeUnique<SpatialGDK::ComponentStorage<SpatialGDK::SpawnData>>(Op.data);
		break;
	case SpatialConstants::SINGLETON_COMPONENT_ID:
		Data = MakeUnique<SpatialGDK::ComponentStorage<SpatialGDK::Singleton>>(Op.data);
		break;
	case SpatialConstants::UNREAL_METADATA_COMPONENT_ID:
		Data = MakeUnique<SpatialGDK::ComponentStorage<SpatialGDK::UnrealMetadata>>(Op.data);
		break;
	case SpatialConstants::INTEREST_COMPONENT_ID:
		Data = MakeUnique<SpatialGDK::ComponentStorage<SpatialGDK::Interest>>(Op.data);
		break;
	case SpatialConstants::HEARTBEAT_COMPONENT_ID:
		Data = MakeUnique<SpatialGDK::ComponentStorage<SpatialGDK::Heartbeat>>(Op.data);
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
	SpatialGDK::Component* Component = nullptr;

	switch (Op.update.component_id)
	{
	case SpatialConstants::ENTITY_ACL_COMPONENT_ID:
		Component = GetComponentData<SpatialGDK::EntityAcl>(Op.entity_id);
		break;
	case SpatialConstants::POSITION_COMPONENT_ID:
		Component = GetComponentData<SpatialGDK::Position>(Op.entity_id);
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
