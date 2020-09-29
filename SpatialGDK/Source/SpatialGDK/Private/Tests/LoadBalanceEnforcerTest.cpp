// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "Tests/TestDefinitions.h"

#include "EngineClasses/SpatialLoadBalanceEnforcer.h"
#include "EngineClasses/SpatialVirtualWorkerTranslator.h"
#include "Interop/SpatialStaticComponentView.h"
#include "Schema/AuthorityIntent.h"
#include "Tests/TestingSchemaHelpers.h"

#include "CoreMinimal.h"

#include "Schema/ClientEndpoint.h"
#include "SpatialConstants.h"
#include "SpatialView/ComponentData.h"
#include "SpatialView/EntityComponentTypes.h"
#include "SpatialView/EntityView.h"
#include "Tests/SpatialView/SpatialViewUtils.h"
#include "Utils/SchemaOption.h"

#define LOADBALANCEENFORCER_TEST(TestName) GDK_TEST(Core, SpatialLoadBalanceEnforcer, TestName)

// Test Globals
namespace
{
const PhysicalWorkerName UnrealWorker = TEXT("UnrealWorker");
const WorkerRequirementSet UnrealRequirementSet = WorkerRequirementSet{ WorkerRequirementSet{ WorkerAttributeSet{ UnrealWorker } } };

const PhysicalWorkerName ThisWorker = TEXT("ThisWorker");
const PhysicalWorkerName OtherWorker = TEXT("OtherWorker");
const PhysicalWorkerName ClientWorker = TEXT("ClientWorker");
const PhysicalWorkerName OtherClientWorker = TEXT("OtherClientWorker");

const WorkerRequirementSet ThisRequirementSet =
	WorkerRequirementSet{ WorkerRequirementSet{ WorkerAttributeSet{ FString::Printf(TEXT("workerId:%s"), *ThisWorker) } } };
const WorkerRequirementSet OtherRequirementSet =
	WorkerRequirementSet{ WorkerRequirementSet{ WorkerAttributeSet{ FString::Printf(TEXT("workerId:%s"), *OtherWorker) } } };
const WorkerRequirementSet ClientRequirementSet =
	WorkerRequirementSet{ WorkerRequirementSet{ WorkerAttributeSet{ FString::Printf(TEXT("workerId:%s"), *ClientWorker) } } };
const WorkerRequirementSet OtherClientRequirementSet =
	WorkerRequirementSet{ WorkerRequirementSet{ WorkerAttributeSet{ FString::Printf(TEXT("workerId:%s"), *OtherClientWorker) } } };

constexpr VirtualWorkerId ThisVirtualWorker = 1;
constexpr VirtualWorkerId OtherVirtualWorker = 2;

constexpr Worker_EntityId EntityIdOne = 1;
constexpr Worker_EntityId EntityIdTwo = 2;

constexpr Worker_ComponentId TestComponentIdOne = 123;
constexpr Worker_ComponentId TestComponentIdTwo = 456;

const TArray<Worker_ComponentId> NonAclLBComponents = { SpatialConstants::AUTHORITY_INTENT_COMPONENT_ID,
														SpatialConstants::COMPONENT_PRESENCE_COMPONENT_ID,
														SpatialConstants::NET_OWNING_CLIENT_WORKER_COMPONENT_ID };

TUniquePtr<SpatialVirtualWorkerTranslator> CreateVirtualWorkerTranslator()
{
	TUniquePtr<SpatialVirtualWorkerTranslator> VirtualWorkerTranslator = MakeUnique<SpatialVirtualWorkerTranslator>(nullptr, ThisWorker);

	Schema_Object* DataObject = TestingSchemaHelpers::CreateTranslationComponentDataFields();

	TestingSchemaHelpers::AddTranslationComponentDataMapping(DataObject, ThisVirtualWorker, ThisWorker);
	TestingSchemaHelpers::AddTranslationComponentDataMapping(DataObject, OtherVirtualWorker, OtherWorker);

	VirtualWorkerTranslator->ApplyVirtualWorkerManagerData(DataObject);

	return VirtualWorkerTranslator;
}

SpatialGDK::ComponentData MakeComponentDataFromData(Worker_ComponentData Data)
{
	return SpatialGDK::ComponentData(SpatialGDK::OwningComponentDataPtr(Data.schema_type), Data.component_id);
}

// Adds an entity to the view with correct LB components. Also assigns the current authority to the passed worker,
// and the auth intent to the passed virtual worker. The ACL is always assigned to "UnrealWorker".
// Optionally pass a client name to designate the entity as net owned by that client.
void AddLBEntityToView(
	SpatialGDK::EntityView& View, const Worker_EntityId EntityId, const WorkerRequirementSet AuthRequirementSet,
	const VirtualWorkerId IntentWorkerId,
	const SpatialGDK::TSchemaOption<PhysicalWorkerName> ClientWorkerName = SpatialGDK::TSchemaOption<PhysicalWorkerName>())
{
	AddEntityToView(View, EntityId);

	const WorkerRequirementSet ReadAcl;
	WriteAclMap WriteAcl;
	WriteAcl.Add(SpatialConstants::ENTITY_ACL_COMPONENT_ID, UnrealRequirementSet);
	WriteAcl.Add(SpatialConstants::AUTHORITY_INTENT_COMPONENT_ID, AuthRequirementSet);
	WriteAcl.Add(SpatialConstants::COMPONENT_PRESENCE_COMPONENT_ID, AuthRequirementSet);
	WriteAcl.Add(SpatialConstants::NET_OWNING_CLIENT_WORKER_COMPONENT_ID, AuthRequirementSet);

	if (ClientWorkerName.IsSet())
	{
		const WorkerRequirementSet AuthClientRequirementSet =
			WorkerRequirementSet{ WorkerRequirementSet{ WorkerAttributeSet{ ClientWorkerName.GetValue() } } };
		WriteAcl.Add(SpatialConstants::CLIENT_ENDPOINT_COMPONENT_ID, AuthClientRequirementSet);
		WriteAcl.Add(SpatialConstants::HEARTBEAT_COMPONENT_ID, AuthClientRequirementSet);
	}

	const TArray<Worker_ComponentId> PresentComponents = {
		SpatialConstants::ENTITY_ACL_COMPONENT_ID,		   SpatialConstants::AUTHORITY_INTENT_COMPONENT_ID,
		SpatialConstants::COMPONENT_PRESENCE_COMPONENT_ID, SpatialConstants::NET_OWNING_CLIENT_WORKER_COMPONENT_ID,
		SpatialConstants::CLIENT_ENDPOINT_COMPONENT_ID,	   SpatialConstants::HEARTBEAT_COMPONENT_ID
	};

	Worker_ComponentData ClientEndpoint = FWorkerComponentData();
	ClientEndpoint.component_id = SpatialConstants::CLIENT_ENDPOINT_COMPONENT_ID;
	Worker_ComponentData HeartBeat = FWorkerComponentData();
	HeartBeat.component_id = SpatialConstants::HEARTBEAT_COMPONENT_ID;

	Worker_ComponentData Tag = FWorkerComponentData();
	Tag.component_id = SpatialConstants::LB_TAG_COMPONENT_ID;

	AddComponentToView(View, EntityId, MakeComponentDataFromData(SpatialGDK::EntityAcl(ReadAcl, WriteAcl).CreateEntityAclData()));
	AddComponentToView(View, EntityId, MakeComponentDataFromData(SpatialGDK::AuthorityIntent::CreateAuthorityIntentData(IntentWorkerId)));
	AddComponentToView(View, EntityId,
					   MakeComponentDataFromData(SpatialGDK::ComponentPresence::CreateComponentPresenceData(PresentComponents)));
	AddComponentToView(View, EntityId,
					   MakeComponentDataFromData(SpatialGDK::NetOwningClientWorker::CreateNetOwningClientWorkerData(ClientWorkerName)));
	AddComponentToView(View, EntityId, MakeComponentDataFromData(ClientEndpoint));
	AddComponentToView(View, EntityId, MakeComponentDataFromData(HeartBeat));
	AddComponentToView(View, EntityId, MakeComponentDataFromData(Tag));

	AddAuthorityToView(View, EntityId, SpatialConstants::ENTITY_ACL_COMPONENT_ID);
}

WriteAclMap GetWriteAclMapFromUpdate(const SpatialGDK::EntityComponentUpdate& Update)
{
	WriteAclMap AclMap;
	Schema_Object* ComponentObject = Schema_GetComponentUpdateFields(Update.Update.GetUnderlying());
	// This is never emptied, so does not need an additional check for cleared fields
	uint32 KVPairCount = Schema_GetObjectCount(ComponentObject, 2);
	for (uint32 i = 0; i < KVPairCount; i++)
	{
		Schema_Object* KVPairObject = Schema_IndexObject(ComponentObject, 2, i);
		uint32 Key = Schema_GetUint32(KVPairObject, SCHEMA_MAP_KEY_FIELD_ID);
		WorkerRequirementSet Value = SpatialGDK::GetWorkerRequirementSetFromSchema(KVPairObject, SCHEMA_MAP_VALUE_FIELD_ID);

		AclMap.Add(Key, Value);
	}

	return AclMap;
}

bool AclMapDelegatesLBComponents(const WriteAclMap& AclMap, const WorkerRequirementSet DelegatedRequirementSet)
{
	for (Worker_ComponentId ComponentId : NonAclLBComponents)
	{
		const auto Entry = AclMap.Find(ComponentId);
		if (Entry == nullptr)
		{
			return false;
		}
		if (*Entry != DelegatedRequirementSet)
		{
			return false;
		}
	}
	return true;
}

} // anonymous namespace

LOADBALANCEENFORCER_TEST(GIVEN_a_view_with_no_data_WHEN_advance_load_balance_enforcer_THEN_return_no_acl_assignment_requests)
{
	SpatialGDK::FDispatcher Dispatcher;
	SpatialGDK::EntityView View;
	const TUniquePtr<SpatialVirtualWorkerTranslator> VirtualWorkerTranslator = CreateVirtualWorkerTranslator();
	SpatialGDK::FSubView SubView(SpatialConstants::LB_TAG_COMPONENT_ID, SpatialGDK::FSubView::NoFilter, &View, Dispatcher,
								 SpatialGDK::FSubView::NoDispatcherCallbacks);

	bool bInvoked = false;

	SpatialGDK::SpatialLoadBalanceEnforcer LoadBalanceEnforcer = SpatialGDK::SpatialLoadBalanceEnforcer(
		ThisWorker, SubView, VirtualWorkerTranslator.Get(), [&bInvoked](SpatialGDK::EntityComponentUpdate) {
			bInvoked = true;
		});

	// The view has no entities in it. We expect the enforcer not to produce any ACL requests.
	LoadBalanceEnforcer.Advance();
	TestTrue("LoadBalanceEnforcer did not try to send an ACL update", !bInvoked);

	return true;
}

LOADBALANCEENFORCER_TEST(
	GIVEN_load_balance_enforcer_with_valid_mapping_WHEN_asked_for_acl_assignments_THEN_return_correct_acl_assignment_requests)
{
	SpatialGDK::FDispatcher Dispatcher;
	SpatialGDK::EntityView View;
	const TUniquePtr<SpatialVirtualWorkerTranslator> VirtualWorkerTranslator = CreateVirtualWorkerTranslator();
	SpatialGDK::FSubView SubView(SpatialConstants::LB_TAG_COMPONENT_ID, SpatialGDK::FSubView::NoFilter, &View, Dispatcher,
								 SpatialGDK::FSubView::NoDispatcherCallbacks);

	TArray<SpatialGDK::EntityComponentUpdate> Updates;

	SpatialGDK::SpatialLoadBalanceEnforcer LoadBalanceEnforcer = SpatialGDK::SpatialLoadBalanceEnforcer(
		ThisWorker, SubView, VirtualWorkerTranslator.Get(), [&Updates](SpatialGDK::EntityComponentUpdate Update) {
			Updates.Add(MoveTemp(Update));
		});

	LoadBalanceEnforcer.Advance();

	AddLBEntityToView(View, EntityIdOne, ThisRequirementSet, OtherVirtualWorker);

	LoadBalanceEnforcer.ShortCircuitMaybeRefreshAcl(EntityIdOne);

	bool bSuccess = true;
	if (Updates.Num() == 1)
	{
		bSuccess &= Updates[0].EntityId == EntityIdOne;
		bSuccess &= Updates[0].Update.GetComponentId() == SpatialConstants::ENTITY_ACL_COMPONENT_ID;
		WriteAclMap AclMap = GetWriteAclMapFromUpdate(Updates[0]);
		bSuccess &= AclMapDelegatesLBComponents(AclMap, OtherRequirementSet);
	}
	else
	{
		bSuccess = false;
	}

	TestTrue("LoadBalanceEnforcer returned expected ACL assignment results", bSuccess);

	return true;
}

// LOADBALANCEENFORCER_TEST(GIVEN_authority_intent_change_op_WHEN_we_inform_load_balance_enforcer_THEN_queue_authority_request)
// {
// 	TUniquePtr<SpatialVirtualWorkerTranslator> VirtualWorkerTranslator = CreateVirtualWorkerTranslator();
//
// 	USpatialStaticComponentView* StaticComponentView = NewObject<USpatialStaticComponentView>();
// 	AddEntityToStaticComponentView(*StaticComponentView, EntityIdOne, ThisVirtualWorker, WORKER_AUTHORITY_NOT_AUTHORITATIVE);
//
// 	TUniquePtr<SpatialLoadBalanceEnforcer> LoadBalanceEnforcer =
// 		MakeUnique<SpatialLoadBalanceEnforcer>(ThisWorker, StaticComponentView, VirtualWorkerTranslator.Get());
//
// 	Worker_ComponentUpdateOp UpdateOp;
// 	UpdateOp.entity_id = EntityIdOne;
// 	UpdateOp.update.component_id = SpatialConstants::AUTHORITY_INTENT_COMPONENT_ID;
//
// 	LoadBalanceEnforcer->OnLoadBalancingComponentUpdated(UpdateOp);
//
// 	TArray<SpatialLoadBalanceEnforcer::AclWriteAuthorityRequest> ACLRequests = LoadBalanceEnforcer->ProcessQueuedAclAssignmentRequests();
//
// 	bool bSuccess = true;
// 	if (ACLRequests.Num() == 1)
// 	{
// 		bSuccess &= ACLRequests[0].EntityId == EntityIdOne;
// 		bSuccess &= ACLRequests[0].OwningWorkerId == ThisWorker;
// 	}
// 	else
// 	{
// 		bSuccess = false;
// 	}
//
// 	TestTrue("LoadBalanceEnforcer returned expected ACL assignment results", bSuccess);
//
// 	return true;
// }
//
// LOADBALANCEENFORCER_TEST(
// 	GIVEN_authority_change_when_not_authoritative_over_authority_intent_component_WHEN_we_inform_load_balance_enforcer_THEN_queue_authority_request)
// {
// 	TUniquePtr<SpatialVirtualWorkerTranslator> VirtualWorkerTranslator = CreateVirtualWorkerTranslator();
//
// 	// The important part of this test is that the worker does not already have authority over the AuthorityIntent component.
// 	// In this case, we expect the load balance enforcer to create an ACL request.
// 	USpatialStaticComponentView* StaticComponentView = NewObject<USpatialStaticComponentView>();
// 	AddEntityToStaticComponentView(*StaticComponentView, EntityIdOne, ThisVirtualWorker, WORKER_AUTHORITY_NOT_AUTHORITATIVE);
//
// 	TUniquePtr<SpatialLoadBalanceEnforcer> LoadBalanceEnforcer =
// 		MakeUnique<SpatialLoadBalanceEnforcer>(ThisWorker, StaticComponentView, VirtualWorkerTranslator.Get());
//
// 	Worker_AuthorityChangeOp UpdateOp;
// 	UpdateOp.entity_id = EntityIdOne;
// 	UpdateOp.authority = WORKER_AUTHORITY_AUTHORITATIVE;
// 	UpdateOp.component_id = SpatialConstants::ENTITY_ACL_COMPONENT_ID;
//
// 	LoadBalanceEnforcer->OnAclAuthorityChanged(UpdateOp);
//
// 	TArray<SpatialLoadBalanceEnforcer::AclWriteAuthorityRequest> ACLRequests = LoadBalanceEnforcer->ProcessQueuedAclAssignmentRequests();
//
// 	bool bSuccess = true;
// 	if (ACLRequests.Num() == 1)
// 	{
// 		bSuccess &= ACLRequests[0].EntityId == EntityIdOne;
// 		bSuccess &= ACLRequests[0].OwningWorkerId == ThisWorker;
// 	}
// 	else
// 	{
// 		bSuccess = false;
// 	}
//
// 	TestTrue("LoadBalanceEnforcer returned expected ACL assignment results", bSuccess);
//
// 	return true;
// }
//
// LOADBALANCEENFORCER_TEST(
// 	GIVEN_authority_change_when_authoritative_over_authority_intent_component_WHEN_we_inform_load_balance_enforcer_THEN_return_no_acl_assignment_requests)
// {
// 	TUniquePtr<SpatialVirtualWorkerTranslator> VirtualWorkerTranslator = CreateVirtualWorkerTranslator();
//
// 	// The important part of this test is that the worker does already have authority over the AuthorityIntent component.
// 	// In this case, we expect the load balance enforcer not to create an ACL request.
// 	USpatialStaticComponentView* StaticComponentView = NewObject<USpatialStaticComponentView>();
// 	AddEntityToStaticComponentView(*StaticComponentView, EntityIdOne, ThisVirtualWorker, WORKER_AUTHORITY_AUTHORITATIVE);
//
// 	TUniquePtr<SpatialLoadBalanceEnforcer> LoadBalanceEnforcer =
// 		MakeUnique<SpatialLoadBalanceEnforcer>(ThisWorker, StaticComponentView, VirtualWorkerTranslator.Get());
//
// 	Worker_AuthorityChangeOp UpdateOp;
// 	UpdateOp.entity_id = EntityIdOne;
// 	UpdateOp.authority = WORKER_AUTHORITY_AUTHORITATIVE;
// 	UpdateOp.component_id = SpatialConstants::ENTITY_ACL_COMPONENT_ID;
//
// 	LoadBalanceEnforcer->OnAclAuthorityChanged(UpdateOp);
//
// 	TArray<SpatialLoadBalanceEnforcer::AclWriteAuthorityRequest> ACLRequests = LoadBalanceEnforcer->ProcessQueuedAclAssignmentRequests();
//
// 	bool bSuccess = ACLRequests.Num() == 0;
// 	TestTrue("LoadBalanceEnforcer returned expected ACL assignment results", bSuccess);
//
// 	return true;
// }
//
// LOADBALANCEENFORCER_TEST(GIVEN_acl_authority_loss_WHEN_request_is_queued_THEN_return_no_acl_assignment_requests)
// {
// 	TUniquePtr<SpatialVirtualWorkerTranslator> VirtualWorkerTranslator = CreateVirtualWorkerTranslator();
//
// 	// Set up the world in such a way that we can enforce the authority, and we are not already the authoritative worker so should try and
// 	// assign authority.
// 	USpatialStaticComponentView* StaticComponentView = NewObject<USpatialStaticComponentView>();
// 	AddEntityToStaticComponentView(*StaticComponentView, EntityIdOne, ThisVirtualWorker, WORKER_AUTHORITY_NOT_AUTHORITATIVE);
//
// 	TUniquePtr<SpatialLoadBalanceEnforcer> LoadBalanceEnforcer =
// 		MakeUnique<SpatialLoadBalanceEnforcer>(ThisWorker, StaticComponentView, VirtualWorkerTranslator.Get());
//
// 	Worker_AuthorityChangeOp AuthOp;
// 	AuthOp.entity_id = EntityIdOne;
// 	AuthOp.authority = WORKER_AUTHORITY_AUTHORITATIVE;
// 	AuthOp.component_id = SpatialConstants::ENTITY_ACL_COMPONENT_ID;
//
// 	LoadBalanceEnforcer->OnAclAuthorityChanged(AuthOp);
//
// 	// At this point, we expect there to be a queued request.
// 	TestTrue("Assignment request is queued", LoadBalanceEnforcer->AclAssignmentRequestIsQueued(EntityIdOne));
//
// 	AuthOp.authority = WORKER_AUTHORITY_NOT_AUTHORITATIVE;
//
// 	LoadBalanceEnforcer->OnAclAuthorityChanged(AuthOp);
//
// 	// Now we should have dropped that request.
//
// 	TArray<SpatialLoadBalanceEnforcer::AclWriteAuthorityRequest> ACLRequests = LoadBalanceEnforcer->ProcessQueuedAclAssignmentRequests();
//
// 	bool bSuccess = ACLRequests.Num() == 0;
// 	TestTrue("LoadBalanceEnforcer returned expected ACL assignment results", bSuccess);
//
// 	return true;
// }
//
// LOADBALANCEENFORCER_TEST(GIVEN_entity_removal_WHEN_request_is_queued_THEN_return_no_acl_assignment_requests)
// {
// 	TUniquePtr<SpatialVirtualWorkerTranslator> VirtualWorkerTranslator = CreateVirtualWorkerTranslator();
//
// 	// Set up the world in such a way that we can enforce the authority, and we are not already the authoritative worker so should try and
// 	// assign authority.
// 	USpatialStaticComponentView* StaticComponentView = NewObject<USpatialStaticComponentView>();
// 	AddEntityToStaticComponentView(*StaticComponentView, EntityIdOne, ThisVirtualWorker, WORKER_AUTHORITY_NOT_AUTHORITATIVE);
//
// 	TUniquePtr<SpatialLoadBalanceEnforcer> LoadBalanceEnforcer =
// 		MakeUnique<SpatialLoadBalanceEnforcer>(ThisWorker, StaticComponentView, VirtualWorkerTranslator.Get());
//
// 	Worker_AuthorityChangeOp AuthOp;
// 	AuthOp.entity_id = EntityIdOne;
// 	AuthOp.authority = WORKER_AUTHORITY_AUTHORITATIVE;
// 	AuthOp.component_id = SpatialConstants::ENTITY_ACL_COMPONENT_ID;
//
// 	LoadBalanceEnforcer->OnAclAuthorityChanged(AuthOp);
//
// 	// At this point, we expect there to be a queued request.
// 	TestTrue("Assignment request is queued", LoadBalanceEnforcer->AclAssignmentRequestIsQueued(EntityIdOne));
//
// 	Worker_RemoveEntityOp EntityOp;
// 	EntityOp.entity_id = EntityIdOne;
//
// 	LoadBalanceEnforcer->OnEntityRemoved(EntityOp);
//
// 	// Now we should have dropped that request.
//
// 	TArray<SpatialLoadBalanceEnforcer::AclWriteAuthorityRequest> ACLRequests = LoadBalanceEnforcer->ProcessQueuedAclAssignmentRequests();
//
// 	bool bSuccess = ACLRequests.Num() == 0;
// 	TestTrue("LoadBalanceEnforcer returned expected ACL assignment results", bSuccess);
//
// 	return true;
// }
//
// LOADBALANCEENFORCER_TEST(GIVEN_authority_intent_component_removal_WHEN_request_is_queued_THEN_return_no_acl_assignment_requests)
// {
// 	TUniquePtr<SpatialVirtualWorkerTranslator> VirtualWorkerTranslator = CreateVirtualWorkerTranslator();
//
// 	// Set up the world in such a way that we can enforce the authority, and we are not already the authoritative worker so should try and
// 	// assign authority.
// 	USpatialStaticComponentView* StaticComponentView = NewObject<USpatialStaticComponentView>();
// 	AddEntityToStaticComponentView(*StaticComponentView, EntityIdOne, ThisVirtualWorker, WORKER_AUTHORITY_NOT_AUTHORITATIVE);
//
// 	TUniquePtr<SpatialLoadBalanceEnforcer> LoadBalanceEnforcer =
// 		MakeUnique<SpatialLoadBalanceEnforcer>(ThisWorker, StaticComponentView, VirtualWorkerTranslator.Get());
//
// 	Worker_AuthorityChangeOp AuthOp;
// 	AuthOp.entity_id = EntityIdOne;
// 	AuthOp.authority = WORKER_AUTHORITY_AUTHORITATIVE;
// 	AuthOp.component_id = SpatialConstants::ENTITY_ACL_COMPONENT_ID;
//
// 	LoadBalanceEnforcer->OnAclAuthorityChanged(AuthOp);
//
// 	// At this point, we expect there to be a queued request.
// 	TestTrue("Assignment request is queued", LoadBalanceEnforcer->AclAssignmentRequestIsQueued(EntityIdOne));
//
// 	Worker_RemoveComponentOp ComponentOp;
// 	ComponentOp.entity_id = EntityIdOne;
// 	ComponentOp.component_id = SpatialConstants::AUTHORITY_INTENT_COMPONENT_ID;
//
// 	LoadBalanceEnforcer->OnLoadBalancingComponentRemoved(ComponentOp);
//
// 	// Now we should have dropped that request.
//
// 	TArray<SpatialLoadBalanceEnforcer::AclWriteAuthorityRequest> ACLRequests = LoadBalanceEnforcer->ProcessQueuedAclAssignmentRequests();
//
// 	bool bSuccess = ACLRequests.Num() == 0;
// 	TestTrue("LoadBalanceEnforcer returned expected ACL assignment results", bSuccess);
//
// 	return true;
// }
//
// LOADBALANCEENFORCER_TEST(GIVEN_acl_component_removal_WHEN_request_is_queued_THEN_return_no_acl_assignment_requests)
// {
// 	TUniquePtr<SpatialVirtualWorkerTranslator> VirtualWorkerTranslator = CreateVirtualWorkerTranslator();
//
// 	// Set up the world in such a way that we can enforce the authority, and we are not already the authoritative worker so should try and
// 	// assign authority.
// 	USpatialStaticComponentView* StaticComponentView = NewObject<USpatialStaticComponentView>();
// 	AddEntityToStaticComponentView(*StaticComponentView, EntityIdOne, ThisVirtualWorker, WORKER_AUTHORITY_NOT_AUTHORITATIVE);
//
// 	TUniquePtr<SpatialLoadBalanceEnforcer> LoadBalanceEnforcer =
// 		MakeUnique<SpatialLoadBalanceEnforcer>(ThisWorker, StaticComponentView, VirtualWorkerTranslator.Get());
//
// 	Worker_AuthorityChangeOp AuthOp;
// 	AuthOp.entity_id = EntityIdOne;
// 	AuthOp.authority = WORKER_AUTHORITY_AUTHORITATIVE;
// 	AuthOp.component_id = SpatialConstants::ENTITY_ACL_COMPONENT_ID;
//
// 	LoadBalanceEnforcer->OnAclAuthorityChanged(AuthOp);
//
// 	// At this point, we expect there to be a queued request.
// 	TestTrue("Assignment request is queued", LoadBalanceEnforcer->AclAssignmentRequestIsQueued(EntityIdOne));
//
// 	Worker_RemoveComponentOp ComponentOp;
// 	ComponentOp.entity_id = EntityIdOne;
// 	ComponentOp.component_id = SpatialConstants::ENTITY_ACL_COMPONENT_ID;
//
// 	LoadBalanceEnforcer->OnLoadBalancingComponentRemoved(ComponentOp);
//
// 	// Now we should have dropped that request.
//
// 	TArray<SpatialLoadBalanceEnforcer::AclWriteAuthorityRequest> ACLRequests = LoadBalanceEnforcer->ProcessQueuedAclAssignmentRequests();
//
// 	bool bSuccess = ACLRequests.Num() == 0;
// 	TestTrue("LoadBalanceEnforcer returned expected ACL assignment results", bSuccess);
//
// 	return true;
// }
//
// LOADBALANCEENFORCER_TEST(GIVEN_component_presence_change_op_WHEN_we_inform_load_balance_enforcer_THEN_queue_authority_request)
// {
// 	TUniquePtr<SpatialVirtualWorkerTranslator> VirtualWorkerTranslator = CreateVirtualWorkerTranslator();
//
// 	USpatialStaticComponentView* StaticComponentView = NewObject<USpatialStaticComponentView>();
// 	AddEntityToStaticComponentView(*StaticComponentView, EntityIdOne, ThisVirtualWorker, WORKER_AUTHORITY_NOT_AUTHORITATIVE);
//
// 	TUniquePtr<SpatialLoadBalanceEnforcer> LoadBalanceEnforcer =
// 		MakeUnique<SpatialLoadBalanceEnforcer>(ThisWorker, StaticComponentView, VirtualWorkerTranslator.Get());
//
// 	TArray<Worker_ComponentId> PresentComponentIds{ TestComponentIdOne, TestComponentIdTwo };
//
// 	// Create a ComponentPresence component update op with the required components.
// 	Worker_ComponentUpdateOp UpdateOp;
// 	UpdateOp.entity_id = EntityIdOne;
// 	UpdateOp.update.component_id = SpatialConstants::COMPONENT_PRESENCE_COMPONENT_ID;
// 	UpdateOp.update.schema_type = Schema_CreateComponentUpdate();
// 	Schema_Object* UpdateFields = Schema_GetComponentUpdateFields(UpdateOp.update.schema_type);
// 	Schema_AddUint32List(UpdateFields, SpatialConstants::COMPONENT_PRESENCE_COMPONENT_LIST_ID, PresentComponentIds.GetData(),
// 						 PresentComponentIds.Num());
//
// 	// Pass the ComponentPresence update to the enforcer to queue an ACL assignment.
// 	LoadBalanceEnforcer->OnLoadBalancingComponentUpdated(UpdateOp);
//
// 	// Pass the update op to the StaticComponentView so that they can be read when the ACL assigment is processed.
// 	StaticComponentView->OnComponentUpdate(UpdateOp);
//
// 	TArray<SpatialLoadBalanceEnforcer::AclWriteAuthorityRequest> ACLRequests = LoadBalanceEnforcer->ProcessQueuedAclAssignmentRequests();
//
// 	bool bSuccess = true;
// 	if (ACLRequests.Num() == 1)
// 	{
// 		bSuccess &= ACLRequests[0].EntityId == EntityIdOne;
// 		bSuccess &= ACLRequests[0].OwningWorkerId == ThisWorker;
// 		bSuccess &= ACLRequests[0].ComponentIds.Contains(TestComponentIdOne);
// 		bSuccess &= ACLRequests[0].ComponentIds.Contains(TestComponentIdTwo);
// 	}
// 	else
// 	{
// 		bSuccess = false;
// 	}
//
// 	TestTrue("LoadBalanceEnforcer returned expected ACL assignment results", bSuccess);
//
// 	return true;
// }
//
// LOADBALANCEENFORCER_TEST(GIVEN_component_presence_component_removal_WHEN_request_is_queued_THEN_return_no_acl_assignment_requests)
// {
// 	TUniquePtr<SpatialVirtualWorkerTranslator> VirtualWorkerTranslator = CreateVirtualWorkerTranslator();
//
// 	// Set up the world in such a way that we can enforce the authority, and we are not already the authoritative worker so should try and
// 	// assign authority.
// 	USpatialStaticComponentView* StaticComponentView = NewObject<USpatialStaticComponentView>();
// 	AddEntityToStaticComponentView(*StaticComponentView, EntityIdOne, ThisVirtualWorker, WORKER_AUTHORITY_NOT_AUTHORITATIVE);
//
// 	TUniquePtr<SpatialLoadBalanceEnforcer> LoadBalanceEnforcer =
// 		MakeUnique<SpatialLoadBalanceEnforcer>(ThisWorker, StaticComponentView, VirtualWorkerTranslator.Get());
//
// 	Worker_AuthorityChangeOp AuthOp;
// 	AuthOp.entity_id = EntityIdOne;
// 	AuthOp.authority = WORKER_AUTHORITY_AUTHORITATIVE;
// 	AuthOp.component_id = SpatialConstants::ENTITY_ACL_COMPONENT_ID;
//
// 	LoadBalanceEnforcer->OnAclAuthorityChanged(AuthOp);
//
// 	// At this point, we expect there to be a queued request.
// 	TestTrue("Assignment request is queued", LoadBalanceEnforcer->AclAssignmentRequestIsQueued(EntityIdOne));
//
// 	Worker_RemoveComponentOp ComponentOp;
// 	ComponentOp.entity_id = EntityIdOne;
// 	ComponentOp.component_id = SpatialConstants::COMPONENT_PRESENCE_COMPONENT_ID;
//
// 	LoadBalanceEnforcer->OnLoadBalancingComponentRemoved(ComponentOp);
//
// 	// Now we should have dropped that request.
//
// 	TArray<SpatialLoadBalanceEnforcer::AclWriteAuthorityRequest> ACLRequests = LoadBalanceEnforcer->ProcessQueuedAclAssignmentRequests();
//
// 	bool bSuccess = ACLRequests.Num() == 0;
// 	TestTrue("LoadBalanceEnforcer returned expected ACL assignment results", bSuccess);
//
// 	return true;
// }
