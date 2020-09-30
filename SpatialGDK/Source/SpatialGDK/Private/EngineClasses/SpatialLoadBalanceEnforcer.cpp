// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "EngineClasses/SpatialLoadBalanceEnforcer.h"

#include "EngineClasses/SpatialVirtualWorkerTranslator.h"
#include "Schema/AuthorityIntent.h"
#include "Schema/Component.h"
#include "Schema/ComponentPresence.h"
#include "Schema/NetOwningClientWorker.h"
#include "SpatialCommonTypes.h"
#include "SpatialConstants.h"
#include "SpatialGDKSettings.h"

DEFINE_LOG_CATEGORY(LogSpatialLoadBalanceEnforcer);

namespace SpatialGDK
{
SpatialLoadBalanceEnforcer::SpatialLoadBalanceEnforcer(const PhysicalWorkerName& InWorkerId, const FSubView& InSubView,
													   const SpatialVirtualWorkerTranslator* InVirtualWorkerTranslator,
													   TUniqueFunction<void(EntityComponentUpdate)> InUpdateSender)
	: WorkerId(InWorkerId)
	, SubView(&InSubView)
	, VirtualWorkerTranslator(InVirtualWorkerTranslator)
	, UpdateSender(MoveTemp(InUpdateSender))
{
	check(InVirtualWorkerTranslator != nullptr);
}

void SpatialLoadBalanceEnforcer::Advance()
{
	const FSubViewDelta& SubViewDelta = SubView->GetViewDelta();
	for (const EntityDelta& Delta : SubViewDelta.EntityDeltas)
	{
		bool bRefresh = false;
		switch (Delta.Type)
		{
		case EntityDelta::UPDATE:
		{
			for (const ComponentChange& Change : Delta.ComponentUpdates)
			{
				bRefresh |= ApplyComponentUpdate(Delta.EntityId, Change.ComponentId, Change.Update);
			}
			for (const ComponentChange& Change : Delta.ComponentsRefreshed)
			{
				bRefresh |= ApplyComponentRefresh(Delta.EntityId, Change.ComponentId, Change.CompleteUpdate.Data);
			}
			break;
		}
		case EntityDelta::ADD:
			PopulateDataStore(Delta.EntityId);
			bRefresh = true;
			break;
		case EntityDelta::REMOVE:
			DataStore.Remove(Delta.EntityId);
			break;
		case EntityDelta::TEMPORARILY_REMOVED:
			DataStore.Remove(Delta.EntityId);
			PopulateDataStore(Delta.EntityId);
			bRefresh = true;
			break;
		default:
			break;
		}

		if (bRefresh)
		{
			RefreshAcl(Delta.EntityId);
		}
	}
}

void SpatialLoadBalanceEnforcer::ShortCircuitMaybeRefreshAcl(const Worker_EntityId EntityId)
{
	const EntityViewElement& Element = SubView->GetView()[EntityId];
	if (Element.Components.ContainsByPredicate(ComponentIdEquality{ SpatialConstants::LB_TAG_COMPONENT_ID }))
	{
		// Our entity will be out of date during a short circuit. Refresh the state here before refreshing the ACL.
		DataStore.Remove(EntityId);
		PopulateDataStore(EntityId);
		RefreshAcl(EntityId);
	}
}

void SpatialLoadBalanceEnforcer::RefreshAcl(const Worker_EntityId EntityId)
{
	const AuthorityIntent& AuthorityIntentComponent = DataStore[EntityId].Intent;

	check(VirtualWorkerTranslator != nullptr);
	const PhysicalWorkerName* DestinationWorkerId =
		VirtualWorkerTranslator->GetPhysicalWorkerForVirtualWorker(AuthorityIntentComponent.VirtualWorkerId);

	check(DestinationWorkerId != nullptr);
	if (DestinationWorkerId == nullptr)
	{
		return;
	}

	UpdateSender(ConstructAclUpdate(EntityId, DestinationWorkerId));
}

EntityComponentUpdate SpatialLoadBalanceEnforcer::ConstructAclUpdate(const Worker_EntityId EntityId,
																	 const PhysicalWorkerName* DestinationWorkerId)
{
	LBComponents& Components = DataStore[EntityId];
	EntityAcl& Acl = Components.Acl;
	const ComponentPresence& ComponentPresenceComponent = Components.Presence;
	const NetOwningClientWorker& NetOwningClientWorkerComponent = Components.OwningClientWorker;

	TArray<Worker_ComponentId> ComponentIds;
	Acl.ComponentWriteAcl.GetKeys(ComponentIds);

	// Ensure that every component ID in ComponentPresence is set in the write ACL.
	for (const auto& RequiredComponentId : ComponentPresenceComponent.ComponentList)
	{
		ComponentIds.AddUnique(RequiredComponentId);
	}

	// Get the client worker ID net-owning this Actor from the NetOwningClientWorker.
	const PhysicalWorkerName PossessingClientId =
		NetOwningClientWorkerComponent.WorkerId.IsSet() ? NetOwningClientWorkerComponent.WorkerId.GetValue() : FString();

	const FString& WriteWorkerId = FString::Printf(TEXT("workerId:%s"), **DestinationWorkerId);

	const WorkerAttributeSet OwningServerWorkerAttributeSet = { WriteWorkerId };

	for (const Worker_ComponentId& ComponentId : ComponentIds)
	{
		switch (ComponentId)
		{
		case SpatialConstants::HEARTBEAT_COMPONENT_ID:
		case SpatialConstants::GetClientAuthorityComponent(GetDefault<USpatialGDKSettings>()->UseRPCRingBuffer()):
			Acl.ComponentWriteAcl.Add(ComponentId, { { PossessingClientId } });
			break;
		case SpatialConstants::ENTITY_ACL_COMPONENT_ID:
			Acl.ComponentWriteAcl.Add(ComponentId, { SpatialConstants::UnrealServerAttributeSet });
			break;
		default:
			Acl.ComponentWriteAcl.Add(ComponentId, { OwningServerWorkerAttributeSet });
			break;
		}
	}

	const FWorkerComponentUpdate Update = Acl.CreateEntityAclUpdate();
	return EntityComponentUpdate{ EntityId, ComponentUpdate(OwningComponentUpdatePtr(Update.schema_type), Update.component_id) };
}

void SpatialLoadBalanceEnforcer::PopulateDataStore(const Worker_EntityId EntityId)
{
	LBComponents& Components = DataStore.Emplace(EntityId, LBComponents{});
	for (auto& ComponentData : SubView->GetView()[EntityId].Components)
	{
		switch (ComponentData.GetComponentId())
		{
		case SpatialConstants::ENTITY_ACL_COMPONENT_ID:
			Components.Acl = EntityAcl(ComponentData.GetUnderlying());
			break;
		case SpatialConstants::AUTHORITY_INTENT_COMPONENT_ID:
			Components.Intent = AuthorityIntent(ComponentData.GetUnderlying());
			break;
		case SpatialConstants::COMPONENT_PRESENCE_COMPONENT_ID:
			Components.Presence = ComponentPresence(ComponentData.GetUnderlying());
			break;
		case SpatialConstants::NET_OWNING_CLIENT_WORKER_COMPONENT_ID:
			Components.OwningClientWorker = NetOwningClientWorker(ComponentData.GetUnderlying());
			break;
		default:
			break;
		}
	}
}

bool SpatialLoadBalanceEnforcer::ApplyComponentUpdate(const Worker_EntityId EntityId, const Worker_ComponentId ComponentId,
													  Schema_ComponentUpdate* Update)
{
	switch (ComponentId)
	{
	case SpatialConstants::AUTHORITY_INTENT_COMPONENT_ID:
		DataStore[EntityId].Intent.ApplyComponentUpdate(Update);
		return true;
	case SpatialConstants::COMPONENT_PRESENCE_COMPONENT_ID:
		DataStore[EntityId].Presence.ApplyComponentUpdate(Update);
		return true;
	case SpatialConstants::NET_OWNING_CLIENT_WORKER_COMPONENT_ID:
		DataStore[EntityId].OwningClientWorker.ApplyComponentUpdate(Update);
		return true;
	default:
		break;
	}
	return false;
}

bool SpatialLoadBalanceEnforcer::ApplyComponentRefresh(const Worker_EntityId EntityId, const Worker_ComponentId ComponentId,
													   Schema_ComponentData* Data)
{
	switch (ComponentId)
	{
	case SpatialConstants::ENTITY_ACL_COMPONENT_ID:
		DataStore[EntityId].Acl = EntityAcl(Data);
		break;
	case SpatialConstants::AUTHORITY_INTENT_COMPONENT_ID:
		DataStore[EntityId].Intent = AuthorityIntent(Data);
		return true;
	case SpatialConstants::COMPONENT_PRESENCE_COMPONENT_ID:
		DataStore[EntityId].Presence = ComponentPresence(Data);
		return true;
	case SpatialConstants::NET_OWNING_CLIENT_WORKER_COMPONENT_ID:
		DataStore[EntityId].OwningClientWorker = NetOwningClientWorker(Data);
		return true;
	default:
		break;
	}
	return false;
}
} // Namespace SpatialGDK
