// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "Tests/TestDefinitions.h"

#include "EngineClasses/SpatialLoadBalanceEnforcer.h"
#include "EngineClasses/SpatialVirtualWorkerTranslator.h"
#include "Interop/SpatialStaticComponentView.h"
#include "Schema/AuthorityIntent.h"
#include "SpatialGDKTests/SpatialGDK/LoadBalancing/AbstractLBStrategy/LBStrategyStub.h"
#include "Tests/TestingComponentViewHelpers.h"
#include "Tests/TestingSchemaHelpers.h"

#include "CoreMinimal.h"

#define LOADBALANCEENFORCER_TEST(TestName) \
	GDK_TEST(Core, SpatialLoadBalanceEnforcer, TestName)

// Test Globals
namespace
{

PhysicalWorkerName ValidWorkerOne = TEXT("ValidWorkerOne");
PhysicalWorkerName ValidWorkerTwo = TEXT("ValidWorkerTwo");

VirtualWorkerId VirtualWorkerOne = 1;
VirtualWorkerId VirtualWorkerTwo = 2;

Worker_EntityId EntityIdOne = 0;
Worker_EntityId EntityIdTwo = 1;

void AddEntityToStaticComponentView(USpatialStaticComponentView& StaticComponentView,
	const Worker_EntityId EntityId, VirtualWorkerId Id, Worker_Authority AuthorityIntentAuthority)
{
	TestingComponentViewHelpers::AddEntityComponentToStaticComponentView(StaticComponentView,
		EntityId, SpatialConstants::AUTHORITY_INTENT_COMPONENT_ID,
		AuthorityIntentAuthority);

	TestingComponentViewHelpers::AddEntityComponentToStaticComponentView(StaticComponentView,
		EntityId, SpatialConstants::ENTITY_ACL_COMPONENT_ID,
		WORKER_AUTHORITY_AUTHORITATIVE);

	if (Id != SpatialConstants::INVALID_VIRTUAL_WORKER_ID)
	{
		SpatialGDK::AuthorityIntent* AuthorityIntentComponent = StaticComponentView.GetComponentData<SpatialGDK::AuthorityIntent>(EntityId);
		AuthorityIntentComponent->VirtualWorkerId = Id;
	}
}

TUniquePtr<SpatialVirtualWorkerTranslator> CreateVirtualWorkerTranslator(bool bAddDefaultMapping = true)
{
	ULBStrategyStub* LoadBalanceStrategy = NewObject<ULBStrategyStub>();
	TUniquePtr<SpatialVirtualWorkerTranslator> VirtualWorkerTranslator = MakeUnique<SpatialVirtualWorkerTranslator>(LoadBalanceStrategy, ValidWorkerOne);

	Schema_Object* DataObject = TestingSchemaHelpers::CreateTranslationComponentDataFields();

	if (bAddDefaultMapping)
	{
		TestingSchemaHelpers::AddTranslationComponentDataMapping(DataObject, 1, ValidWorkerOne);
		TestingSchemaHelpers::AddTranslationComponentDataMapping(DataObject, 2, ValidWorkerTwo);
	}

	VirtualWorkerTranslator->ApplyVirtualWorkerManagerData(DataObject);

	return VirtualWorkerTranslator;
}

} // anonymous namespace

LOADBALANCEENFORCER_TEST(GIVEN_load_balance_enforcer_with_no_mapping_WHEN_asked_for_acl_assignments_THEN_return_no_acl_assignment_requests)
{
	// This will create a virtual worker translator with no virtual to physical workerId mapping.
	// This mean the load balance enforcer will not be able to find the physical workerId from entities AuthorityIntent components and therefore fail to produce ACL requests.
	TUniquePtr<SpatialVirtualWorkerTranslator>  VirtualWorkerTranslator = CreateVirtualWorkerTranslator(false);

	USpatialStaticComponentView* StaticComponentView = NewObject<USpatialStaticComponentView>();
	AddEntityToStaticComponentView(*StaticComponentView, EntityIdOne, VirtualWorkerOne, WORKER_AUTHORITY_AUTHORITATIVE);
	AddEntityToStaticComponentView(*StaticComponentView, EntityIdTwo, VirtualWorkerTwo, WORKER_AUTHORITY_AUTHORITATIVE);

	TUniquePtr<SpatialLoadBalanceEnforcer> LoadBalanceEnforcer = MakeUnique<SpatialLoadBalanceEnforcer>(ValidWorkerOne, StaticComponentView, VirtualWorkerTranslator.Get());

	LoadBalanceEnforcer->QueueAclAssignmentRequest(EntityIdOne);
	LoadBalanceEnforcer->QueueAclAssignmentRequest(EntityIdTwo);

	TArray<SpatialLoadBalanceEnforcer::AclWriteAuthorityRequest> ACLRequests = LoadBalanceEnforcer->ProcessQueuedAclAssignmentRequests();

	bool bSuccess = ACLRequests.Num() == 0;
	TestTrue("LoadBalanceEnforcer returned expected ACL assignment results", bSuccess);

	return true;
}

LOADBALANCEENFORCER_TEST(GIVEN_a_static_component_view_with_no_data_WHEN_asking_load_balance_enforcer_for_acl_assignments_THEN_return_no_acl_assignment_requests)
{
	TUniquePtr<SpatialVirtualWorkerTranslator>  VirtualWorkerTranslator = CreateVirtualWorkerTranslator();

	// Here we simply create a static component view but do not add any data to it.
	// This means that the load balance enforcer will not be able to find the virtual worker id associated with an entity and therefore fail to produce ACL requests.
	USpatialStaticComponentView* StaticComponentView = NewObject<USpatialStaticComponentView>();

	TUniquePtr<SpatialLoadBalanceEnforcer> LoadBalanceEnforcer = MakeUnique<SpatialLoadBalanceEnforcer>(ValidWorkerOne, StaticComponentView, VirtualWorkerTranslator.Get());

	LoadBalanceEnforcer->QueueAclAssignmentRequest(EntityIdOne);
	LoadBalanceEnforcer->QueueAclAssignmentRequest(EntityIdTwo);

	TArray<SpatialLoadBalanceEnforcer::AclWriteAuthorityRequest> ACLRequests = LoadBalanceEnforcer->ProcessQueuedAclAssignmentRequests();

	bool bSuccess = ACLRequests.Num() == 0;
	TestTrue("LoadBalanceEnforcer returned expected ACL assignment results", bSuccess);

	return true;
}

LOADBALANCEENFORCER_TEST(GIVEN_a_static_component_view_with_uninitialised_authority_intent_component_WHEN_asking_load_balance_enforcer_for_acl_assignments_THEN_return_no_acl_assignment_requests)
{
	TUniquePtr<SpatialVirtualWorkerTranslator>  VirtualWorkerTranslator = CreateVirtualWorkerTranslator();

	// Here we create a static component view and add entities to it but do not assign the AuthorityIntent virtual worker id.
	// This means that the load balance enforcer will not be able to find the physical worker id associated with an entity and therefore fail to produce ACL requests.
	USpatialStaticComponentView* StaticComponentView = NewObject<USpatialStaticComponentView>();
	AddEntityToStaticComponentView(*StaticComponentView, EntityIdOne, SpatialConstants::INVALID_VIRTUAL_WORKER_ID, WORKER_AUTHORITY_AUTHORITATIVE);
	AddEntityToStaticComponentView(*StaticComponentView, EntityIdTwo, SpatialConstants::INVALID_VIRTUAL_WORKER_ID, WORKER_AUTHORITY_AUTHORITATIVE);

	TUniquePtr<SpatialLoadBalanceEnforcer> LoadBalanceEnforcer = MakeUnique<SpatialLoadBalanceEnforcer>(ValidWorkerOne, StaticComponentView, VirtualWorkerTranslator.Get());

	LoadBalanceEnforcer->QueueAclAssignmentRequest(EntityIdOne);
	LoadBalanceEnforcer->QueueAclAssignmentRequest(EntityIdTwo);

	TArray<SpatialLoadBalanceEnforcer::AclWriteAuthorityRequest> ACLRequests = LoadBalanceEnforcer->ProcessQueuedAclAssignmentRequests();

	bool bSuccess = ACLRequests.Num() == 0;
	TestTrue("LoadBalanceEnforcer returned expected ACL assignment results", bSuccess);

	return true;
}

LOADBALANCEENFORCER_TEST(GIVEN_load_balance_enforcer_with_valid_mapping_WHEN_asked_for_acl_assignments_THEN_return_correct_acl_assignment_requests)
{
	TUniquePtr<SpatialVirtualWorkerTranslator>  VirtualWorkerTranslator = CreateVirtualWorkerTranslator();

	USpatialStaticComponentView* StaticComponentView = NewObject<USpatialStaticComponentView>();
	AddEntityToStaticComponentView(*StaticComponentView, EntityIdOne, VirtualWorkerOne, WORKER_AUTHORITY_AUTHORITATIVE);
	AddEntityToStaticComponentView(*StaticComponentView, EntityIdTwo, VirtualWorkerTwo, WORKER_AUTHORITY_AUTHORITATIVE);

	TUniquePtr<SpatialLoadBalanceEnforcer> LoadBalanceEnforcer = MakeUnique<SpatialLoadBalanceEnforcer>(ValidWorkerOne, StaticComponentView, VirtualWorkerTranslator.Get());

	LoadBalanceEnforcer->QueueAclAssignmentRequest(EntityIdOne);
	LoadBalanceEnforcer->QueueAclAssignmentRequest(EntityIdTwo);

	TArray<SpatialLoadBalanceEnforcer::AclWriteAuthorityRequest> ACLRequests = LoadBalanceEnforcer->ProcessQueuedAclAssignmentRequests();

	bool bSuccess = true;
	if (ACLRequests.Num() == 2)
	{
		bSuccess &= ACLRequests[0].EntityId == EntityIdOne;
		bSuccess &= ACLRequests[0].OwningWorkerId == ValidWorkerOne;
		bSuccess &= ACLRequests[1].EntityId == EntityIdTwo;
		bSuccess &= ACLRequests[1].OwningWorkerId == ValidWorkerTwo;
	}
	else
	{
		bSuccess = false;
	}

	TestTrue("LoadBalanceEnforcer returned expected ACL assignment results", bSuccess);

	return true;
}

LOADBALANCEENFORCER_TEST(GIVEN_authority_intent_change_op_WHEN_we_inform_load_balance_enforcer_THEN_queue_authority_request)
{
	TUniquePtr<SpatialVirtualWorkerTranslator> VirtualWorkerTranslator = CreateVirtualWorkerTranslator();

	USpatialStaticComponentView* StaticComponentView = NewObject<USpatialStaticComponentView>();
	AddEntityToStaticComponentView(*StaticComponentView, EntityIdOne, VirtualWorkerOne, WORKER_AUTHORITY_AUTHORITATIVE);

	TUniquePtr<SpatialLoadBalanceEnforcer> LoadBalanceEnforcer = MakeUnique<SpatialLoadBalanceEnforcer>(ValidWorkerOne, StaticComponentView, VirtualWorkerTranslator.Get());

	Worker_ComponentUpdateOp UpdateOp;
	UpdateOp.entity_id = EntityIdOne;
	UpdateOp.update.component_id = SpatialConstants::AUTHORITY_INTENT_COMPONENT_ID;

	LoadBalanceEnforcer->OnAuthorityIntentComponentUpdated(UpdateOp);

	TArray<SpatialLoadBalanceEnforcer::AclWriteAuthorityRequest> ACLRequests = LoadBalanceEnforcer->ProcessQueuedAclAssignmentRequests();

	bool bSuccess = true;
	if (ACLRequests.Num() == 1)
	{
		bSuccess &= ACLRequests[0].EntityId == EntityIdOne;
		bSuccess &= ACLRequests[0].OwningWorkerId == ValidWorkerOne;
	}
	else
	{
		bSuccess = false;
	}

	TestTrue("LoadBalanceEnforcer returned expected ACL assignment results", bSuccess);

	return true;
}

LOADBALANCEENFORCER_TEST(GIVEN_authority_change_when_not_authoritative_over_authority_intent_component_WHEN_we_inform_load_balance_enforcer_THEN_queue_authority_request)
{
	TUniquePtr<SpatialVirtualWorkerTranslator>  VirtualWorkerTranslator = CreateVirtualWorkerTranslator();

	// The important part of this test is that the work does not already have authority over the AuthorityIntent component.
		// In this case, we the load balance enforcer needs to create an ACL request.
	USpatialStaticComponentView* StaticComponentView = NewObject<USpatialStaticComponentView>();
	AddEntityToStaticComponentView(*StaticComponentView, EntityIdOne, VirtualWorkerOne, WORKER_AUTHORITY_NOT_AUTHORITATIVE);

	TUniquePtr<SpatialLoadBalanceEnforcer> LoadBalanceEnforcer = MakeUnique<SpatialLoadBalanceEnforcer>(ValidWorkerOne, StaticComponentView, VirtualWorkerTranslator.Get());

	Worker_AuthorityChangeOp UpdateOp;
	UpdateOp.entity_id = EntityIdOne;
	UpdateOp.authority = WORKER_AUTHORITY_AUTHORITATIVE;
	UpdateOp.component_id = SpatialConstants::ENTITY_ACL_COMPONENT_ID;

	LoadBalanceEnforcer->AuthorityChanged(UpdateOp);

	TArray<SpatialLoadBalanceEnforcer::AclWriteAuthorityRequest> ACLRequests = LoadBalanceEnforcer->ProcessQueuedAclAssignmentRequests();

	bool bSuccess = true;
	if (ACLRequests.Num() == 1)
	{
		bSuccess &= ACLRequests[0].EntityId == EntityIdOne;
		bSuccess &= ACLRequests[0].OwningWorkerId == ValidWorkerOne;
	}
	else
	{
		bSuccess = false;
	}

	TestTrue("LoadBalanceEnforcer returned expected ACL assignment results", bSuccess);

	return true;
}

LOADBALANCEENFORCER_TEST(GIVEN_authority_change_when_authoritative_over_authority_intent_component_WHEN_we_inform_load_balance_enforcer_THEN_return_no_acl_assignment_requests)
{
	TUniquePtr<SpatialVirtualWorkerTranslator> VirtualWorkerTranslator = CreateVirtualWorkerTranslator();

	// The important part of this test is that the work does already have authority over the AuthorityIntent component.
	// In this case, we the load balance enforcer does not need to create an ACL request.
	USpatialStaticComponentView* StaticComponentView = NewObject<USpatialStaticComponentView>();
	AddEntityToStaticComponentView(*StaticComponentView, EntityIdOne, VirtualWorkerOne, WORKER_AUTHORITY_AUTHORITATIVE);

	TUniquePtr<SpatialLoadBalanceEnforcer> LoadBalanceEnforcer = MakeUnique<SpatialLoadBalanceEnforcer>(ValidWorkerOne, StaticComponentView, VirtualWorkerTranslator.Get());

	Worker_AuthorityChangeOp UpdateOp;
	UpdateOp.entity_id = EntityIdOne;
	UpdateOp.authority = WORKER_AUTHORITY_AUTHORITATIVE;
	UpdateOp.component_id = SpatialConstants::ENTITY_ACL_COMPONENT_ID;

	LoadBalanceEnforcer->AuthorityChanged(UpdateOp);

	TArray<SpatialLoadBalanceEnforcer::AclWriteAuthorityRequest> ACLRequests = LoadBalanceEnforcer->ProcessQueuedAclAssignmentRequests();

	bool bSuccess = ACLRequests.Num() == 0;
	TestTrue("LoadBalanceEnforcer returned expected ACL assignment results", bSuccess);

	return true;
}
