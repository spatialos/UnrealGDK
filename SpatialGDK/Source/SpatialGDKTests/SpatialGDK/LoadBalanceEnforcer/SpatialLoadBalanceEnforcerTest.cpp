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

void AddEntityToStaticComponentView(USpatialStaticComponentView& StaticComponentView,
	const Worker_EntityId EntityId, VirtualWorkerId Id)
{
	TestingComponentViewHelpers::AddEntityComponentToStaticComponentView(StaticComponentView,
		EntityId, SpatialConstants::AUTHORITY_INTENT_COMPONENT_ID,
		WORKER_AUTHORITY_AUTHORITATIVE);

	TestingComponentViewHelpers::AddEntityComponentToStaticComponentView(StaticComponentView,
		EntityId, SpatialConstants::ENTITY_ACL_COMPONENT_ID,
		WORKER_AUTHORITY_AUTHORITATIVE);

	SpatialGDK::AuthorityIntent* AuthorityIntentComponent = StaticComponentView.GetComponentData<SpatialGDK::AuthorityIntent>(EntityId);
	AuthorityIntentComponent->VirtualWorkerId = Id;
}

} // anonymous namespace

LOADBALANCEENFORCER_TEST(GIVEN_load_balance_enforcer_with_no_mapping_WHEN_asked_for_acl_assignments_THEN_return_no_acl_assignment_requests)
{
	ULBStrategyStub* LoadBalanceStrategy = NewObject<ULBStrategyStub>();
	TUniquePtr<SpatialVirtualWorkerTranslator> VirtualWorkerTranslator = MakeUnique<SpatialVirtualWorkerTranslator>(LoadBalanceStrategy, ValidWorkerOne);

	Schema_Object* DataObject = TestingSchemaHelpers::CreateTranslationComponentDataFields();
	VirtualWorkerTranslator->ApplyVirtualWorkerManagerData(DataObject);

	USpatialStaticComponentView* StaticComponentView = NewObject<USpatialStaticComponentView>();
	AddEntityToStaticComponentView(*StaticComponentView, 0, 1);
	AddEntityToStaticComponentView(*StaticComponentView, 1, 2);
	AddEntityToStaticComponentView(*StaticComponentView, 2, 1);

	TUniquePtr<SpatialLoadBalanceEnforcer> LoadBalanceEnforcer = MakeUnique<SpatialLoadBalanceEnforcer>(ValidWorkerOne, StaticComponentView, VirtualWorkerTranslator.Get());

	LoadBalanceEnforcer->QueueAclAssignmentRequest(0);
	LoadBalanceEnforcer->QueueAclAssignmentRequest(1);
	LoadBalanceEnforcer->QueueAclAssignmentRequest(2);

	TArray<SpatialLoadBalanceEnforcer::AclWriteAuthorityRequest> ACLRequests = LoadBalanceEnforcer->ProcessQueuedAclAssignmentRequests();

	bool bSuccess = ACLRequests.Num() == 0;
	TestTrue("LoadBalanceEnforcer returned expected ACL assignment results", bSuccess);

	return true;
}

LOADBALANCEENFORCER_TEST(GIVEN_load_balance_enforcer_with_valid_mapping_WHEN_asked_for_acl_assignments_THEN_return_correct_acl_assignment_requests)
{
	ULBStrategyStub* LoadBalanceStrategy= NewObject<ULBStrategyStub>();
	TUniquePtr<SpatialVirtualWorkerTranslator> VirtualWorkerTranslator = MakeUnique<SpatialVirtualWorkerTranslator>(LoadBalanceStrategy, ValidWorkerOne);

	Schema_Object* DataObject = TestingSchemaHelpers::CreateTranslationComponentDataFields();
	TestingSchemaHelpers::AddTranslationComponentDataMapping(DataObject, 1, ValidWorkerOne);
	TestingSchemaHelpers::AddTranslationComponentDataMapping(DataObject, 2, ValidWorkerTwo);

	VirtualWorkerTranslator->ApplyVirtualWorkerManagerData(DataObject);

	USpatialStaticComponentView* StaticComponentView = NewObject<USpatialStaticComponentView>();
	AddEntityToStaticComponentView(*StaticComponentView, 0, 1);
	AddEntityToStaticComponentView(*StaticComponentView, 1, 2);
	AddEntityToStaticComponentView(*StaticComponentView, 2, 1);

	TUniquePtr<SpatialLoadBalanceEnforcer> LoadBalanceEnforcer = MakeUnique<SpatialLoadBalanceEnforcer>(ValidWorkerOne, StaticComponentView, VirtualWorkerTranslator.Get());

	LoadBalanceEnforcer->QueueAclAssignmentRequest(0);
	LoadBalanceEnforcer->QueueAclAssignmentRequest(1);
	LoadBalanceEnforcer->QueueAclAssignmentRequest(2);

	TArray<SpatialLoadBalanceEnforcer::AclWriteAuthorityRequest> ACLRequests = LoadBalanceEnforcer->ProcessQueuedAclAssignmentRequests();

	bool bSuccess = true;
	if (ACLRequests.Num() == 3)
	{
		bSuccess &= ACLRequests[0].EntityId == 0;
		bSuccess &= ACLRequests[0].OwningWorkerId == ValidWorkerOne;
		bSuccess &= ACLRequests[1].EntityId == 1;
		bSuccess &= ACLRequests[1].OwningWorkerId == ValidWorkerTwo;
		bSuccess &= ACLRequests[2].EntityId == 2;
		bSuccess &= ACLRequests[2].OwningWorkerId == ValidWorkerOne;
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
	ULBStrategyStub* LoadBalanceStrategy = NewObject<ULBStrategyStub>();
	TUniquePtr<SpatialVirtualWorkerTranslator> VirtualWorkerTranslator = MakeUnique<SpatialVirtualWorkerTranslator>(LoadBalanceStrategy, ValidWorkerOne);

	Schema_Object* DataObject = TestingSchemaHelpers::CreateTranslationComponentDataFields();
	TestingSchemaHelpers::AddTranslationComponentDataMapping(DataObject, 1, ValidWorkerOne);
	TestingSchemaHelpers::AddTranslationComponentDataMapping(DataObject, 2, ValidWorkerTwo);

	VirtualWorkerTranslator->ApplyVirtualWorkerManagerData(DataObject);

	USpatialStaticComponentView* StaticComponentView = NewObject<USpatialStaticComponentView>();
	AddEntityToStaticComponentView(*StaticComponentView, 0, 1);

	TUniquePtr<SpatialLoadBalanceEnforcer> LoadBalanceEnforcer = MakeUnique<SpatialLoadBalanceEnforcer>(ValidWorkerOne, StaticComponentView, VirtualWorkerTranslator.Get());

	Worker_ComponentUpdateOp Update;
	Update.entity_id = 0;
	Update.update = ;

	LoadBalanceEnforcer->OnAuthorityIntentComponentUpdated(Update);

	TArray<SpatialLoadBalanceEnforcer::AclWriteAuthorityRequest> ACLRequests = LoadBalanceEnforcer->ProcessQueuedAclAssignmentRequests();

	bool bSuccess = true;
	if (ACLRequests.Num() == 1)
	{
		bSuccess &= ACLRequests[0].EntityId == 0;
		bSuccess &= ACLRequests[0].OwningWorkerId == ValidWorkerOne;
	}
	else
	{
		bSuccess = false;
	}

	TestTrue("LoadBalanceEnforcer returned expected ACL assignment results", bSuccess);

	return true;
}
