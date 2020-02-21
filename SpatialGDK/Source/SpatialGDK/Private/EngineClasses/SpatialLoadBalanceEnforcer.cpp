// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "EngineClasses/SpatialLoadBalanceEnforcer.h"
#include "EngineClasses/SpatialVirtualWorkerTranslator.h"
#include "Schema/AuthorityIntent.h"
#include "Schema/Component.h"
#include "SpatialCommonTypes.h"

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

void SpatialLoadBalanceEnforcer::OnAuthorityIntentComponentUpdated(const Worker_ComponentUpdateOp& Op)
{
	check(Op.update.component_id == SpatialConstants::AUTHORITY_INTENT_COMPONENT_ID);
	if (CanEnforce(Op.entity_id))
	{
		MaybeQueueAclAssignmentRequest(Op.entity_id);
	}
}

void SpatialLoadBalanceEnforcer::OnLoadBalancingComponentAdded(const Worker_AddComponentOp& Op)
{
	if (!(Op.data.component_id == SpatialConstants::AUTHORITY_INTENT_COMPONENT_ID || Op.data.component_id == SpatialConstants::ENTITY_ACL_COMPONENT_ID))
	{
		UE_LOG(LogSpatialLoadBalanceEnforcer, Warning,
			TEXT("Load balancer notified of add component %d for entity %lld which is not a load balancing component"),
			Op.data.component_id, Op.entity_id);
		return;
	}

	if (CanEnforce(Op.entity_id))
	{
		MaybeQueueAclAssignmentRequest(Op.entity_id);
	}
}

void SpatialLoadBalanceEnforcer::OnLoadBalancingComponentRemoved(const Worker_RemoveComponentOp& Op)
{
	if (!(Op.component_id == SpatialConstants::AUTHORITY_INTENT_COMPONENT_ID || Op.component_id == SpatialConstants::ENTITY_ACL_COMPONENT_ID))
	{
		UE_LOG(LogSpatialLoadBalanceEnforcer, Warning,
			TEXT("Load balancer notified of remove component %d for entity %lld which is not a load balancing component"),
			Op.component_id, Op.entity_id);
		return;
	}

	if (AclAssignmentRequestIsQueued(Op.entity_id))
	{
		UE_LOG(LogSpatialLoadBalanceEnforcer, Warning,
			TEXT("Component %d for entity %lld removed. Can no longer enforce the previous request for this entity."),
			Op.component_id, Op.entity_id);
		return;
	}
}

void SpatialLoadBalanceEnforcer::OnEntityRemoved(const Worker_RemoveEntityOp& Op)
{
	if (AclAssignmentRequestIsQueued(Op.entity_id))
	{
		UE_LOG(LogSpatialLoadBalanceEnforcer, Warning,
			TEXT("Entity %lld removed. Can no longer enforce the previous request for this entity."), Op.entity_id);
		return;
	}
}

void SpatialLoadBalanceEnforcer::OnAclAuthorityChanged(const Worker_AuthorityChangeOp& AuthOp)
{
	if (AuthOp.component_id != SpatialConstants::ENTITY_ACL_COMPONENT_ID)
	{
		UE_LOG(LogSpatialLoadBalanceEnforcer, Warning, TEXT("Loadbalancer informed of authority change for entity %lld that was not related to the ACL component."), AuthOp.entity_id);
		return;
	}

	if (AclAssignmentRequestIsQueued(AuthOp.entity_id) && AuthOp.authority == WORKER_AUTHORITY_NOT_AUTHORITATIVE)
	{
		UE_LOG(LogSpatialLoadBalanceEnforcer, Warning,
			TEXT("ACL authority lost for entity %lld. Can no longer enforce the previous request for this entity."),
			AuthOp.entity_id);
		return;
	}
	if (CanEnforce(AuthOp.entity_id))
	{
		MaybeQueueAclAssignmentRequest(AuthOp.entity_id);
	}
}

// MaybeQueueAclAssignmentRequest is called from three places.
// 1) AuthorityIntent change - Intent is not authoritative on this worker - ACL is authoritative on this worker.
//    (another worker changed the intent, but this worker is responsible for the ACL, so update it.)
// 2) ACL change - Intent may be anything - ACL just became authoritative on this worker.
//    (this worker just became responsible, so check to make sure intent and ACL agree.)
// 3) AuthorityIntent change - Intent is authoritative on this worker but no longer assigned to this worker - ACL is authoritative on this worker.
//    (this worker had responsibility for both and is giving up authority.)
// Queuing an ACL assignment request may not occur if the assignment is the same as before, or if the request is already queued.
void SpatialLoadBalanceEnforcer::MaybeQueueAclAssignmentRequest(const Worker_EntityId EntityId)
{
	const SpatialGDK::AuthorityIntent* AuthorityIntentComponent = StaticComponentView->GetComponentData<SpatialGDK::AuthorityIntent>(EntityId);
	if (AuthorityIntentComponent == nullptr)
	{
		// We should have checked the existence of the auth intent component before calling this function.
		checkNoEntry();
		return;
	}

	const PhysicalWorkerName* OwningWorkerId = VirtualWorkerTranslator->GetPhysicalWorkerForVirtualWorker(AuthorityIntentComponent->VirtualWorkerId);
	if (OwningWorkerId != nullptr &&
		*OwningWorkerId == WorkerId &&
		StaticComponentView->HasAuthority(EntityId, SpatialConstants::AUTHORITY_INTENT_COMPONENT_ID))
	{
		UE_LOG(LogSpatialLoadBalanceEnforcer, Verbose, TEXT("No need to queue newly authoritative entity %lld because this worker is already authoritative."), EntityId);
		return;
	}

	if (AclAssignmentRequestIsQueued(EntityId))
	{
		UE_LOG(LogSpatialLoadBalanceEnforcer, Verbose, TEXT("An ACL assignment request already exists for entity %lld on worker %s."), EntityId, *WorkerId);
		return;
	}

	QueueAclAssignmentRequest(EntityId);
}

TArray<SpatialLoadBalanceEnforcer::AclWriteAuthorityRequest> SpatialLoadBalanceEnforcer::ProcessQueuedAclAssignmentRequests()
{
	TArray<SpatialLoadBalanceEnforcer::AclWriteAuthorityRequest> PendingRequests;

	TArray<Worker_EntityId> CompletedRequests;
	CompletedRequests.Reserve(AclWriteAuthAssignmentRequests.Num());

	for (WriteAuthAssignmentRequest& Request : AclWriteAuthAssignmentRequests)
	{
		const SpatialGDK::AuthorityIntent* AuthorityIntentComponent = StaticComponentView->GetComponentData<SpatialGDK::AuthorityIntent>(Request.EntityId);
		if (AuthorityIntentComponent == nullptr)
		{
			// This happens if the authority intent component is removed in the same tick as a request is queued.
			UE_LOG(LogSpatialLoadBalanceEnforcer, Warning, TEXT("Cannot process entity as AuthIntent component has been removed since the request was queued. EntityId: %lld"), Request.EntityId);
			CompletedRequests.Add(Request.EntityId);
			continue;
		}

		if (AuthorityIntentComponent->VirtualWorkerId == SpatialConstants::INVALID_VIRTUAL_WORKER_ID)
		{
			UE_LOG(LogSpatialLoadBalanceEnforcer, Warning, TEXT("Entity with invalid virtual worker ID assignment will not be processed. EntityId: %lld. This should not happen - investigate if you see this warning."), Request.EntityId);
			CompletedRequests.Add(Request.EntityId);
			continue;
		}

		check(VirtualWorkerTranslator != nullptr);
		const PhysicalWorkerName* DestinationWorkerId = VirtualWorkerTranslator->GetPhysicalWorkerForVirtualWorker(AuthorityIntentComponent->VirtualWorkerId);
		if (DestinationWorkerId == nullptr)
		{
			const int32 WarnOnAttemptNum = 5;
			Request.ProcessAttempts++;
			// TODO(zoning): Revisit this when we support replacing crashed workers.
			if (Request.ProcessAttempts % WarnOnAttemptNum == 0)
			{
				UE_LOG(LogSpatialLoadBalanceEnforcer, Warning, TEXT("Failed to process WriteAuthAssignmentRequest because virtual worker mapping is unset for EntityID: %lld. Process attempts made: %d"), Request.EntityId, Request.ProcessAttempts);
			}
			// A virtual worker -> physical worker mapping may not be established yet.
			// We'll retry on the next Tick().
			continue;
		}

		if (StaticComponentView->HasAuthority(Request.EntityId, SpatialConstants::ENTITY_ACL_COMPONENT_ID))
		{
			EntityAcl* Acl = StaticComponentView->GetComponentData<EntityAcl>(Request.EntityId);
			auto* HeartbeatSets = Acl->ComponentWriteAcl.Find(SpatialConstants::HEARTBEAT_COMPONENT_ID);
			WorkerRequirementSet ClientRequirementSet;
			if (HeartbeatSets != nullptr)
			{
				// Always exactly one requirement set. This is a list only for legacy reasons.
				ClientRequirementSet = HeartbeatSets[0];
			}
			TArray<Worker_ComponentId> ComponentIds;
			Acl->ComponentWriteAcl.GetKeys(ComponentIds);
			PendingRequests.Push(
				AclWriteAuthorityRequest{
					Request.EntityId,
					*DestinationWorkerId,
					Acl->ReadAcl,
					ClientRequirementSet,
					ComponentIds
				});
		}
		else
		{
			UE_LOG(LogSpatialLoadBalanceEnforcer, Log, TEXT("Failed to update the EntityACL to match the authority intent; this worker does lost authority over the EntityACL since the request was queued."
				" Source worker ID: %s. Entity ID %lld. Desination worker ID: %s."), *WorkerId, Request.EntityId, **DestinationWorkerId);
		}
		CompletedRequests.Add(Request.EntityId);
	}

	AclWriteAuthAssignmentRequests.RemoveAll([CompletedRequests](const WriteAuthAssignmentRequest& Request) { return CompletedRequests.Contains(Request.EntityId); });

	return PendingRequests;
}

void SpatialLoadBalanceEnforcer::QueueAclAssignmentRequest(const Worker_EntityId EntityId)
{
	UE_LOG(LogSpatialLoadBalanceEnforcer, Verbose, TEXT("Queueing ACL assignment request for entity %lld on worker %s."), EntityId, *WorkerId);
	AclWriteAuthAssignmentRequests.Add(WriteAuthAssignmentRequest(EntityId));
}

bool SpatialLoadBalanceEnforcer::AclAssignmentRequestIsQueued(const Worker_EntityId EntityId)
{
	return AclWriteAuthAssignmentRequests.ContainsByPredicate([EntityId](const WriteAuthAssignmentRequest& Request) { return Request.EntityId == EntityId; });
}

bool SpatialLoadBalanceEnforcer::CanEnforce(Worker_EntityId EntityId)
{
	EntityAcl* Acl = StaticComponentView->GetComponentData<EntityAcl>(EntityId);
	AuthorityIntent* AuthIntent = StaticComponentView->GetComponentData<AuthorityIntent>(EntityId);
	// We need to be able to see the ACL and auth intent components, and be able to write to the ACL component.
	return Acl != nullptr && AuthIntent != nullptr && StaticComponentView->HasAuthority(EntityId, SpatialConstants::ENTITY_ACL_COMPONENT_ID);
}
