// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "EngineClasses/SpatialLoadBalanceEnforcer.h"
#include "EngineClasses/SpatialVirtualWorkerTranslator.h"
#include "Schema/AuthorityIntent.h"
#include "Schema/Component.h"
#include "Schema/ComponentPresence.h"
#include "Schema/NetOwningClientWorker.h"
#include "SpatialCommonTypes.h"
#include "SpatialGDKSettings.h"

DEFINE_LOG_CATEGORY(LogSpatialLoadBalanceEnforcer);

using namespace SpatialGDK;

SpatialLoadBalanceEnforcer::SpatialLoadBalanceEnforcer(const PhysicalWorkerName& InWorkerId,
													   const USpatialStaticComponentView* InStaticComponentView, const FSubView& InSubView,
													   const SpatialVirtualWorkerTranslator* InVirtualWorkerTranslator)
	: WorkerId(InWorkerId)
	, StaticComponentView(InStaticComponentView)
	, SubView(&InSubView)
	, VirtualWorkerTranslator(InVirtualWorkerTranslator)
{
	check(InStaticComponentView != nullptr);
	check(InVirtualWorkerTranslator != nullptr);
}

void SpatialLoadBalanceEnforcer::Advance()
{
	AclUpdates.Empty();

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
					CreateAclUpdate(Delta.EntityId);
				}
			}

			for (const ComponentChange& Change : Delta.ComponentsRefreshed)
			{
				if (HandlesComponent(Change.ComponentId))
				{
					CreateAclUpdate(Delta.EntityId);
				}
			}
		}
		break;
		case EntityDelta::ADD:
		{
			CreateAclUpdate(Delta.EntityId);
		}
		break;
		case EntityDelta::REMOVE:
			break;
		case EntityDelta::TEMPORARILY_REMOVED:
		{
			CreateAclUpdate(Delta.EntityId);
		}
		break;
		default:
			break;
		}
	}
}

TArray<EntityComponentUpdate> SpatialLoadBalanceEnforcer::GetAndClearAclUpdates()
{
	TArray<EntityComponentUpdate> Temp = MoveTemp(AclUpdates);
	AclUpdates.Empty();
	return Temp;
}

void SpatialLoadBalanceEnforcer::MaybeCreateAclUpdate(const Worker_EntityId EntityId)
{
	if (StaticComponentView->HasAuthority(EntityId, SpatialConstants::ENTITY_ACL_COMPONENT_ID))
	{
		CreateAclUpdate(EntityId);
	}
}

void SpatialLoadBalanceEnforcer::TagQuery(Query& QueryToTag) const
{
	SubView->TagQuery(QueryToTag);
}

void SpatialLoadBalanceEnforcer::TagEntity(TArray<FWorkerComponentData>& Components) const
{
	SubView->TagEntity(Components);
}

void SpatialLoadBalanceEnforcer::CreateAclUpdate(const Worker_EntityId EntityId)
{
	const AuthorityIntent* AuthorityIntentComponent = StaticComponentView->GetComponentData<AuthorityIntent>(EntityId);
	const PhysicalWorkerName* OwningWorkerId =
		VirtualWorkerTranslator->GetPhysicalWorkerForVirtualWorker(AuthorityIntentComponent->VirtualWorkerId);

	check(OwningWorkerId != nullptr);
	if (OwningWorkerId == nullptr)
	{
		UE_LOG(LogSpatialLoadBalanceEnforcer, Error,
			   TEXT("Couldn't find mapped worker for entity %lld. This shouldn't happen! Virtual worker ID: %d"), EntityId,
			   AuthorityIntentComponent->VirtualWorkerId);
		return;
	}

	if (*OwningWorkerId == WorkerId && StaticComponentView->HasAuthority(EntityId, SpatialConstants::AUTHORITY_INTENT_COMPONENT_ID))
	{
		UE_LOG(
			LogSpatialLoadBalanceEnforcer, Verbose,
			TEXT("No need to process newly authoritative entity because this worker is already authoritative. Entity: %lld. Worker: %s."),
			EntityId, *WorkerId);
		return;
	}

	const NetOwningClientWorker* NetOwningClientWorkerComponent = StaticComponentView->GetComponentData<NetOwningClientWorker>(EntityId);

	const ComponentPresence* ComponentPresenceComponent = StaticComponentView->GetComponentData<ComponentPresence>(EntityId);

	if (AuthorityIntentComponent->VirtualWorkerId == SpatialConstants::INVALID_VIRTUAL_WORKER_ID)
	{
		UE_LOG(LogSpatialLoadBalanceEnforcer, Error,
			   TEXT("Entity with invalid virtual worker ID assignment will not be processed. EntityId: %lld. This should not happen - "
					"investigate if you see this."),
			   EntityId);
		return;
	}

	check(VirtualWorkerTranslator != nullptr);
	const PhysicalWorkerName* DestinationWorkerId =
		VirtualWorkerTranslator->GetPhysicalWorkerForVirtualWorker(AuthorityIntentComponent->VirtualWorkerId);
	if (DestinationWorkerId == nullptr)
	{
		UE_LOG(LogSpatialLoadBalanceEnforcer, Error,
			   TEXT("This worker is not assigned a virtual worker. This shouldn't happen! Worker: %s"), *WorkerId);
		return;
	}

	TArray<Worker_ComponentId> ComponentIds;

	EntityAcl* Acl = StaticComponentView->GetComponentData<EntityAcl>(EntityId);
	Acl->ComponentWriteAcl.GetKeys(ComponentIds);

	// Ensure that every component ID in ComponentPresence is set in the write ACL.
	for (const auto& RequiredComponentId : ComponentPresenceComponent->ComponentList)
	{
		ComponentIds.AddUnique(RequiredComponentId);
	}

	// Get the client worker ID net-owning this Actor from the NetOwningClientWorker.
	PhysicalWorkerName PossessingClientId =
		NetOwningClientWorkerComponent->WorkerId.IsSet() ? NetOwningClientWorkerComponent->WorkerId.GetValue() : FString();

	const FString& WriteWorkerId = FString::Printf(TEXT("workerId:%s"), **OwningWorkerId);

	const WorkerAttributeSet OwningServerWorkerAttributeSet = { WriteWorkerId };

	EntityAcl* NewAcl = StaticComponentView->GetComponentData<EntityAcl>(EntityId);
	NewAcl->ReadAcl = Acl->ReadAcl;

	for (const Worker_ComponentId& ComponentId : ComponentIds)
	{
		if (ComponentId == SpatialConstants::HEARTBEAT_COMPONENT_ID
			|| ComponentId == SpatialConstants::GetClientAuthorityComponent(GetDefault<USpatialGDKSettings>()->UseRPCRingBuffer()))
		{
			NewAcl->ComponentWriteAcl.Add(ComponentId, { { PossessingClientId } });
			continue;
		}

		if (ComponentId == SpatialConstants::ENTITY_ACL_COMPONENT_ID)
		{
			NewAcl->ComponentWriteAcl.Add(ComponentId, { SpatialConstants::UnrealServerAttributeSet });
			continue;
		}

		NewAcl->ComponentWriteAcl.Add(ComponentId, { OwningServerWorkerAttributeSet });
	}
	FWorkerComponentUpdate Update = NewAcl->CreateEntityAclUpdate();
	AclUpdates.Emplace(
		EntityComponentUpdate{ EntityId, ComponentUpdate(OwningComponentUpdatePtr(Update.schema_type), Update.component_id) });
}

bool SpatialLoadBalanceEnforcer::HandlesComponent(Worker_ComponentId ComponentId)
{
	return ComponentId == SpatialConstants::AUTHORITY_INTENT_COMPONENT_ID
		   || ComponentId == SpatialConstants::COMPONENT_PRESENCE_COMPONENT_ID
		   || ComponentId == SpatialConstants::NET_OWNING_CLIENT_WORKER_COMPONENT_ID;
}
