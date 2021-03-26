// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "EngineClasses/SpatialLoadBalanceEnforcer.h"

#include "EngineClasses/SpatialNetDriver.h"
#include "EngineClasses/SpatialPackageMapClient.h"
#include "EngineClasses/SpatialVirtualWorkerTranslator.h"
#include "Schema/AuthorityIntent.h"
#include "Schema/NetOwningClientWorker.h"
#include "SpatialCommonTypes.h"
#include "SpatialConstants.h"
#include "SpatialView/EntityDelta.h"
#include "SpatialView/SubView.h"
#include "SpatialView/ViewDelta.h"

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
			RefreshAuthority(Delta.EntityId);
		}
	}
}

void SpatialLoadBalanceEnforcer::ShortCircuitMaybeRefreshAuthorityDelegation(const Worker_EntityId EntityId)
{
	const EntityViewElement& Element = SubView->GetView()[EntityId];
	if (Element.Components.ContainsByPredicate(ComponentIdEquality{ SpatialConstants::LB_TAG_COMPONENT_ID }))
	{
		// Our entity will be out of date during a short circuit. Refresh the state here before refreshing the authority delegation.
		DataStore.Remove(EntityId);
		PopulateDataStore(EntityId);
		RefreshAuthority(EntityId);
	}
}

void SpatialLoadBalanceEnforcer::RefreshAuthority(const Worker_EntityId EntityId)
{
	const Worker_ComponentUpdate Update = CreateAuthorityDelegationUpdate(EntityId);

	UpdateSender(EntityComponentUpdate{ EntityId, ComponentUpdate(OwningComponentUpdatePtr(Update.schema_type), Update.component_id) });
}

Worker_ComponentUpdate SpatialLoadBalanceEnforcer::CreateAuthorityDelegationUpdate(const Worker_EntityId EntityId)
{
	LBComponents& Components = DataStore[EntityId];

	const Worker_PartitionId AuthoritativeServerPartition =
		VirtualWorkerTranslator->GetPartitionEntityForVirtualWorker(Components.Intent.VirtualWorkerId);

	const NetOwningClientWorker& NetOwningClientWorker = Components.OwningClientWorker;
	const Worker_PartitionId ClientWorkerPartitionId = NetOwningClientWorker.ClientPartitionId.IsSet()
														   ? NetOwningClientWorker.ClientPartitionId.GetValue()
														   : SpatialConstants::INVALID_PARTITION_ID;

	AuthorityDelegation& AuthorityDelegationComponent = Components.Delegation;
	AuthorityDelegationComponent.Delegations.Add(SpatialConstants::CLIENT_AUTH_COMPONENT_SET_ID, ClientWorkerPartitionId);
	AuthorityDelegationComponent.Delegations.Add(SpatialConstants::SERVER_AUTH_COMPONENT_SET_ID, AuthoritativeServerPartition);

	return AuthorityDelegationComponent.CreateAuthorityDelegationUpdate();
}

void SpatialLoadBalanceEnforcer::PopulateDataStore(const Worker_EntityId EntityId)
{
	LBComponents& Components = DataStore.Emplace(EntityId, LBComponents{});
	for (const ComponentData& Data : SubView->GetView()[EntityId].Components)
	{
		switch (Data.GetComponentId())
		{
		case SpatialConstants::AUTHORITY_DELEGATION_COMPONENT_ID:
			Components.Delegation = AuthorityDelegation(Data.GetUnderlying());
			break;
		case SpatialConstants::AUTHORITY_INTENT_COMPONENT_ID:
			Components.Intent = AuthorityIntent(Data.GetUnderlying());
			break;
		case SpatialConstants::NET_OWNING_CLIENT_WORKER_COMPONENT_ID:
			Components.OwningClientWorker = NetOwningClientWorker(Data.GetUnderlying());
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
	case SpatialConstants::AUTHORITY_DELEGATION_COMPONENT_ID:
		DataStore[EntityId].Delegation = AuthorityDelegation(Data);
		break;
	case SpatialConstants::AUTHORITY_INTENT_COMPONENT_ID:
		DataStore[EntityId].Intent = AuthorityIntent(Data);
		return true;
	case SpatialConstants::NET_OWNING_CLIENT_WORKER_COMPONENT_ID:
		DataStore[EntityId].OwningClientWorker = NetOwningClientWorker(Data);
		return true;
	default:
		break;
	}
	return false;
}

} // namespace SpatialGDK
