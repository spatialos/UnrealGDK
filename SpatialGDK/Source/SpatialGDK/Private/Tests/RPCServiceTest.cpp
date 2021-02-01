// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "CoreMinimal.h"

// Engine
#include "Misc/AutomationTest.h"
#include "Tests/TestDefinitions.h"

// GDK
#include "Interop/RPCs/SpatialRPCService.h"
#include "Interop/SpatialReceiver.h"
#include "Schema/RPCPayload.h"
#include "SpatialConstants.h"
#include "SpatialGDKSettings.h"
#include "SpatialView/ComponentData.h"
#include "SpatialView/Dispatcher.h"
#include "SpatialView/EntityComponentTypes.h"
#include "SpatialView/EntityView.h"
#include "SpatialView/SubView.h"
#include "SpatialView/ViewDelta.h"
#include "Tests/SpatialView/SpatialViewUtils.h"
#include "Utils/RPCRingBuffer.h"

#define RPC_SERVICE_TEST(TestName) GDK_TEST(Core, SpatialRPCService, TestName)

// Test Globals
namespace
{
enum ERPCEndpointType : uint8_t
{
	SERVER_AUTH,
	CLIENT_AUTH,
	SERVER_AND_CLIENT_AUTH,
	NO_AUTH
};

struct TestData
{
	UWorld* TestWorld = nullptr;
	AActor* Actor = nullptr;
};

struct EntityPayload
{
	EntityPayload(Worker_EntityId InEntityID, const SpatialGDK::RPCPayload& InPayload)
		: EntityId(InEntityID)
		, Payload(InPayload)
	{
	}

	Worker_EntityId EntityId;
	SpatialGDK::RPCPayload Payload;
};

constexpr Worker_EntityId RPCTestEntityId_1 = 201;
constexpr Worker_EntityId RPCTestEntityId_2 = 42;

const SpatialGDK::RPCPayload SimplePayload = SpatialGDK::RPCPayload(1, 0, 0, TArray<uint8>({ 1 }, 1));

// Initialise view and subviews. These will be overwritten before using.
SpatialGDK::EntityView TestView;
SpatialGDK::FDispatcher TestDispatcher;
SpatialGDK::FSubView AuthSubView = SpatialGDK::FSubView(SpatialConstants::ACTOR_AUTH_TAG_COMPONENT_ID, SpatialGDK::FSubView::NoFilter,
														&TestView, TestDispatcher, SpatialGDK::FSubView::NoDispatcherCallbacks);
SpatialGDK::FSubView NonAuthSubView =
	SpatialGDK::FSubView(SpatialConstants::ACTOR_NON_AUTH_TAG_COMPONENT_ID, SpatialGDK::FSubView::NoFilter, &TestView, TestDispatcher,
						 SpatialGDK::FSubView::NoDispatcherCallbacks);
FTimerManager Timer;

SpatialGDK::ComponentData MakeComponentDataFromData(Schema_ComponentData* Data, const Worker_ComponentId ComponentId)
{
	return SpatialGDK::ComponentData(SpatialGDK::OwningComponentDataPtr(Data), ComponentId);
}

void AddClientAuthorityFromRPCEndpointType(SpatialGDK::EntityView& View, const Worker_EntityId EntityId,
										   const ERPCEndpointType RPCEndpointType)
{
	if (RPCEndpointType == CLIENT_AUTH || RPCEndpointType == SERVER_AND_CLIENT_AUTH)
	{
		AddAuthorityToView(View, EntityId, SpatialConstants::CLIENT_AUTH_COMPONENT_SET_ID);
	}
}

void AddServerAuthorityFromRPCEndpointType(SpatialGDK::EntityView& View, const Worker_EntityId EntityId,
										   const ERPCEndpointType RPCEndpointType)
{
	if (RPCEndpointType == SERVER_AUTH || RPCEndpointType == SERVER_AND_CLIENT_AUTH)
	{
		AddAuthorityToView(View, EntityId, SpatialConstants::SERVER_AUTH_COMPONENT_SET_ID);
	}
}

void AddMulticastAuthorityFromRPCEndpointType(SpatialGDK::EntityView& View, const Worker_EntityId EntityId,
											  const ERPCEndpointType RPCEndpointType)
{
	if (RPCEndpointType == SERVER_AUTH || RPCEndpointType == SERVER_AND_CLIENT_AUTH)
	{
		AddAuthorityToView(View, EntityId, SpatialConstants::SERVER_AUTH_COMPONENT_SET_ID);
	}
}

void AddRPCEntityToView(SpatialGDK::EntityView& View, const Worker_EntityId EntityId, const ERPCEndpointType RPCEndpointType,
						SpatialGDK::ComponentData ClientData = SpatialGDK::ComponentData{ SpatialConstants::CLIENT_ENDPOINT_COMPONENT_ID })
{
	AddEntityToView(View, EntityId);
	AddComponentToView(View, EntityId, SpatialGDK::ComponentData{ SpatialConstants::ACTOR_NON_AUTH_TAG_COMPONENT_ID });
	if (RPCEndpointType != NO_AUTH)
	{
		AddComponentToView(View, EntityId, SpatialGDK::ComponentData{ SpatialConstants::ACTOR_AUTH_TAG_COMPONENT_ID });
	}
	AddComponentToView(View, EntityId, MoveTemp(ClientData));
	AddComponentToView(View, EntityId, SpatialGDK::ComponentData{ SpatialConstants::SERVER_ENDPOINT_COMPONENT_ID });
	AddComponentToView(View, EntityId, SpatialGDK::ComponentData{ SpatialConstants::MULTICAST_RPCS_COMPONENT_ID });
	AddClientAuthorityFromRPCEndpointType(View, EntityId, RPCEndpointType);
	AddServerAuthorityFromRPCEndpointType(View, EntityId, RPCEndpointType);
	AddMulticastAuthorityFromRPCEndpointType(View, EntityId, RPCEndpointType);
};

void PopulateView(SpatialGDK::EntityView& View, const TArray<Worker_EntityId>& EntityIdArray, const ERPCEndpointType RPCEndpointType)
{
	for (Worker_EntityId EntityId : EntityIdArray)
	{
		AddRPCEntityToView(View, EntityId, RPCEndpointType);
	}
}

// Creates an RPC service with initial authority dependent on the RPCEndpointType for the entities specified in
// EntityIdArray.
SpatialGDK::SpatialRPCService CreateRPCService(const TArray<Worker_EntityId>& EntityIdArray, const ERPCEndpointType RPCEndpointType,
											   SpatialGDK::EntityView& View)
{
	// Remove all callbacks.
	TestDispatcher = SpatialGDK::FDispatcher();
	// If the passed view is empty, we populate it with new entities with the given entity IDs.
	if (View.Num() == 0)
	{
		PopulateView(View, EntityIdArray, RPCEndpointType);
	}

	AuthSubView = SpatialGDK::FSubView(SpatialConstants::ACTOR_AUTH_TAG_COMPONENT_ID, SpatialGDK::FSubView::NoFilter, &View, TestDispatcher,
									   SpatialGDK::FSubView::NoDispatcherCallbacks);
	NonAuthSubView = SpatialGDK::FSubView(SpatialConstants::ACTOR_NON_AUTH_TAG_COMPONENT_ID, SpatialGDK::FSubView::NoFilter, &View,
										  TestDispatcher, SpatialGDK::FSubView::NoDispatcherCallbacks);
	SpatialGDK::SpatialRPCService RPCService = SpatialGDK::SpatialRPCService(AuthSubView, NonAuthSubView, nullptr, nullptr, nullptr);

	const SpatialGDK::ViewDelta Delta;
	AuthSubView.Advance(Delta);
	NonAuthSubView.Advance(Delta);

	RPCService.AdvanceView();
	RPCService.ProcessChanges(0);

	return RPCService;
}

bool CompareRPCPayload(const SpatialGDK::RPCPayload& Payload1, const SpatialGDK::RPCPayload& Payload2)
{
	return Payload1.Index == Payload2.Index && Payload1.Offset == Payload2.Offset && Payload1.PayloadData == Payload2.PayloadData;
}

bool CompareSchemaObjectToSendAndPayload(Schema_Object* SchemaObject, const SpatialGDK::RPCPayload& Payload, const ERPCType RPCType,
										 const uint64 RPCId)
{
	const SpatialGDK::RPCRingBufferDescriptor Descriptor = SpatialGDK::RPCRingBufferUtils::GetRingBufferDescriptor(RPCType);
	Schema_Object* RPCObject = Schema_GetObject(SchemaObject, Descriptor.GetRingBufferElementFieldId(RPCId));
	return CompareRPCPayload(SpatialGDK::RPCPayload(RPCObject), Payload);
}

bool CompareUpdateToSendAndEntityPayload(SpatialGDK::SpatialRPCService::UpdateToSend& Update, const EntityPayload& EntityPayloadItem,
										 ERPCType RPCType, uint64 RPCId)
{
	return CompareSchemaObjectToSendAndPayload(Schema_GetComponentUpdateFields(Update.Update.schema_type), EntityPayloadItem.Payload,
											   RPCType, RPCId)
		   && Update.EntityId == EntityPayloadItem.EntityId;
}

bool CompareComponentDataAndEntityPayload(const FWorkerComponentData& ComponentData, const EntityPayload& EntityPayloadItem,
										  const ERPCType RPCType, const uint64 RPCId)
{
	return CompareSchemaObjectToSendAndPayload(Schema_GetComponentDataFields(ComponentData.schema_type), EntityPayloadItem.Payload, RPCType,
											   RPCId);
}

FWorkerComponentData GetComponentDataOnEntityCreationFromRPCService(SpatialGDK::SpatialRPCService& RPCService,
																	const Worker_EntityId EntityID, const ERPCType RPCType)
{
	Worker_ComponentId ExpectedUpdateComponentId = SpatialGDK::RPCRingBufferUtils::GetRingBufferComponentId(RPCType);
	TArray<FWorkerComponentData> ComponentDataArray = RPCService.GetRPCComponentsOnEntityCreation(EntityID);

	const FWorkerComponentData* ComponentData =
		ComponentDataArray.FindByPredicate([ExpectedUpdateComponentId](const FWorkerComponentData& CompData) {
			return CompData.component_id == ExpectedUpdateComponentId;
		});

	if (ComponentData == nullptr)
	{
		return {};
	}
	return *ComponentData;
}

} // anonymous namespace

RPC_SERVICE_TEST(GIVEN_authority_over_server_endpoint_WHEN_push_client_reliable_rpcs_to_the_service_THEN_rpc_push_result_success)
{
	SpatialGDK::EntityView View;
	SpatialGDK::SpatialRPCService RPCService = CreateRPCService({ RPCTestEntityId_1 }, SERVER_AUTH, View);
	const SpatialGDK::EPushRPCResult Result = RPCService.PushRPC(RPCTestEntityId_1, ERPCType::ClientReliable, SimplePayload, false);
	TestTrue("Push RPC returned expected results", Result == SpatialGDK::EPushRPCResult::Success);
	return true;
}

RPC_SERVICE_TEST(GIVEN_authority_over_server_endpoint_WHEN_push_client_unreliable_rpcs_to_the_service_THEN_rpc_push_result_success)
{
	SpatialGDK::EntityView View;
	SpatialGDK::SpatialRPCService RPCService = CreateRPCService({ RPCTestEntityId_1 }, SERVER_AUTH, View);
	const SpatialGDK::EPushRPCResult Result = RPCService.PushRPC(RPCTestEntityId_1, ERPCType::ClientUnreliable, SimplePayload, false);
	TestTrue("Push RPC returned expected results", Result == SpatialGDK::EPushRPCResult::Success);
	return true;
}

RPC_SERVICE_TEST(
	GIVEN_authority_over_server_endpoint_WHEN_push_server_reliable_rpcs_to_the_service_THEN_rpc_push_result_no_buffer_authority)
{
	SpatialGDK::EntityView View;
	SpatialGDK::SpatialRPCService RPCService = CreateRPCService({ RPCTestEntityId_1 }, SERVER_AUTH, View);
	const SpatialGDK::EPushRPCResult Result = RPCService.PushRPC(RPCTestEntityId_1, ERPCType::ServerReliable, SimplePayload, false);
	TestTrue("Push RPC returned expected results", Result == SpatialGDK::EPushRPCResult::NoRingBufferAuthority);
	return true;
}

RPC_SERVICE_TEST(
	GIVEN_authority_over_server_endpoint_WHEN_push_server_unreliable_rpcs_to_the_service_THEN_rpc_push_result_no_buffer_authority)
{
	SpatialGDK::EntityView View;
	SpatialGDK::SpatialRPCService RPCService = CreateRPCService({ RPCTestEntityId_1 }, SERVER_AUTH, View);
	const SpatialGDK::EPushRPCResult Result = RPCService.PushRPC(RPCTestEntityId_1, ERPCType::ServerUnreliable, SimplePayload, false);
	TestTrue("Push RPC returned expected results", Result == SpatialGDK::EPushRPCResult::NoRingBufferAuthority);
	return true;
}

RPC_SERVICE_TEST(
	GIVEN_authority_over_client_endpoint_WHEN_push_client_reliable_rpcs_to_the_service_THEN_rpc_push_result_no_buffer_authority)
{
	SpatialGDK::EntityView View;
	SpatialGDK::SpatialRPCService RPCService = CreateRPCService({ RPCTestEntityId_1 }, CLIENT_AUTH, View);
	const SpatialGDK::EPushRPCResult Result = RPCService.PushRPC(RPCTestEntityId_1, ERPCType::ClientReliable, SimplePayload, false);
	TestTrue("Push RPC returned expected results", Result == SpatialGDK::EPushRPCResult::NoRingBufferAuthority);
	return true;
}

RPC_SERVICE_TEST(
	GIVEN_authority_over_client_endpoint_WHEN_push_client_unreliable_rpcs_to_the_service_THEN_rpc_push_result_no_buffer_authority)
{
	SpatialGDK::EntityView View;
	SpatialGDK::SpatialRPCService RPCService = CreateRPCService({ RPCTestEntityId_1 }, CLIENT_AUTH, View);
	const SpatialGDK::EPushRPCResult Result = RPCService.PushRPC(RPCTestEntityId_1, ERPCType::ClientUnreliable, SimplePayload, false);
	TestTrue("Push RPC returned expected results", Result == SpatialGDK::EPushRPCResult::NoRingBufferAuthority);
	return true;
}

RPC_SERVICE_TEST(GIVEN_authority_over_client_endpoint_WHEN_push_server_reliable_rpcs_to_the_service_THEN_rpc_push_result_success)
{
	SpatialGDK::EntityView View;
	SpatialGDK::SpatialRPCService RPCService = CreateRPCService({ RPCTestEntityId_1 }, CLIENT_AUTH, View);
	const SpatialGDK::EPushRPCResult Result = RPCService.PushRPC(RPCTestEntityId_1, ERPCType::ServerReliable, SimplePayload, false);
	TestTrue("Push RPC returned expected results", Result == SpatialGDK::EPushRPCResult::Success);
	return true;
}

RPC_SERVICE_TEST(GIVEN_authority_over_client_endpoint_WHEN_push_server_unreliable_rpcs_to_the_service_THEN_rpc_push_result_success)
{
	SpatialGDK::EntityView View;
	SpatialGDK::SpatialRPCService RPCService = CreateRPCService({ RPCTestEntityId_1 }, CLIENT_AUTH, View);
	const SpatialGDK::EPushRPCResult Result = RPCService.PushRPC(RPCTestEntityId_1, ERPCType::ServerUnreliable, SimplePayload, false);
	TestTrue("Push RPC returned expected results", Result == SpatialGDK::EPushRPCResult::Success);
	return true;
}

RPC_SERVICE_TEST(GIVEN_authority_over_client_endpoint_WHEN_push_multicast_rpcs_to_the_service_THEN_rpc_push_result_no_buffer_authority)
{
	SpatialGDK::EntityView View;
	SpatialGDK::SpatialRPCService RPCService = CreateRPCService({ RPCTestEntityId_1 }, CLIENT_AUTH, View);
	const SpatialGDK::EPushRPCResult Result = RPCService.PushRPC(RPCTestEntityId_1, ERPCType::NetMulticast, SimplePayload, false);
	TestTrue("Push RPC returned expected results", Result == SpatialGDK::EPushRPCResult::NoRingBufferAuthority);
	return true;
}

RPC_SERVICE_TEST(GIVEN_authority_over_server_endpoint_WHEN_push_multicast_rpcs_to_the_service_THEN_rpc_push_result_success)
{
	SpatialGDK::EntityView View;
	SpatialGDK::SpatialRPCService RPCService = CreateRPCService({ RPCTestEntityId_1 }, SERVER_AUTH, View);
	const SpatialGDK::EPushRPCResult Result = RPCService.PushRPC(RPCTestEntityId_1, ERPCType::NetMulticast, SimplePayload, false);
	TestTrue("Push RPC returned expected results", Result == SpatialGDK::EPushRPCResult::Success);
	return true;
}

RPC_SERVICE_TEST(GIVEN_authority_over_server_and_client_endpoint_WHEN_push_rpcs_to_the_service_THEN_rpc_push_result_has_ack_authority)
{
	SpatialGDK::EntityView View;
	SpatialGDK::SpatialRPCService RPCService = CreateRPCService({ RPCTestEntityId_1 }, SERVER_AND_CLIENT_AUTH, View);
	const SpatialGDK::EPushRPCResult Result = RPCService.PushRPC(RPCTestEntityId_1, ERPCType::ClientReliable, SimplePayload, false);
	TestTrue("Push RPC returned expected results", Result == SpatialGDK::EPushRPCResult::HasAckAuthority);
	return true;
}

RPC_SERVICE_TEST(
	GIVEN_authority_over_server_endpoint_WHEN_push_overflow_client_reliable_rpcs_to_the_service_THEN_rpc_push_result_queue_overflowed)
{
	SpatialGDK::EntityView View;
	SpatialGDK::SpatialRPCService RPCService = CreateRPCService({ RPCTestEntityId_1 }, SERVER_AUTH, View);

	// Send RPCs to the point where we will overflow
	const uint32 RPCsToSend = GetDefault<USpatialGDKSettings>()->GetRPCRingBufferSize(ERPCType::ClientReliable);
	for (uint32 i = 0; i < RPCsToSend; ++i)
	{
		RPCService.PushRPC(RPCTestEntityId_1, ERPCType::ClientReliable, SimplePayload, false);
	}

	const SpatialGDK::EPushRPCResult Result = RPCService.PushRPC(RPCTestEntityId_1, ERPCType::ClientReliable, SimplePayload, false);
	TestTrue("Push RPC returned expected results", Result == SpatialGDK::EPushRPCResult::QueueOverflowed);
	return true;
}

RPC_SERVICE_TEST(
	GIVEN_authority_over_server_endpoint_WHEN_push_overflow_client_unreliable_rpcs_to_the_service_THEN_rpc_push_result_drop_overflow)
{
	SpatialGDK::EntityView View;
	SpatialGDK::SpatialRPCService RPCService = CreateRPCService({ RPCTestEntityId_1 }, SERVER_AUTH, View);

	// Send RPCs to the point where we will overflow
	const uint32 RPCsToSend = GetDefault<USpatialGDKSettings>()->GetRPCRingBufferSize(ERPCType::ClientUnreliable);
	for (uint32 i = 0; i < RPCsToSend; ++i)
	{
		RPCService.PushRPC(RPCTestEntityId_1, ERPCType::ClientUnreliable, SimplePayload, false);
	}

	const SpatialGDK::EPushRPCResult Result = RPCService.PushRPC(RPCTestEntityId_1, ERPCType::ClientUnreliable, SimplePayload, false);
	TestTrue("Push RPC returned expected results", Result == SpatialGDK::EPushRPCResult::DropOverflowed);
	return true;
}

RPC_SERVICE_TEST(
	GIVEN_authority_over_client_endpoint_WHEN_push_overflow_client_reliable_rpcs_to_the_service_THEN_rpc_push_result_queue_overflowed)
{
	SpatialGDK::EntityView View;
	SpatialGDK::SpatialRPCService RPCService = CreateRPCService({ RPCTestEntityId_1 }, CLIENT_AUTH, View);

	// Send RPCs to the point where we will overflow
	const uint32 RPCsToSend = GetDefault<USpatialGDKSettings>()->GetRPCRingBufferSize(ERPCType::ServerReliable);
	for (uint32 i = 0; i < RPCsToSend; ++i)
	{
		RPCService.PushRPC(RPCTestEntityId_1, ERPCType::ServerReliable, SimplePayload, false);
	}

	const SpatialGDK::EPushRPCResult Result = RPCService.PushRPC(RPCTestEntityId_1, ERPCType::ServerReliable, SimplePayload, false);
	TestTrue("Push RPC returned expected results", Result == SpatialGDK::EPushRPCResult::QueueOverflowed);
	return true;
}

RPC_SERVICE_TEST(
	GIVEN_authority_over_client_endpoint_WHEN_push_overflow_client_unreliable_rpcs_to_the_service_THEN_rpc_push_result_drop_overflow)
{
	SpatialGDK::EntityView View;
	SpatialGDK::SpatialRPCService RPCService = CreateRPCService({ RPCTestEntityId_1 }, CLIENT_AUTH, View);

	// Send RPCs to the point where we will overflow
	const uint32 RPCsToSend = GetDefault<USpatialGDKSettings>()->GetRPCRingBufferSize(ERPCType::ServerUnreliable);
	for (uint32 i = 0; i < RPCsToSend; ++i)
	{
		RPCService.PushRPC(RPCTestEntityId_1, ERPCType::ServerUnreliable, SimplePayload, false);
	}

	const SpatialGDK::EPushRPCResult Result = RPCService.PushRPC(RPCTestEntityId_1, ERPCType::ServerUnreliable, SimplePayload, false);
	TestTrue("Push RPC returned expected results", (Result == SpatialGDK::EPushRPCResult::DropOverflowed));
	return true;
}

RPC_SERVICE_TEST(GIVEN_authority_over_server_endpoint_WHEN_push_overflow_multicast_rpcs_to_the_service_THEN_rpc_push_result_success)
{
	SpatialGDK::EntityView View;
	SpatialGDK::SpatialRPCService RPCService = CreateRPCService({ RPCTestEntityId_1 }, SERVER_AUTH, View);

	// Send RPCs to the point where we will overflow
	const uint32 RPCsToSend = GetDefault<USpatialGDKSettings>()->GetRPCRingBufferSize(ERPCType::NetMulticast);
	for (uint32 i = 0; i < RPCsToSend; ++i)
	{
		RPCService.PushRPC(RPCTestEntityId_1, ERPCType::NetMulticast, SimplePayload, false);
	}

	const SpatialGDK::EPushRPCResult Result = RPCService.PushRPC(RPCTestEntityId_1, ERPCType::NetMulticast, SimplePayload, false);
	TestTrue("Push RPC returned expected results", Result == SpatialGDK::EPushRPCResult::Success);
	return true;
}

RPC_SERVICE_TEST(
	GIVEN_authority_over_client_endpoint_WHEN_push_server_unreliable_rpcs_to_the_service_THEN_payloads_are_written_correctly_to_component_updates)
{
	SpatialGDK::EntityView View;
	TArray<EntityPayload> EntityPayloads;
	EntityPayloads.Add(EntityPayload(RPCTestEntityId_1, SimplePayload));
	EntityPayloads.Add(EntityPayload(RPCTestEntityId_2, SimplePayload));

	TArray<Worker_EntityId> EntityIdArray;
	EntityIdArray.Add(RPCTestEntityId_1);
	EntityIdArray.Add(RPCTestEntityId_2);

	SpatialGDK::SpatialRPCService RPCService = CreateRPCService(EntityIdArray, CLIENT_AUTH, View);
	for (const EntityPayload& EntityPayloadItem : EntityPayloads)
	{
		RPCService.PushRPC(EntityPayloadItem.EntityId, ERPCType::ServerUnreliable, EntityPayloadItem.Payload, false);
	}

	TArray<SpatialGDK::SpatialRPCService::UpdateToSend> UpdateToSendArray = RPCService.GetRPCsAndAcksToSend();

	bool bTestPassed = true;
	if (UpdateToSendArray.Num() != EntityPayloads.Num())
	{
		bTestPassed = false;
	}
	else
	{
		for (int i = 0; i < EntityPayloads.Num(); ++i)
		{
			if (!CompareUpdateToSendAndEntityPayload(UpdateToSendArray[i], EntityPayloads[i], ERPCType::ServerUnreliable, 1))
			{
				bTestPassed = false;
				break;
			}
		}
	}

	TestTrue("UpdateToSend have expected payloads", bTestPassed);
	return true;
}

RPC_SERVICE_TEST(GIVEN_no_authority_over_rpc_endpoint_WHEN_push_client_reliable_rpcs_to_the_service_THEN_component_data_matches_payload)
{
	SpatialGDK::EntityView View;
	// Create RPCService with empty component view
	SpatialGDK::SpatialRPCService RPCService = CreateRPCService({}, NO_AUTH, View);

	RPCService.PushRPC(RPCTestEntityId_1, ERPCType::ClientReliable, SimplePayload, false);

	const FWorkerComponentData ComponentData =
		GetComponentDataOnEntityCreationFromRPCService(RPCService, RPCTestEntityId_1, ERPCType::ClientReliable);
	const bool bTestPassed =
		CompareComponentDataAndEntityPayload(ComponentData, EntityPayload(RPCTestEntityId_1, SimplePayload), ERPCType::ClientReliable, 1);
	TestTrue("Entity creation test returned expected results", bTestPassed);
	return true;
}

RPC_SERVICE_TEST(GIVEN_no_authority_over_rpc_endpoint_WHEN_push_multicast_rpcs_to_the_service_THEN_initially_present_set)
{
	SpatialGDK::EntityView View;
	// Create RPCService with empty component view
	SpatialGDK::SpatialRPCService RPCService = CreateRPCService({}, NO_AUTH, View);

	RPCService.PushRPC(RPCTestEntityId_1, ERPCType::NetMulticast, SimplePayload, false);
	RPCService.PushRPC(RPCTestEntityId_1, ERPCType::NetMulticast, SimplePayload, false);

	const FWorkerComponentData ComponentData =
		GetComponentDataOnEntityCreationFromRPCService(RPCService, RPCTestEntityId_1, ERPCType::NetMulticast);
	const Schema_Object* SchemaObject = Schema_GetComponentDataFields(ComponentData.schema_type);
	const uint32 InitiallyPresent =
		Schema_GetUint32(SchemaObject, SpatialGDK::RPCRingBufferUtils::GetInitiallyPresentMulticastRPCsCountFieldId());
	TestTrue("Entity creation multicast test returned expected results", (InitiallyPresent == 2));
	return true;
}

RPC_SERVICE_TEST(GIVEN_ring_buffer_sizes_WHEN_get_descriptors_and_ack_field_ids_THEN_ring_buffer_descriptors_and_ack_field_ids_are_consistent)
{
	// Client Endpoint should have this layout:
	// - Server Reliable buffer
	// - Server Reliable last sent ID
	// - Server Unreliable buffer
	// - Server Unreliable last sent ID
	// - Client Reliable ack
	// - Client Unreliable ack
	{
		Schema_FieldId CurrentId = 1;

		{
			const SpatialGDK::RPCRingBufferDescriptor ServerReliableDescriptor =
				SpatialGDK::RPCRingBufferUtils::GetRingBufferDescriptor(ERPCType::ServerReliable);
			TestTrue("Server Reliable buffer starts as expected", ServerReliableDescriptor.SchemaFieldStart == CurrentId);
			const uint32 ExpectedBufferSize = SpatialGDK::RPCRingBufferUtils::GetRingBufferSize(ERPCType::ServerReliable);
			TestTrue("Server Reliable buffer has expected size", ServerReliableDescriptor.RingBufferSize == ExpectedBufferSize);
			CurrentId += ExpectedBufferSize;
			TestTrue("Server Reliable last sent ID field has expected value", ServerReliableDescriptor.LastSentRPCFieldId == CurrentId);
			CurrentId++;
		}

		{
			const SpatialGDK::RPCRingBufferDescriptor ServerUnreliableDescriptor =
				SpatialGDK::RPCRingBufferUtils::GetRingBufferDescriptor(ERPCType::ServerUnreliable);
			TestTrue("Server Unreliable buffer starts as expected", ServerUnreliableDescriptor.SchemaFieldStart == CurrentId);
			const uint32 ExpectedBufferSize = SpatialGDK::RPCRingBufferUtils::GetRingBufferSize(ERPCType::ServerUnreliable);
			TestTrue("Server Unreliable buffer has expected size", ServerUnreliableDescriptor.RingBufferSize == ExpectedBufferSize);
			CurrentId += ExpectedBufferSize;
			TestTrue("Server Unreliable last sent ID field has expected value", ServerUnreliableDescriptor.LastSentRPCFieldId == CurrentId);
			CurrentId++;
		}

		{
			const Schema_FieldId ClientReliableAckFieldId = SpatialGDK::RPCRingBufferUtils::GetAckFieldId(ERPCType::ClientReliable);
			TestTrue("Client Reliable ack field ID has expected value", ClientReliableAckFieldId == CurrentId);
			CurrentId++;
		}

		{
			const Schema_FieldId ClientUnreliableAckFieldId = SpatialGDK::RPCRingBufferUtils::GetAckFieldId(ERPCType::ClientUnreliable);
			TestTrue("Client Unreliable ack field ID has expected value", ClientUnreliableAckFieldId == CurrentId);
			CurrentId++;
		}
	}

	// Server Endpoint should have this layout:
	// - Client Reliable buffer
	// - Client Reliable last sent ID
	// - Client Unreliable buffer
	// - Client Unreliable last sent ID
	// - Server Reliable ack
	// - Server Unreliable ack
	{
		Schema_FieldId CurrentId = 1;

		{
			const SpatialGDK::RPCRingBufferDescriptor ClientReliableDescriptor =
				SpatialGDK::RPCRingBufferUtils::GetRingBufferDescriptor(ERPCType::ClientReliable);
			TestTrue("Client Reliable buffer starts as expected", ClientReliableDescriptor.SchemaFieldStart == CurrentId);
			const uint32 ExpectedBufferSize = SpatialGDK::RPCRingBufferUtils::GetRingBufferSize(ERPCType::ClientReliable);
			TestTrue("Client Reliable buffer has expected size", ClientReliableDescriptor.RingBufferSize == ExpectedBufferSize);
			CurrentId += ExpectedBufferSize;
			TestTrue("Client Reliable last sent ID field has expected value", ClientReliableDescriptor.LastSentRPCFieldId == CurrentId);
			CurrentId++;
		}

		{
			const SpatialGDK::RPCRingBufferDescriptor ClientUnreliableDescriptor =
				SpatialGDK::RPCRingBufferUtils::GetRingBufferDescriptor(ERPCType::ClientUnreliable);
			TestTrue("Client Unreliable buffer starts as expected", ClientUnreliableDescriptor.SchemaFieldStart == CurrentId);
			const uint32 ExpectedBufferSize = SpatialGDK::RPCRingBufferUtils::GetRingBufferSize(ERPCType::ClientUnreliable);
			TestTrue("Client Unreliable buffer has expected size", ClientUnreliableDescriptor.RingBufferSize == ExpectedBufferSize);
			CurrentId += ExpectedBufferSize;
			TestTrue("Client Unreliable last sent ID field has expected value", ClientUnreliableDescriptor.LastSentRPCFieldId == CurrentId);
			CurrentId++;
		}

		{
			const Schema_FieldId ServerReliableAckFieldId = SpatialGDK::RPCRingBufferUtils::GetAckFieldId(ERPCType::ServerReliable);
			TestTrue("Server Reliable ack field ID has expected value", ServerReliableAckFieldId == CurrentId);
			CurrentId++;
		}

		{
			const Schema_FieldId ServerUnreliableAckFieldId = SpatialGDK::RPCRingBufferUtils::GetAckFieldId(ERPCType::ServerUnreliable);
			TestTrue("Server Unreliable ack field ID has expected value", ServerUnreliableAckFieldId == CurrentId);
			CurrentId++;
		}
	}

	// Multicast RPCs should have this layout:
	// - Multicast buffer
	// - Multicast last sent ID
	// - Initially present RPCs count
	{
		Schema_FieldId CurrentId = 1;

		{
			const SpatialGDK::RPCRingBufferDescriptor MulticastDescriptor =
				SpatialGDK::RPCRingBufferUtils::GetRingBufferDescriptor(ERPCType::NetMulticast);
			TestTrue("Multicast buffer starts as expected", MulticastDescriptor.SchemaFieldStart == CurrentId);
			const uint32 ExpectedBufferSize = SpatialGDK::RPCRingBufferUtils::GetRingBufferSize(ERPCType::NetMulticast);
			TestTrue("Multicast buffer has expected size", MulticastDescriptor.RingBufferSize == ExpectedBufferSize);
			CurrentId += ExpectedBufferSize;
			TestTrue("Multicast last sent ID field has expected value", MulticastDescriptor.LastSentRPCFieldId == CurrentId);
			CurrentId++;
		}

		{
			const Schema_FieldId InitiallyPresentRPCsFieldId = SpatialGDK::RPCRingBufferUtils::GetInitiallyPresentMulticastRPCsCountFieldId();
			TestTrue("Initially Present Multicast RPCs Count field ID has expected value", InitiallyPresentRPCsFieldId == CurrentId);
			CurrentId++;
		}
	}

	return true;
}
