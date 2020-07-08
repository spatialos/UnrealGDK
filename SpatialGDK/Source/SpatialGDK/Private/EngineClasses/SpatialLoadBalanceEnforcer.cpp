// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "EngineClasses/SpatialLoadBalanceEnforcer.h"

#include "EngineClasses/SpatialNetDriver.h"
#include "EngineClasses/SpatialPackageMapClient.h"
#include "EngineClasses/SpatialVirtualWorkerTranslator.h"
#include "Schema/AuthorityIntent.h"
#include "Schema/Component.h"
#include "Schema/ComponentPresence.h"
#include "Schema/NetOwningClientWorker.h"
#include "SpatialCommonTypes.h"
#include "SpatialGDKSettings.h"

DEFINE_LOG_CATEGORY(LogSpatialLoadBalanceEnforcer);

using namespace SpatialGDK;

SpatialLoadBalanceEnforcer::SpatialLoadBalanceEnforcer(const PhysicalWorkerName& InWorkerId, const USpatialStaticComponentView* InStaticComponentView, const SpatialVirtualWorkerTranslator* InVirtualWorkerTranslator)
	: WorkerId(InWorkerId)
	, StaticComponentView(InStaticComponentView)
	, VirtualWorkerTranslator(InVirtualWorkerTranslator)
{
	check(InStaticComponentView != nullptr);
	check(InVirtualWorkerTranslator != nullptr);
}

void SpatialLoadBalanceEnforcer::OnLoadBalancingComponentAdded(const Worker_AddComponentOp& Op)
{
	check(HandlesComponent(Op.data.component_id));

	MaybeQueueAclAssignmentRequest(Op.entity_id);
}

void SpatialLoadBalanceEnforcer::OnLoadBalancingComponentUpdated(const Worker_ComponentUpdateOp& Op)
{
	check(HandlesComponent(Op.update.component_id));

	MaybeQueueAclAssignmentRequest(Op.entity_id);
}

void SpatialLoadBalanceEnforcer::OnLoadBalancingComponentRemoved(const Worker_RemoveComponentOp& Op)
{
	check(HandlesComponent(Op.component_id));

	if (AclAssignmentRequestIsQueued(Op.entity_id))
	{
		UE_LOG(LogSpatialLoadBalanceEnforcer, Log,
			TEXT("Component %d for entity %lld removed. Can no longer enforce the previous request for this entity."),
			Op.component_id, Op.entity_id);
		PendingEntityAuthorityChanges.Remove(Op.entity_id);
	}
}

void SpatialLoadBalanceEnforcer::OnEntityRemoved(const Worker_RemoveEntityOp& Op)
{
	if (AclAssignmentRequestIsQueued(Op.entity_id))
	{
		UE_LOG(LogSpatialLoadBalanceEnforcer, Log, TEXT("Entity %lld removed. Can no longer enforce the previous request for this entity."),
			Op.entity_id);
		PendingEntityAuthorityChanges.Remove(Op.entity_id);
	}
}

void SpatialLoadBalanceEnforcer::OnAclAuthorityChanged(const Worker_AuthorityChangeOp& AuthOp)
{
	// This class should only be informed of ACL authority changes.
	check(AuthOp.component_id == SpatialConstants::ENTITY_ACL_COMPONENT_ID);

	if (AuthOp.authority != WORKER_AUTHORITY_AUTHORITATIVE)
	{
		if (AclAssignmentRequestIsQueued(AuthOp.entity_id))
		{
			UE_LOG(LogSpatialLoadBalanceEnforcer, Log,
				TEXT("ACL authority lost for entity %lld. Can no longer enforce the previous request for this entity."),
				AuthOp.entity_id);
			PendingEntityAuthorityChanges.Remove(AuthOp.entity_id);
		}
		return;
	}

	MaybeQueueAclAssignmentRequest(AuthOp.entity_id);
}

// MaybeQueueAclAssignmentRequest is called from three places.
// 1) AuthorityIntent change - Intent is not authoritative on this worker - ACL is authoritative on this worker.
//    (another worker changed the intent, but this worker is responsible for the ACL, so update it.)
// 2) ACL change - Intent may be anything - ACL just became authoritative on this worker.
//    (this worker just became responsible, so check to make sure intent and ACL agree.)
// 3) AuthorityIntent change - Intent is authoritative on this worker but no longer assigned to this worker - ACL is authoritative on this worker.
//    (this worker had responsibility for both and is giving up authority.)
// Queuing an ACL assignment request may not occur if the assignment is the same as before, or if the request is already queued,
// or if we don't meet the predicate required to enforce the assignment.
void SpatialLoadBalanceEnforcer::MaybeQueueAclAssignmentRequest(const Worker_EntityId EntityId)
{
	if (!CanEnforce(EntityId))
	{
		return;
	}

	if (!EntityNeedsToBeEnforced(EntityId))
	{
		return;
	}

	if (AclAssignmentRequestIsQueued(EntityId))
	{
		UE_LOG(LogSpatialLoadBalanceEnforcer, Verbose, TEXT("Avoiding queueing a duplicate ACL assignment request. "
			"Entity: %lld. Worker: %s."), EntityId, *WorkerId);
		return;
	}

	UE_LOG(LogSpatialLoadBalanceEnforcer, Log, TEXT("Queueing ACL assignment request. Entity %lld. Worker %s."), EntityId, *WorkerId);
	PendingEntityAuthorityChanges.Add(EntityId);
}

bool SpatialLoadBalanceEnforcer::EntityNeedsToBeEnforced(const Worker_EntityId EntityId) const
{
	bool AuthorityChangeRequired = false;

	const AuthorityIntent* AuthorityIntentComponent = StaticComponentView->GetComponentData<AuthorityIntent>(EntityId);
	check(AuthorityIntentComponent != nullptr);

	const ComponentPresence* ComponentPresenceList = StaticComponentView->GetComponentData<ComponentPresence>(EntityId);
	check(ComponentPresenceList != nullptr);

	const NetOwningClientWorker* OwningClientWorker = StaticComponentView->GetComponentData<NetOwningClientWorker>(EntityId);
	check(OwningClientWorker != nullptr);

	const Worker_ComponentId ClientRpcAuthComponent = SpatialConstants::GetClientAuthorityComponent(GetDefault<USpatialGDKSettings>()->UseRPCRingBuffer());

	TArray<Worker_ComponentId> AuthoritativeComponents;

	if (GetDefault<USpatialGDKSettings>()->bEnableUserSpaceLoadBalancing)
	{
		// Check to see if target authoritative server has changed.
		const AuthorityDelegation* DelegationMap = StaticComponentView->GetComponentData<AuthorityDelegation>(EntityId);
		const Worker_PartitionId* CurrentAuthoritativePartition = DelegationMap->Delegations.Find(SpatialConstants::AUTHORITY_INTENT_COMPONENT_ID);
		check(CurrentAuthoritativePartition != nullptr);

		const Worker_PartitionId TargetAuthoritativePartition = VirtualWorkerTranslator->GetPartitionEntityForVirtualWorker(AuthorityIntentComponent->VirtualWorkerId);
		checkf(TargetAuthoritativePartition != SpatialConstants::INVALID_ENTITY_ID, TEXT("Couldn't find mapped worker for entity %lld. Virtual worker ID: %d"),
            EntityId, AuthorityIntentComponent->VirtualWorkerId);

		AuthorityChangeRequired |= *CurrentAuthoritativePartition != TargetAuthoritativePartition;

		// Check to see if component presence has changed.
		for (const auto& RequiredComponent : ComponentPresenceList->ComponentList)
		{
			AuthorityChangeRequired |= !DelegationMap->Delegations.Contains(RequiredComponent);
		}

		// Check to see if net owning client worker has changed.
		const Worker_PartitionId CurrentOwningClientPartition = *DelegationMap->Delegations.Find(ClientRpcAuthComponent);
		const Worker_PartitionId NewOwningClientPartition = OwningClientWorker->ClientPartitionId.IsSet() ?
			OwningClientWorker->ClientPartitionId.GetValue() : SpatialConstants::INVALID_ENTITY_ID;
		AuthorityChangeRequired |= CurrentOwningClientPartition != NewOwningClientPartition;
	}
	else
	{
		// Check to see if target authoritative server has changed.
		const EntityAcl* Acl = StaticComponentView->GetComponentData<EntityAcl>(EntityId);
		const WorkerRequirementSet* CurrentAuthoritativeWorker = Acl->ComponentWriteAcl.Find(SpatialConstants::AUTHORITY_INTENT_COMPONENT_ID);
		check(CurrentAuthoritativeWorker != nullptr);
		const PhysicalWorkerName* TargetAuthoritativeWorker = VirtualWorkerTranslator->GetPhysicalWorkerForVirtualWorker(AuthorityIntentComponent->VirtualWorkerId);
		checkf(TargetAuthoritativeWorker != nullptr, TEXT("Couldn't find mapped worker for entity %lld. Virtual worker ID: %d"),
			EntityId, AuthorityIntentComponent->VirtualWorkerId);
		AuthorityChangeRequired |= (*CurrentAuthoritativeWorker)[0][0] != *TargetAuthoritativeWorker;

		// Check to see if component presence has changed.
		for (const auto& RequiredComponent : ComponentPresenceList->ComponentList)
		{
			AuthorityChangeRequired |= !Acl->ComponentWriteAcl.Contains(RequiredComponent);
		}

		// Check to see if net owning client worker has changed.
        const PhysicalWorkerName CurrentOwningClientWorkerId = (*Acl->ComponentWriteAcl.Find(ClientRpcAuthComponent))[0][0];
		const PhysicalWorkerName NewOwningClientWorkerId = OwningClientWorker->WorkerId.IsSet() ?
            OwningClientWorker->WorkerId.GetValue() : FString();
		AuthorityChangeRequired |= CurrentOwningClientWorkerId != NewOwningClientWorkerId;
	}

	return AuthorityChangeRequired;
}

bool SpatialLoadBalanceEnforcer::AclAssignmentRequestIsQueued(const Worker_EntityId EntityId) const
{
	return PendingEntityAuthorityChanges.Contains(EntityId);
}

bool SpatialLoadBalanceEnforcer::GetAuthorityChangeState(Worker_EntityId EntityId, AuthorityStateChange& OutAuthorityChange) const
{
	const AuthorityIntent* AuthorityIntentComponent = StaticComponentView->GetComponentData<AuthorityIntent>(EntityId);
	checkf(AuthorityIntentComponent != nullptr, TEXT("Can't process entity authority change. AuthorityIntent component has been removed since the request was queued. "
		"EntityId: %lld"), EntityId);
	checkf(AuthorityIntentComponent->VirtualWorkerId != SpatialConstants::INVALID_VIRTUAL_WORKER_ID,
		TEXT("Entity with invalid virtual worker ID assignment will not be processed. EntityId: %lld."), EntityId);

	const NetOwningClientWorker* NetOwningClientWorkerComponent = StaticComponentView->GetComponentData<NetOwningClientWorker>(EntityId);
	checkf(NetOwningClientWorkerComponent != nullptr, TEXT("Can't process entity authority change. NetOwningClientWorker component has been removed since the request was queued. "
		"EntityId: %lld"), EntityId);

	const ComponentPresence* ComponentPresenceComponent = StaticComponentView->GetComponentData<ComponentPresence>(EntityId);
	checkf(ComponentPresenceComponent != nullptr, TEXT("Can't process entity authority change. ComponentPresence component has been removed since the request was queued. "
		"EntityId: %lld"), EntityId);

	const EntityAcl* Acl = StaticComponentView->GetComponentData<EntityAcl>(EntityId);
	checkf(Acl != nullptr, TEXT("Can't process entity authority change. EntityAcl component has been removed since the request was queued. "
		"EntityId: %lld"), EntityId);

	TArray<Worker_ComponentId> ComponentIds;

	// With USLB enabled, we get the entity component list from the AuthorityDelegation component.
	// With USLB disabled, it's the EntityACL component.
	if (GetDefault<USpatialGDKSettings>()->bEnableUserSpaceLoadBalancing)
	{
		const AuthorityDelegation* AuthDelegation = StaticComponentView->GetComponentData<AuthorityDelegation>(EntityId);
		checkf(AuthDelegation != nullptr, TEXT("Can't process entity authority change. AuthorityDelegation component was removed while the request was queued. "
			"EntityId: %lld"), EntityId);
		AuthDelegation->Delegations.GetKeys(ComponentIds);
	}
	else
	{
		Acl->ComponentWriteAcl.GetKeys(ComponentIds);
	}

	// Ensure that every component ID in ComponentPresence will be assigned authority.
	for (const auto& RequiredComponentId : ComponentPresenceComponent->ComponentList)
	{
		ComponentIds.AddUnique(RequiredComponentId);
	}

	OutAuthorityChange = AuthorityStateChange{
		EntityId,
		Acl->ReadAcl,
		ComponentIds,
		AuthorityIntentComponent->VirtualWorkerId
	};

	// Commented this out for now because it only detects changes in authoritative server, not in changes to component presence,
	// or net owning client worker.
	//if (!EntityNeedsToBeEnforced)
	//{
	//	return false;
	//}

	return true;
}

TArray<SpatialLoadBalanceEnforcer::AuthorityStateChange> SpatialLoadBalanceEnforcer::ProcessAuthorityChangeRequests()
{
	TArray<AuthorityStateChange> AuthorityChangesToProcess;
	AuthorityChangesToProcess.Reserve(PendingEntityAuthorityChanges.Num());

	for (Worker_EntityId EntityId : PendingEntityAuthorityChanges)
	{
		AuthorityStateChange AuthorityChange{};
		const bool bIsAuthorityChangeNeeded = GetAuthorityChangeState(EntityId, AuthorityChange);
		if (bIsAuthorityChangeNeeded)
		{
			AuthorityChangesToProcess.Emplace(AuthorityChange);
		}
	}

	PendingEntityAuthorityChanges.Empty();

	return AuthorityChangesToProcess;
}

bool SpatialLoadBalanceEnforcer::CanEnforce(Worker_EntityId EntityId) const
{
	if (GetDefault<USpatialGDKSettings>()->bEnableUserSpaceLoadBalancing)
	{
		return StaticComponentView->HasComponent(EntityId, SpatialConstants::ENTITY_ACL_COMPONENT_ID)
            && StaticComponentView->HasComponent(EntityId, SpatialConstants::AUTHORITY_INTENT_COMPONENT_ID)
            && StaticComponentView->HasComponent(EntityId, SpatialConstants::COMPONENT_PRESENCE_COMPONENT_ID)
            && StaticComponentView->HasComponent(EntityId, SpatialConstants::NET_OWNING_CLIENT_WORKER_COMPONENT_ID)
            && StaticComponentView->HasAuthority(EntityId, SpatialConstants::AUTHORITY_DELEGATION_COMPONENT_ID)
			// With USLB we also need the AuthorityDelegation component.
			&& StaticComponentView->HasComponent(EntityId, SpatialConstants::AUTHORITY_DELEGATION_COMPONENT_ID);
	}

	// We need to be able to see the ACL component
	return StaticComponentView->HasComponent(EntityId, SpatialConstants::ENTITY_ACL_COMPONENT_ID)
		// and the authority intent component
		&& StaticComponentView->HasComponent(EntityId, SpatialConstants::AUTHORITY_INTENT_COMPONENT_ID)
		// and the component presence component
		&& StaticComponentView->HasComponent(EntityId, SpatialConstants::COMPONENT_PRESENCE_COMPONENT_ID)
		// and the net owning client worker component
		&& StaticComponentView->HasComponent(EntityId, SpatialConstants::NET_OWNING_CLIENT_WORKER_COMPONENT_ID)
		// and we have to be able to write to the ACL component.
		&& StaticComponentView->HasAuthority(EntityId, SpatialConstants::ENTITY_ACL_COMPONENT_ID);
}

bool SpatialLoadBalanceEnforcer::HandlesComponent(Worker_ComponentId ComponentId)
{
	return ComponentId == SpatialConstants::AUTHORITY_INTENT_COMPONENT_ID
		|| ComponentId == SpatialConstants::AUTHORITY_DELEGATION_COMPONENT_ID
		|| ComponentId == SpatialConstants::ENTITY_ACL_COMPONENT_ID
		|| ComponentId == SpatialConstants::COMPONENT_PRESENCE_COMPONENT_ID
		|| ComponentId == SpatialConstants::NET_OWNING_CLIENT_WORKER_COMPONENT_ID;
}
