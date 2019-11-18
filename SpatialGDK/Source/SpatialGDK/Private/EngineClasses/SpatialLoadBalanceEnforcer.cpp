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

void USpatialLoadBalanceEnforcer::AuthorityChanged(const Worker_AuthorityChangeOp& AuthOp)
{
	if (AuthOp.component_id == SpatialConstants::ENTITY_ACL_COMPONENT_ID &&
		AuthOp.authority == WORKER_AUTHORITY_AUTHORITATIVE)
	{
		QueueAclAssignmentRequest(AuthOp.entity_id);
	}
}

void USpatialLoadBalanceEnforcer::QueueAclAssignmentRequest(const Worker_EntityId EntityId)
{
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
		static const int32 WarnOnAttemptNum = 5;
		if (Request.ProcessAttempts >= WarnOnAttemptNum)
		{
			UE_LOG(LogSpatialLoadBalanceEnforcer, Warning, TEXT("Failed to process WriteAuthAssignmentRequest with EntityID: %lld. Process attempts made: %d"), Request.EntityId, Request.ProcessAttempts);
		}

		Request.ProcessAttempts++;

		// TODO - if some entities won't have the component we should detect that before queueing the request.
		// Need to be certain it is invalid to get here before receiving the AuthIntentComponent for an entity, then we can check() on it.
		const AuthorityIntent* AuthorityIntentComponent = StaticComponentView->GetComponentData<SpatialGDK::AuthorityIntent>(Request.EntityId);
		if (AuthorityIntentComponent == nullptr)
		{
			UE_LOG(LogSpatialLoadBalanceEnforcer, Warning, TEXT("Detected entity without AuthIntent component. EntityId: %lld"), Request.EntityId);
			continue;
		}

		const FString* OwningWorkerId = VirtualWorkerTranslator->GetPhysicalWorkerForVirtualWorker(AuthorityIntentComponent->VirtualWorkerId);
		if (OwningWorkerId == nullptr)
		{
			// A virtual worker -> physical worker mapping may not be established yet.
			// We'll retry on the next Tick().
			continue;
		}

		Sender->SetAclWriteAuthority(Request.EntityId, *OwningWorkerId);
		CompletedRequests.Add(Request.EntityId);
	}

	AclWriteAuthAssignmentRequests.RemoveAll([CompletedRequests](const WriteAuthAssignmentRequest& Request) { return CompletedRequests.Contains(Request.EntityId); });
}
