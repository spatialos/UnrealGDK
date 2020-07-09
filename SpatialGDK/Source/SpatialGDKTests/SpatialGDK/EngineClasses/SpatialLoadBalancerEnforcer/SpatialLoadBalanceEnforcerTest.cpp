// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "Tests/TestDefinitions.h"

#include "EngineClasses/SpatialLoadBalanceEnforcer.h"
#include "EngineClasses/SpatialVirtualWorkerTranslator.h"
#include "Interop/SpatialStaticComponentView.h"
#include "Schema/AuthorityIntent.h"
#include "Schema/ComponentPresence.h"
#include "Schema/NetOwningClientWorker.h"
#include "Schema/StandardLibrary.h"
#include "SpatialConstants.h"
#include "SpatialGDKSettings.h"
#include "SpatialGDKTests/SpatialGDK/LoadBalancing/AbstractLBStrategy/LBStrategyStub.h"
#include "Tests/TestingComponentViewHelpers.h"
#include "Tests/TestingSchemaHelpers.h"

#include "CoreMinimal.h"

#define LOADBALANCEENFORCER_TEST(TestName) \
	GDK_TEST(Core, SpatialLoadBalanceEnforcer, TestName)

// Test Globals
namespace
{

const PhysicalWorkerName ValidWorkerOne = TEXT("ValidWorkerOne");
const PhysicalWorkerName ValidWorkerTwo = TEXT("ValidWorkerTwo");

constexpr VirtualWorkerId VirtualWorkerOne = 1;
constexpr VirtualWorkerId VirtualWorkerTwo = 2;

constexpr Worker_EntityId EntityIdOne = 1;
constexpr Worker_EntityId EntityIdTwo = 2;

constexpr Worker_ComponentId TestComponentIdOne = 123;
constexpr Worker_ComponentId TestComponentIdTwo = 456;

void AddEntityToStaticComponentView(USpatialStaticComponentView& StaticComponentView,
	const Worker_EntityId EntityId, VirtualWorkerId Id, Worker_Authority AuthorityIntentAuthority
	//, PhysicalWorkerName ClientWorkerId
)
{
	TestingComponentViewHelpers::AddEntityComponentToStaticComponentView(StaticComponentView,
		EntityId, SpatialConstants::AUTHORITY_INTENT_COMPONENT_ID,
		AuthorityIntentAuthority);

	const Worker_ComponentId ClientRpcAuthComponent = SpatialConstants::GetClientAuthorityComponent(GetDefault<USpatialGDKSettings>()->UseRPCRingBuffer());

	// When queue authority changes, we verify whether a change needs to be applied based on existing load balancing
	// data, so we need to setup something here.
	WriteAclMap WriteAcl;
	WriteAcl.Add(SpatialConstants::AUTHORITY_INTENT_COMPONENT_ID, { { TEXT("") } });
	WriteAcl.Add(ClientRpcAuthComponent, { { TEXT("") } });
	Schema_ComponentData* EntityAclData = SpatialGDK::EntityAcl({}, WriteAcl).CreateEntityAclData().schema_type;
	TestingComponentViewHelpers::AddEntityComponentToStaticComponentView(StaticComponentView,
		EntityId, SpatialConstants::ENTITY_ACL_COMPONENT_ID, EntityAclData,
		WORKER_AUTHORITY_AUTHORITATIVE);

	TestingComponentViewHelpers::AddEntityComponentToStaticComponentView(StaticComponentView,
		EntityId, SpatialConstants::COMPONENT_PRESENCE_COMPONENT_ID,
		AuthorityIntentAuthority);

	TestingComponentViewHelpers::AddEntityComponentToStaticComponentView(StaticComponentView,
		EntityId, SpatialConstants::NET_OWNING_CLIENT_WORKER_COMPONENT_ID,
		AuthorityIntentAuthority);

	if (Id != SpatialConstants::INVALID_VIRTUAL_WORKER_ID)
	{
		StaticComponentView.GetComponentData<SpatialGDK::AuthorityIntent>(EntityId)->VirtualWorkerId = Id;
	}
}

TUniquePtr<SpatialVirtualWorkerTranslator> CreateVirtualWorkerTranslator()
{
	ULBStrategyStub* LoadBalanceStrategy = NewObject<ULBStrategyStub>();
	TUniquePtr<SpatialVirtualWorkerTranslator> VirtualWorkerTranslator = MakeUnique<SpatialVirtualWorkerTranslator>(LoadBalanceStrategy, ValidWorkerOne);

	Schema_Object* DataObject = TestingSchemaHelpers::CreateTranslationComponentDataFields();

	TestingSchemaHelpers::AddTranslationComponentDataMapping(DataObject, VirtualWorkerOne, ValidWorkerOne);
	TestingSchemaHelpers::AddTranslationComponentDataMapping(DataObject, VirtualWorkerTwo, ValidWorkerTwo);

	VirtualWorkerTranslator->ApplyVirtualWorkerManagerData(DataObject);

	return VirtualWorkerTranslator;
}

} // anonymous namespace

LOADBALANCEENFORCER_TEST(GIVEN_a_static_component_view_with_no_data_WHEN_asking_load_balance_enforcer_for_authority_changes_THEN_return_no_authority_change_requests)
{
	const TUniquePtr<SpatialVirtualWorkerTranslator> VirtualWorkerTranslator = CreateVirtualWorkerTranslator();

	// Here we simply create a static component view but do not add any data to it.
	// This means that the load balance enforcer will not be able to find the virtual worker id associated with an entity and therefore fail to produce ACL requests.
	USpatialStaticComponentView* StaticComponentView = NewObject<USpatialStaticComponentView>();

	TUniquePtr<SpatialLoadBalanceEnforcer> LoadBalanceEnforcer = MakeUnique<SpatialLoadBalanceEnforcer>(ValidWorkerOne, StaticComponentView, VirtualWorkerTranslator.Get());

	LoadBalanceEnforcer->MaybeQueueAuthorityChange(EntityIdOne);
	LoadBalanceEnforcer->MaybeQueueAuthorityChange(EntityIdTwo);

	TArray<SpatialLoadBalanceEnforcer::AuthorityStateChange> AuthorityChangeRequests = LoadBalanceEnforcer->ProcessAuthorityChangeRequests();

	bool bSuccess = AuthorityChangeRequests.Num() == 0;

	// Now add components to the StaticComponentView and retry getting the ACL requests.
	AddEntityToStaticComponentView(*StaticComponentView, EntityIdOne, VirtualWorkerOne, WORKER_AUTHORITY_NOT_AUTHORITATIVE);
	AddEntityToStaticComponentView(*StaticComponentView, EntityIdTwo, VirtualWorkerTwo, WORKER_AUTHORITY_NOT_AUTHORITATIVE);

	AuthorityChangeRequests.Empty();
	AuthorityChangeRequests = LoadBalanceEnforcer->ProcessAuthorityChangeRequests();

	bSuccess &= AuthorityChangeRequests.Num() == 0;

	TestTrue("LoadBalanceEnforcer returned expected ACL assignment results", bSuccess);

	return true;
}

LOADBALANCEENFORCER_TEST(GIVEN_load_balance_enforcer_with_valid_mapping_WHEN_asked_for_authority_changes_THEN_return_correct_authority_change_requests)
{
	const TUniquePtr<SpatialVirtualWorkerTranslator> VirtualWorkerTranslator = CreateVirtualWorkerTranslator();

	USpatialStaticComponentView* StaticComponentView = NewObject<USpatialStaticComponentView>();
	AddEntityToStaticComponentView(*StaticComponentView, EntityIdOne, VirtualWorkerOne, WORKER_AUTHORITY_NOT_AUTHORITATIVE);
	AddEntityToStaticComponentView(*StaticComponentView, EntityIdTwo, VirtualWorkerTwo, WORKER_AUTHORITY_NOT_AUTHORITATIVE);

	TUniquePtr<SpatialLoadBalanceEnforcer> LoadBalanceEnforcer = MakeUnique<SpatialLoadBalanceEnforcer>(ValidWorkerOne, StaticComponentView, VirtualWorkerTranslator.Get());

	LoadBalanceEnforcer->MaybeQueueAuthorityChange(EntityIdOne);
	LoadBalanceEnforcer->MaybeQueueAuthorityChange(EntityIdTwo);

	TArray<SpatialLoadBalanceEnforcer::AuthorityStateChange> AuthorityChangeRequests = LoadBalanceEnforcer->ProcessAuthorityChangeRequests();

	bool bSuccess = true;
	if (AuthorityChangeRequests.Num() == 2)
	{
		bSuccess &= AuthorityChangeRequests[0].EntityId == EntityIdOne;
		bSuccess &= AuthorityChangeRequests[0].TargetVirtualWorker == VirtualWorkerOne;
		bSuccess &= AuthorityChangeRequests[1].EntityId == EntityIdTwo;
		bSuccess &= AuthorityChangeRequests[1].TargetVirtualWorker == VirtualWorkerTwo;
	}
	else
	{
		bSuccess = false;
	}

	TestTrue("LoadBalanceEnforcer returned expected ACL assignment results", bSuccess);

	return true;
}

LOADBALANCEENFORCER_TEST(GIVEN_load_balance_enforcer_with_valid_mapping_WHEN_queueing_two_authority_requests_for_the_same_entity_THEN_return_one_authority_change_request_for_that_entity)
{
	const TUniquePtr<SpatialVirtualWorkerTranslator> VirtualWorkerTranslator = CreateVirtualWorkerTranslator();

	USpatialStaticComponentView* StaticComponentView = NewObject<USpatialStaticComponentView>();
	AddEntityToStaticComponentView(*StaticComponentView, EntityIdOne, VirtualWorkerOne, WORKER_AUTHORITY_NOT_AUTHORITATIVE);

	TUniquePtr<SpatialLoadBalanceEnforcer> LoadBalanceEnforcer = MakeUnique<SpatialLoadBalanceEnforcer>(ValidWorkerOne, StaticComponentView, VirtualWorkerTranslator.Get());

	LoadBalanceEnforcer->MaybeQueueAuthorityChange(EntityIdOne);
	LoadBalanceEnforcer->MaybeQueueAuthorityChange(EntityIdOne);

	TArray<SpatialLoadBalanceEnforcer::AuthorityStateChange> AuthorityChangeRequests = LoadBalanceEnforcer->ProcessAuthorityChangeRequests();

	bool bSuccess = true;
	if (AuthorityChangeRequests.Num() == 1)
	{
		bSuccess &= AuthorityChangeRequests[0].EntityId == EntityIdOne;
		bSuccess &= AuthorityChangeRequests[0].TargetVirtualWorker == VirtualWorkerOne;
	}
	else
	{
		bSuccess = false;
	}

	TestTrue("LoadBalanceEnforcer returned expected authority change results", bSuccess);

	return true;
}

LOADBALANCEENFORCER_TEST(GIVEN_authority_intent_change_op_WHEN_we_inform_load_balance_enforcer_THEN_queue_authority_request)
{
	const TUniquePtr<SpatialVirtualWorkerTranslator> VirtualWorkerTranslator = CreateVirtualWorkerTranslator();

	USpatialStaticComponentView* StaticComponentView = NewObject<USpatialStaticComponentView>();
	AddEntityToStaticComponentView(*StaticComponentView, EntityIdOne, VirtualWorkerOne, WORKER_AUTHORITY_NOT_AUTHORITATIVE);

	TUniquePtr<SpatialLoadBalanceEnforcer> LoadBalanceEnforcer = MakeUnique<SpatialLoadBalanceEnforcer>(ValidWorkerOne, StaticComponentView, VirtualWorkerTranslator.Get());

	Worker_ComponentUpdateOp UpdateOp;
	UpdateOp.entity_id = EntityIdOne;
	UpdateOp.update.component_id = SpatialConstants::AUTHORITY_INTENT_COMPONENT_ID;

	LoadBalanceEnforcer->OnLoadBalancingComponentUpdated(UpdateOp);

	TArray<SpatialLoadBalanceEnforcer::AuthorityStateChange> AuthorityChangeRequests = LoadBalanceEnforcer->ProcessAuthorityChangeRequests();

	bool bSuccess = true;
	if (AuthorityChangeRequests.Num() == 1)
	{
		bSuccess &= AuthorityChangeRequests[0].EntityId == EntityIdOne;
		bSuccess &= AuthorityChangeRequests[0].TargetVirtualWorker == VirtualWorkerOne;
	}
	else
	{
		bSuccess = false;
	}

	TestTrue("LoadBalanceEnforcer returned expected authority change assignment results", bSuccess);

	return true;
}

LOADBALANCEENFORCER_TEST(GIVEN_authority_change_when_not_authoritative_over_authority_intent_component_WHEN_we_inform_load_balance_enforcer_THEN_queue_authority_request)
{
	const TUniquePtr<SpatialVirtualWorkerTranslator> VirtualWorkerTranslator = CreateVirtualWorkerTranslator();

	// The important part of this test is that the worker does not already have authority over the AuthorityIntent component.
	// In this case, we expect the load balance enforcer to create an authority change request.
	USpatialStaticComponentView* StaticComponentView = NewObject<USpatialStaticComponentView>();
	AddEntityToStaticComponentView(*StaticComponentView, EntityIdOne, VirtualWorkerOne, WORKER_AUTHORITY_NOT_AUTHORITATIVE);

	TUniquePtr<SpatialLoadBalanceEnforcer> LoadBalanceEnforcer = MakeUnique<SpatialLoadBalanceEnforcer>(ValidWorkerOne, StaticComponentView, VirtualWorkerTranslator.Get());

	Worker_AuthorityChangeOp UpdateOp;
	UpdateOp.entity_id = EntityIdOne;
	UpdateOp.authority = WORKER_AUTHORITY_AUTHORITATIVE;
	UpdateOp.component_id = SpatialConstants::ENTITY_ACL_COMPONENT_ID;

	LoadBalanceEnforcer->OnAclAuthorityChanged(UpdateOp);

	TArray<SpatialLoadBalanceEnforcer::AuthorityStateChange> AuthorityChangeRequests = LoadBalanceEnforcer->ProcessAuthorityChangeRequests();

	bool bSuccess = true;
	if (AuthorityChangeRequests.Num() == 1)
	{
		bSuccess &= AuthorityChangeRequests[0].EntityId == EntityIdOne;
		bSuccess &= AuthorityChangeRequests[0].TargetVirtualWorker == VirtualWorkerOne;
	}
	else
	{
		bSuccess = false;
	}

	TestTrue("LoadBalanceEnforcer returned expected authority change assignment results", bSuccess);

	return true;
}

//LOADBALANCEENFORCER_TEST(GIVEN_authority_change_when_authoritative_over_authority_intent_component_WHEN_we_inform_load_balance_enforcer_THEN_return_no_authority_change_requests)
//{
//	const TUniquePtr<SpatialVirtualWorkerTranslator> VirtualWorkerTranslator = CreateVirtualWorkerTranslator();
//
//	// The important part of this test is that the worker does already have authority over the AuthorityIntent component.
//	// In this case, we expect the load balance enforcer not to create an authority change request.
//	USpatialStaticComponentView* StaticComponentView = NewObject<USpatialStaticComponentView>();
//	AddEntityToStaticComponentView(*StaticComponentView, EntityIdOne, VirtualWorkerOne, WORKER_AUTHORITY_AUTHORITATIVE);
//
//	TUniquePtr<SpatialLoadBalanceEnforcer> LoadBalanceEnforcer = MakeUnique<SpatialLoadBalanceEnforcer>(ValidWorkerOne, StaticComponentView, VirtualWorkerTranslator.Get());
//
//	Worker_AuthorityChangeOp UpdateOp;
//	UpdateOp.entity_id = EntityIdOne;
//	UpdateOp.authority = WORKER_AUTHORITY_AUTHORITATIVE;
//	UpdateOp.component_id = SpatialConstants::ENTITY_ACL_COMPONENT_ID;
//
//	LoadBalanceEnforcer->OnAclAuthorityChanged(UpdateOp);
//
//	const TArray<SpatialLoadBalanceEnforcer::AuthorityStateChange> AuthorityChangeRequests = LoadBalanceEnforcer->ProcessAuthorityChangeRequests();
//
//	const bool bSuccess = AuthorityChangeRequests.Num() == 0;
//	TestTrue("LoadBalanceEnforcer returned expected authority change assignment results", bSuccess);
//
//	return true;
//}

LOADBALANCEENFORCER_TEST(GIVEN_acl_authority_loss_WHEN_request_is_queued_THEN_return_no_authority_change_requests)
{
	const TUniquePtr<SpatialVirtualWorkerTranslator> VirtualWorkerTranslator = CreateVirtualWorkerTranslator();

	// Set up the world in such a way that we can enforce the authority, and we are not already the authoritative worker so should try and assign authority.
	USpatialStaticComponentView* StaticComponentView = NewObject<USpatialStaticComponentView>();
	AddEntityToStaticComponentView(*StaticComponentView, EntityIdOne, VirtualWorkerOne, WORKER_AUTHORITY_NOT_AUTHORITATIVE);

	TUniquePtr<SpatialLoadBalanceEnforcer> LoadBalanceEnforcer = MakeUnique<SpatialLoadBalanceEnforcer>(ValidWorkerOne, StaticComponentView, VirtualWorkerTranslator.Get());

	Worker_AuthorityChangeOp AuthOp;
	AuthOp.entity_id = EntityIdOne;
	AuthOp.authority = WORKER_AUTHORITY_AUTHORITATIVE;
	AuthOp.component_id = SpatialConstants::ENTITY_ACL_COMPONENT_ID;

	LoadBalanceEnforcer->OnAclAuthorityChanged(AuthOp);

	// At this point, we expect there to be a queued request.
	TestTrue("Assignment request is queued", LoadBalanceEnforcer->AuthorityChangeRequestIsQueued(EntityIdOne));

	AuthOp.authority = WORKER_AUTHORITY_NOT_AUTHORITATIVE;

	LoadBalanceEnforcer->OnAclAuthorityChanged(AuthOp);

	// Now we should have dropped that request.

	const TArray<SpatialLoadBalanceEnforcer::AuthorityStateChange> AuthorityChangeRequests = LoadBalanceEnforcer->ProcessAuthorityChangeRequests();

	const bool bSuccess = AuthorityChangeRequests.Num() == 0;
	TestTrue("LoadBalanceEnforcer returned expected authority change assignment results", bSuccess);

	return true;
}

LOADBALANCEENFORCER_TEST(GIVEN_entity_removal_WHEN_request_is_queued_THEN_return_no_authority_change_requests)
{
	const TUniquePtr<SpatialVirtualWorkerTranslator> VirtualWorkerTranslator = CreateVirtualWorkerTranslator();

	// Set up the world in such a way that we can enforce the authority, and we are not already the authoritative worker so should try and assign authority.
	USpatialStaticComponentView* StaticComponentView = NewObject<USpatialStaticComponentView>();
	AddEntityToStaticComponentView(*StaticComponentView, EntityIdOne, VirtualWorkerOne, WORKER_AUTHORITY_NOT_AUTHORITATIVE);

	TUniquePtr<SpatialLoadBalanceEnforcer> LoadBalanceEnforcer = MakeUnique<SpatialLoadBalanceEnforcer>(ValidWorkerOne, StaticComponentView, VirtualWorkerTranslator.Get());

	Worker_AuthorityChangeOp AuthOp;
	AuthOp.entity_id = EntityIdOne;
	AuthOp.authority = WORKER_AUTHORITY_AUTHORITATIVE;
	AuthOp.component_id = SpatialConstants::ENTITY_ACL_COMPONENT_ID;

	LoadBalanceEnforcer->OnAclAuthorityChanged(AuthOp);

	// At this point, we expect there to be a queued request.
	TestTrue("Assignment request is queued", LoadBalanceEnforcer->AuthorityChangeRequestIsQueued(EntityIdOne));

	Worker_RemoveEntityOp EntityOp;
	EntityOp.entity_id = EntityIdOne;

	LoadBalanceEnforcer->OnEntityRemoved(EntityOp);

	// Now we should have dropped that request.

	const TArray<SpatialLoadBalanceEnforcer::AuthorityStateChange> AuthorityChangeRequests = LoadBalanceEnforcer->ProcessAuthorityChangeRequests();

	const bool bSuccess = AuthorityChangeRequests.Num() == 0;
	TestTrue("LoadBalanceEnforcer returned expected authority change assignment results", bSuccess);

	return true;
}

LOADBALANCEENFORCER_TEST(GIVEN_authority_intent_component_removal_WHEN_request_is_queued_THEN_return_no_authority_change_requests)
{
	const TUniquePtr<SpatialVirtualWorkerTranslator> VirtualWorkerTranslator = CreateVirtualWorkerTranslator();

	// Set up the world in such a way that we can enforce the authority, and we are not already the authoritative worker so should try and assign authority.
	USpatialStaticComponentView* StaticComponentView = NewObject<USpatialStaticComponentView>();
	AddEntityToStaticComponentView(*StaticComponentView, EntityIdOne, VirtualWorkerOne, WORKER_AUTHORITY_NOT_AUTHORITATIVE);

	TUniquePtr<SpatialLoadBalanceEnforcer> LoadBalanceEnforcer = MakeUnique<SpatialLoadBalanceEnforcer>(ValidWorkerOne, StaticComponentView, VirtualWorkerTranslator.Get());

	Worker_AuthorityChangeOp AuthOp;
	AuthOp.entity_id = EntityIdOne;
	AuthOp.authority = WORKER_AUTHORITY_AUTHORITATIVE;
	AuthOp.component_id = SpatialConstants::ENTITY_ACL_COMPONENT_ID;

	LoadBalanceEnforcer->OnAclAuthorityChanged(AuthOp);

	// At this point, we expect there to be a queued request.
	TestTrue("Assignment request is queued", LoadBalanceEnforcer->AuthorityChangeRequestIsQueued(EntityIdOne));

	Worker_RemoveComponentOp ComponentOp;
	ComponentOp.entity_id = EntityIdOne;
	ComponentOp.component_id = SpatialConstants::AUTHORITY_INTENT_COMPONENT_ID;

	LoadBalanceEnforcer->OnLoadBalancingComponentRemoved(ComponentOp);

	// Now we should have dropped that request.

	const TArray<SpatialLoadBalanceEnforcer::AuthorityStateChange> AuthorityChangeRequests = LoadBalanceEnforcer->ProcessAuthorityChangeRequests();

	const bool bSuccess = AuthorityChangeRequests.Num() == 0;
	TestTrue("LoadBalanceEnforcer returned expected authority change assignment results", bSuccess);

	return true;
}

LOADBALANCEENFORCER_TEST(GIVEN_acl_component_removal_WHEN_request_is_queued_THEN_return_no_authority_change_requests)
{
	const TUniquePtr<SpatialVirtualWorkerTranslator> VirtualWorkerTranslator = CreateVirtualWorkerTranslator();

	// Set up the world in such a way that we can enforce the authority, and we are not already the authoritative worker so should try and assign authority.
	USpatialStaticComponentView* StaticComponentView = NewObject<USpatialStaticComponentView>();
	AddEntityToStaticComponentView(*StaticComponentView, EntityIdOne, VirtualWorkerOne, WORKER_AUTHORITY_NOT_AUTHORITATIVE);

	TUniquePtr<SpatialLoadBalanceEnforcer> LoadBalanceEnforcer = MakeUnique<SpatialLoadBalanceEnforcer>(ValidWorkerOne, StaticComponentView, VirtualWorkerTranslator.Get());

	Worker_AuthorityChangeOp AuthOp;
	AuthOp.entity_id = EntityIdOne;
	AuthOp.authority = WORKER_AUTHORITY_AUTHORITATIVE;
	AuthOp.component_id = SpatialConstants::ENTITY_ACL_COMPONENT_ID;

	LoadBalanceEnforcer->OnAclAuthorityChanged(AuthOp);

	// At this point, we expect there to be a queued request.
	TestTrue("Assignment request is queued", LoadBalanceEnforcer->AuthorityChangeRequestIsQueued(EntityIdOne));

	Worker_RemoveComponentOp ComponentOp;
	ComponentOp.entity_id = EntityIdOne;
	ComponentOp.component_id = SpatialConstants::ENTITY_ACL_COMPONENT_ID;

	LoadBalanceEnforcer->OnLoadBalancingComponentRemoved(ComponentOp);

	// Now we should have dropped that request.

	const TArray<SpatialLoadBalanceEnforcer::AuthorityStateChange> AuthorityChangeRequests = LoadBalanceEnforcer->ProcessAuthorityChangeRequests();

	const bool bSuccess = AuthorityChangeRequests.Num() == 0;
	TestTrue("LoadBalanceEnforcer returned expected authority change assignment results", bSuccess);

	return true;
}

LOADBALANCEENFORCER_TEST(GIVEN_component_presence_change_op_WHEN_we_inform_load_balance_enforcer_THEN_queue_authority_request)
{
	const TUniquePtr<SpatialVirtualWorkerTranslator> VirtualWorkerTranslator = CreateVirtualWorkerTranslator();

	USpatialStaticComponentView* StaticComponentView = NewObject<USpatialStaticComponentView>();
	AddEntityToStaticComponentView(*StaticComponentView, EntityIdOne, VirtualWorkerOne, WORKER_AUTHORITY_NOT_AUTHORITATIVE);

	TUniquePtr<SpatialLoadBalanceEnforcer> LoadBalanceEnforcer = MakeUnique<SpatialLoadBalanceEnforcer>(ValidWorkerOne, StaticComponentView, VirtualWorkerTranslator.Get());

	TArray<Worker_ComponentId> PresentComponentIds{ TestComponentIdOne, TestComponentIdTwo };

	// Create a ComponentPresence component update op with the required components.
	Worker_ComponentUpdateOp UpdateOp;
	UpdateOp.entity_id = EntityIdOne;
	UpdateOp.update.component_id = SpatialConstants::COMPONENT_PRESENCE_COMPONENT_ID;
	UpdateOp.update.schema_type = Schema_CreateComponentUpdate();
	Schema_Object* UpdateFields = Schema_GetComponentUpdateFields(UpdateOp.update.schema_type);
	Schema_AddUint32List(UpdateFields, SpatialConstants::COMPONENT_PRESENCE_COMPONENT_LIST_ID, PresentComponentIds.GetData(), PresentComponentIds.Num());

	// Pass the ComponentPresence update to the enforcer to queue an authority change assignment.
	LoadBalanceEnforcer->OnLoadBalancingComponentUpdated(UpdateOp);

	// Pass the update op to the StaticComponentView so that they can be read when the authority change assigment is processed.
	StaticComponentView->OnComponentUpdate(UpdateOp);

	TArray<SpatialLoadBalanceEnforcer::AuthorityStateChange> AuthorityChangeRequests = LoadBalanceEnforcer->ProcessAuthorityChangeRequests();

	bool bSuccess = true;
	if (AuthorityChangeRequests.Num() == 1)
	{
		bSuccess &= AuthorityChangeRequests[0].EntityId == EntityIdOne;
		bSuccess &= AuthorityChangeRequests[0].TargetVirtualWorker == VirtualWorkerOne;
		bSuccess &= AuthorityChangeRequests[0].ComponentIds.Contains(TestComponentIdOne);
		bSuccess &= AuthorityChangeRequests[0].ComponentIds.Contains(TestComponentIdTwo);
	}
	else
	{
		bSuccess = false;
	}

	TestTrue("LoadBalanceEnforcer returned expected authority change assignment results", bSuccess);

	return true;
}

LOADBALANCEENFORCER_TEST(GIVEN_component_presence_component_removal_WHEN_request_is_queued_THEN_return_no_authority_change_requests)
{
	const TUniquePtr<SpatialVirtualWorkerTranslator> VirtualWorkerTranslator = CreateVirtualWorkerTranslator();

	// Set up the world in such a way that we can enforce the authority, and we are not already the authoritative worker so should try and assign authority.
	USpatialStaticComponentView* StaticComponentView = NewObject<USpatialStaticComponentView>();
	AddEntityToStaticComponentView(*StaticComponentView, EntityIdOne, VirtualWorkerOne, WORKER_AUTHORITY_NOT_AUTHORITATIVE);

	TUniquePtr<SpatialLoadBalanceEnforcer> LoadBalanceEnforcer = MakeUnique<SpatialLoadBalanceEnforcer>(ValidWorkerOne, StaticComponentView, VirtualWorkerTranslator.Get());

	Worker_AuthorityChangeOp AuthOp;
	AuthOp.entity_id = EntityIdOne;
	AuthOp.authority = WORKER_AUTHORITY_AUTHORITATIVE;
	AuthOp.component_id = SpatialConstants::ENTITY_ACL_COMPONENT_ID;

	LoadBalanceEnforcer->OnAclAuthorityChanged(AuthOp);

	// At this point, we expect there to be a queued request.
	TestTrue("Assignment request is queued", LoadBalanceEnforcer->AuthorityChangeRequestIsQueued(EntityIdOne));

	Worker_RemoveComponentOp ComponentOp;
	ComponentOp.entity_id = EntityIdOne;
	ComponentOp.component_id = SpatialConstants::COMPONENT_PRESENCE_COMPONENT_ID;

	LoadBalanceEnforcer->OnLoadBalancingComponentRemoved(ComponentOp);

	// Now we should have dropped that request.

	const TArray<SpatialLoadBalanceEnforcer::AuthorityStateChange> AuthorityChangeRequests = LoadBalanceEnforcer->ProcessAuthorityChangeRequests();

	const bool bSuccess = AuthorityChangeRequests.Num() == 0;
	TestTrue("LoadBalanceEnforcer returned expected authority change assignment results", bSuccess);

	return true;
}
