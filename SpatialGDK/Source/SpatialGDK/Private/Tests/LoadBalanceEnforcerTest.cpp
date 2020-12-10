// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "Tests/TestDefinitions.h"

#include "EngineClasses/SpatialLoadBalanceEnforcer.h"
#include "EngineClasses/SpatialVirtualWorkerTranslator.h"
#include "Interop/SpatialStaticComponentView.h"
#include "Schema/AuthorityIntent.h"
#include "Tests/TestingSchemaHelpers.h"

#include "CoreMinimal.h"

#include "SpatialConstants.h"
#include "SpatialView/ComponentData.h"
#include "SpatialView/Dispatcher.h"
#include "SpatialView/EntityComponentTypes.h"
#include "SpatialView/EntityView.h"
#include "SpatialView/SubView.h"
#include "SpatialView/ViewDelta.h"
#include "Tests/SpatialView/SpatialViewUtils.h"
#include "Utils/SchemaOption.h"

#define LOADBALANCEENFORCER_TEST(TestName) GDK_TEST(Core, SpatialLoadBalanceEnforcer, TestName)

// Test Globals
namespace
{
const PhysicalWorkerName ThisWorker = TEXT("ThisWorker");
const PhysicalWorkerName OtherWorker = TEXT("OtherWorker");
const PhysicalWorkerName ClientWorker = TEXT("ClientWorker");
const PhysicalWorkerName OtherClientWorker = TEXT("OtherClientWorker");

const Worker_PartitionId ThisWorkerId = 101;
const Worker_PartitionId OtherWorkerId = 102;
const Worker_PartitionId ClientWorkerId = 103;
const Worker_PartitionId OtherClientWorkerId = 104;

constexpr VirtualWorkerId ThisVirtualWorker = 1;
constexpr VirtualWorkerId OtherVirtualWorker = 2;

constexpr Worker_EntityId EntityIdOne = 1;

constexpr Worker_ComponentId TestComponentIdOne = 123;
constexpr Worker_ComponentId TestComponentIdTwo = 456;

const TArray<Worker_ComponentId> PresentComponents = { SpatialConstants::AUTHORITY_DELEGATION_COMPONENT_ID,
													   SpatialConstants::AUTHORITY_INTENT_COMPONENT_ID,
													   SpatialConstants::NET_OWNING_CLIENT_WORKER_COMPONENT_ID,
													   SpatialConstants::CLIENT_ENDPOINT_COMPONENT_ID,
													   SpatialConstants::HEARTBEAT_COMPONENT_ID };
const TArray<Worker_ComponentId> NonAuthDelegationLBComponents = { SpatialConstants::AUTHORITY_INTENT_COMPONENT_ID,
																   SpatialConstants::NET_OWNING_CLIENT_WORKER_COMPONENT_ID };
const TArray<Worker_ComponentId> TestComponentIds = { TestComponentIdOne, TestComponentIdTwo };
const TArray<Worker_ComponentId> ClientComponentIds = { SpatialConstants::HEARTBEAT_COMPONENT_ID,
														SpatialConstants::CLIENT_ENDPOINT_COMPONENT_ID };

TUniquePtr<SpatialVirtualWorkerTranslator> CreateVirtualWorkerTranslator()
{
	TUniquePtr<SpatialVirtualWorkerTranslator> VirtualWorkerTranslator =
		MakeUnique<SpatialVirtualWorkerTranslator>(nullptr, nullptr, ThisWorker);

	Schema_Object* DataObject = TestingSchemaHelpers::CreateTranslationComponentDataFields();

	TestingSchemaHelpers::AddTranslationComponentDataMapping(DataObject, ThisVirtualWorker, ThisWorker, ThisWorkerId);
	TestingSchemaHelpers::AddTranslationComponentDataMapping(DataObject, OtherVirtualWorker, OtherWorker, OtherWorkerId);

	VirtualWorkerTranslator->ApplyVirtualWorkerManagerData(DataObject);

	return VirtualWorkerTranslator;
}

SpatialGDK::ComponentData MakeComponentDataFromData(const Worker_ComponentData Data)
{
	return SpatialGDK::ComponentData(SpatialGDK::OwningComponentDataPtr(Data.schema_type), Data.component_id);
}

SpatialGDK::ComponentUpdate MakeComponentUpdateFromUpdate(const Worker_ComponentUpdate Update)
{
	return SpatialGDK::ComponentUpdate(SpatialGDK::OwningComponentUpdatePtr(Update.schema_type), Update.component_id);
}

// Adds an entity to the view with correct LB components. Also assigns the current authority to the passed worker,
// and the auth intent to the passed virtual worker.
// Optionally pass a client name to designate the entity as net owned by that client.
void AddLBEntityToView(SpatialGDK::EntityView& View, const Worker_EntityId EntityId, const Worker_PartitionId AuthPartitionId,
					   const VirtualWorkerId IntentWorkerId,
					   const Worker_PartitionId ClientAuthPartitionId = SpatialConstants::INVALID_ENTITY_ID)
{
	AddEntityToView(View, EntityId);

	AuthorityDelegationMap DelegationMap;
	DelegationMap.Add(SpatialConstants::SERVER_AUTH_COMPONENT_SET_ID, AuthPartitionId);

	if (ClientAuthPartitionId != SpatialConstants::INVALID_ENTITY_ID)
	{
		DelegationMap.Add(SpatialConstants::CLIENT_AUTH_COMPONENT_SET_ID, ClientAuthPartitionId);
	}

	AddComponentToView(View, EntityId,
					   MakeComponentDataFromData(SpatialGDK::AuthorityDelegation(DelegationMap).CreateAuthorityDelegationData()));
	AddComponentToView(View, EntityId, MakeComponentDataFromData(SpatialGDK::AuthorityIntent::CreateAuthorityIntentData(IntentWorkerId)));
	AddComponentToView(
		View, EntityId,
		MakeComponentDataFromData(SpatialGDK::NetOwningClientWorker::CreateNetOwningClientWorkerData(ClientAuthPartitionId)));
	AddComponentToView(View, EntityId, SpatialGDK::ComponentData(SpatialConstants::CLIENT_ENDPOINT_COMPONENT_ID));
	AddComponentToView(View, EntityId, SpatialGDK::ComponentData(SpatialConstants::HEARTBEAT_COMPONENT_ID));
	AddComponentToView(View, EntityId, SpatialGDK::ComponentData(SpatialConstants::LB_TAG_COMPONENT_ID));

	AddAuthorityToView(View, EntityId, SpatialConstants::WELL_KNOWN_COMPONENT_SET_ID);
}

AuthorityDelegationMap GetAuthDelegationMapFromUpdate(const SpatialGDK::EntityComponentUpdate& Update)
{
	AuthorityDelegationMap AuthDelegation;
	Schema_Object* ComponentObject = Schema_GetComponentUpdateFields(Update.Update.GetUnderlying());
	// This is never emptied, so does not need an additional check for cleared fields
	uint32 KVPairCount = Schema_GetObjectCount(ComponentObject, 1);
	for (uint32 i = 0; i < KVPairCount; i++)
	{
		Schema_Object* KVPairObject = Schema_IndexObject(ComponentObject, 1, i);
		uint32 Key = Schema_GetUint32(KVPairObject, SCHEMA_MAP_KEY_FIELD_ID);
		Worker_PartitionId Value = Schema_GetInt64(KVPairObject, SCHEMA_MAP_VALUE_FIELD_ID);

		AuthDelegation.Add(Key, Value);
	}

	return AuthDelegation;
}

bool AuthorityMapDelegatesComponents(const AuthorityDelegationMap& DelegationMap, const Worker_PartitionId DelegatedPartitionId,
									 const TArray<Worker_ComponentId>& DelegatedComponents)
{
	for (Worker_ComponentId ComponentId : DelegatedComponents)
	{
		const auto Entry = DelegationMap.Find(ComponentId);
		if (Entry == nullptr)
		{
			return false;
		}
		if (*Entry != DelegatedPartitionId)
		{
			return false;
		}
	}
	return true;
}

} // anonymous namespace

LOADBALANCEENFORCER_TEST(GIVEN_a_view_with_no_data_WHEN_advance_load_balance_enforcer_THEN_return_no_auth_delegation_assignment_requests)
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

	// The view has no entities in it. We expect the enforcer not to produce any authority delegation requests.
	LoadBalanceEnforcer.Advance();
	TestFalse("LoadBalanceEnforcer did not try to send an authority delegation update", bInvoked);

	return true;
}

LOADBALANCEENFORCER_TEST(
	GIVEN_load_balance_enforcer_with_valid_mapping_WHEN_asked_for_auth_delegation_assignments_THEN_return_correct_auth_delegation_assignment_requests)
{
	SpatialGDK::FDispatcher Dispatcher;
	SpatialGDK::EntityView View;
	const TUniquePtr<SpatialVirtualWorkerTranslator> VirtualWorkerTranslator = CreateVirtualWorkerTranslator();

	AddLBEntityToView(View, EntityIdOne, ThisWorkerId, OtherVirtualWorker);

	SpatialGDK::FSubView SubView(SpatialConstants::LB_TAG_COMPONENT_ID, SpatialGDK::FSubView::NoFilter, &View, Dispatcher,
								 SpatialGDK::FSubView::NoDispatcherCallbacks);

	TArray<SpatialGDK::EntityComponentUpdate> Updates;

	SpatialGDK::SpatialLoadBalanceEnforcer LoadBalanceEnforcer = SpatialGDK::SpatialLoadBalanceEnforcer(
		ThisWorker, SubView, VirtualWorkerTranslator.Get(), [&Updates](SpatialGDK::EntityComponentUpdate Update) {
			Updates.Add(MoveTemp(Update));
		});

	LoadBalanceEnforcer.ShortCircuitMaybeRefreshAuthorityDelegation(EntityIdOne);

	bool bSuccess = true;
	if (Updates.Num() == 1)
	{
		bSuccess &= Updates[0].EntityId == EntityIdOne;
		bSuccess &= Updates[0].Update.GetComponentId() == SpatialConstants::AUTHORITY_DELEGATION_COMPONENT_ID;
		AuthorityDelegationMap AuthDelegationMap = GetAuthDelegationMapFromUpdate(Updates[0]);
		bSuccess &= AuthorityMapDelegatesComponents(AuthDelegationMap, OtherWorkerId, NonAuthDelegationLBComponents);
	}
	else
	{
		bSuccess = false;
	}

	TestTrue("LoadBalanceEnforcer returned expected authority delegation assignment results", bSuccess);

	return true;
}

LOADBALANCEENFORCER_TEST(GIVEN_authority_intent_change_op_WHEN_we_inform_load_balance_enforcer_THEN_send_auth_delegation_update)
{
	SpatialGDK::FDispatcher Dispatcher;
	SpatialGDK::EntityView View;
	SpatialGDK::ViewDelta Delta;
	const TUniquePtr<SpatialVirtualWorkerTranslator> VirtualWorkerTranslator = CreateVirtualWorkerTranslator();

	AddLBEntityToView(View, EntityIdOne, OtherWorkerId, ThisVirtualWorker);

	SpatialGDK::FSubView SubView(SpatialConstants::LB_TAG_COMPONENT_ID, SpatialGDK::FSubView::NoFilter, &View, Dispatcher,
								 SpatialGDK::FSubView::NoDispatcherCallbacks);

	TArray<SpatialGDK::EntityComponentUpdate> Updates;

	SpatialGDK::SpatialLoadBalanceEnforcer LoadBalanceEnforcer = SpatialGDK::SpatialLoadBalanceEnforcer(
		ThisWorker, SubView, VirtualWorkerTranslator.Get(), [&Updates](SpatialGDK::EntityComponentUpdate Update) {
			Updates.Add(MoveTemp(Update));
		});

	PopulateViewDeltaWithComponentUpdated(
		Delta, View, EntityIdOne,
		MakeComponentUpdateFromUpdate(SpatialGDK::AuthorityIntent::CreateAuthorityIntentUpdate(ThisVirtualWorker)));
	SubView.Advance(Delta);
	LoadBalanceEnforcer.Advance();

	bool bSuccess = true;
	if (Updates.Num() == 1)
	{
		bSuccess &= Updates[0].EntityId == EntityIdOne;
		bSuccess &= Updates[0].Update.GetComponentId() == SpatialConstants::AUTHORITY_DELEGATION_COMPONENT_ID;
		AuthorityDelegationMap AuthDelegationMap = GetAuthDelegationMapFromUpdate(Updates[0]);
		bSuccess &= AuthorityMapDelegatesComponents(AuthDelegationMap, ThisWorkerId, NonAuthDelegationLBComponents);
	}
	else
	{
		bSuccess = false;
	}

	TestTrue("LoadBalanceEnforcer returned expected authority delegation assignment results", bSuccess);

	return true;
}

LOADBALANCEENFORCER_TEST(
	GIVEN_net_owning_client_change_op_WHEN_we_advance_load_balance_enforcer_THEN_auth_delegation_update_contains_new_client_delegation)
{
	SpatialGDK::FDispatcher Dispatcher;
	SpatialGDK::EntityView View;
	SpatialGDK::ViewDelta Delta;
	const TUniquePtr<SpatialVirtualWorkerTranslator> VirtualWorkerTranslator = CreateVirtualWorkerTranslator();

	AddLBEntityToView(View, EntityIdOne, ThisWorkerId, ThisVirtualWorker, ClientWorkerId);

	SpatialGDK::FSubView SubView(SpatialConstants::LB_TAG_COMPONENT_ID, SpatialGDK::FSubView::NoFilter, &View, Dispatcher,
								 SpatialGDK::FSubView::NoDispatcherCallbacks);

	TArray<SpatialGDK::EntityComponentUpdate> Updates;

	SpatialGDK::SpatialLoadBalanceEnforcer LoadBalanceEnforcer = SpatialGDK::SpatialLoadBalanceEnforcer(
		ThisWorker, SubView, VirtualWorkerTranslator.Get(), [&Updates](SpatialGDK::EntityComponentUpdate Update) {
			Updates.Add(MoveTemp(Update));
		});

	// The net owning component uses the full workerdId:{worker} form for its data format, which is a real gotcha.
	PopulateViewDeltaWithComponentUpdated(
		Delta, View, EntityIdOne,
		MakeComponentUpdateFromUpdate(SpatialGDK::NetOwningClientWorker::CreateNetOwningClientWorkerUpdate(OtherClientWorkerId)));
	SubView.Advance(Delta);
	LoadBalanceEnforcer.Advance();

	bool bSuccess = true;
	if (Updates.Num() == 1)
	{
		bSuccess &= Updates[0].EntityId == EntityIdOne;
		bSuccess &= Updates[0].Update.GetComponentId() == SpatialConstants::AUTHORITY_DELEGATION_COMPONENT_ID;
		AuthorityDelegationMap AuthDelegationMap = GetAuthDelegationMapFromUpdate(Updates[0]);
		bSuccess &= AuthorityMapDelegatesComponents(AuthDelegationMap, OtherClientWorkerId, ClientComponentIds);
	}
	else
	{
		bSuccess = false;
	}

	TestTrue("LoadBalanceEnforcer returned expected authority delegation assignment results", bSuccess);

	return true;
}
