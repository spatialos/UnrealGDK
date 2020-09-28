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

using namespace SpatialGDK;

SpatialLoadBalanceEnforcer::SpatialLoadBalanceEnforcer(const PhysicalWorkerName& InWorkerId,
													   const USpatialStaticComponentView* InStaticComponentView, const FSubView& InSubView,
													   const SpatialVirtualWorkerTranslator* InVirtualWorkerTranslator,
													   TUniqueFunction<void(EntityComponentUpdate)> InUpdateSender)
	: WorkerId(InWorkerId)
	, StaticComponentView(InStaticComponentView)
	, SubView(&InSubView)
	, VirtualWorkerTranslator(InVirtualWorkerTranslator)
	, UpdateSender(MoveTemp(InUpdateSender))
{
	check(InStaticComponentView != nullptr);
	check(InVirtualWorkerTranslator != nullptr);
}

void SpatialLoadBalanceEnforcer::Advance()
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
				if (HandlesComponent(Change.ComponentId))
				{
					RefreshAcl(Delta.EntityId);
				}
			}

			for (const ComponentChange& Change : Delta.ComponentsRefreshed)
			{
				if (HandlesComponent(Change.ComponentId))
				{
					RefreshAcl(Delta.EntityId);
				}
			}

			for (const ComponentChange& Change : Delta.ComponentsAdded)
			{
				if (HandlesComponent(Change.ComponentId))
				{
					RefreshAcl(Delta.EntityId);
				}
			}

			for (const ComponentChange& Change : Delta.ComponentsRemoved)
			{
				if (HandlesComponent(Change.ComponentId))
				{
					RefreshAcl(Delta.EntityId);
				}
			}
			break;
		}
		case EntityDelta::ADD:
		{
			RefreshAcl(Delta.EntityId);
			break;
		}
		case EntityDelta::REMOVE:
			break;
		case EntityDelta::TEMPORARILY_REMOVED:
		{
			RefreshAcl(Delta.EntityId);
			break;
		}
		default:
			break;
		}
	}
}

void SpatialLoadBalanceEnforcer::ShortCircuitMaybeRefreshAcl(const Worker_EntityId EntityId)
{
	if (StaticComponentView->HasAuthority(EntityId, SpatialConstants::ENTITY_ACL_COMPONENT_ID))
	{
		RefreshAcl(EntityId);
	}
}

void SpatialLoadBalanceEnforcer::RefreshAcl(const Worker_EntityId EntityId)
{
	const AuthorityIntent* AuthorityIntentComponent = StaticComponentView->GetComponentData<AuthorityIntent>(EntityId);

	check(VirtualWorkerTranslator != nullptr);
	const PhysicalWorkerName* DestinationWorkerId =
		VirtualWorkerTranslator->GetPhysicalWorkerForVirtualWorker(AuthorityIntentComponent->VirtualWorkerId);

	check(DestinationWorkerId != nullptr);
	if (DestinationWorkerId == nullptr)
	{
		UE_LOG(LogSpatialLoadBalanceEnforcer, Error,
			   TEXT("Couldn't find mapped worker for entity %lld. This shouldn't happen! Virtual worker ID: %d"), EntityId,
			   AuthorityIntentComponent->VirtualWorkerId);
		return;
	}

	if (*DestinationWorkerId == WorkerId && StaticComponentView->HasAuthority(EntityId, SpatialConstants::AUTHORITY_INTENT_COMPONENT_ID))
	{
		UE_LOG(LogSpatialLoadBalanceEnforcer, Verbose,
			   TEXT("No need to process entity because this worker is already authoritative. Entity: %lld. Worker: %s."), EntityId,
			   *WorkerId);
		return;
	}

	UpdateSender(ConstructAclUpdate(EntityId, DestinationWorkerId));
}

EntityComponentUpdate SpatialLoadBalanceEnforcer::ConstructAclUpdate(const Worker_EntityId EntityId,
																	 const PhysicalWorkerName* DestinationWorkerId) const
{
	EntityAcl* Acl = StaticComponentView->GetComponentData<EntityAcl>(EntityId);
	const NetOwningClientWorker* NetOwningClientWorkerComponent = StaticComponentView->GetComponentData<NetOwningClientWorker>(EntityId);
	const ComponentPresence* ComponentPresenceComponent = StaticComponentView->GetComponentData<ComponentPresence>(EntityId);

	TArray<Worker_ComponentId> ComponentIds;
	Acl->ComponentWriteAcl.GetKeys(ComponentIds);

	// Ensure that every component ID in ComponentPresence is set in the write ACL.
	for (const auto& RequiredComponentId : ComponentPresenceComponent->ComponentList)
	{
		ComponentIds.AddUnique(RequiredComponentId);
	}

	// Get the client worker ID net-owning this Actor from the NetOwningClientWorker.
	const PhysicalWorkerName PossessingClientId =
		NetOwningClientWorkerComponent->WorkerId.IsSet() ? NetOwningClientWorkerComponent->WorkerId.GetValue() : FString();

	const FString& WriteWorkerId = FString::Printf(TEXT("workerId:%s"), **DestinationWorkerId);

	const WorkerAttributeSet OwningServerWorkerAttributeSet = { WriteWorkerId };

	for (const Worker_ComponentId& ComponentId : ComponentIds)
	{
		if (ComponentId == SpatialConstants::HEARTBEAT_COMPONENT_ID
			|| ComponentId == SpatialConstants::GetClientAuthorityComponent(GetDefault<USpatialGDKSettings>()->UseRPCRingBuffer()))
		{
			Acl->ComponentWriteAcl.Add(ComponentId, { { PossessingClientId } });
			continue;
		}

		if (ComponentId == SpatialConstants::ENTITY_ACL_COMPONENT_ID)
		{
			Acl->ComponentWriteAcl.Add(ComponentId, { SpatialConstants::UnrealServerAttributeSet });
			continue;
		}

		Acl->ComponentWriteAcl.Add(ComponentId, { OwningServerWorkerAttributeSet });
	}

	const FWorkerComponentUpdate Update = Acl->CreateEntityAclUpdate();
	return EntityComponentUpdate{ EntityId, ComponentUpdate(OwningComponentUpdatePtr(Update.schema_type), Update.component_id) };
}

bool SpatialLoadBalanceEnforcer::HandlesComponent(Worker_ComponentId ComponentId)
{
	return ComponentId == SpatialConstants::AUTHORITY_INTENT_COMPONENT_ID
		   || ComponentId == SpatialConstants::COMPONENT_PRESENCE_COMPONENT_ID
		   || ComponentId == SpatialConstants::NET_OWNING_CLIENT_WORKER_COMPONENT_ID;
}
