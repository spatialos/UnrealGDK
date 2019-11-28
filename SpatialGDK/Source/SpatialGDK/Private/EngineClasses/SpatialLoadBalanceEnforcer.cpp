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

USpatialLoadBalanceEnforcer::USpatialLoadBalanceEnforcer(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, StaticComponentView(nullptr)
	, Sender(nullptr)
	, VirtualWorkerTranslator(nullptr)
{
}

void USpatialLoadBalanceEnforcer::Init(const FString &InWorkerId,
	USpatialStaticComponentView* InStaticComponentView,
	USpatialSender* InSpatialSender,
	SpatialVirtualWorkerTranslator* InVirtualWorkerTranslator)
{
	WorkerId = InWorkerId;
	StaticComponentView = InStaticComponentView;
	Sender = InSpatialSender;
	VirtualWorkerTranslator = InVirtualWorkerTranslator;
}

void USpatialLoadBalanceEnforcer::Tick()
{
	ProcessQueuedAclAssignmentRequests();
}

void USpatialLoadBalanceEnforcer::OnAuthorityIntentComponentUpdated(const Worker_ComponentUpdateOp& Op)
{
	check(StaticComponentView != nullptr)
	check(Op.update.component_id == SpatialConstants::AUTHORITY_INTENT_COMPONENT_ID)
	if (StaticComponentView->GetAuthority(Op.entity_id, SpatialConstants::ENTITY_ACL_COMPONENT_ID) == WORKER_AUTHORITY_AUTHORITATIVE)
	{
		QueueAclAssignmentRequest(Op.entity_id);
	}
}

// This is called whenever this worker becomes authoritative for the ACL component on an entity.
// It is now this worker's responsibility to make sure that the ACL reflects the current intent,
// which may have been received before this call.
void USpatialLoadBalanceEnforcer::AuthorityChanged(const Worker_AuthorityChangeOp& AuthOp)
{
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
void USpatialLoadBalanceEnforcer::QueueAclAssignmentRequest(const Worker_EntityId EntityId)
{
	// TODO(zoning): measure the performance impact of this.
	if (AclWriteAuthAssignmentRequests.ContainsByPredicate([EntityId](const WriteAuthAssignmentRequest& Request) { return Request.EntityId == EntityId; }))
	{
		UE_LOG(LogSpatialLoadBalanceEnforcer, Log, TEXT("An ACL assignment request already exists for entity %lld on worker %s."), EntityId, *WorkerId);
	}
	else
	{
		UE_LOG(LogSpatialLoadBalanceEnforcer, Log, TEXT("Queueing ACL assignment request for entity %lld on worker %s."), EntityId, *WorkerId);
		AclWriteAuthAssignmentRequests.Add(WriteAuthAssignmentRequest(EntityId));
	}
}

void USpatialLoadBalanceEnforcer::ProcessQueuedAclAssignmentRequests()
{
	TArray<Worker_EntityId> CompletedRequests;
	CompletedRequests.Reserve(AclWriteAuthAssignmentRequests.Num());

	for (WriteAuthAssignmentRequest& Request : AclWriteAuthAssignmentRequests)
	{
 		const AuthorityIntent* AuthorityIntentComponent = StaticComponentView->GetComponentData<SpatialGDK::AuthorityIntent>(Request.EntityId);
		if (AuthorityIntentComponent == nullptr)
		{
			// TODO(zoning): This is possible if the ACL update lags behind the intent update. Evaluate whether this is a serious problem.
			UE_LOG(LogSpatialLoadBalanceEnforcer, Warning, TEXT("(%s) Entity without AuthIntent component will not be processed. EntityId: %lld"), *WorkerId, Request.EntityId);
			CompletedRequests.Add(Request.EntityId);
			return;
		}

		const FString* OwningWorkerId = VirtualWorkerTranslator->GetPhysicalWorkerForVirtualWorker(AuthorityIntentComponent->VirtualWorkerId);
		if (OwningWorkerId == nullptr)
		{
			const int32 WarnOnAttemptNum = 5;
			Request.ProcessAttempts++;
			if (Request.ProcessAttempts % WarnOnAttemptNum == 0)
			{
				UE_LOG(LogSpatialLoadBalanceEnforcer, Warning, TEXT("Failed to process WriteAuthAssignmentRequest because virtual worker mapping is unset for EntityID: %lld. Process attempts made: %d"), Request.EntityId, Request.ProcessAttempts);
			}
			// A virtual worker -> physical worker mapping may not be established yet.
			// We'll retry on the next Tick().
			continue;
		}

		Sender->SetAclWriteAuthority(Request.EntityId, *OwningWorkerId);
		CompletedRequests.Add(Request.EntityId);
	}

	AclWriteAuthAssignmentRequests.RemoveAll([CompletedRequests](const WriteAuthAssignmentRequest& Request) { return CompletedRequests.Contains(Request.EntityId); });
}
