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
		AclWriteAuthAssignmentRequests.Remove(Op.entity_id);
	}
}

void SpatialLoadBalanceEnforcer::OnEntityRemoved(const Worker_RemoveEntityOp& Op)
{
	if (AclAssignmentRequestIsQueued(Op.entity_id))
	{
		UE_LOG(LogSpatialLoadBalanceEnforcer, Log, TEXT("Entity %lld removed. Can no longer enforce the previous request for this entity."),
			Op.entity_id);
		AclWriteAuthAssignmentRequests.Remove(Op.entity_id);
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
			AclWriteAuthAssignmentRequests.Remove(AuthOp.entity_id);
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

	const SpatialGDK::AuthorityIntent* AuthorityIntentComponent = StaticComponentView->GetComponentData<SpatialGDK::AuthorityIntent>(EntityId);
	const PhysicalWorkerName* OwningWorkerId = VirtualWorkerTranslator->GetPhysicalWorkerForVirtualWorker(AuthorityIntentComponent->VirtualWorkerId);

	check(OwningWorkerId != nullptr);
	if (OwningWorkerId == nullptr)
	{
		UE_LOG(LogSpatialLoadBalanceEnforcer, Error, TEXT("Couldn't find mapped worker for entity %lld. This shouldn't happen! Virtual worker ID: %d"),
			EntityId, AuthorityIntentComponent->VirtualWorkerId);
		return;
	}

	if (*OwningWorkerId == WorkerId && StaticComponentView->HasAuthority(EntityId, SpatialConstants::AUTHORITY_INTENT_COMPONENT_ID))
	{
		UE_LOG(LogSpatialLoadBalanceEnforcer, Verbose, TEXT("No need to queue newly authoritative entity because this worker is already authoritative. Entity: %lld. Worker: %s."),
			EntityId, *WorkerId);
		return;
	}

	if (AclAssignmentRequestIsQueued(EntityId))
	{
		UE_LOG(LogSpatialLoadBalanceEnforcer, Verbose, TEXT("Avoiding queueing a duplicate ACL assignment request. Entity: %lld. Worker: %s."),
			EntityId, *WorkerId);
		return;
	}

	QueueAclAssignmentRequest(EntityId);
}

bool SpatialLoadBalanceEnforcer::AclAssignmentRequestIsQueued(const Worker_EntityId EntityId) const
{
	return AclWriteAuthAssignmentRequests.Contains(EntityId);
}

bool SpatialLoadBalanceEnforcer::GetAuthorityChangeState(Worker_EntityId EntityId, SpatialLoadBalanceEnforcer::AuthorityStateChange& OutAuthorityStateChange) const
{
	if (GetDefault<USpatialGDKSettings>()->bEnableUserSpaceLoadBalancing)
	{
		const AuthorityDelegation* AuthorityDelegationComponent = StaticComponentView->GetComponentData<SpatialGDK::AuthorityDelegation>(EntityId);
		if (AuthorityDelegationComponent == nullptr)
		{
			// This happens if the AuthorityDelegation component is removed in the same tick as a request is queued, but the request was not removed from the queue - shouldn't happen.
			UE_LOG(LogSpatialLoadBalanceEnforcer, Error, TEXT("Cannot process entity as AuthorityDelegation component has been removed since the request was queued. EntityId: %lld"), EntityId);
			return false;
		}
	}

	const AuthorityIntent* AuthorityIntentComponent = StaticComponentView->GetComponentData<SpatialGDK::AuthorityIntent>(EntityId);
	if (AuthorityIntentComponent == nullptr)
	{
		// This happens if the AuthorityIntent component is removed in the same tick as a request is queued, but the request was not removed from the queue - shouldn't happen.
		UE_LOG(LogSpatialLoadBalanceEnforcer, Error, TEXT("Cannot process entity as AuthorityIntent component has been removed since the request was queued. EntityId: %lld"), EntityId);
		return false;
	}

	if (AuthorityIntentComponent->VirtualWorkerId == SpatialConstants::INVALID_VIRTUAL_WORKER_ID)
	{
		UE_LOG(LogSpatialLoadBalanceEnforcer, Warning, TEXT("Entity with invalid virtual worker ID assignment will not be processed. EntityId: %lld. This should not happen - investigate if you see this warning."), EntityId);
		return false;
	}

	const NetOwningClientWorker* NetOwningClientWorkerComponent = StaticComponentView->GetComponentData<SpatialGDK::NetOwningClientWorker>(EntityId);
	if (NetOwningClientWorkerComponent == nullptr)
	{
		// This happens if the NetOwningClientWorker component is removed in the same tick as a request is queued, but the request was not removed from the queue - shouldn't happen.
		UE_LOG(LogSpatialLoadBalanceEnforcer, Error, TEXT("Cannot process entity as NetOwningClientWorker component has been removed since the request was queued. EntityId: %lld"), EntityId);
		return false;
	}

	const ComponentPresence* ComponentPresenceComponent = StaticComponentView->GetComponentData<ComponentPresence>(EntityId);
	if (ComponentPresenceComponent == nullptr)
	{
		// This happens if the ComponentPresence component is removed in the same tick as a request is queued, but the request was not removed from the queue - shouldn't happen.
		UE_LOG(LogSpatialLoadBalanceEnforcer, Error, TEXT("Cannot process entity as ComponentPresence component has been removed since the request was queued. EntityId: %lld"), EntityId);
		return false;
	}

	if (!StaticComponentView->HasAuthority(EntityId, SpatialConstants::ENTITY_ACL_COMPONENT_ID))
	{
		UE_LOG(LogSpatialLoadBalanceEnforcer, Log, TEXT("Failed to process ACL assignment; this worker lost authority over the EntityACL since the request was queued."
            " Source worker ID: %s. Entity ID %lld."), *WorkerId, EntityId);
		return false;
	}

	TArray<Worker_ComponentId> ComponentIds;

	const EntityAcl* Acl = StaticComponentView->GetComponentData<EntityAcl>(EntityId);

	// With USLB enabled, we grab our current component authorities from the AuthorityDelegation component.
	// With USLB disabled, it's EntityACL.
	if (GetDefault<USpatialGDKSettings>()->bEnableUserSpaceLoadBalancing)
	{
		AuthorityDelegation* AuthDelegation = StaticComponentView->GetComponentData<AuthorityDelegation>(EntityId);
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

	OutAuthorityStateChange = AuthorityStateChange{
		EntityId,
        Acl->ReadAcl,
        ComponentIds,
        AuthorityIntentComponent->VirtualWorkerId
    };

	return true;
}

TArray<SpatialLoadBalanceEnforcer::AuthorityStateChange> SpatialLoadBalanceEnforcer::ProcessQueuedAclAssignmentRequests()
{
	TArray<AuthorityStateChange> PendingRequests;

	TArray<Worker_EntityId> CompletedRequests;
	CompletedRequests.Reserve(AclWriteAuthAssignmentRequests.Num());

	for (Worker_EntityId EntityId : AclWriteAuthAssignmentRequests)
	{
		AuthorityStateChange AuthorityChangeData{};
		const bool bAuthorityChangeDataSuccess = GetAuthorityChangeState(EntityId, AuthorityChangeData);

		if (bAuthorityChangeDataSuccess)
		{
			PendingRequests.Push(AuthorityChangeData);
		}

		CompletedRequests.Add(EntityId);
	}

	AclWriteAuthAssignmentRequests.RemoveAll([CompletedRequests](const Worker_EntityId& EntityId) { return CompletedRequests.Contains(EntityId); });

	return PendingRequests;
}

void SpatialLoadBalanceEnforcer::QueueAclAssignmentRequest(const Worker_EntityId EntityId)
{
	UE_LOG(LogSpatialLoadBalanceEnforcer, Verbose, TEXT("Queueing ACL assignment request for entity %lld on worker %s."), EntityId, *WorkerId);
	AclWriteAuthAssignmentRequests.Add(EntityId);
}

bool SpatialLoadBalanceEnforcer::CanEnforce(Worker_EntityId EntityId) const
{
	if (GetDefault<USpatialGDKSettings>()->bEnableUserSpaceLoadBalancing)
	{
		return StaticComponentView->HasComponent(EntityId, SpatialConstants::ENTITY_ACL_COMPONENT_ID)
            && StaticComponentView->HasComponent(EntityId, SpatialConstants::AUTHORITY_INTENT_COMPONENT_ID)
            && StaticComponentView->HasComponent(EntityId, SpatialConstants::AUTHORITY_DELEGATION_COMPONENT_ID)
            && StaticComponentView->HasComponent(EntityId, SpatialConstants::COMPONENT_PRESENCE_COMPONENT_ID)
            && StaticComponentView->HasAuthority(EntityId, SpatialConstants::ENTITY_ACL_COMPONENT_ID);
	}

	// We need to be able to see the ACL component
	return StaticComponentView->HasComponent(EntityId, SpatialConstants::ENTITY_ACL_COMPONENT_ID)
		// and the authority intent component
		&& StaticComponentView->HasComponent(EntityId, SpatialConstants::AUTHORITY_INTENT_COMPONENT_ID)
		// and the component presence component
		&& StaticComponentView->HasComponent(EntityId, SpatialConstants::COMPONENT_PRESENCE_COMPONENT_ID)
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
