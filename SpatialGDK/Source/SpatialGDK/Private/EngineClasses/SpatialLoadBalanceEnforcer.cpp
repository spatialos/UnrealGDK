// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "EngineClasses/SpatialLoadBalanceEnforcer.h"
#include "EngineClasses/SpatialVirtualWorkerTranslator.h"
#include "EngineClasses/SpatialNetDriver.h"
#include "Interop/Connection/SpatialWorkerConnection.h"
#include "Interop/SpatialSender.h"
#include "Interop/SpatialStaticComponentView.h"
#include "Schema/AuthorityIntent.h"
#include "SpatialCommonTypes.h"

DEFINE_LOG_CATEGORY(LogSpatialLoadBalanceEnforcer);

using namespace SpatialGDK;

USpatialLoadBalanceEnforcer::USpatialLoadBalanceEnforcer(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, NetDriver(nullptr)
	, Sender(nullptr)
	, VirtualWorkerTranslator(nullptr)
{
}

void USpatialLoadBalanceEnforcer::Init(USpatialNetDriver* InNetDriver,
	USpatialSender* InUpatialSender,
	SpatialVirtualWorkerTranslator* InVirtualWorkerTranslator)
{
	NetDriver = InNetDriver;
	Sender = InUpatialSender;
	VirtualWorkerTranslator = InVirtualWorkerTranslator;
}

void USpatialLoadBalanceEnforcer::Tick()
{
	ProcessQueuedAclAssignmentRequests();
}

void USpatialLoadBalanceEnforcer::OnComponentUpdated(const Worker_ComponentUpdateOp& Op)
{
	check(NetDriver != nullptr)
	check(NetDriver->StaticComponentView != nullptr)
	if (Op.update.component_id == SpatialConstants::AUTHORITY_INTENT_COMPONENT_ID &&
		NetDriver->StaticComponentView->GetAuthority(Op.entity_id, SpatialConstants::ENTITY_ACL_COMPONENT_ID) == WORKER_AUTHORITY_AUTHORITATIVE)
	{
		QueueAclAssignmentRequest(Op.entity_id);
	}
}

void USpatialLoadBalanceEnforcer::AuthorityChanged(const Worker_AuthorityChangeOp& AuthOp)
{
	if (AuthOp.component_id == SpatialConstants::ENTITY_ACL_COMPONENT_ID &&
		AuthOp.authority == WORKER_AUTHORITY_AUTHORITATIVE)
	{
		// We have gained authority over the ACL component for some entity
		// If we have authority, the responsibility here is to set the EntityACL write auth to match the worker requested via the virtual worker component
		QueueAclAssignmentRequest(AuthOp.entity_id);
	}
}

void USpatialLoadBalanceEnforcer::QueueAclAssignmentRequest(const Worker_EntityId EntityId)
{
	if (AclWriteAuthAssignmentRequests.ContainsByPredicate([EntityId](const WriteAuthAssignmentRequest& Request) { return Request.EntityId == EntityId; }))
	{
		UE_LOG(LogSpatialLoadBalanceEnforcer, Log, TEXT("(%s) Already has an ACL authority request for entity %lld"), *NetDriver->Connection->GetWorkerId(), EntityId);
	}
	else
	{
		UE_LOG(LogSpatialLoadBalanceEnforcer, Log, TEXT("(%s) Queueing ACL assignment request for %lld"), *NetDriver->Connection->GetWorkerId(), EntityId);
		AclWriteAuthAssignmentRequests.Add(WriteAuthAssignmentRequest(EntityId));
	}
}

void USpatialLoadBalanceEnforcer::ProcessQueuedAclAssignmentRequests()
{
	TArray<Worker_EntityId> CompletedRequests;
	CompletedRequests.Reserve(AclWriteAuthAssignmentRequests.Num());

	for (WriteAuthAssignmentRequest& Request : AclWriteAuthAssignmentRequests)
	{
		static const int16 MaxNumProcessAttempts = 5;
		if (Request.ProcessAttempts >= MaxNumProcessAttempts)
		{
			UE_LOG(LogSpatialLoadBalanceEnforcer, Log, TEXT("Failed to process WriteAuthAssignmentRequest with EntityID: %lld. Process attempts made: %d"), Request.EntityId, Request.ProcessAttempts);
		}

		Request.ProcessAttempts++;

		// TODO - if some entities won't have the component we should detect that before queueing the request.
		// Need to be certain it is invalid to get here before receiving the AuthIntentComponent for an entity, then we can check() on it.
		const AuthorityIntent* AuthorityIntentComponent = NetDriver->StaticComponentView->GetComponentData<SpatialGDK::AuthorityIntent>(Request.EntityId);
		if (AuthorityIntentComponent == nullptr)
		{
			UE_LOG(LogSpatialLoadBalanceEnforcer, Log, TEXT("Detected entity without AuthIntent component. EntityId: %lld"), Request.EntityId);
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
