// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "EngineClasses/SpatialLoadBalanceEnforcer.h"

#include "EngineClasses/SpatialNetDriver.h"
#include "EngineClasses/SpatialPackageMapClient.h"
#include "EngineClasses/SpatialVirtualWorkerTranslator.h"
#include "Schema/AuthorityDelegation.h"
#include "Schema/AuthorityIntent.h"
#include "Schema/Component.h"
#include "Schema/ComponentPresence.h"
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

// void SpatialLoadBalanceEnforcer::OnLoadBalancingComponentAdded(const Worker_AddComponentOp& Op)
// {
// 	check(HandlesComponent(Op.data.component_id));
//
// 	MaybeQueueAuthorityChange(Op.entity_id);
// }
//
// void SpatialLoadBalanceEnforcer::OnLoadBalancingComponentUpdated(const Worker_ComponentUpdateOp& Op)
// {
// 	check(HandlesComponent(Op.update.component_id));
//
// 	MaybeQueueAuthorityChange(Op.entity_id);
// }
//
// void SpatialLoadBalanceEnforcer::OnLoadBalancingComponentRemoved(const Worker_RemoveComponentOp& Op)
// {
// 	check(HandlesComponent(Op.component_id));
//
// 	if (AuthorityChangeRequestIsQueued(Op.entity_id))
// 	{
// 		UE_LOG(LogSpatialLoadBalanceEnforcer, Log,
// 			TEXT("Component %d for entity %lld removed. Can no longer enforce the previous request for this entity."),
// 			Op.component_id, Op.entity_id);
// 		PendingEntityAuthorityChanges.Remove(Op.entity_id);
// 	}
// }
//
// void SpatialLoadBalanceEnforcer::OnEntityRemoved(const Worker_RemoveEntityOp& Op)
// {
// 	if (AuthorityChangeRequestIsQueued(Op.entity_id))
// 	{
// 		UE_LOG(LogSpatialLoadBalanceEnforcer, Log, TEXT("Entity %lld removed. Can no longer enforce the previous request for this entity."),
// 			Op.entity_id);
// 		PendingEntityAuthorityChanges.Remove(Op.entity_id);
// 	}
// }
//
// void SpatialLoadBalanceEnforcer::OnAclAuthorityChanged(const Worker_AuthorityChangeOp& AuthOp)
// {
// 	// This class should only be informed of ACL authority changes.
// 	check(AuthOp.component_id == SpatialConstants::ENTITY_ACL_COMPONENT_ID);
//
// 	if (AuthOp.authority != WORKER_AUTHORITY_AUTHORITATIVE)
// 	{
// 		if (AuthorityChangeRequestIsQueued(AuthOp.entity_id))
// 		{
// 			UE_LOG(LogSpatialLoadBalanceEnforcer, Log,
// 				TEXT("ACL authority lost for entity %lld. Can no longer enforce the previous request for this entity."),
// 				AuthOp.entity_id);
// 			PendingEntityAuthorityChanges.Remove(AuthOp.entity_id);
// 		MaybeQueueAuthorityChange(AuthOp.entity_id);
// }

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

// MaybeQueueAuthorityChange can be updated from several places:
// 1) AuthorityIntent change - If intent changes, we should be updating the entity authoritative server worker.
// 2) Component additional / removal - If we trying to add a component, we need to update the authority for the new
//    component.
// 3) NetOwningClientWorker change - When Actor owner changes, this can result in becoming net owned by a client - if
//    this happens, we need to set client RPC endpoints (and maybe Heartbeat component) to client authoritative.
// 3) Load balancing component addition / change - If any of the above happen, but we don't have all the components
//    locally yet for enforcing an authority change, we can't do. So we reevaluate when we get everything we need (or
//    something is remotely updated).
// Queuing an authority change may not occur if the authority is the same as before, or if the request is already
// queued, or if we don't meet the predicate required to enforce the change.
// void SpatialLoadBalanceEnforcer::MaybeQueueAuthorityChange(const Worker_EntityId EntityId)
// 	if (!EntityNeedsToBeEnforced(EntityId))
// 	{
// 		return;
// 	}
//
// 	if (AuthorityChangeRequestIsQueued(EntityId))
// 	{
// 		UE_LOG(LogSpatialLoadBalanceEnforcer, Verbose, TEXT("Avoiding queueing a duplicate ACL assignment request. "
//             "Entity: %lld. Worker: %s."), EntityId, *WorkerId);
// 		return;
// 	}
//
// 	UE_LOG(LogSpatialLoadBalanceEnforcer, Log, TEXT("Queueing ACL assignment request. Entity %lld. Worker %s."), EntityId, *WorkerId);
// 	PendingEntityAuthorityChanges.Add(EntityId);
// }

void SpatialLoadBalanceEnforcer::ShortCircuitMaybeRefreshAcl(const Worker_EntityId EntityId)
{
	const EntityViewElement& Element = SubView->GetView()[EntityId];
	if (Element.Components.ContainsByPredicate(ComponentIdEquality{ SpatialConstants::LB_TAG_COMPONENT_ID }))
	{
		// Our entity will be out of date during a short circuit. Refresh the state here before refreshing the ACL.
		DataStore.Remove(EntityId);
		PopulateDataStore(EntityId);
		RefreshAuthority(EntityId);
	}
}

// bool SpatialLoadBalanceEnforcer::EntityNeedsToBeEnforced(const Worker_EntityId EntityId) const
// {
// 	bool AuthorityChangeRequired = false;
//
// 	const AuthorityIntent* AuthorityIntentComponent = StaticComponentView->GetComponentData<AuthorityIntent>(EntityId);
// 	check(AuthorityIntentComponent != nullptr);
//
// 	const ComponentPresence* ComponentPresenceList = StaticComponentView->GetComponentData<ComponentPresence>(EntityId);
// 	check(ComponentPresenceList != nullptr);
//
// 	const NetOwningClientWorker* OwningClientWorker = StaticComponentView->GetComponentData<NetOwningClientWorker>(EntityId);
// 	check(OwningClientWorker != nullptr);
//
// 	const Worker_ComponentId ClientRpcAuthComponent = SpatialConstants::GetClientAuthorityComponent(GetDefault<USpatialGDKSettings>()->UseRPCRingBuffer());
//
// 	TArray<Worker_ComponentId> AuthoritativeComponents;
//
// 	if (GetDefault<USpatialGDKSettings>()->bEnableUserSpaceLoadBalancing)
// 	{
// 		// Check to see if target authoritative server has changed.
// 		const AuthorityDelegation* DelegationMap = StaticComponentView->GetComponentData<AuthorityDelegation>(EntityId);
// 		const Worker_PartitionId* CurrentAuthoritativePartition = DelegationMap->Delegations.Find(SpatialConstants::AUTHORITY_INTENT_COMPONENT_ID);
// 		check(CurrentAuthoritativePartition != nullptr);
//
// 		const Worker_PartitionId TargetAuthoritativePartition = VirtualWorkerTranslator->GetPartitionEntityForVirtualWorker(AuthorityIntentComponent->VirtualWorkerId);
// 		checkf(TargetAuthoritativePartition != SpatialConstants::INVALID_ENTITY_ID, TEXT("Couldn't find mapped worker for entity %lld. Virtual worker ID: %d"),
//             EntityId, AuthorityIntentComponent->VirtualWorkerId);
//
// 		AuthorityChangeRequired |= *CurrentAuthoritativePartition != TargetAuthoritativePartition;
//
// 		// Check to see if component presence has changed.
// 		for (const auto& RequiredComponent : ComponentPresenceList->ComponentList)
// 		{
// 			AuthorityChangeRequired |= !DelegationMap->Delegations.Contains(RequiredComponent);
// 		}
//
// 		// Check to see if net owning client worker has changed.
// 		const Worker_PartitionId CurrentOwningClientPartition = *DelegationMap->Delegations.Find(ClientRpcAuthComponent);
// 		const Worker_PartitionId NewOwningClientPartition = OwningClientWorker->ClientPartitionId.IsSet() ?
// 			OwningClientWorker->ClientPartitionId.GetValue() : SpatialConstants::INVALID_ENTITY_ID;
// 		AuthorityChangeRequired |= CurrentOwningClientPartition != NewOwningClientPartition;
// 	}
// 	else
// 	{
// 		// Check to see if target authoritative server has changed.
// 		const EntityAcl* Acl = StaticComponentView->GetComponentData<EntityAcl>(EntityId);
// 		const WorkerRequirementSet* CurrentAuthoritativeWorker = Acl->ComponentWriteAcl.Find(SpatialConstants::AUTHORITY_INTENT_COMPONENT_ID);
// 		check(CurrentAuthoritativeWorker != nullptr);
// 		const PhysicalWorkerName* TargetAuthoritativeWorker = VirtualWorkerTranslator->GetPhysicalWorkerForVirtualWorker(AuthorityIntentComponent->VirtualWorkerId);
// 		checkf(TargetAuthoritativeWorker != nullptr, TEXT("Couldn't find mapped worker for entity %lld. Virtual worker ID: %d"),
// 			EntityId, AuthorityIntentComponent->VirtualWorkerId);
// 		AuthorityChangeRequired |= (*CurrentAuthoritativeWorker)[0][0] != *TargetAuthoritativeWorker;
//
// 		// Check to see if component presence has changed.
// 		for (const auto& RequiredComponent : ComponentPresenceList->ComponentList)
// 		{
// 			AuthorityChangeRequired |= !Acl->ComponentWriteAcl.Contains(RequiredComponent);
// 		}
//
// 		// Check to see if net owning client worker has changed.
//         const PhysicalWorkerName CurrentOwningClientWorkerId = (*Acl->ComponentWriteAcl.Find(ClientRpcAuthComponent))[0][0];
// 		const PhysicalWorkerName NewOwningClientWorkerId = OwningClientWorker->WorkerId.IsSet() ?
//             OwningClientWorker->WorkerId.GetValue() : FString();
// 		AuthorityChangeRequired |= CurrentOwningClientWorkerId != NewOwningClientWorkerId;
// 	}
//
// 	return AuthorityChangeRequired;
// }

// bool SpatialLoadBalanceEnforcer::AuthorityChangeRequestIsQueued(const Worker_EntityId EntityId) const
// {
// 	return PendingEntityAuthorityChanges.Contains(EntityId);
// }
//
// bool SpatialLoadBalanceEnforcer::GetAuthorityChangeState(Worker_EntityId EntityId, AuthorityStateChange& OutAuthorityChange) const
// {
// 	const AuthorityIntent* AuthorityIntentComponent = StaticComponentView->GetComponentData<AuthorityIntent>(EntityId);
// 	checkf(AuthorityIntentComponent != nullptr, TEXT("Can't process entity authority change. AuthorityIntent component has been removed since the request was queued. "
// 		"EntityId: %lld"), EntityId);
// 	checkf(AuthorityIntentComponent->VirtualWorkerId != SpatialConstants::INVALID_VIRTUAL_WORKER_ID,
// 		TEXT("Entity with invalid virtual worker ID assignment will not be processed. EntityId: %lld."), EntityId);
//
// 	const NetOwningClientWorker* NetOwningClientWorkerComponent = StaticComponentView->GetComponentData<NetOwningClientWorker>(EntityId);
// 	checkf(NetOwningClientWorkerComponent != nullptr, TEXT("Can't process entity authority change. NetOwningClientWorker component has been removed since the request was queued. "
// 		"EntityId: %lld"), EntityId);
//
// 	const ComponentPresence* ComponentPresenceComponent = StaticComponentView->GetComponentData<ComponentPresence>(EntityId);
// 	checkf(ComponentPresenceComponent != nullptr, TEXT("Can't process entity authority change. ComponentPresence component has been removed since the request was queued. "
// 		"EntityId: %lld"), EntityId);
//
// 	const EntityAcl* Acl = StaticComponentView->GetComponentData<EntityAcl>(EntityId);
// 	checkf(Acl != nullptr, TEXT("Can't process entity authority change. EntityAcl component has been removed since the request was queued. "
// 		"EntityId: %lld"), EntityId);
//
// 	TArray<Worker_ComponentId> ComponentIds;
//
// 	// With USLB enabled, we get the entity component list from the AuthorityDelegation component.
// 	// With USLB disabled, it's the EntityACL component.
// 	if (GetDefault<USpatialGDKSettings>()->bEnableUserSpaceLoadBalancing)
// 	{
// 		const AuthorityDelegation* AuthDelegation = StaticComponentView->GetComponentData<AuthorityDelegation>(EntityId);
// 		checkf(AuthDelegation != nullptr, TEXT("Can't process entity authority change. AuthorityDelegation component was removed while the request was queued. "
// 			"EntityId: %lld"), EntityId);
// 		AuthDelegation->Delegations.GetKeys(ComponentIds);
// 	}
// 	else
// 	{
// 		Acl->ComponentWriteAcl.GetKeys(ComponentIds);
// 	}
//
// 	// Ensure that every component ID in ComponentPresence will be assigned authority.
// 	for (const auto& RequiredComponentId : ComponentPresenceComponent->ComponentList)
// 	{
// 		ComponentIds.AddUnique(RequiredComponentId);
// 	}
//
// 	OutAuthorityChange = AuthorityStateChange{
// 		EntityId,
// 		Acl->ReadAcl,
// 		ComponentIds,
// 		AuthorityIntentComponent->VirtualWorkerId
// 	};
//
// 	return true;
// }

// TArray<SpatialLoadBalanceEnforcer::AuthorityStateChange> SpatialLoadBalanceEnforcer::ProcessAuthorityChangeRequests()
// {
// 	TArray<AuthorityStateChange> AuthorityChangesToProcess;
// 	AuthorityChangesToProcess.Reserve(PendingEntityAuthorityChanges.Num());
//
// 	for (Worker_EntityId EntityId : PendingEntityAuthorityChanges)
// 	{
// 		AuthorityStateChange AuthorityChange{};
// 		const bool bIsAuthorityChangeNeeded = GetAuthorityChangeState(EntityId, AuthorityChange);
// 		if (bIsAuthorityChangeNeeded)
// 		{
// 			AuthorityChangesToProcess.Emplace(AuthorityChange);
// 		}
// 	}
//
// 	PendingEntityAuthorityChanges.Empty();
//
// 	return AuthorityChangesToProcess;
// }

void SpatialLoadBalanceEnforcer::RefreshAuthority(const Worker_EntityId EntityId)
{
	const AuthorityIntent& AuthorityIntentComponent = DataStore[EntityId].Intent;

	check(VirtualWorkerTranslator != nullptr);
	const PhysicalWorkerName* DestinationWorkerId =
        VirtualWorkerTranslator->GetPhysicalWorkerForVirtualWorker(AuthorityIntentComponent.VirtualWorkerId);

	LBComponents& Components = DataStore[EntityId];
	EntityAcl& Acl = Components.Acl;
	const ComponentPresence& ComponentPresenceComponent = Components.Presence;
	const NetOwningClientWorker& NetOwningClientWorkerComponent = Components.OwningClientWorker;

	TArray<Worker_ComponentId> ComponentIds;
	Acl.ComponentWriteAcl.GetKeys(ComponentIds);

	// Ensure that every component ID in ComponentPresence is set in the write ACL.
	for (const Worker_ComponentId RequiredComponentId : ComponentPresenceComponent.ComponentList)
	{
		ComponentIds.AddUnique(RequiredComponentId);
	}

	// Get the client worker ID net-owning this Actor from the NetOwningClientWorker.
	const PhysicalWorkerName PossessingClientId =
		NetOwningClientWorkerComponent.WorkerId.IsSet() ? NetOwningClientWorkerComponent.WorkerId.GetValue() : FString();

	const FString& WriteWorkerId = FString::Printf(TEXT("workerId:%s"), **DestinationWorkerId);

	const WorkerAttributeSet OwningServerWorkerAttributeSet = { WriteWorkerId };

	for (const Worker_ComponentId ComponentId : ComponentIds)
	{
		switch (ComponentId)
		{
		case SpatialConstants::HEARTBEAT_COMPONENT_ID:
		case SpatialConstants::CLIENT_RPC_ENDPOINT_COMPONENT_ID_LEGACY:
		case SpatialConstants::CLIENT_ENDPOINT_COMPONENT_ID:
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
	UpdateSender(EntityComponentUpdate{ EntityId, ComponentUpdate(OwningComponentUpdatePtr(Update.schema_type), Update.component_id) });

}

void USpatialSender::SendAuthorityDelegationUpdate(const SpatialLoadBalanceEnforcer::AuthorityStateChange& Request) const
{
	check(Request.EntityId != SpatialConstants::INVALID_ENTITY_ID);
	check(Request.TargetVirtualWorker != SpatialConstants::INVALID_VIRTUAL_WORKER_ID);
	check(NetDriver->StaticComponentView->HasAuthority(Request.EntityId, SpatialConstants::AUTHORITY_DELEGATION_COMPONENT_ID));

	const Worker_PartitionId AuthoritativeServerPartition = NetDriver->VirtualWorkerTranslator->GetPartitionEntityForVirtualWorker(Request.TargetVirtualWorker);

	// There's a case where this function is called after a Tombstone component is added to the entity (when a startup Actor is deleted).
	// In this case, we won't be able to access the Actor* via the PackageMap (since the Actor was removed when the Tombstone was added).
	// Currently, we use the Actor* to grab the client worker partition ID that should be authoritative over the Heartbeat and client
	// RPC endpoint components (if the entity has them). As a workaround, since the delegation of deleted Actors should be relevant,
	// we'll just leave the partition ID as invalid if the Actor* cannot be retrieved.
	const AActor* Actor = Cast<AActor>(PackageMap->GetObjectFromEntityId(Request.EntityId));
	const Worker_PartitionId ClientWorkerPartitionId = Actor != nullptr ? GetConnectionOwningPartitionId(Actor) : SpatialConstants::INVALID_ENTITY_ID;

	AuthorityDelegation* AuthorityDelegationComponent = StaticComponentView->GetComponentData<AuthorityDelegation>(Request.EntityId);
	check(AuthorityDelegationComponent != nullptr);

	const Worker_ComponentId ClientRpcAuthComponent = SpatialConstants::GetClientAuthorityComponent(GetDefault<USpatialGDKSettings>()->UseRPCRingBuffer());

	for (const Worker_ComponentId& ComponentId : Request.ComponentIds)
	{
		if (ComponentId == SpatialConstants::HEARTBEAT_COMPONENT_ID || ComponentId == ClientRpcAuthComponent)
		{
			AuthorityDelegationComponent->Delegations.FindOrAdd(ComponentId, ClientWorkerPartitionId);
			continue;
		}

		AuthorityDelegationComponent->Delegations.Add(ComponentId, AuthoritativeServerPartition);
	}

	UE_LOG(LogSpatialSender, Log, TEXT("(%s) Sending AuthorityDelegation update. Entity: %lld. Actor %s. Server partition: %lld."),
		*NetDriver->Connection->GetWorkerId(), Request.EntityId, *GetNameSafe(Actor), AuthoritativeServerPartition);

	FWorkerComponentUpdate Update = AuthorityDelegationComponent->CreateAuthorityDelegationUpdate();
	Connection->SendComponentUpdate(Request.EntityId, &Update);
}

void USpatialSender::SendEntityACLUpdate(const SpatialLoadBalanceEnforcer::AuthorityStateChange& Request) const
{
	check(StaticComponentView->HasComponent(Request.EntityId, SpatialConstants::ENTITY_ACL_COMPONENT_ID));

	// Get the server worker name mapped to the Actor's authoritative virtual worker.
	const PhysicalWorkerName* DestinationWorkerId = NetDriver->VirtualWorkerTranslator->GetPhysicalWorkerForVirtualWorker(Request.TargetVirtualWorker);
	check(DestinationWorkerId != nullptr)
	const FString& AuthoritativePhysicalWorkerId = FString::Printf(TEXT("workerId:%s"), **DestinationWorkerId);
	const WorkerAttributeSet OwningServerWorkerAttributeSet = { AuthoritativePhysicalWorkerId };

	// Get the client worker ID net-owning this Actor from the NetOwningClientWorker.
	const NetOwningClientWorker* NetOwningClientWorkerComponent = StaticComponentView->GetComponentData<NetOwningClientWorker>(Request.EntityId);
	check(NetOwningClientWorkerComponent != nullptr);
	const PhysicalWorkerName PossessingClientId = NetOwningClientWorkerComponent->WorkerId.IsSet() ?
        NetOwningClientWorkerComponent->WorkerId.GetValue() :
        FString();

	EntityAcl* NewAcl = StaticComponentView->GetComponentData<EntityAcl>(Request.EntityId);
	NewAcl->ReadAcl = Request.ReadAcl;

	const Worker_ComponentId ClientRpcAuthComponent = SpatialConstants::GetClientAuthorityComponent(GetDefault<USpatialGDKSettings>()->UseRPCRingBuffer());

	for (const Worker_ComponentId& ComponentId : Request.ComponentIds)
	{
		if (ComponentId == SpatialConstants::HEARTBEAT_COMPONENT_ID || ComponentId == ClientRpcAuthComponent)
		{
			NewAcl->ComponentWriteAcl.Add(ComponentId, { { PossessingClientId } });
			continue;
		}

		if (ComponentId == SpatialConstants::ENTITY_ACL_COMPONENT_ID)
		{
			NewAcl->ComponentWriteAcl.Add(ComponentId, { SpatialConstants::UnrealServerAttributeSet } );
			continue;
		}

		NewAcl->ComponentWriteAcl.Add(ComponentId, { OwningServerWorkerAttributeSet });
	}

	UE_LOG(LogSpatialLoadBalanceEnforcer, Verbose, TEXT("(%s) Setting Acl WriteAuth for entity %lld to %s"), *NetDriver->Connection->GetWorkerId(), Request.EntityId, **DestinationWorkerId);

	FWorkerComponentUpdate Update = NewAcl->CreateEntityAclUpdate();
	NetDriver->Connection->SendComponentUpdate(Request.EntityId, &Update);
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
		case SpatialConstants::ENTITY_ACL_COMPONENT_ID:
			Components.Acl = EntityAcl(Data.GetUnderlying());
			break;
		case SpatialConstants::AUTHORITY_INTENT_COMPONENT_ID:
			Components.Intent = AuthorityIntent(Data.GetUnderlying());
			break;
		case SpatialConstants::COMPONENT_PRESENCE_COMPONENT_ID:
			Components.Presence = ComponentPresence(Data.GetUnderlying());
			break;
		case SpatialConstants::NET_OWNING_CLIENT_WORKER_COMPONENT_ID:
			Components.OwningClientWorker = NetOwningClientWorker(Data.GetUnderlying());
			break;
		default:
			break;
		}
	}
}

// 	<<<<<<< HEAD
//         if (GetDefault<USpatialGDKSettings>()->bEnableUserSpaceLoadBalancing)
//         {
//         	return StaticComponentView->HasComponent(EntityId, SpatialConstants::ENTITY_ACL_COMPONENT_ID)
//                 && StaticComponentView->HasComponent(EntityId, SpatialConstants::AUTHORITY_INTENT_COMPONENT_ID)
//                 && StaticComponentView->HasComponent(EntityId, SpatialConstants::COMPONENT_PRESENCE_COMPONENT_ID)
//                 && StaticComponentView->HasComponent(EntityId, SpatialConstants::NET_OWNING_CLIENT_WORKER_COMPONENT_ID)
//                 && StaticComponentView->HasAuthority(EntityId, SpatialConstants::AUTHORITY_DELEGATION_COMPONENT_ID)
//                 // With USLB we also need the AuthorityDelegation component.
//                 && StaticComponentView->HasComponent(EntityId, SpatialConstants::AUTHORITY_DELEGATION_COMPONENT_ID);
//         }
//
// 	// We need to be able to see the ACL component
// 	return StaticComponentView->HasComponent(EntityId, SpatialConstants::ENTITY_ACL_COMPONENT_ID)
//         // and the authority intent component
//         && StaticComponentView->HasComponent(EntityId, SpatialConstants::AUTHORITY_INTENT_COMPONENT_ID)
//         // and the component presence component
//         && StaticComponentView->HasComponent(EntityId, SpatialConstants::COMPONENT_PRESENCE_COMPONENT_ID)
//         // and the net owning client worker component
//         && StaticComponentView->HasComponent(EntityId, SpatialConstants::NET_OWNING_CLIENT_WORKER_COMPONENT_ID)
//         // and we have to be able to write to the ACL component.
//         && StaticComponentView->HasAuthority(EntityId, SpatialConstants::ENTITY_ACL_COMPONENT_ID);
// }
//
// bool SpatialLoadBalanceEnforcer::HandlesComponent(Worker_ComponentId ComponentId)
// {
// 	return ComponentId == SpatialConstants::AUTHORITY_INTENT_COMPONENT_ID
//         || ComponentId == SpatialConstants::AUTHORITY_DELEGATION_COMPONENT_ID
//         || ComponentId == SpatialConstants::ENTITY_ACL_COMPONENT_ID
//         || ComponentId == SpatialConstants::COMPONENT_PRESENCE_COMPONENT_ID
//         || ComponentId == SpatialConstants::NET_OWNING_CLIENT_WORKER_COMPONENT_ID;
// 	=======

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
	case SpatialConstants::AUTHORITY_DELEGATION_COMPONENT_ID:
		DataStore[EntityId].Delegation = AuthorityDelegation(Data);
		break;
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

} // namespace SpatialGDK
