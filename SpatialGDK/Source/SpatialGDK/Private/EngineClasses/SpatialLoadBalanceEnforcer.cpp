// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "EngineClasses/SpatialLoadBalanceEnforcer.h"
#include "EngineClasses/SpatialVirtualWorkerTranslator.h"
#include "Interop/Connection/SpatialWorkerConnection.h"
#include "Interop/SpatialSender.h"
#include "Interop/SpatialStaticComponentView.h"
#include "Schema/AuthorityIntent.h"
#include "SpatialCommonTypes.h"

DEFINE_LOG_CATEGORY(LogSpatialLoadBalanceEnforcer);

using namespace SpatialGDK;

SpatialLoadBalanceEnforcer::SpatialLoadBalanceEnforcer()
	: StaticComponentView(nullptr)
	, Sender(nullptr)
	, VirtualWorkerTranslator(nullptr)
{
}

void SpatialLoadBalanceEnforcer::Init(const FString &InWorkerId,
	USpatialStaticComponentView* InStaticComponentView,
	USpatialSender* InSpatialSender,
	SpatialVirtualWorkerTranslator* InVirtualWorkerTranslator)
{
	WorkerId = InWorkerId;

	check(InStaticComponentView != nullptr);
	StaticComponentView = InStaticComponentView;

	check(InSpatialSender != nullptr);
	Sender = InSpatialSender;

	check(InVirtualWorkerTranslator != nullptr);
	VirtualWorkerTranslator = InVirtualWorkerTranslator;
}

void SpatialLoadBalanceEnforcer::Tick()
{
	ProcessQueuedAclAssignmentRequests();
}

void SpatialLoadBalanceEnforcer::OnAuthorityIntentComponentUpdated(const Worker_ComponentUpdateOp& Op)
{
	check(StaticComponentView.IsValid())
	check(Op.update.component_id == SpatialConstants::AUTHORITY_INTENT_COMPONENT_ID)
	if (StaticComponentView->GetAuthority(Op.entity_id, SpatialConstants::ENTITY_ACL_COMPONENT_ID) == WORKER_AUTHORITY_AUTHORITATIVE)
	{
		QueueAclAssignmentRequest(Op.entity_id);
	}
}

// This is called whenever this worker becomes authoritative for the ACL component on an entity.
// It is now this worker's responsibility to make sure that the ACL reflects the current intent,
// which may have been received before this call.
void SpatialLoadBalanceEnforcer::AuthorityChanged(const Worker_AuthorityChangeOp& AuthOp)
{
	check(StaticComponentView.IsValid())

	if (AuthOp.component_id == SpatialConstants::ENTITY_ACL_COMPONENT_ID &&
		AuthOp.authority == WORKER_AUTHORITY_AUTHORITATIVE)
	{
		const AuthorityIntent* AuthorityIntentComponent = StaticComponentView->GetComponentData<SpatialGDK::AuthorityIntent>(AuthOp.entity_id);
		if (AuthorityIntentComponent == nullptr)
		{
			// TODO(zoning): There are still some entities being created without an authority intent component.
			// For example, the Unreal created worker entities don't have one. Even though those won't be able to
			// transition, we should have the intent component on them for completeness.
		 	UE_LOG(LogSpatialLoadBalanceEnforcer, Log, TEXT("Requested authority change for entity without AuthorityIntent component. EntityId: %lld"), AuthOp.entity_id);
			return;
		}

		const FString* OwningWorkerId = VirtualWorkerTranslator->GetPhysicalWorkerForVirtualWorker(AuthorityIntentComponent->VirtualWorkerId);
		if (OwningWorkerId != nullptr &&
			*OwningWorkerId == WorkerId &&
			StaticComponentView->GetAuthority(AuthOp.entity_id, SpatialConstants::AUTHORITY_INTENT_COMPONENT_ID) == WORKER_AUTHORITY_AUTHORITATIVE)
		{
			UE_LOG(LogSpatialLoadBalanceEnforcer, Verbose, TEXT("No need to queue newly authoritative entity %lld because this worker is already authoritative."), AuthOp.entity_id);
			return;
		}
		QueueAclAssignmentRequest(AuthOp.entity_id);
	}
}

// QueueAclAssignmentRequest is called from three places.
// 1) AuthorityIntent change - Intent is not authoritative on this worker - ACL is authoritative on this worker.
//    (another worker changed the intent, but this worker is responsible for the ACL, so update it.)
// 2) ACL change - Intent may be anything - ACL just became authoritative on this worker.
//    (this worker just became responsible, so check to make sure intent and ACL agree.)
// 3) AuthorityIntent change - Intent is authoritative on this worker but no longer assigned to this worker - ACL is authoritative on this worker.
//    (this worker had responsibility for both and is giving up authority.)
void SpatialLoadBalanceEnforcer::QueueAclAssignmentRequest(const Worker_EntityId EntityId)
{
	// TODO(zoning): measure the performance impact of this.
	if (AclWriteAuthAssignmentRequests.ContainsByPredicate([EntityId](const WriteAuthAssignmentRequest& Request) { return Request.EntityId == EntityId; }))
	{
		UE_LOG(LogSpatialLoadBalanceEnforcer, Verbose, TEXT("An ACL assignment request already exists for entity %lld on worker %s."), EntityId, *WorkerId);
	}
	else
	{
		UE_LOG(LogSpatialLoadBalanceEnforcer, Verbose, TEXT("Queueing ACL assignment request for entity %lld on worker %s."), EntityId, *WorkerId);
		AclWriteAuthAssignmentRequests.Add(WriteAuthAssignmentRequest(EntityId));
	}
}

void SpatialLoadBalanceEnforcer::ProcessQueuedAclAssignmentRequests()
{
	TArray<Worker_EntityId> CompletedRequests;
	CompletedRequests.Reserve(AclWriteAuthAssignmentRequests.Num());

	for (WriteAuthAssignmentRequest& Request : AclWriteAuthAssignmentRequests)
	{
 		const AuthorityIntent* AuthorityIntentComponent = StaticComponentView->GetComponentData<SpatialGDK::AuthorityIntent>(Request.EntityId);
		if (AuthorityIntentComponent == nullptr)
		{
			// TODO(zoning): Not sure whether this should be possible or not. Remove if we don't see the warning again.
			UE_LOG(LogSpatialLoadBalanceEnforcer, Warning, TEXT("Entity without AuthIntent component will not be processed. EntityId: %lld"), Request.EntityId);
			CompletedRequests.Add(Request.EntityId);
			return;
		}

		if (AuthorityIntentComponent->VirtualWorkerId == SpatialConstants::INVALID_VIRTUAL_WORKER_ID)
		{
			UE_LOG(LogSpatialLoadBalanceEnforcer, Warning, TEXT("Entity with invalid virtual worker ID assignment will not be processed. EntityId: %lld. This should not happen - investigate if you see this warning."), Request.EntityId);
			CompletedRequests.Add(Request.EntityId);
			return;
		}

		const FString* OwningWorkerId = VirtualWorkerTranslator->GetPhysicalWorkerForVirtualWorker(AuthorityIntentComponent->VirtualWorkerId);
		if (OwningWorkerId == nullptr)
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

		check(Sender.IsValid());
		Sender->SetAclWriteAuthority(Request.EntityId, *OwningWorkerId);
		CompletedRequests.Add(Request.EntityId);
	}

	AclWriteAuthAssignmentRequests.RemoveAll([CompletedRequests](const WriteAuthAssignmentRequest& Request) { return CompletedRequests.Contains(Request.EntityId); });
}
