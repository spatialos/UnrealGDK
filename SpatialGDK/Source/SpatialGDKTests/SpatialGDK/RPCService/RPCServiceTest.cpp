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
	EntityPayload SimpleEntityPayload = EntityPayload(RPCTestEntityId_1, SimplePayload);

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

	TUniquePtr<SpatialGDK::SpatialRPCService> CreateRPCService(const TArray<Worker_EntityId>& EntityIdArray,
		ERPCEndpointType RPCEndpointType,
		ExtractRPCDelegate RPCDelegate = DefaultRPCDelegate,
		USpatialStaticComponentView* StaticComponentView = nullptr)
	{
		if (StaticComponentView == nullptr)
		{
			StaticComponentView = CreateStaticComponentView(EntityIdArray, RPCEndpointType);
		}

		TUniquePtr<SpatialGDK::SpatialRPCService> RPCService = MakeUnique<SpatialGDK::SpatialRPCService>(RPCDelegate, StaticComponentView);

		for (const Worker_EntityId& EntityId : EntityIdArray)
		{
			if (RPCEndpointType == CLIENT_AUTH ||
				RPCEndpointType == SERVER_AND_CLIENT_AUTH)
			{
				RPCService->OnEndpointAuthorityGained(EntityId, SpatialConstants::CLIENT_ENDPOINT_COMPONENT_ID);
			}

			if (RPCEndpointType == SERVER_AUTH ||
				RPCEndpointType == SERVER_AND_CLIENT_AUTH)
			{
				RPCService->OnEndpointAuthorityGained(EntityId, SpatialConstants::SERVER_ENDPOINT_COMPONENT_ID);
			}
		}

		return RPCService;
	}

	bool CheckPushRPC(SpatialGDK::SpatialRPCService& RPCService, const EntityPayload& EntityPayloadItem, ERPCType RPCType, SpatialGDK::EPushRPCResult ExpectedResult)
	{
		SpatialGDK::EPushRPCResult Result = RPCService.PushRPC(EntityPayloadItem.EntityId, RPCType, EntityPayloadItem.Payload);
		return Result == ExpectedResult;
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

	const Worker_ComponentData GetComponentDataOnEntityCreationFromRPCService(SpatialGDK::SpatialRPCService& RPCService, Worker_EntityId EntityID, ERPCType RPCType)
	{
		Worker_ComponentId ExpectedUpdateComponentId = SpatialGDK::RPCRingBufferUtils::GetRingBufferComponentId(RPCType);
		TArray<Worker_ComponentData> ComponentDataArray = RPCService.GetRPCComponentsOnEntityCreation(EntityID);
		const Worker_ComponentData* ComponentData = ComponentDataArray.FindByPredicate([ExpectedUpdateComponentId](const Worker_ComponentData& ComponentData) { return ComponentData.component_id == ExpectedUpdateComponentId; });
		if (ComponentData == nullptr)
		{
			return {};
		}
		return *ComponentData;
	}
}

namespace RPCServiceTests
{
	bool PushRPCTest(const TArray<Worker_EntityId>& EntityIdArray,
		ERPCEndpointType RPCEndpointType,
		ERPCType RPCType,
		const EntityPayload& EntityPayloadItem,
		SpatialGDK::EPushRPCResult ExpectedResult)
	{
		if (EntityIdArray.Num() == 0)
		{
			return false;
		}

		TUniquePtr<SpatialGDK::SpatialRPCService> RPCService = CreateRPCService(EntityIdArray, RPCEndpointType);
		return CheckPushRPC(*RPCService.Get(), EntityPayloadItem, RPCType, ExpectedResult);
	}

	bool PushRPCOverflowTest(const TArray<Worker_EntityId>& EntityIdArray,
		ERPCEndpointType RPCEndpointType,
		ERPCType RPCType,
		const EntityPayload& EntityPayloadItem,
		SpatialGDK::EPushRPCResult ExpectedResult)
	{
		if (EntityIdArray.Num() == 0)
		{
			return false;
		}

		TUniquePtr<SpatialGDK::SpatialRPCService> RPCService = CreateRPCService(EntityIdArray, RPCEndpointType);
		int RPCsToSend = static_cast<int>(GetDefault<USpatialGDKSettings>()->GetRPCRingBufferSize(RPCType));
		for (int i = 0; i < RPCsToSend; ++i)
		{
			if (!CheckPushRPC(*RPCService.Get(), EntityPayloadItem, RPCType, SpatialGDK::EPushRPCResult::Success))
			{
				return false;
			}
		}

		return CheckPushRPC(*RPCService.Get(), EntityPayloadItem, RPCType, ExpectedResult);
	}

	bool EntityCreationTest()
	{
		ERPCType RPCType = ERPCType::ClientReliable;
		const EntityPayload& EntityPayloadItem = SimpleEntityPayload;
		TUniquePtr<SpatialGDK::SpatialRPCService> RPCService = CreateRPCService({}, INVALID);

		if (!CheckPushRPC(*RPCService.Get(), EntityPayloadItem, RPCType, SpatialGDK::EPushRPCResult::Success))
		{
			return false;
		}

		Worker_ComponentData ComponentData = GetComponentDataOnEntityCreationFromRPCService(*RPCService.Get(), EntityPayloadItem.EntityId, RPCType);
		return CompareComponentDataAndEntityPayload(ComponentData, EntityPayloadItem);
	}

	bool EntityCreationMultiCastTest()
	{
		ERPCType RPCType = ERPCType::NetMulticast;
		const EntityPayload& EntityPayloadItem = SimpleEntityPayload;
		TUniquePtr<SpatialGDK::SpatialRPCService> RPCService = CreateRPCService({}, INVALID);

		if (!CheckPushRPC(*RPCService.Get(), EntityPayloadItem, RPCType, SpatialGDK::EPushRPCResult::Success))
		{
			return false;
		}

		Worker_ComponentData ComponentData = GetComponentDataOnEntityCreationFromRPCService(*RPCService.Get(), EntityPayloadItem.EntityId, RPCType);

		// Todo - check ComponentData to see if initially_present field is set.

		return true;
	}

	bool PayloadWriteTest(const TArray<Worker_EntityId>& EntityIdArray, ERPCEndpointType RPCEndpointType, ERPCType RPCType, const TArray<EntityPayload>& EntityPayloads)
	{
		if (EntityIdArray.Num() == 0)
		{
			return false;
		}

		TUniquePtr<SpatialGDK::SpatialRPCService> RPCService = CreateRPCService(EntityIdArray, RPCEndpointType);

		for (const EntityPayload& EntityPayloadItem : EntityPayloads)
		{
			if (!CheckPushRPC(*RPCService.Get(), EntityPayloadItem, RPCType, SpatialGDK::EPushRPCResult::Success))
			{
				return false;
			}
		}

		TArray<SpatialGDK::SpatialRPCService::UpdateToSend> UpdatedToSendArray = RPCService->GetRPCsAndAcksToSend();
		if (EntityPayloads.Num() != UpdatedToSendArray.Num())
		{
			return false;
		}

		Worker_ComponentId ExpectedUpdateComponentId = SpatialGDK::RPCRingBufferUtils::GetRingBufferComponentId(RPCType);

		for (int i = 0; i < EntityPayloads.Num(); ++i)
		{
			const EntityPayload& EntityPayloadItem = EntityPayloads[i];
			SpatialGDK::SpatialRPCService::UpdateToSend& Update = UpdatedToSendArray[i];

			if (Update.Update.component_id != ExpectedUpdateComponentId ||
				!CompareUpdateToSendAndEntityPayload(Update, EntityPayloadItem))
			{
				return false;
			}
		}

		return true;
	}

	bool ReceiveRPCTest(const TArray<Worker_EntityId>& EntityIdArray, ERPCEndpointType RPCEndpointType, ERPCType RPCType, const EntityPayload& EntityPayloadItem)
	{
		USpatialStaticComponentView* StaticComponentView = NewObject<USpatialStaticComponentView>();
		for (Worker_EntityId EntityId : EntityIdArray)
		{
			Worker_Authority ClientAuth = GetClientAuthorityFromRPCEndpointType(RPCEndpointType);
			Worker_Authority ServerAuth = GetServerAuthorityFromRPCEndpointType(RPCEndpointType);

			Schema_ComponentData* ClientComponentData = Schema_CreateComponentData();
			Schema_ComponentData* ServerComponentData = Schema_CreateComponentData();
			Schema_Object* ClientSchemaObject = Schema_GetComponentDataFields(ClientComponentData);
			Schema_Object* ServerSchemaObject = Schema_GetComponentDataFields(ServerComponentData);

			if (EntityPayloadItem.EntityId == EntityId)
			{
				SpatialGDK::RPCRingBufferUtils::WriteRPCToSchema(ClientSchemaObject, RPCType, 1, EntityPayloadItem.Payload);
				SpatialGDK::RPCRingBufferUtils::WriteRPCToSchema(ServerSchemaObject, RPCType, 1, EntityPayloadItem.Payload);
			}

			AddEntityComponentToStaticComponentView(*StaticComponentView,
				SpatialGDK::EntityComponentId(EntityId, SpatialConstants::CLIENT_ENDPOINT_COMPONENT_ID),
				ClientComponentData,
				ClientAuth);

			AddEntityComponentToStaticComponentView(*StaticComponentView,
				SpatialGDK::EntityComponentId(EntityId, SpatialConstants::SERVER_ENDPOINT_COMPONENT_ID),
				ServerComponentData,
				ServerAuth);
		}

		bool bExtractCallbackSuccesful = true;
		ExtractRPCDelegate RPCDelegate = ExtractRPCDelegate::CreateLambda([&bExtractCallbackSuccesful, EntityPayloadItem](Worker_EntityId EntityId, ERPCType RPCType, const SpatialGDK::RPCPayload& Payload) {
			bExtractCallbackSuccesful &= CompareRPCPayload(Payload, EntityPayloadItem.Payload);
			bExtractCallbackSuccesful &= EntityId == EntityPayloadItem.EntityId;
			return true;
		});

		TUniquePtr<SpatialGDK::SpatialRPCService> RPCService = CreateRPCService(EntityIdArray, RPCEndpointType, RPCDelegate, StaticComponentView);

		Worker_ComponentId ComponentId = SpatialConstants::SERVER_ENDPOINT_COMPONENT_ID;
		switch (RPCType)
		{
		case ERPCType::ClientReliable:
		case ERPCType::ClientUnreliable:
			ComponentId = SpatialConstants::CLIENT_ENDPOINT_COMPONENT_ID;
			break;
		case ERPCType::NetMulticast:
			ComponentId = SpatialConstants::MULTICAST_RPCS_COMPONENT_ID;
			break;
		default:
			break;
		}

		RPCService->ExtractRPCsForEntity(EntityPayloadItem.EntityId, ComponentId);
		return bExtractCallbackSuccesful;
	}
}

RPC_SERVICE_TEST(GIVEN_authority_over_server_endpoint_WHEN_push_client_reliable_rpcs_on_the_service_THEN_check_success)
{
	bool bSuccess = RPCServiceTests::PushRPCTest({ RPCTestEntityId_1 }, SERVER_AUTH, ERPCType::ClientReliable, SimpleEntityPayload, SpatialGDK::EPushRPCResult::Success);
	TestTrue("Push RPC returned expected results", bSuccess);
	return bSuccess;
}

RPC_SERVICE_TEST(GIVEN_authority_over_server_endpoint_WHEN_push_client_unreliable_rpcs_on_the_service_THEN_check_success)
{
	bool bSuccess = RPCServiceTests::PushRPCTest({ RPCTestEntityId_1 }, SERVER_AUTH, ERPCType::ClientUnreliable, SimpleEntityPayload, SpatialGDK::EPushRPCResult::Success);
	TestTrue("Push RPC returned expected results", bSuccess);
	return bSuccess;
}

RPC_SERVICE_TEST(GIVEN_authority_over_server_endpoint_WHEN_push_server_reliable_rpcs_on_the_service_THEN_check_no_buffer_authority)
{
	bool bSuccess = RPCServiceTests::PushRPCTest({ RPCTestEntityId_1 }, SERVER_AUTH, ERPCType::ServerReliable, SimpleEntityPayload, SpatialGDK::EPushRPCResult::NoRingBufferAuthority);
	TestTrue("Push RPC returned expected results", bSuccess);
	return bSuccess;
}

RPC_SERVICE_TEST(GIVEN_authority_over_server_endpoint_WHEN_push_server_unreliable_rpcs_on_the_service_THEN_check_no_buffer_authority)
{
	bool bSuccess = RPCServiceTests::PushRPCTest({ RPCTestEntityId_1 }, SERVER_AUTH, ERPCType::ServerUnreliable, SimpleEntityPayload, SpatialGDK::EPushRPCResult::NoRingBufferAuthority);
	TestTrue("Push RPC returned expected results", bSuccess);
	return bSuccess;
}

RPC_SERVICE_TEST(GIVEN_authority_over_client_endpoint_WHEN_push_client_reliable_rpcs_on_the_service_THEN_check_no_buffer_authority)
{
	bool bSuccess = RPCServiceTests::PushRPCTest({ RPCTestEntityId_1 }, CLIENT_AUTH, ERPCType::ClientReliable, SimpleEntityPayload, SpatialGDK::EPushRPCResult::NoRingBufferAuthority);
	TestTrue("Push RPC returned expected results", bSuccess);
	return bSuccess;
}

RPC_SERVICE_TEST(GIVEN_authority_over_client_endpoint_WHEN_push_client_unreliable_rpcs_on_the_service_THEN_check_no_buffer_authority)
{
	bool bSuccess = RPCServiceTests::PushRPCTest({ RPCTestEntityId_1 }, CLIENT_AUTH, ERPCType::ClientUnreliable, SimpleEntityPayload, SpatialGDK::EPushRPCResult::NoRingBufferAuthority);
	TestTrue("Push RPC returned expected results", bSuccess);
	return bSuccess;
}

RPC_SERVICE_TEST(GIVEN_authority_over_client_endpoint_WHEN_push_server_reliable_rpcs_on_the_service_THEN_check_success)
{
	bool bSuccess = RPCServiceTests::PushRPCTest({ RPCTestEntityId_1 }, CLIENT_AUTH, ERPCType::ServerReliable, SimpleEntityPayload, SpatialGDK::EPushRPCResult::Success);
	TestTrue("Push RPC returned expected results", bSuccess);
	return bSuccess;
}

RPC_SERVICE_TEST(GIVEN_authority_over_client_endpoint_WHEN_push_server_unreliable_rpcs_on_the_service_THEN_check_success)
{
	bool bSuccess = RPCServiceTests::PushRPCTest({ RPCTestEntityId_1 }, CLIENT_AUTH, ERPCType::ServerUnreliable, SimpleEntityPayload, SpatialGDK::EPushRPCResult::Success);
	TestTrue("Push RPC returned expected results", bSuccess);
	return bSuccess;
}

RPC_SERVICE_TEST(GIVEN_authority_over_server_endpoint_and_client_endpoint_WHEN_push_rpcs_on_the_service_THEN_check_push_results)
{
	bool bSuccess = RPCServiceTests::PushRPCTest({ RPCTestEntityId_1 }, SERVER_AND_CLIENT_AUTH, ERPCType::ClientReliable, SimpleEntityPayload, SpatialGDK::EPushRPCResult::HasAckAuthority);
	TestTrue("Push RPC returned expected results", bSuccess);
	return bSuccess;
}

RPC_SERVICE_TEST(GIVEN_authority_over_server_endpoint_WHEN_push_overflow_rpcs_on_the_service_as_server_reliable_THEN_check_push_results)
{
	bool bSuccess = RPCServiceTests::PushRPCOverflowTest({ RPCTestEntityId_1 }, SERVER_AUTH, ERPCType::ClientReliable, SimpleEntityPayload, SpatialGDK::EPushRPCResult::QueueOverflowed);
	TestTrue("Overflow test returned expected results", bSuccess);
	return bSuccess;
}

RPC_SERVICE_TEST(GIVEN_authority_over_server_endpoint_WHEN_push_overflow_rpcs_on_the_service_as_server_unreliable_THEN_check_push_results)
{
	bool bSuccess = RPCServiceTests::PushRPCOverflowTest({ RPCTestEntityId_1 }, SERVER_AUTH, ERPCType::ClientUnreliable, SimpleEntityPayload, SpatialGDK::EPushRPCResult::DropOverflowed);
	TestTrue("Overflow test returned expected results", bSuccess);
	return bSuccess;
}

RPC_SERVICE_TEST(GIVEN_authority_over_client_endpoint_WHEN_push_overflow_rpcs_on_the_service_as_client_reliable_THEN_check_push_results)
{
	bool bSuccess = RPCServiceTests::PushRPCOverflowTest({ RPCTestEntityId_1 }, CLIENT_AUTH, ERPCType::ServerReliable, SimpleEntityPayload, SpatialGDK::EPushRPCResult::QueueOverflowed);
	TestTrue("Overflow test returned expected results", bSuccess);
	return bSuccess;
}

RPC_SERVICE_TEST(GIVEN_authority_over_client_endpoint_WHEN_push_overflow_rpcs_on_the_service_as_client_unreliable_THEN_check_push_results)
{
	bool bSuccess = RPCServiceTests::PushRPCOverflowTest({ RPCTestEntityId_1 }, CLIENT_AUTH, ERPCType::ServerUnreliable, SimpleEntityPayload, SpatialGDK::EPushRPCResult::DropOverflowed);
	TestTrue("Overflow test returned expected results", bSuccess);
	return bSuccess;
}

RPC_SERVICE_TEST(GIVEN_authority_over_client_endpoint_WHEN_push_server_unreliable_rpcs_on_the_service_THEN_check_payloads_are_writen_correctly_to_component_updates)
{
	TArray<EntityPayload> EntityPayloads;
	EntityPayloads.Add(EntityPayload(RPCTestEntityId_1, SimplePayload));
	EntityPayloads.Add(EntityPayload(RPCTestEntityId_2, SimplePayload));
	EntityPayloads.Add(EntityPayload(RPCTestEntityId_3, SimplePayload));
	EntityPayloads.Add(EntityPayload(RPCTestEntityId_4, SimplePayload));
	EntityPayloads.Add(EntityPayload(RPCTestEntityId_5, SimplePayload));

	TArray<Worker_EntityId> EntityRPCEndpointArray;
	EntityRPCEndpointArray.Add(RPCTestEntityId_1);
	EntityRPCEndpointArray.Add(RPCTestEntityId_2);
	EntityRPCEndpointArray.Add(RPCTestEntityId_3);
	EntityRPCEndpointArray.Add(RPCTestEntityId_4);
	EntityRPCEndpointArray.Add(RPCTestEntityId_5);

	bool bSuccess = RPCServiceTests::PayloadWriteTest(EntityRPCEndpointArray, CLIENT_AUTH, ERPCType::ServerUnreliable, EntityPayloads);
	TestTrue("Payload write test returned expected results", bSuccess);
	return bSuccess;
}

RPC_SERVICE_TEST(GIVEN_no_authority_over_rpc_endpoint_WHEN_push_client_reliable_rpcs_on_the_service_THEN_check_component_data)
{
	bool bSuccess = RPCServiceTests::EntityCreationTest();
	TestTrue("Entity creation test returned expected results", bSuccess);
	return bSuccess;
}

RPC_SERVICE_TEST(GIVEN_no_authority_over_rpc_endpoint_WHEN_push_multicast_rpcs_on_the_service_THEN_check_component_data)
{
	bool bSuccess = RPCServiceTests::EntityCreationMultiCastTest();
	TestTrue("Entity creation multi-cast test returned expected results", bSuccess);
	return bSuccess;
}

RPC_SERVICE_TEST(GIVEN_authority_over_server_endpoint_WHEN_push_server_reliable_rpcs_on_the_service_THEN_check_component_data)
{
	TArray<Worker_EntityId> EntityIdArray;
	EntityIdArray.Add(RPCTestEntityId_1);
	EntityIdArray.Add(RPCTestEntityId_2);

	bool bSuccess = RPCServiceTests::ReceiveRPCTest(EntityIdArray, SERVER_AUTH, ERPCType::ClientReliable, SimpleEntityPayload);
	TestTrue("Receive RPC test returned expected results", bSuccess);
	return bSuccess;
}
