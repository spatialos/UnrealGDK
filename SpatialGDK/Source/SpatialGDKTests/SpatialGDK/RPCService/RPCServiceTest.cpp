// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "CoreMinimal.h"
#include "Interop/SpatialRPCService.h"
#include "Interop/SpatialStaticComponentView.h"
#include "Schema/RPCPayload.h"
#include "SpatialConstants.h"
#include "SpatialGDKSettings.h"
#include "TestDefinitions.h"
#include "Utils/RPCRingBuffer.h"

#define RPC_SERVICE_TEST(TestName) \
	GDK_TEST(Core, SpatialRPCService, TestName)

// Test Globals
namespace
{
	enum ERPCEndpointType : uint8_t
	{
		INVALID,
		SERVER_AUTH,
		CLIENT_AUTH,
		SERVER_AND_CLIENT_AUTH,
		NO_AUTH
	};

	struct EntityPayload
	{
		EntityPayload(Worker_EntityId InEntityID, const SpatialGDK::RPCPayload& InPayload)
			: EntityId(InEntityID)
			, Payload(InPayload)
		{}

		Worker_EntityId EntityId;
		SpatialGDK::RPCPayload Payload;
	};

	Worker_EntityId RPCTestEntityId_1 = 1;
	Worker_EntityId RPCTestEntityId_2 = 24;
	Worker_EntityId RPCTestEntityId_3 = 65;
	Worker_EntityId RPCTestEntityId_4 = 4;
	Worker_EntityId RPCTestEntityId_5 = 100;

	SpatialGDK::RPCPayload SimplePayload = SpatialGDK::RPCPayload(1, 0, TArray<uint8>({ 1 }, 1));

	ExtractRPCDelegate DefaultRPCDelegate = ExtractRPCDelegate::CreateLambda([](Worker_EntityId EntityId, ERPCType RPCType, const SpatialGDK::RPCPayload& Payload) {
		return true;
	});

	void AddEntityComponentToStaticComponentView(USpatialStaticComponentView& StaticComponentView,
		const SpatialGDK::EntityComponentId& EntityComponentId,
		Schema_ComponentData* ComponentData,
		Worker_Authority Authority)
	{
		Worker_AddComponentOp AddComponentOp;
		AddComponentOp.entity_id = EntityComponentId.EntityId;
		AddComponentOp.data.component_id = EntityComponentId.ComponentId;
		AddComponentOp.data.schema_type = ComponentData;
		StaticComponentView.OnAddComponent(AddComponentOp);

		Worker_AuthorityChangeOp AuthorityChangeOp;
		AuthorityChangeOp.entity_id = EntityComponentId.EntityId;
		AuthorityChangeOp.component_id = EntityComponentId.ComponentId;
		AuthorityChangeOp.authority = Authority;
		StaticComponentView.OnAuthorityChange(AuthorityChangeOp);
	}

	void AddEntityComponentToStaticComponentView(USpatialStaticComponentView& StaticComponentView, const SpatialGDK::EntityComponentId& EntityComponentId, Worker_Authority Authority)
	{
		AddEntityComponentToStaticComponentView(StaticComponentView, EntityComponentId, Schema_CreateComponentData(), Authority);
	}

	Worker_Authority GetClientAuthorityFromRPCEndpointType(ERPCEndpointType RPCEndpointType)
	{
		switch (RPCEndpointType)
		{
		case CLIENT_AUTH:
		case SERVER_AND_CLIENT_AUTH:
			return WORKER_AUTHORITY_AUTHORITATIVE;
			break;
		case SERVER_AUTH:
		default:
			return WORKER_AUTHORITY_NOT_AUTHORITATIVE;
			break;
		}
	}

	Worker_Authority GetServerAuthorityFromRPCEndpointType(ERPCEndpointType RPCEndpointType)
	{
		switch (RPCEndpointType)
		{
		case SERVER_AUTH:
		case SERVER_AND_CLIENT_AUTH:
			return WORKER_AUTHORITY_AUTHORITATIVE;
			break;
		case CLIENT_AUTH:
		default:
			return WORKER_AUTHORITY_NOT_AUTHORITATIVE;
			break;
		}
	}

	Worker_Authority GetMultiCastAuthorityFromRPCEndpointType(ERPCEndpointType RPCEndpointType)
	{
		return GetServerAuthorityFromRPCEndpointType(RPCEndpointType);
	}

	void AddEntityToStaticComponentView(USpatialStaticComponentView& StaticComponentView, Worker_EntityId EntityId, ERPCEndpointType RPCEndpointType)
	{
		Worker_Authority ClientAuth = GetClientAuthorityFromRPCEndpointType(RPCEndpointType);
		Worker_Authority ServerAuth = GetServerAuthorityFromRPCEndpointType(RPCEndpointType);
		Worker_Authority MultiCastAuth = GetMultiCastAuthorityFromRPCEndpointType(RPCEndpointType);

		AddEntityComponentToStaticComponentView(StaticComponentView,
			SpatialGDK::EntityComponentId(EntityId, SpatialConstants::CLIENT_ENDPOINT_COMPONENT_ID),
			ClientAuth);

		AddEntityComponentToStaticComponentView(StaticComponentView,
			SpatialGDK::EntityComponentId(EntityId, SpatialConstants::SERVER_ENDPOINT_COMPONENT_ID),
			ServerAuth);

		AddEntityComponentToStaticComponentView(StaticComponentView,
			SpatialGDK::EntityComponentId(EntityId, SpatialConstants::MULTICAST_RPCS_COMPONENT_ID),
			MultiCastAuth);
	};

	USpatialStaticComponentView* CreateStaticComponentView(const TArray<Worker_EntityId>& EntityIdArray, ERPCEndpointType RPCEndpointType)
	{
		USpatialStaticComponentView* StaticComponentView = NewObject<USpatialStaticComponentView>();
		for (Worker_EntityId EntityId : EntityIdArray)
		{
			AddEntityToStaticComponentView(*StaticComponentView, EntityId, RPCEndpointType);
		}
		return StaticComponentView;
	}

	SpatialGDK::SpatialRPCService CreateRPCService(const TArray<Worker_EntityId>& EntityIdArray,
		ERPCEndpointType RPCEndpointType,
		ExtractRPCDelegate RPCDelegate = DefaultRPCDelegate,
		USpatialStaticComponentView* StaticComponentView = nullptr)
	{
		if (StaticComponentView == nullptr)
		{
			StaticComponentView = CreateStaticComponentView(EntityIdArray, RPCEndpointType);
		}

		SpatialGDK::SpatialRPCService RPCService = SpatialGDK::SpatialRPCService(RPCDelegate, StaticComponentView);

		for (const Worker_EntityId& EntityId : EntityIdArray)
		{
			if (RPCEndpointType == CLIENT_AUTH ||
				RPCEndpointType == SERVER_AND_CLIENT_AUTH)
			{
				RPCService.OnEndpointAuthorityGained(EntityId, SpatialConstants::CLIENT_ENDPOINT_COMPONENT_ID);
			}

			if (RPCEndpointType == SERVER_AUTH ||
				RPCEndpointType == SERVER_AND_CLIENT_AUTH)
			{
				RPCService.OnEndpointAuthorityGained(EntityId, SpatialConstants::SERVER_ENDPOINT_COMPONENT_ID);
			}
		}

		return RPCService;
	}

	bool CompareRPCPayload(const SpatialGDK::RPCPayload& Payload1, const SpatialGDK::RPCPayload& Payload2)
	{
		return Payload1.Index == Payload2.Index &&
			   Payload1.Offset == Payload2.Offset &&
			   Payload1.PayloadData == Payload2.PayloadData;
	}

	bool CompareSchemaObjectToSendAndEntityPayload(const Schema_Object* SchemaObject, const EntityPayload& EntityPayloadItem)
	{
		return CompareRPCPayload(SpatialGDK::RPCPayload(SchemaObject), EntityPayloadItem.Payload);
	}

	bool CompareUpdateToSendAndEntityPayload(SpatialGDK::SpatialRPCService::UpdateToSend& Update, const EntityPayload& EntityPayloadItem)
	{
		const Schema_Object* ComponentObject = Schema_GetComponentUpdateFields(Update.Update.schema_type);
		return CompareSchemaObjectToSendAndEntityPayload(ComponentObject, EntityPayloadItem) &&
			   Update.EntityId == EntityPayloadItem.EntityId;
	}

	bool CompareComponentDataAndEntityPayload(const Worker_ComponentData& ComponentData, const EntityPayload& EntityPayloadItem)
	{
		const Schema_Object* ComponentObject = Schema_GetComponentDataFields(ComponentData.schema_type);
		return CompareSchemaObjectToSendAndEntityPayload(ComponentObject, EntityPayloadItem);
	}

	bool CompareUpdateToSendArrayAndEntityPayloadArray(TArray<SpatialGDK::SpatialRPCService::UpdateToSend> &UpdatedToSendArray, const TArray<EntityPayload>& EntityPayloadArray)
	{
		if (UpdatedToSendArray.Num() != EntityPayloadArray.Num())
		{
			return false;
		}

		for (int i = 0; i < EntityPayloadArray.Num(); ++i)
		{
			const EntityPayload& EntityPayloadItem = EntityPayloadArray[i];
			SpatialGDK::SpatialRPCService::UpdateToSend& Update = UpdatedToSendArray[i];

			if (!CompareUpdateToSendAndEntityPayload(Update, EntityPayloadItem))
			{
				return false;
			}
		}

		return true;
	}

	const Worker_ComponentData GetComponentDataOnEntityCreationFromRPCService(SpatialGDK::SpatialRPCService& RPCService, Worker_EntityId EntityID, ERPCType RPCType)
	{
		Worker_ComponentId ExpectedUpdateComponentId = SpatialGDK::RPCRingBufferUtils::GetRingBufferComponentId(RPCType);
		TArray<Worker_ComponentData> ComponentDataArray = RPCService.GetRPCComponentsOnEntityCreation(EntityID);

		const Worker_ComponentData* ComponentData = ComponentDataArray.FindByPredicate([ExpectedUpdateComponentId](const Worker_ComponentData& ComponentData) {
			return ComponentData.component_id == ExpectedUpdateComponentId;
		});

		if (ComponentData == nullptr)
		{
			return {};
		}
		return *ComponentData;
	}
} // anonymous namespace

RPC_SERVICE_TEST(GIVEN_authority_over_server_endpoint_WHEN_push_client_reliable_rpcs_to_the_service_THEN_check_success)
{
	SpatialGDK::SpatialRPCService RPCService = CreateRPCService({ RPCTestEntityId_1 }, SERVER_AUTH);
	SpatialGDK::EPushRPCResult Result = RPCService.PushRPC(RPCTestEntityId_1, ERPCType::ClientReliable, SimplePayload);
	bool bTestPassed = Result == SpatialGDK::EPushRPCResult::Success;
	TestTrue("Push RPC returned expected results", bTestPassed);
	return bTestPassed;
}

RPC_SERVICE_TEST(GIVEN_authority_over_server_endpoint_WHEN_push_client_unreliable_rpcs_to_the_service_THEN_check_success)
{
	SpatialGDK::SpatialRPCService RPCService = CreateRPCService({ RPCTestEntityId_1 }, SERVER_AUTH);
	SpatialGDK::EPushRPCResult Result = RPCService.PushRPC(RPCTestEntityId_1, ERPCType::ClientUnreliable, SimplePayload);
	bool bTestPassed = Result == SpatialGDK::EPushRPCResult::Success;
	TestTrue("Push RPC returned expected results", bTestPassed);
	return bTestPassed;
}

RPC_SERVICE_TEST(GIVEN_authority_over_server_endpoint_WHEN_push_server_reliable_rpcs_to_the_service_THEN_check_no_buffer_authority)
{
	SpatialGDK::SpatialRPCService RPCService = CreateRPCService({ RPCTestEntityId_1 }, SERVER_AUTH);
	SpatialGDK::EPushRPCResult Result = RPCService.PushRPC(RPCTestEntityId_1, ERPCType::ServerReliable, SimplePayload);
	bool bTestPassed = Result == SpatialGDK::EPushRPCResult::NoRingBufferAuthority;
	TestTrue("Push RPC returned expected results", bTestPassed);
	return bTestPassed;
}

RPC_SERVICE_TEST(GIVEN_authority_over_server_endpoint_WHEN_push_server_unreliable_rpcs_to_the_service_THEN_check_no_buffer_authority)
{
	SpatialGDK::SpatialRPCService RPCService = CreateRPCService({ RPCTestEntityId_1 }, SERVER_AUTH);
	SpatialGDK::EPushRPCResult Result = RPCService.PushRPC(RPCTestEntityId_1, ERPCType::ServerUnreliable, SimplePayload);
	bool bTestPassed = Result == SpatialGDK::EPushRPCResult::NoRingBufferAuthority;
	TestTrue("Push RPC returned expected results", bTestPassed);
	return bTestPassed;
}

RPC_SERVICE_TEST(GIVEN_authority_over_client_endpoint_WHEN_push_client_reliable_rpcs_to_the_service_THEN_check_no_buffer_authority)
{
	SpatialGDK::SpatialRPCService RPCService = CreateRPCService({ RPCTestEntityId_1 }, CLIENT_AUTH);
	SpatialGDK::EPushRPCResult Result = RPCService.PushRPC(RPCTestEntityId_1, ERPCType::ClientReliable, SimplePayload);
	bool bTestPassed = Result == SpatialGDK::EPushRPCResult::NoRingBufferAuthority;
	TestTrue("Push RPC returned expected results", bTestPassed);
	return bTestPassed;
}

RPC_SERVICE_TEST(GIVEN_authority_over_client_endpoint_WHEN_push_client_unreliable_rpcs_to_the_service_THEN_check_no_buffer_authority)
{
	SpatialGDK::SpatialRPCService RPCService = CreateRPCService({ RPCTestEntityId_1 }, CLIENT_AUTH);
	SpatialGDK::EPushRPCResult Result = RPCService.PushRPC(RPCTestEntityId_1, ERPCType::ClientUnreliable, SimplePayload);
	bool bTestPassed = Result == SpatialGDK::EPushRPCResult::NoRingBufferAuthority;
	TestTrue("Push RPC returned expected results", bTestPassed);
	return bTestPassed;
}

RPC_SERVICE_TEST(GIVEN_authority_over_client_endpoint_WHEN_push_server_reliable_rpcs_to_the_service_THEN_check_success)
{
	SpatialGDK::SpatialRPCService RPCService = CreateRPCService({ RPCTestEntityId_1 }, CLIENT_AUTH);
	SpatialGDK::EPushRPCResult Result = RPCService.PushRPC(RPCTestEntityId_1, ERPCType::ServerReliable, SimplePayload);
	bool bTestPassed = Result == SpatialGDK::EPushRPCResult::Success;
	TestTrue("Push RPC returned expected results", bTestPassed);
	return bTestPassed;
}

RPC_SERVICE_TEST(GIVEN_authority_over_client_endpoint_WHEN_push_server_unreliable_rpcs_to_the_service_THEN_check_success)
{
	SpatialGDK::SpatialRPCService RPCService = CreateRPCService({ RPCTestEntityId_1 }, CLIENT_AUTH);
	SpatialGDK::EPushRPCResult Result = RPCService.PushRPC(RPCTestEntityId_1, ERPCType::ServerUnreliable, SimplePayload);
	bool bTestPassed = Result == SpatialGDK::EPushRPCResult::Success;
	TestTrue("Push RPC returned expected results", bTestPassed);
	return bTestPassed;
}

RPC_SERVICE_TEST(GIVEN_authority_over_client_endpoint_WHEN_push_multicast_rpcs_to_the_service_THEN_check_success)
{
	SpatialGDK::SpatialRPCService RPCService = CreateRPCService({ RPCTestEntityId_1 }, CLIENT_AUTH);
	SpatialGDK::EPushRPCResult Result = RPCService.PushRPC(RPCTestEntityId_1, ERPCType::NetMulticast, SimplePayload);
	bool bTestPassed = Result == SpatialGDK::EPushRPCResult::NoRingBufferAuthority;
	TestTrue("Push RPC returned expected results", bTestPassed);
	return bTestPassed;
}

RPC_SERVICE_TEST(GIVEN_authority_over_server_endpoint_WHEN_push_multicast_rpcs_to_the_service_THEN_check_success)
{
	SpatialGDK::SpatialRPCService RPCService = CreateRPCService({ RPCTestEntityId_1 }, SERVER_AUTH);
	SpatialGDK::EPushRPCResult Result = RPCService.PushRPC(RPCTestEntityId_1, ERPCType::NetMulticast, SimplePayload);
	bool bTestPassed = Result == SpatialGDK::EPushRPCResult::Success;
	TestTrue("Push RPC returned expected results", bTestPassed);
	return bTestPassed;
}

RPC_SERVICE_TEST(GIVEN_authority_over_server_and_client_endpoint_WHEN_push_rpcs_to_the_service_THEN_check_push_results)
{
	SpatialGDK::SpatialRPCService RPCService = CreateRPCService({ RPCTestEntityId_1 }, SERVER_AND_CLIENT_AUTH);
	SpatialGDK::EPushRPCResult Result = RPCService.PushRPC(RPCTestEntityId_1, ERPCType::ClientReliable, SimplePayload);
	bool bTestPassed = Result == SpatialGDK::EPushRPCResult::HasAckAuthority;
	TestTrue("Push RPC returned expected results", bTestPassed);
	return bTestPassed;
}

RPC_SERVICE_TEST(GIVEN_authority_over_server_endpoint_WHEN_push_overflow_client_reliable_rpcs_to_the_service_THEN_check_push_results)
{
	SpatialGDK::SpatialRPCService RPCService = CreateRPCService({ RPCTestEntityId_1 }, SERVER_AUTH);

	// Send RPCs to the point where we will overflow
	int RPCsToSend = static_cast<int>(GetDefault<USpatialGDKSettings>()->GetRPCRingBufferSize(ERPCType::ClientReliable));
	for (int i = 0; i < RPCsToSend; ++i)
	{
		RPCService.PushRPC(RPCTestEntityId_1, ERPCType::ClientReliable, SimplePayload);
	}

	SpatialGDK::EPushRPCResult Result = RPCService.PushRPC(RPCTestEntityId_1, ERPCType::ClientReliable, SimplePayload);
	bool bTestPassed = Result == SpatialGDK::EPushRPCResult::QueueOverflowed;
	TestTrue("Push RPC returned expected results", bTestPassed);
	return bTestPassed;
}

RPC_SERVICE_TEST(GIVEN_authority_over_server_endpoint_WHEN_push_overflow_client_unreliable_rpcs_to_the_service_THEN_check_push_results)
{
	SpatialGDK::SpatialRPCService RPCService = CreateRPCService({ RPCTestEntityId_1 }, SERVER_AUTH);

	// Send RPCs to the point where we will overflow
	int RPCsToSend = static_cast<int>(GetDefault<USpatialGDKSettings>()->GetRPCRingBufferSize(ERPCType::ClientReliable));
	for (int i = 0; i < RPCsToSend; ++i)
	{
		RPCService.PushRPC(RPCTestEntityId_1, ERPCType::ClientUnreliable, SimplePayload);
	}

	SpatialGDK::EPushRPCResult Result = RPCService.PushRPC(RPCTestEntityId_1, ERPCType::ClientUnreliable, SimplePayload);
	bool bTestPassed = Result == SpatialGDK::EPushRPCResult::DropOverflowed;
	TestTrue("Push RPC returned expected results", bTestPassed);
	return bTestPassed;
}

RPC_SERVICE_TEST(GIVEN_authority_over_client_endpoint_WHEN_push_overflow_client_reliable_rpcs_to_the_service_THEN_check_push_results)
{
	SpatialGDK::SpatialRPCService RPCService = CreateRPCService({ RPCTestEntityId_1 }, CLIENT_AUTH);

	// Send RPCs to the point where we will overflow
	int RPCsToSend = static_cast<int>(GetDefault<USpatialGDKSettings>()->GetRPCRingBufferSize(ERPCType::ServerReliable));
	for (int i = 0; i < RPCsToSend; ++i)
	{
		RPCService.PushRPC(RPCTestEntityId_1, ERPCType::ServerReliable, SimplePayload);
	}

	SpatialGDK::EPushRPCResult Result = RPCService.PushRPC(RPCTestEntityId_1, ERPCType::ServerReliable, SimplePayload);
	bool bTestPassed = Result == SpatialGDK::EPushRPCResult::QueueOverflowed;
	TestTrue("Push RPC returned expected results", bTestPassed);
	return bTestPassed;
}

RPC_SERVICE_TEST(GIVEN_authority_over_client_endpoint_WHEN_push_overflow_client_unreliable_rpcs_to_the_service_THEN_check_push_results)
{
	SpatialGDK::SpatialRPCService RPCService = CreateRPCService({ RPCTestEntityId_1 }, CLIENT_AUTH);

	// Send RPCs to the point where we will overflow
	int RPCsToSend = static_cast<int>(GetDefault<USpatialGDKSettings>()->GetRPCRingBufferSize(ERPCType::ServerReliable));
	for (int i = 0; i < RPCsToSend; ++i)
	{
		RPCService.PushRPC(RPCTestEntityId_1, ERPCType::ServerUnreliable, SimplePayload);
	}

	SpatialGDK::EPushRPCResult Result = RPCService.PushRPC(RPCTestEntityId_1, ERPCType::ServerUnreliable, SimplePayload);
	bool bTestPassed = Result == SpatialGDK::EPushRPCResult::DropOverflowed;
	TestTrue("Push RPC returned expected results", bTestPassed);
	return bTestPassed;
}

RPC_SERVICE_TEST(GIVEN_authority_over_server_endpoint_WHEN_push_overflow_multicast_rpcs_to_the_service_THEN_check_push_results)
{
	SpatialGDK::SpatialRPCService RPCService = CreateRPCService({ RPCTestEntityId_1 }, SERVER_AUTH);

	// Send RPCs to the point where we will overflow
	int RPCsToSend = static_cast<int>(GetDefault<USpatialGDKSettings>()->GetRPCRingBufferSize(ERPCType::NetMulticast));
	for (int i = 0; i < RPCsToSend; ++i)
	{
		RPCService.PushRPC(RPCTestEntityId_1, ERPCType::NetMulticast, SimplePayload);
	}

	SpatialGDK::EPushRPCResult Result = RPCService.PushRPC(RPCTestEntityId_1, ERPCType::NetMulticast, SimplePayload);
	bool bTestPassed = Result == SpatialGDK::EPushRPCResult::DropOverflowed;
	TestTrue("Push RPC returned expected results", bTestPassed);
	return bTestPassed;
}

RPC_SERVICE_TEST(GIVEN_authority_over_client_endpoint_WHEN_push_server_unreliable_rpcs_to_the_service_THEN_check_payloads_are_writen_correctly_to_component_updates)
{
	TArray<EntityPayload> EntityPayloads;
	EntityPayloads.Add(EntityPayload(RPCTestEntityId_1, SimplePayload));
	EntityPayloads.Add(EntityPayload(RPCTestEntityId_2, SimplePayload));
	EntityPayloads.Add(EntityPayload(RPCTestEntityId_3, SimplePayload));
	EntityPayloads.Add(EntityPayload(RPCTestEntityId_4, SimplePayload));
	EntityPayloads.Add(EntityPayload(RPCTestEntityId_5, SimplePayload));

	TArray<Worker_EntityId> EntityIdArray;
	EntityIdArray.Add(RPCTestEntityId_1);
	EntityIdArray.Add(RPCTestEntityId_2);
	EntityIdArray.Add(RPCTestEntityId_3);
	EntityIdArray.Add(RPCTestEntityId_4);
	EntityIdArray.Add(RPCTestEntityId_5);

	SpatialGDK::SpatialRPCService RPCService = CreateRPCService(EntityIdArray, CLIENT_AUTH);
	for (const EntityPayload& EntityPayloadItem : EntityPayloads)
	{
		RPCService.PushRPC(EntityPayloadItem.EntityId, ERPCType::ServerUnreliable, SimplePayload);
	}

	TArray<SpatialGDK::SpatialRPCService::UpdateToSend> UpdateToSendArray = RPCService.GetRPCsAndAcksToSend();
	bool bTestPassed = CompareUpdateToSendArrayAndEntityPayloadArray(UpdateToSendArray, EntityPayloads);
	TestTrue("UpdateToSend have expected payloads", bTestPassed);
	return bTestPassed;
}

RPC_SERVICE_TEST(GIVEN_no_authority_over_rpc_endpoint_WHEN_push_client_reliable_rpcs_to_the_service_THEN_check_component_data)
{
	// Create RPCService with empty component view
	SpatialGDK::SpatialRPCService RPCService = CreateRPCService({}, INVALID);

	RPCService.PushRPC(RPCTestEntityId_1, ERPCType::ClientReliable, SimplePayload);

	Worker_ComponentData ComponentData = GetComponentDataOnEntityCreationFromRPCService(RPCService, RPCTestEntityId_1, ERPCType::ClientReliable);
	bool bTestPassed = CompareComponentDataAndEntityPayload(ComponentData, EntityPayload(RPCTestEntityId_1, SimplePayload));
	TestTrue("Entity creation test returned expected results", bTestPassed);
	return bTestPassed;
}

RPC_SERVICE_TEST(GIVEN_no_authority_over_rpc_endpoint_WHEN_push_multicast_rpcs_to_the_service_THEN_check_component_data)
{
	// Create RPCService with empty component view
	SpatialGDK::SpatialRPCService RPCService = CreateRPCService({}, INVALID);

	const static int RPCsToSend = 5;
	for (int i = 0; i < RPCsToSend; ++i)
	{
		RPCService.PushRPC(RPCTestEntityId_1, ERPCType::NetMulticast, SimplePayload);
	}

	Worker_ComponentData ComponentData = GetComponentDataOnEntityCreationFromRPCService(RPCService, RPCTestEntityId_1, ERPCType::NetMulticast);
	const Schema_Object* SchemaObject = Schema_GetComponentDataFields(ComponentData.schema_type);
	uint32 InitiallyPresent = Schema_GetUint32(SchemaObject, SpatialGDK::RPCRingBufferUtils::GetInitiallyPresentMulticastRPCsCountFieldId());
	bool bTestPassed = InitiallyPresent == RPCsToSend;

	TestTrue("Entity creation multi-cast test returned expected results", bTestPassed);
	return bTestPassed;
}

RPC_SERVICE_TEST(GIVEN_authority_over_server_endpoint_WHEN_push_server_reliable_rpcs_to_the_service_THEN_check_component_data)
{
	USpatialStaticComponentView* StaticComponentView = NewObject<USpatialStaticComponentView>();

	Worker_Authority ClientAuth = GetClientAuthorityFromRPCEndpointType(SERVER_AUTH);
	Worker_Authority ServerAuth = GetServerAuthorityFromRPCEndpointType(SERVER_AUTH);

	Schema_ComponentData* ClientComponentData = Schema_CreateComponentData();
	Schema_Object* ClientSchemaObject = Schema_GetComponentDataFields(ClientComponentData);
	SpatialGDK::RPCRingBufferUtils::WriteRPCToSchema(ClientSchemaObject, ERPCType::ClientReliable, 1, SimplePayload);
	SpatialGDK::RPCRingBufferUtils::WriteRPCToSchema(ClientSchemaObject, ERPCType::ClientReliable, 2, SimplePayload);
	SpatialGDK::RPCRingBufferUtils::WriteRPCToSchema(ClientSchemaObject, ERPCType::ClientReliable, 3, SimplePayload);
	SpatialGDK::RPCRingBufferUtils::WriteRPCToSchema(ClientSchemaObject, ERPCType::ClientReliable, 4, SimplePayload);
	SpatialGDK::RPCRingBufferUtils::WriteRPCToSchema(ClientSchemaObject, ERPCType::ClientReliable, 5, SimplePayload);

	AddEntityComponentToStaticComponentView(*StaticComponentView,
		SpatialGDK::EntityComponentId(RPCTestEntityId_1, SpatialConstants::CLIENT_ENDPOINT_COMPONENT_ID),
		ClientComponentData,
		ClientAuth);

	AddEntityComponentToStaticComponentView(*StaticComponentView,
		SpatialGDK::EntityComponentId(RPCTestEntityId_1, SpatialConstants::SERVER_ENDPOINT_COMPONENT_ID),
		ServerAuth);

	bool bAttemptedRPCExtraction = false;
	bool bPayloadsMatch = true;
	ExtractRPCDelegate RPCDelegate = ExtractRPCDelegate::CreateLambda([&bAttemptedRPCExtraction, &bPayloadsMatch](Worker_EntityId EntityId, ERPCType RPCType, const SpatialGDK::RPCPayload& Payload) {
		bAttemptedRPCExtraction = true;
		bPayloadsMatch &= CompareRPCPayload(Payload, SimplePayload);
		bPayloadsMatch &= EntityId == RPCTestEntityId_1;
		return true;
	});

	SpatialGDK::SpatialRPCService RPCService = CreateRPCService({ RPCTestEntityId_1 }, SERVER_AUTH, RPCDelegate, StaticComponentView);
	RPCService.ExtractRPCsForEntity(RPCTestEntityId_1, SpatialConstants::CLIENT_ENDPOINT_COMPONENT_ID);

	bool bTestPassed = bAttemptedRPCExtraction && bPayloadsMatch;
	TestTrue("Extracted RPCs match expected payloads", bTestPassed);
	return bTestPassed;
}

RPC_SERVICE_TEST(GIVEN_authority_over_server_endpoint_WHEN_return_false_from_extract_callback_THEN_check_rpcs_persist_on_component)
{
	USpatialStaticComponentView* StaticComponentView = NewObject<USpatialStaticComponentView>();

	Worker_Authority ClientAuth = GetClientAuthorityFromRPCEndpointType(SERVER_AUTH);
	Worker_Authority ServerAuth = GetServerAuthorityFromRPCEndpointType(SERVER_AUTH);

	Schema_ComponentData* ClientComponentData = Schema_CreateComponentData();
	Schema_Object* ClientSchemaObject = Schema_GetComponentDataFields(ClientComponentData);
	SpatialGDK::RPCRingBufferUtils::WriteRPCToSchema(ClientSchemaObject, ERPCType::ClientReliable, 1, SimplePayload);
	SpatialGDK::RPCRingBufferUtils::WriteRPCToSchema(ClientSchemaObject, ERPCType::ClientReliable, 2, SimplePayload);
	SpatialGDK::RPCRingBufferUtils::WriteRPCToSchema(ClientSchemaObject, ERPCType::ClientReliable, 3, SimplePayload);
	SpatialGDK::RPCRingBufferUtils::WriteRPCToSchema(ClientSchemaObject, ERPCType::ClientReliable, 4, SimplePayload);
	SpatialGDK::RPCRingBufferUtils::WriteRPCToSchema(ClientSchemaObject, ERPCType::ClientReliable, 5, SimplePayload);

	AddEntityComponentToStaticComponentView(*StaticComponentView,
		SpatialGDK::EntityComponentId(RPCTestEntityId_1, SpatialConstants::CLIENT_ENDPOINT_COMPONENT_ID),
		ClientComponentData,
		ClientAuth);

	AddEntityComponentToStaticComponentView(*StaticComponentView,
		SpatialGDK::EntityComponentId(RPCTestEntityId_1, SpatialConstants::SERVER_ENDPOINT_COMPONENT_ID),
		ServerAuth);

	const static int8 MaxRPCsToProccess = 2;
	int8 RPCsToProccess = MaxRPCsToProccess;
	ExtractRPCDelegate RPCDelegate = ExtractRPCDelegate::CreateLambda([&RPCsToProccess](Worker_EntityId EntityId, ERPCType RPCType, const SpatialGDK::RPCPayload& Payload) {
		bool bKeepProcessing = RPCsToProccess != 0;
		if (bKeepProcessing)
		{
			RPCsToProccess--;
		}
		return bKeepProcessing;
	});

	SpatialGDK::SpatialRPCService RPCService = CreateRPCService({ RPCTestEntityId_1 }, SERVER_AUTH, RPCDelegate, StaticComponentView);

	RPCService.ExtractRPCsForEntity(RPCTestEntityId_1, SpatialConstants::CLIENT_ENDPOINT_COMPONENT_ID);

	TArray<SpatialGDK::SpatialRPCService::UpdateToSend> UpdateToSendArray = RPCService.GetRPCsAndAcksToSend();

	bool bTestPassed = false;
	SpatialGDK::SpatialRPCService::UpdateToSend* Update = UpdateToSendArray.FindByPredicate([](const SpatialGDK::SpatialRPCService::UpdateToSend& Update){
		return Update.Update.component_id == SpatialConstants::SERVER_ENDPOINT_COMPONENT_ID;
	});

	if (Update)
	{
		const Schema_Object* ComponentObject = Schema_GetComponentUpdateFields(Update->Update.schema_type);
		uint64 Ack = 0;
		SpatialGDK::RPCRingBufferUtils::ReadAckFromSchema(ComponentObject, ERPCType::ClientReliable, Ack);
		bTestPassed = MaxRPCsToProccess == Ack;
	}

	TestTrue("Returning false in extraction callback correctly stopped processing RPCs", bTestPassed);
	return bTestPassed;
}
