// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "CoreMinimal.h"

// Engine
#include "Engine/Engine.h"
#include "GameFramework/GameStateBase.h"
#include "Misc/AutomationTest.h"
#include "Tests/AutomationCommon.h"
#include "Tests/TestActor.h"
#include "Tests/TestDefinitions.h"
#include "Tests/TestingComponentViewHelpers.h"

// GDK
#include "Interop/SpatialRPCService.h"
#include "Interop/SpatialReceiver.h"
#include "Interop/SpatialStaticComponentView.h"
#include "Schema/RPCPayload.h"
#include "SpatialConstants.h"
#include "SpatialGDKSettings.h"
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

const SpatialGDK::RPCPayload SimplePayload = SpatialGDK::RPCPayload(1, 0, TArray<uint8>({ 1 }, 1));

ExtractRPCDelegate DefaultRPCDelegate =
	ExtractRPCDelegate::CreateLambda([](Worker_EntityId EntityId, ERPCType RPCType, const SpatialGDK::RPCPayload& Payload) {
		return true;
	});

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

Worker_Authority GetMulticastAuthorityFromRPCEndpointType(ERPCEndpointType RPCEndpointType)
{
	return GetServerAuthorityFromRPCEndpointType(RPCEndpointType);
}

void AddEntityToStaticComponentView(USpatialStaticComponentView& StaticComponentView, Worker_EntityId EntityId,
									ERPCEndpointType RPCEndpointType)
{
	TestingComponentViewHelpers::AddEntityComponentToStaticComponentView(StaticComponentView, EntityId,
																		 SpatialConstants::CLIENT_ENDPOINT_COMPONENT_ID,
																		 GetClientAuthorityFromRPCEndpointType(RPCEndpointType));

	TestingComponentViewHelpers::AddEntityComponentToStaticComponentView(StaticComponentView, EntityId,
																		 SpatialConstants::SERVER_ENDPOINT_COMPONENT_ID,
																		 GetServerAuthorityFromRPCEndpointType(RPCEndpointType));

	TestingComponentViewHelpers::AddEntityComponentToStaticComponentView(StaticComponentView, EntityId,
																		 SpatialConstants::MULTICAST_RPCS_COMPONENT_ID,
																		 GetMulticastAuthorityFromRPCEndpointType(RPCEndpointType));
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

SpatialGDK::SpatialRPCService CreateRPCService(const TArray<Worker_EntityId>& EntityIdArray, ERPCEndpointType RPCEndpointType,
											   ExtractRPCDelegate RPCDelegate = DefaultRPCDelegate,
											   USpatialStaticComponentView* StaticComponentView = nullptr)
{
	if (StaticComponentView == nullptr)
	{
		StaticComponentView = CreateStaticComponentView(EntityIdArray, RPCEndpointType);
	}

	SpatialGDK::SpatialRPCService RPCService = SpatialGDK::SpatialRPCService(RPCDelegate, StaticComponentView, nullptr);

	for (Worker_EntityId EntityId : EntityIdArray)
	{
		if (GetClientAuthorityFromRPCEndpointType(RPCEndpointType) == WORKER_AUTHORITY_AUTHORITATIVE)
		{
			RPCService.OnEndpointAuthorityGained(EntityId, SpatialConstants::CLIENT_ENDPOINT_COMPONENT_ID);
		}

		if (GetServerAuthorityFromRPCEndpointType(RPCEndpointType) == WORKER_AUTHORITY_AUTHORITATIVE)
		{
			RPCService.OnEndpointAuthorityGained(EntityId, SpatialConstants::SERVER_ENDPOINT_COMPONENT_ID);
		}
	}

	return RPCService;
}

bool CompareRPCPayload(const SpatialGDK::RPCPayload& Payload1, const SpatialGDK::RPCPayload& Payload2)
{
	return Payload1.Index == Payload2.Index && Payload1.Offset == Payload2.Offset && Payload1.PayloadData == Payload2.PayloadData;
}

bool CompareSchemaObjectToSendAndPayload(Schema_Object* SchemaObject, const SpatialGDK::RPCPayload& Payload, ERPCType RPCType, uint64 RPCId)
{
	SpatialGDK::RPCRingBufferDescriptor Descriptor = SpatialGDK::RPCRingBufferUtils::GetRingBufferDescriptor(RPCType);
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
										  ERPCType RPCType, uint64 RPCId)
{
	return CompareSchemaObjectToSendAndPayload(Schema_GetComponentDataFields(ComponentData.schema_type), EntityPayloadItem.Payload, RPCType,
											   RPCId);
}

FWorkerComponentData GetComponentDataOnEntityCreationFromRPCService(SpatialGDK::SpatialRPCService& RPCService, Worker_EntityId EntityID,
																	ERPCType RPCType)
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
	SpatialGDK::SpatialRPCService RPCService = CreateRPCService({ RPCTestEntityId_1 }, SERVER_AUTH);
	SpatialGDK::EPushRPCResult Result = RPCService.PushRPC(RPCTestEntityId_1, ERPCType::ClientReliable, SimplePayload, false);
	TestTrue("Push RPC returned expected results", (Result == SpatialGDK::EPushRPCResult::Success));
	return true;
}

RPC_SERVICE_TEST(GIVEN_authority_over_server_endpoint_WHEN_push_client_unreliable_rpcs_to_the_service_THEN_rpc_push_result_success)
{
	SpatialGDK::SpatialRPCService RPCService = CreateRPCService({ RPCTestEntityId_1 }, SERVER_AUTH);
	SpatialGDK::EPushRPCResult Result = RPCService.PushRPC(RPCTestEntityId_1, ERPCType::ClientUnreliable, SimplePayload, false);
	TestTrue("Push RPC returned expected results", (Result == SpatialGDK::EPushRPCResult::Success));
	return true;
}

RPC_SERVICE_TEST(
	GIVEN_authority_over_server_endpoint_WHEN_push_server_reliable_rpcs_to_the_service_THEN_rpc_push_result_no_buffer_authority)
{
	SpatialGDK::SpatialRPCService RPCService = CreateRPCService({ RPCTestEntityId_1 }, SERVER_AUTH);
	SpatialGDK::EPushRPCResult Result = RPCService.PushRPC(RPCTestEntityId_1, ERPCType::ServerReliable, SimplePayload, false);
	TestTrue("Push RPC returned expected results", (Result == SpatialGDK::EPushRPCResult::NoRingBufferAuthority));
	return true;
}

RPC_SERVICE_TEST(
	GIVEN_authority_over_server_endpoint_WHEN_push_server_unreliable_rpcs_to_the_service_THEN_rpc_push_result_no_buffer_authority)
{
	SpatialGDK::SpatialRPCService RPCService = CreateRPCService({ RPCTestEntityId_1 }, SERVER_AUTH);
	SpatialGDK::EPushRPCResult Result = RPCService.PushRPC(RPCTestEntityId_1, ERPCType::ServerUnreliable, SimplePayload, false);
	TestTrue("Push RPC returned expected results", (Result == SpatialGDK::EPushRPCResult::NoRingBufferAuthority));
	return true;
}

RPC_SERVICE_TEST(
	GIVEN_authority_over_client_endpoint_WHEN_push_client_reliable_rpcs_to_the_service_THEN_rpc_push_result_no_buffer_authority)
{
	SpatialGDK::SpatialRPCService RPCService = CreateRPCService({ RPCTestEntityId_1 }, CLIENT_AUTH);
	SpatialGDK::EPushRPCResult Result = RPCService.PushRPC(RPCTestEntityId_1, ERPCType::ClientReliable, SimplePayload, false);
	TestTrue("Push RPC returned expected results", (Result == SpatialGDK::EPushRPCResult::NoRingBufferAuthority));
	return true;
}

RPC_SERVICE_TEST(
	GIVEN_authority_over_client_endpoint_WHEN_push_client_unreliable_rpcs_to_the_service_THEN_rpc_push_result_no_buffer_authority)
{
	SpatialGDK::SpatialRPCService RPCService = CreateRPCService({ RPCTestEntityId_1 }, CLIENT_AUTH);
	SpatialGDK::EPushRPCResult Result = RPCService.PushRPC(RPCTestEntityId_1, ERPCType::ClientUnreliable, SimplePayload, false);
	TestTrue("Push RPC returned expected results", (Result == SpatialGDK::EPushRPCResult::NoRingBufferAuthority));
	return true;
}

RPC_SERVICE_TEST(GIVEN_authority_over_client_endpoint_WHEN_push_server_reliable_rpcs_to_the_service_THEN_rpc_push_result_success)
{
	SpatialGDK::SpatialRPCService RPCService = CreateRPCService({ RPCTestEntityId_1 }, CLIENT_AUTH);
	SpatialGDK::EPushRPCResult Result = RPCService.PushRPC(RPCTestEntityId_1, ERPCType::ServerReliable, SimplePayload, false);
	TestTrue("Push RPC returned expected results", (Result == SpatialGDK::EPushRPCResult::Success));
	return true;
}

RPC_SERVICE_TEST(GIVEN_authority_over_client_endpoint_WHEN_push_server_unreliable_rpcs_to_the_service_THEN_rpc_push_result_success)
{
	SpatialGDK::SpatialRPCService RPCService = CreateRPCService({ RPCTestEntityId_1 }, CLIENT_AUTH);
	SpatialGDK::EPushRPCResult Result = RPCService.PushRPC(RPCTestEntityId_1, ERPCType::ServerUnreliable, SimplePayload, false);
	TestTrue("Push RPC returned expected results", (Result == SpatialGDK::EPushRPCResult::Success));
	return true;
}

RPC_SERVICE_TEST(GIVEN_authority_over_client_endpoint_WHEN_push_multicast_rpcs_to_the_service_THEN_rpc_push_result_no_buffer_authority)
{
	SpatialGDK::SpatialRPCService RPCService = CreateRPCService({ RPCTestEntityId_1 }, CLIENT_AUTH);
	SpatialGDK::EPushRPCResult Result = RPCService.PushRPC(RPCTestEntityId_1, ERPCType::NetMulticast, SimplePayload, false);
	TestTrue("Push RPC returned expected results", (Result == SpatialGDK::EPushRPCResult::NoRingBufferAuthority));
	return true;
}

RPC_SERVICE_TEST(GIVEN_authority_over_server_endpoint_WHEN_push_multicast_rpcs_to_the_service_THEN_rpc_push_result_success)
{
	SpatialGDK::SpatialRPCService RPCService = CreateRPCService({ RPCTestEntityId_1 }, SERVER_AUTH);
	SpatialGDK::EPushRPCResult Result = RPCService.PushRPC(RPCTestEntityId_1, ERPCType::NetMulticast, SimplePayload, false);
	TestTrue("Push RPC returned expected results", (Result == SpatialGDK::EPushRPCResult::Success));
	return true;
}

RPC_SERVICE_TEST(GIVEN_authority_over_server_and_client_endpoint_WHEN_push_rpcs_to_the_service_THEN_rpc_push_result_has_ack_authority)
{
	SpatialGDK::SpatialRPCService RPCService = CreateRPCService({ RPCTestEntityId_1 }, SERVER_AND_CLIENT_AUTH);
	SpatialGDK::EPushRPCResult Result = RPCService.PushRPC(RPCTestEntityId_1, ERPCType::ClientReliable, SimplePayload, false);
	TestTrue("Push RPC returned expected results", (Result == SpatialGDK::EPushRPCResult::HasAckAuthority));
	return true;
}

RPC_SERVICE_TEST(
	GIVEN_authority_over_server_endpoint_WHEN_push_overflow_client_reliable_rpcs_to_the_service_THEN_rpc_push_result_queue_overflowed)
{
	SpatialGDK::SpatialRPCService RPCService = CreateRPCService({ RPCTestEntityId_1 }, SERVER_AUTH);

	// Send RPCs to the point where we will overflow
	uint32 RPCsToSend = GetDefault<USpatialGDKSettings>()->GetRPCRingBufferSize(ERPCType::ClientReliable);
	for (uint32 i = 0; i < RPCsToSend; ++i)
	{
		RPCService.PushRPC(RPCTestEntityId_1, ERPCType::ClientReliable, SimplePayload, false);
	}

	SpatialGDK::EPushRPCResult Result = RPCService.PushRPC(RPCTestEntityId_1, ERPCType::ClientReliable, SimplePayload, false);
	TestTrue("Push RPC returned expected results", (Result == SpatialGDK::EPushRPCResult::QueueOverflowed));
	return true;
}

RPC_SERVICE_TEST(
	GIVEN_authority_over_server_endpoint_WHEN_push_overflow_client_unreliable_rpcs_to_the_service_THEN_rpc_push_result_drop_overflow)
{
	SpatialGDK::SpatialRPCService RPCService = CreateRPCService({ RPCTestEntityId_1 }, SERVER_AUTH);

	// Send RPCs to the point where we will overflow
	uint32 RPCsToSend = GetDefault<USpatialGDKSettings>()->GetRPCRingBufferSize(ERPCType::ClientUnreliable);
	for (uint32 i = 0; i < RPCsToSend; ++i)
	{
		RPCService.PushRPC(RPCTestEntityId_1, ERPCType::ClientUnreliable, SimplePayload, false);
	}

	SpatialGDK::EPushRPCResult Result = RPCService.PushRPC(RPCTestEntityId_1, ERPCType::ClientUnreliable, SimplePayload, false);
	TestTrue("Push RPC returned expected results", (Result == SpatialGDK::EPushRPCResult::DropOverflowed));
	return true;
}

RPC_SERVICE_TEST(
	GIVEN_authority_over_client_endpoint_WHEN_push_overflow_client_reliable_rpcs_to_the_service_THEN_rpc_push_result_queue_overflowed)
{
	SpatialGDK::SpatialRPCService RPCService = CreateRPCService({ RPCTestEntityId_1 }, CLIENT_AUTH);

	// Send RPCs to the point where we will overflow
	uint32 RPCsToSend = GetDefault<USpatialGDKSettings>()->GetRPCRingBufferSize(ERPCType::ServerReliable);
	for (uint32 i = 0; i < RPCsToSend; ++i)
	{
		RPCService.PushRPC(RPCTestEntityId_1, ERPCType::ServerReliable, SimplePayload, false);
	}

	SpatialGDK::EPushRPCResult Result = RPCService.PushRPC(RPCTestEntityId_1, ERPCType::ServerReliable, SimplePayload, false);
	TestTrue("Push RPC returned expected results", (Result == SpatialGDK::EPushRPCResult::QueueOverflowed));
	return true;
}

RPC_SERVICE_TEST(
	GIVEN_authority_over_client_endpoint_WHEN_push_overflow_client_unreliable_rpcs_to_the_service_THEN_rpc_push_result_drop_overflow)
{
	SpatialGDK::SpatialRPCService RPCService = CreateRPCService({ RPCTestEntityId_1 }, CLIENT_AUTH);

	// Send RPCs to the point where we will overflow
	uint32 RPCsToSend = GetDefault<USpatialGDKSettings>()->GetRPCRingBufferSize(ERPCType::ServerUnreliable);
	for (uint32 i = 0; i < RPCsToSend; ++i)
	{
		RPCService.PushRPC(RPCTestEntityId_1, ERPCType::ServerUnreliable, SimplePayload, false);
	}

	SpatialGDK::EPushRPCResult Result = RPCService.PushRPC(RPCTestEntityId_1, ERPCType::ServerUnreliable, SimplePayload, false);
	TestTrue("Push RPC returned expected results", (Result == SpatialGDK::EPushRPCResult::DropOverflowed));
	return true;
}

RPC_SERVICE_TEST(GIVEN_authority_over_server_endpoint_WHEN_push_overflow_multicast_rpcs_to_the_service_THEN_rpc_push_result_success)
{
	SpatialGDK::SpatialRPCService RPCService = CreateRPCService({ RPCTestEntityId_1 }, SERVER_AUTH);

	// Send RPCs to the point where we will overflow
	uint32 RPCsToSend = GetDefault<USpatialGDKSettings>()->GetRPCRingBufferSize(ERPCType::NetMulticast);
	for (uint32 i = 0; i < RPCsToSend; ++i)
	{
		RPCService.PushRPC(RPCTestEntityId_1, ERPCType::NetMulticast, SimplePayload, false);
	}

	SpatialGDK::EPushRPCResult Result = RPCService.PushRPC(RPCTestEntityId_1, ERPCType::NetMulticast, SimplePayload, false);
	TestTrue("Push RPC returned expected results", (Result == SpatialGDK::EPushRPCResult::Success));
	return true;
}

RPC_SERVICE_TEST(
	GIVEN_authority_over_client_endpoint_WHEN_push_server_unreliable_rpcs_to_the_service_THEN_payloads_are_writen_correctly_to_component_updates)
{
	TArray<EntityPayload> EntityPayloads;
	EntityPayloads.Add(EntityPayload(RPCTestEntityId_1, SimplePayload));
	EntityPayloads.Add(EntityPayload(RPCTestEntityId_2, SimplePayload));

	TArray<Worker_EntityId> EntityIdArray;
	EntityIdArray.Add(RPCTestEntityId_1);
	EntityIdArray.Add(RPCTestEntityId_2);

	SpatialGDK::SpatialRPCService RPCService = CreateRPCService(EntityIdArray, CLIENT_AUTH);
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
	// Create RPCService with empty component view
	SpatialGDK::SpatialRPCService RPCService = CreateRPCService({}, NO_AUTH);

	RPCService.PushRPC(RPCTestEntityId_1, ERPCType::ClientReliable, SimplePayload, false);

	FWorkerComponentData ComponentData =
		GetComponentDataOnEntityCreationFromRPCService(RPCService, RPCTestEntityId_1, ERPCType::ClientReliable);
	bool bTestPassed =
		CompareComponentDataAndEntityPayload(ComponentData, EntityPayload(RPCTestEntityId_1, SimplePayload), ERPCType::ClientReliable, 1);
	TestTrue("Entity creation test returned expected results", bTestPassed);
	return true;
}

RPC_SERVICE_TEST(GIVEN_no_authority_over_rpc_endpoint_WHEN_push_multicast_rpcs_to_the_service_THEN_initially_present_set)
{
	// Create RPCService with empty component view
	SpatialGDK::SpatialRPCService RPCService = CreateRPCService({}, NO_AUTH);

	RPCService.PushRPC(RPCTestEntityId_1, ERPCType::NetMulticast, SimplePayload, false);
	RPCService.PushRPC(RPCTestEntityId_1, ERPCType::NetMulticast, SimplePayload, false);

	FWorkerComponentData ComponentData =
		GetComponentDataOnEntityCreationFromRPCService(RPCService, RPCTestEntityId_1, ERPCType::NetMulticast);
	const Schema_Object* SchemaObject = Schema_GetComponentDataFields(ComponentData.schema_type);
	uint32 InitiallyPresent =
		Schema_GetUint32(SchemaObject, SpatialGDK::RPCRingBufferUtils::GetInitiallyPresentMulticastRPCsCountFieldId());
	TestTrue("Entity creation multicast test returned expected results", (InitiallyPresent == 2));
	return true;
}

RPC_SERVICE_TEST(
	GIVEN_client_endpoint_with_rpcs_in_view_and_authority_over_server_endpoint_WHEN_extract_rpcs_from_the_service_THEN_extracted_payloads_match_pushed_payloads)
{
	USpatialStaticComponentView* StaticComponentView = NewObject<USpatialStaticComponentView>();

	Schema_ComponentData* ClientComponentData = Schema_CreateComponentData();
	Schema_Object* ClientSchemaObject = Schema_GetComponentDataFields(ClientComponentData);
	SpatialGDK::RPCRingBufferUtils::WriteRPCToSchema(ClientSchemaObject, ERPCType::ClientReliable, 1, SimplePayload);
	SpatialGDK::RPCRingBufferUtils::WriteRPCToSchema(ClientSchemaObject, ERPCType::ClientReliable, 2, SimplePayload);

	TestingComponentViewHelpers::AddEntityComponentToStaticComponentView(
		*StaticComponentView, RPCTestEntityId_1, SpatialConstants::CLIENT_ENDPOINT_COMPONENT_ID, ClientComponentData,
		GetClientAuthorityFromRPCEndpointType(SERVER_AUTH));

	TestingComponentViewHelpers::AddEntityComponentToStaticComponentView(*StaticComponentView, RPCTestEntityId_1,
																		 SpatialConstants::SERVER_ENDPOINT_COMPONENT_ID,
																		 GetServerAuthorityFromRPCEndpointType(SERVER_AUTH));

	int RPCsExtracted = 0;
	bool bPayloadsMatch = true;
	ExtractRPCDelegate RPCDelegate = ExtractRPCDelegate::CreateLambda(
		[&RPCsExtracted, &bPayloadsMatch](Worker_EntityId EntityId, ERPCType RPCType, const SpatialGDK::RPCPayload& Payload) {
			RPCsExtracted++;
			bPayloadsMatch &= CompareRPCPayload(Payload, SimplePayload);
			bPayloadsMatch &= EntityId == RPCTestEntityId_1;
			return true;
		});

	SpatialGDK::SpatialRPCService RPCService = CreateRPCService({ RPCTestEntityId_1 }, SERVER_AUTH, RPCDelegate, StaticComponentView);
	RPCService.ExtractRPCsForEntity(RPCTestEntityId_1, SpatialConstants::CLIENT_ENDPOINT_COMPONENT_ID);

	TestTrue("Extracted RPCs match expected payloads", (RPCsExtracted == 2 && bPayloadsMatch));
	return true;
}

DEFINE_LATENT_AUTOMATION_COMMAND_ONE_PARAMETER(FWaitForWorld, TSharedPtr<TestData>, Data);
bool FWaitForWorld::Update()
{
	UWorld* World = nullptr;
	const TIndirectArray<FWorldContext>& WorldContexts = GEngine->GetWorldContexts();
	for (const FWorldContext& Context : WorldContexts)
	{
		if ((Context.WorldType == EWorldType::PIE || Context.WorldType == EWorldType::Game) && (Context.World() != nullptr))
		{
			World = Context.World();
			break;
		}
	}

	if (World != nullptr && World->AreActorsInitialized())
	{
		AGameStateBase* GameState = World->GetGameState();
		if (GameState != nullptr && GameState->HasMatchStarted())
		{
			Data->TestWorld = World;
			return true;
		}
	}

	return false;
}

DEFINE_LATENT_AUTOMATION_COMMAND_ONE_PARAMETER(FSpawnActor, TSharedPtr<TestData>, Data);
bool FSpawnActor::Update()
{
	FActorSpawnParameters SpawnParams;
	SpawnParams.bNoFail = true;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	AActor* Actor = Data->TestWorld->SpawnActor<ATestActor>(SpawnParams);
	Data->Actor = Actor;

	return true;
}

DEFINE_LATENT_AUTOMATION_COMMAND_ONE_PARAMETER(FWaitForActor, TSharedPtr<TestData>, Data);
bool FWaitForActor::Update()
{
	AActor* Actor = Data->Actor;
	return (IsValid(Actor) && Actor->IsActorInitialized() && Actor->HasActorBegunPlay());
}

DEFINE_LATENT_AUTOMATION_COMMAND_TWO_PARAMETER(FDropRPCQueueTest, FAutomationTestBase*, Test, TSharedPtr<TestData>, Data);
bool FDropRPCQueueTest::Update()
{
	if (!ensure(Test != nullptr))
	{
		return true;
	}

	if (!ensure(Data != nullptr) || !ensure(Data->Actor != nullptr))
	{
		Test->TestTrue(
			"Correct RPC queue command was returned by the receiver after attempting to process an RPC without authority over the actor",
			false);
	}

	AActor* Actor = Data->Actor;
	USpatialNetDriver* SpatialNetDriver = Cast<USpatialNetDriver>(Data->TestWorld->NetDriver);
	Worker_EntityId EntityId = SpatialNetDriver->PackageMap->GetEntityIdFromObject(Actor);

	Schema_ComponentData* ClientComponentData = Schema_CreateComponentData();
	Schema_Object* ClientSchemaObject = Schema_GetComponentDataFields(ClientComponentData);

	// Write RPC to ring buffer.
	const FRPCInfo& RPCInfo =
		SpatialNetDriver->ClassInfoManager->GetRPCInfo(Actor, ATestActor::StaticClass()->FindFunctionByName("TestServerRPC"));
	const SpatialGDK::RPCPayload RPCPayload = SpatialGDK::RPCPayload(0, RPCInfo.Index, TArray<uint8>({}, 0));
	SpatialGDK::RPCRingBufferUtils::WriteRPCToSchema(ClientSchemaObject, ERPCType::ClientReliable, 1, RPCPayload);

	const ERPCEndpointType EndpointType = SERVER_AUTH;
	TestingComponentViewHelpers::AddEntityComponentToStaticComponentView(
		*SpatialNetDriver->StaticComponentView, EntityId, SpatialConstants::CLIENT_ENDPOINT_COMPONENT_ID, ClientComponentData,
		GetClientAuthorityFromRPCEndpointType(EndpointType));
	TestingComponentViewHelpers::AddEntityComponentToStaticComponentView(*SpatialNetDriver->StaticComponentView, EntityId,
																		 SpatialConstants::SERVER_ENDPOINT_COMPONENT_ID,
																		 GetServerAuthorityFromRPCEndpointType(EndpointType));

	USpatialReceiver* Receiver = SpatialNetDriver->Receiver;
	SpatialGDK::SpatialRPCService* RPCService = SpatialNetDriver->GetRPCService();
	RPCService->OnEndpointAuthorityGained(EntityId, SpatialConstants::SERVER_ENDPOINT_COMPONENT_ID);

	bool bTestSuccess = false;
	FProcessRPCDelegate RPCDelegate = FProcessRPCDelegate::CreateLambda([&Receiver, &bTestSuccess](const FPendingRPCParams& Params) {
		FRPCErrorInfo RPCErrorInfo = Receiver->ApplyRPC(Params);
		bTestSuccess = RPCErrorInfo.ErrorCode == ERPCResult::NoAuthority;
		bTestSuccess &= RPCErrorInfo.QueueProcessResult == ERPCQueueProcessResult::DropEntireQueue;

		return RPCErrorInfo;
	});

	// Bind new process function.
	FRPCContainer& RPCContainer = Receiver->GetRPCContainer();
	RPCContainer.BindProcessingFunction(RPCDelegate);

	// Change actor authority and process.
	Actor->Role = ROLE_SimulatedProxy;
	RPCService->ExtractRPCsForEntity(EntityId, SpatialConstants::CLIENT_ENDPOINT_COMPONENT_ID);

	RPCContainer.BindProcessingFunction(FProcessRPCDelegate::CreateUObject(Receiver, &USpatialReceiver::ApplyRPC));

	Test->TestTrue(
		"Correct RPC queue command was returned by the receiver after attempting to process an RPC without authority over the actor",
		bTestSuccess);

	return true;
}

#if (WITH_DEV_AUTOMATION_TESTS || WITH_PERF_AUTOMATION_TESTS) // Automation* functions only available with these flags

RPC_SERVICE_TEST(
	GIVEN_receiving_an_rpc_whose_target_we_do_not_have_authority_over_WHEN_we_process_the_rpc_THEN_return_DropEntireQueue_queue_command)
{
	AutomationOpenMap("/Engine/Maps/Entry");

	TSharedPtr<TestData> Data = MakeShared<TestData>();

	ADD_LATENT_AUTOMATION_COMMAND(FWaitForWorld(Data));
	ADD_LATENT_AUTOMATION_COMMAND(FSpawnActor(Data));
	ADD_LATENT_AUTOMATION_COMMAND(FWaitForActor(Data));
	ADD_LATENT_AUTOMATION_COMMAND(FDropRPCQueueTest(this, Data));

	return true;
}

#endif // (WITH_DEV_AUTOMATION_TESTS || WITH_PERF_AUTOMATION_TESTS)
