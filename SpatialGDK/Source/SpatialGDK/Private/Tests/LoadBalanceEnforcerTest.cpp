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
const PhysicalWorkerName UnrealWorker = TEXT("UnrealWorker");
const WorkerRequirementSet UnrealRequirementSet = WorkerRequirementSet{ WorkerAttributeSet{ UnrealWorker } };

const PhysicalWorkerName ThisWorker = TEXT("ThisWorker");
const PhysicalWorkerName OtherWorker = TEXT("OtherWorker");
const PhysicalWorkerName ClientWorker = TEXT("ClientWorker");
const PhysicalWorkerName OtherClientWorker = TEXT("OtherClientWorker");

const WorkerRequirementSet ThisRequirementSet =
	WorkerRequirementSet{ WorkerAttributeSet{ FString::Printf(TEXT("workerId:%s"), *ThisWorker) } };
const WorkerRequirementSet OtherRequirementSet =
	WorkerRequirementSet{ WorkerAttributeSet{ FString::Printf(TEXT("workerId:%s"), *OtherWorker) } };
const WorkerRequirementSet OtherClientRequirementSet =
	WorkerRequirementSet{ WorkerAttributeSet{ FString::Printf(TEXT("workerId:%s"), *OtherClientWorker) } };

constexpr VirtualWorkerId ThisVirtualWorker = 1;
constexpr VirtualWorkerId OtherVirtualWorker = 2;

constexpr FEntityId EntityIdOne = 1;

constexpr Worker_ComponentId TestComponentIdOne = 123;
constexpr Worker_ComponentId TestComponentIdTwo = 456;

const TArray<Worker_ComponentId> PresentComponents = {
	SpatialConstants::ENTITY_ACL_COMPONENT_ID,		   SpatialConstants::AUTHORITY_INTENT_COMPONENT_ID,
	SpatialConstants::COMPONENT_PRESENCE_COMPONENT_ID, SpatialConstants::NET_OWNING_CLIENT_WORKER_COMPONENT_ID,
	SpatialConstants::CLIENT_ENDPOINT_COMPONENT_ID,	   SpatialConstants::HEARTBEAT_COMPONENT_ID
};
const TArray<Worker_ComponentId> NonAclLBComponents = { SpatialConstants::AUTHORITY_INTENT_COMPONENT_ID,
														SpatialConstants::COMPONENT_PRESENCE_COMPONENT_ID,
														SpatialConstants::NET_OWNING_CLIENT_WORKER_COMPONENT_ID };
const TArray<Worker_ComponentId> TestComponentIds = { TestComponentIdOne, TestComponentIdTwo };
const TArray<Worker_ComponentId> ClientComponentIds = { SpatialConstants::HEARTBEAT_COMPONENT_ID,
														SpatialConstants::CLIENT_ENDPOINT_COMPONENT_ID };

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

SpatialGDK::ComponentUpdate MakeComponentUpdateFromUpdate(Worker_ComponentUpdate Update)
{
	return SpatialGDK::ComponentUpdate(SpatialGDK::OwningComponentUpdatePtr(Update.schema_type), Update.component_id);
}

// Adds an entity to the view with correct LB components. Also assigns the current authority to the passed worker,
// and the auth intent to the passed virtual worker. The ACL is always assigned to "UnrealWorker".
// Optionally pass a client name to designate the entity as net owned by that client.
void AddLBEntityToView(
	SpatialGDK::EntityView& View, const FEntityId EntityId, const WorkerRequirementSet AuthRequirementSet,
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
			WorkerRequirementSet{ WorkerAttributeSet{ FString::Printf(TEXT("workerId:%s"), *ClientWorkerName.GetValue()) } };
		WriteAcl.Add(SpatialConstants::CLIENT_ENDPOINT_COMPONENT_ID, AuthClientRequirementSet);
		WriteAcl.Add(SpatialConstants::HEARTBEAT_COMPONENT_ID, AuthClientRequirementSet);
	}

	AddComponentToView(View, EntityId, MakeComponentDataFromData(SpatialGDK::EntityAcl(ReadAcl, WriteAcl).CreateEntityAclData()));
	AddComponentToView(View, EntityId, MakeComponentDataFromData(SpatialGDK::AuthorityIntent::CreateAuthorityIntentData(IntentWorkerId)));
	AddComponentToView(View, EntityId,
					   MakeComponentDataFromData(SpatialGDK::ComponentPresence::CreateComponentPresenceData(PresentComponents)));
	AddComponentToView(View, EntityId,
					   MakeComponentDataFromData(SpatialGDK::NetOwningClientWorker::CreateNetOwningClientWorkerData(ClientWorkerName)));
	AddComponentToView(View, EntityId, SpatialGDK::ComponentData(SpatialConstants::CLIENT_ENDPOINT_COMPONENT_ID));
	AddComponentToView(View, EntityId, SpatialGDK::ComponentData(SpatialConstants::HEARTBEAT_COMPONENT_ID));
	AddComponentToView(View, EntityId, SpatialGDK::ComponentData(SpatialConstants::LB_TAG_COMPONENT_ID));

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

bool AclMapDelegatesComponents(const WriteAclMap& AclMap, const WorkerRequirementSet DelegatedRequirementSet,
							   const TArray<Worker_ComponentId>& DelegatedComponents)
{
	for (Worker_ComponentId ComponentId : DelegatedComponents)
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
	TestFalse("LoadBalanceEnforcer did not try to send an ACL update", bInvoked);

	return true;
}

LOADBALANCEENFORCER_TEST(
	GIVEN_load_balance_enforcer_with_valid_mapping_WHEN_asked_for_acl_assignments_THEN_return_correct_acl_assignment_requests)
{
	SpatialGDK::FDispatcher Dispatcher;
	SpatialGDK::EntityView View;
	const TUniquePtr<SpatialVirtualWorkerTranslator> VirtualWorkerTranslator = CreateVirtualWorkerTranslator();

	AddLBEntityToView(View, EntityIdOne, ThisRequirementSet, OtherVirtualWorker);

	SpatialGDK::FSubView SubView(SpatialConstants::LB_TAG_COMPONENT_ID, SpatialGDK::FSubView::NoFilter, &View, Dispatcher,
								 SpatialGDK::FSubView::NoDispatcherCallbacks);

	TArray<SpatialGDK::EntityComponentUpdate> Updates;

	SpatialGDK::SpatialLoadBalanceEnforcer LoadBalanceEnforcer = SpatialGDK::SpatialLoadBalanceEnforcer(
		ThisWorker, SubView, VirtualWorkerTranslator.Get(), [&Updates](SpatialGDK::EntityComponentUpdate Update) {
			Updates.Add(MoveTemp(Update));
		});

	LoadBalanceEnforcer.ShortCircuitMaybeRefreshAcl(EntityIdOne);

	bool bSuccess = true;
	if (Updates.Num() == 1)
	{
		bSuccess &= Updates[0].EntityId == EntityIdOne;
		bSuccess &= Updates[0].Update.GetComponentId() == SpatialConstants::ENTITY_ACL_COMPONENT_ID;
		WriteAclMap AclMap = GetWriteAclMapFromUpdate(Updates[0]);
		bSuccess &= AclMapDelegatesComponents(AclMap, OtherRequirementSet, NonAclLBComponents);
	}
	else
	{
		bSuccess = false;
	}

	TestTrue("LoadBalanceEnforcer returned expected ACL assignment results", bSuccess);

	return true;
}

LOADBALANCEENFORCER_TEST(GIVEN_authority_intent_change_op_WHEN_we_inform_load_balance_enforcer_THEN_send_acl_update)
{
	SpatialGDK::FDispatcher Dispatcher;
	SpatialGDK::EntityView View;
	SpatialGDK::ViewDelta Delta;
	const TUniquePtr<SpatialVirtualWorkerTranslator> VirtualWorkerTranslator = CreateVirtualWorkerTranslator();

	AddLBEntityToView(View, EntityIdOne, OtherRequirementSet, ThisVirtualWorker);

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
		bSuccess &= Updates[0].Update.GetComponentId() == SpatialConstants::ENTITY_ACL_COMPONENT_ID;
		WriteAclMap AclMap = GetWriteAclMapFromUpdate(Updates[0]);
		bSuccess &= AclMapDelegatesComponents(AclMap, ThisRequirementSet, NonAclLBComponents);
	}
	else
	{
		bSuccess = false;
	}

	TestTrue("LoadBalanceEnforcer returned expected ACL assignment results", bSuccess);

	return true;
}

LOADBALANCEENFORCER_TEST(GIVEN_component_presence_change_op_WHEN_we_advance_load_balance_enforcer_THEN_acl_update_contains_new_components)
{
	SpatialGDK::FDispatcher Dispatcher;
	SpatialGDK::EntityView View;
	SpatialGDK::ViewDelta Delta;
	const TUniquePtr<SpatialVirtualWorkerTranslator> VirtualWorkerTranslator = CreateVirtualWorkerTranslator();

	AddLBEntityToView(View, EntityIdOne, ThisRequirementSet, ThisVirtualWorker);

	SpatialGDK::FSubView SubView(SpatialConstants::LB_TAG_COMPONENT_ID, SpatialGDK::FSubView::NoFilter, &View, Dispatcher,
								 SpatialGDK::FSubView::NoDispatcherCallbacks);

	TArray<SpatialGDK::EntityComponentUpdate> Updates;

	SpatialGDK::SpatialLoadBalanceEnforcer LoadBalanceEnforcer = SpatialGDK::SpatialLoadBalanceEnforcer(
		ThisWorker, SubView, VirtualWorkerTranslator.Get(), [&Updates](SpatialGDK::EntityComponentUpdate Update) {
			Updates.Add(MoveTemp(Update));
		});

	// Create a ComponentPresence component update op with the required components.
	TArray<Worker_ComponentId> NewPresentIds = PresentComponents;
	NewPresentIds.Append(TestComponentIds);
	Worker_ComponentUpdate Update;
	Update.component_id = SpatialConstants::COMPONENT_PRESENCE_COMPONENT_ID;
	Update.schema_type = Schema_CreateComponentUpdate();
	Schema_Object* UpdateFields = Schema_GetComponentUpdateFields(Update.schema_type);
	Schema_AddUint32List(UpdateFields, SpatialConstants::COMPONENT_PRESENCE_COMPONENT_LIST_ID, NewPresentIds.GetData(),
						 NewPresentIds.Num());

	PopulateViewDeltaWithComponentUpdated(Delta, View, EntityIdOne, MakeComponentUpdateFromUpdate(Update));
	SubView.Advance(Delta);
	LoadBalanceEnforcer.Advance();

	bool bSuccess = true;
	if (Updates.Num() == 1)
	{
		bSuccess &= Updates[0].EntityId == EntityIdOne;
		bSuccess &= Updates[0].Update.GetComponentId() == SpatialConstants::ENTITY_ACL_COMPONENT_ID;
		WriteAclMap AclMap = GetWriteAclMapFromUpdate(Updates[0]);
		bSuccess &= AclMapDelegatesComponents(AclMap, ThisRequirementSet, TestComponentIds);
	}
	TestTrue("LoadBalanceEnforcer returned expected ACL assignment results", bSuccess);

	return true;
}

LOADBALANCEENFORCER_TEST(
	GIVEN_net_owning_client_change_op_WHEN_we_advance_load_balance_enforcer_THEN_acl_update_contains_new_client_delegation)
{
	SpatialGDK::FDispatcher Dispatcher;
	SpatialGDK::EntityView View;
	SpatialGDK::ViewDelta Delta;
	const TUniquePtr<SpatialVirtualWorkerTranslator> VirtualWorkerTranslator = CreateVirtualWorkerTranslator();

	AddLBEntityToView(View, EntityIdOne, ThisRequirementSet, ThisVirtualWorker, ClientWorker);

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
		MakeComponentUpdateFromUpdate(SpatialGDK::NetOwningClientWorker::CreateNetOwningClientWorkerUpdate(
			FString::Printf(TEXT("workerId:%s"), *OtherClientWorker))));
	SubView.Advance(Delta);
	LoadBalanceEnforcer.Advance();

	bool bSuccess = true;
	if (Updates.Num() == 1)
	{
		bSuccess &= Updates[0].EntityId == EntityIdOne;
		bSuccess &= Updates[0].Update.GetComponentId() == SpatialConstants::ENTITY_ACL_COMPONENT_ID;
		WriteAclMap AclMap = GetWriteAclMapFromUpdate(Updates[0]);
		bSuccess &= AclMapDelegatesComponents(AclMap, OtherClientRequirementSet, ClientComponentIds);
	}
	else
	{
		bSuccess = false;
	}

	TestTrue("LoadBalanceEnforcer returned expected ACL assignment results", bSuccess);

	return true;
}
