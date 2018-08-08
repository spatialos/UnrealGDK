// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "DTBManager.h"

#include "SpatialActorChannel.h"
#include "SpatialInterop.h"

#include <map>

const Worker_ComponentId SPECIAL_SPAWNER_COMPONENT_ID = 100003;
const Worker_EntityId SPECIAL_SPAWNER_ENTITY_ID = 3;

UDTBManager::UDTBManager()
{
}

void Schema_AddString(Schema_Object* Object, Schema_FieldId Id, const std::string& Value)
{
	uint32_t StringLength = Value.size();
	uint8_t* StringBuffer = Schema_AllocateBuffer(Object, sizeof(char) * StringLength);
	memcpy(StringBuffer, Value.c_str(), sizeof(char) * StringLength);
	Schema_AddBytes(Object, Id, StringBuffer, sizeof(char) * StringLength);
}

using WorkerAttributeSet = std::vector<std::string>;
using WorkerRequirementSet = std::vector<WorkerAttributeSet>;

void Schema_AddWorkerRequirementSet(Schema_Object* Object, Schema_FieldId Id, const WorkerRequirementSet& Value)
{
	auto RequirementSetObject = Schema_AddObject(Object, Id);
	for (auto& AttributeSet : Value)
	{
		auto AttributeSetObject = Schema_AddObject(RequirementSetObject, 1);

		for (auto& Attribute : AttributeSet)
		{
			Schema_AddString(AttributeSetObject, 1, Attribute);
		}
	}
}

const Worker_ComponentId ENTITY_ACL_COMPONENT_ID = 50;
void CreateEntityAclData(Worker_ComponentData& Data, const WorkerRequirementSet& ReadAcl, const std::map<Worker_ComponentId, WorkerRequirementSet>& ComponentWriteAcl)
{
	Data.component_id = ENTITY_ACL_COMPONENT_ID;
	Data.schema_type = Schema_CreateComponentData(ENTITY_ACL_COMPONENT_ID);
	auto ComponentObject = Schema_GetComponentDataFields(Data.schema_type);

	Schema_AddWorkerRequirementSet(ComponentObject, 1, ReadAcl);

	for (auto& KVPair : ComponentWriteAcl)
	{
		auto KVPairObject = Schema_AddObject(ComponentObject, 2);
		Schema_AddUint32(KVPairObject, SCHEMA_MAP_KEY_FIELD_ID, KVPair.first);
		Schema_AddWorkerRequirementSet(KVPairObject, SCHEMA_MAP_VALUE_FIELD_ID, KVPair.second);
	}
}

const Worker_ComponentId METADATA_COMPONENT_ID = 53;
void CreateMetadataData(Worker_ComponentData& Data, std::string EntityType)
{
	Data.component_id = METADATA_COMPONENT_ID;
	Data.schema_type = Schema_CreateComponentData(METADATA_COMPONENT_ID);
	auto ComponentObject = Schema_GetComponentDataFields(Data.schema_type);

	Schema_AddString(ComponentObject, 1, EntityType);
}

const Worker_ComponentId POSITION_COMPONENT_ID = 54;
void CreatePositionData(Worker_ComponentData& Data, const improbable::Coordinates& Coords)
{
	Data.component_id = POSITION_COMPONENT_ID;
	Data.schema_type = Schema_CreateComponentData(POSITION_COMPONENT_ID);
	auto ComponentObject = Schema_GetComponentDataFields(Data.schema_type);

	auto CoordsObject = Schema_AddObject(ComponentObject, 1);

	Schema_AddDouble(CoordsObject, 1, Coords.x());
	Schema_AddDouble(CoordsObject, 2, Coords.y());
	Schema_AddDouble(CoordsObject, 3, Coords.z());
}

const Worker_ComponentId PERSISTENCE_COMPONENT_ID = 55;
void CreatePersistenceData(Worker_ComponentData& Data)
{
	Data.component_id = PERSISTENCE_COMPONENT_ID;
	Data.schema_type = Schema_CreateComponentData(PERSISTENCE_COMPONENT_ID);
}

void UDTBManager::InitClient()
{
	if (Connection) return;

	//auto Vtables = CreateVtables();

	Worker_ConnectionParameters Params = Worker_DefaultConnectionParameters();
	Params.worker_type = "CAPIClient";
	Params.network.tcp.multiplex_level = 4;
	//Params.component_vtable_count = 0;
	//Params.component_vtables = Vtables.Vtables;
	Worker_ComponentVtable DefaultVtable = {};
	Params.default_component_vtable = &DefaultVtable;

	auto* ConnectionFuture = Worker_ConnectAsync("localhost", 7777, "CAPIClient42", &Params);
	Connection = Worker_ConnectionFuture_Get(ConnectionFuture, nullptr);
	Worker_ConnectionFuture_Destroy(ConnectionFuture);

	if (Worker_Connection_IsConnected(Connection))
	{
		Worker_LogMessage Message = { WORKER_LOG_LEVEL_WARN, "Client", "Whaduuuup", nullptr };
		Worker_Connection_SendLogMessage(Connection, &Message);

		Worker_CommandRequest CommandRequest = {};
		CommandRequest.component_id = SPECIAL_SPAWNER_COMPONENT_ID;
		CommandRequest.schema_type = Schema_CreateCommandRequest(SPECIAL_SPAWNER_COMPONENT_ID, 1);
		auto* RequestObject = Schema_GetCommandRequestObject(CommandRequest.schema_type);
		Schema_AddString(RequestObject, 1, "Yo dawg");
		Worker_CommandParameters CommandParams = {};
		Worker_Connection_SendCommandRequest(Connection, SPECIAL_SPAWNER_ENTITY_ID, &CommandRequest, 1, nullptr, &CommandParams);
	}
}

void UDTBManager::InitServer()
{
	if (Connection) return;

	//auto Vtables = CreateVtables();

	Worker_ConnectionParameters Params = Worker_DefaultConnectionParameters();
	Params.worker_type = "CAPIWorker";
	Params.network.tcp.multiplex_level = 4;
	Params.enable_protocol_logging_at_startup = true;
	Params.protocol_logging.log_prefix = "C:\\workspace\\UnrealGDK\\UnrealGDKStarterProject\\spatial\\logs\\whyyy";
	//Params.component_vtable_count = 0;
	//Params.component_vtables = Vtables.Vtables;
	Worker_ComponentVtable DefaultVtable = {};
	Params.default_component_vtable = &DefaultVtable;

	auto* ConnectionFuture = Worker_ConnectAsync("localhost", 7777, "CAPIWorker42", &Params);
	Connection = Worker_ConnectionFuture_Get(ConnectionFuture, nullptr);
	Worker_ConnectionFuture_Destroy(ConnectionFuture);

	if (Worker_Connection_IsConnected(Connection))
	{
		Worker_LogMessage Message = { WORKER_LOG_LEVEL_WARN, "Server", "Whaduuuup", nullptr };
		Worker_Connection_SendLogMessage(Connection, &Message);
	}
}

void UDTBManager::OnCommandRequest(Worker_CommandRequestOp& Op)
{
	auto CommandIndex = Schema_GetCommandRequestCommandIndex(Op.request.schema_type);
	UE_LOG(LogTemp, Log, TEXT("!!! Received command request (entity: %lld, component: %d, command: %d)"), Op.entity_id, Op.request.component_id, CommandIndex);

	if (Op.request.component_id == SPECIAL_SPAWNER_COMPONENT_ID && CommandIndex == 1)
	{
		auto* Payload = Schema_GetCommandRequestObject(Op.request.schema_type);
		std::string Url((char*)Schema_GetBytes(Payload, 1), Schema_GetBytesLength(Payload, 1));

		UE_LOG(LogTemp, Log, TEXT("!!! Url: %s"), UTF8_TO_TCHAR(Url.c_str()));

		if (OnSpawnRequest)
		{
			OnSpawnRequest();
		}

		Worker_CommandResponse Response = {};
		Response.component_id = Op.request.component_id;
		Response.schema_type = Schema_CreateCommandResponse(Op.request.component_id, CommandIndex);
		auto* ResponseObject = Schema_GetCommandResponseObject(Response.schema_type);
		Schema_AddBool(ResponseObject, 1, 1);
		Schema_AddBytes(ResponseObject, 2, (const uint8_t*)"", 0);
		Schema_AddEntityId(ResponseObject, 3, 0);
		Worker_Connection_SendCommandResponse(Connection, Op.request_id, &Response);
	}
}

void UDTBManager::OnCommandResponse(Worker_CommandResponseOp& Op)
{
	auto CommandIndex = Schema_GetCommandResponseCommandIndex(Op.response.schema_type);
	UE_LOG(LogTemp, Log, TEXT("!!! Received command response (entity: %lld, component: %d, command: %d)"), Op.entity_id, Op.response.component_id, CommandIndex);
}

void UDTBManager::SendReserveEntityIdRequest(USpatialActorChannel* Channel)
{
	auto RequestId = Worker_Connection_SendReserveEntityIdRequest(Connection, nullptr);
	AddPendingActorRequest(RequestId, Channel);
	UE_LOG(LogTemp, Log, TEXT("!!! Opened channel for actor with no entity ID. Initiated reserve entity ID. Request id: %d"), RequestId);
}

void UDTBManager::AddPendingActorRequest(Worker_RequestId RequestId, USpatialActorChannel* Channel)
{
	PendingActorRequests.Emplace(RequestId, Channel);
}

USpatialActorChannel* UDTBManager::RemovePendingActorRequest(Worker_RequestId RequestId)
{
	USpatialActorChannel** Channel = PendingActorRequests.Find(RequestId);
	if (Channel == nullptr)
	{
		return nullptr;
	}
	PendingActorRequests.Remove(RequestId);
	return *Channel;
}

void UDTBManager::OnReserveEntityIdResponse(Worker_ReserveEntityIdResponseOp& Op)
{
	if (auto* Channel = RemovePendingActorRequest(Op.request_id))
	{
		Channel->OnReserveEntityIdResponseCAPI(Op);
	}
}

Worker_RequestId UDTBManager::SendCreateEntityRequest(USpatialActorChannel* Channel, const FVector& Location, const FString& PlayerWorkerId, const TArray<uint16>& RepChanged, const TArray<uint16>& HandoverChanged)
{
	if (!Connection) return 0;

	AActor* Actor = Channel->Actor;

	FStringAssetReference ActorClassRef(Actor->GetClass());
	FString PathStr = ActorClassRef.ToString();

	auto CreateEntityRequestId = CreateActorEntity(PlayerWorkerId, Location, PathStr, Channel->GetChangeState(RepChanged, HandoverChanged), Channel);

	UE_LOG(LogTemp, Log, TEXT("!!! Creating entity for actor %s (%lld) using initial changelist. Request ID: %d"),
		*Actor->GetName(), Channel->GetEntityId().ToSpatialEntityId(), CreateEntityRequestId);

	return CreateEntityRequestId;
}



Worker_RequestId UDTBManager::CreateActorEntity(const FString& ClientWorkerId, const FVector& Position, const FString& Metadata, const FPropertyChangeState& InitialChanges, USpatialActorChannel* Channel)
{
	//for (auto& Rep : InitialChanges.RepChanged)
	//{
	//	checkf(Rep < InitialChanges.RepBaseHandleToCmdIndex.Num(), TEXT("How did we get a handle that Unreal doesn't know about itself?!"));
	//}

	// BUILD REP COMPONENT

	std::string ClientWorkerIdString = TCHAR_TO_UTF8(*ClientWorkerId);

	WorkerAttributeSet WorkerAttribute = {"CAPIWorker"};
	WorkerAttributeSet ClientAttribute = {"CAPIClient"};
	WorkerAttributeSet OwningClientAttribute = {"workerId:" + ClientWorkerIdString};

	WorkerRequirementSet WorkersOnly = {WorkerAttribute};
	WorkerRequirementSet ClientsOnly = {ClientAttribute};
	WorkerRequirementSet OwningClientOnly = {OwningClientAttribute};
	WorkerRequirementSet AnyUnrealWorkerOrClient = {WorkerAttribute, ClientAttribute};
	WorkerRequirementSet AnyUnrealWorkerOrOwningClient = {WorkerAttribute, OwningClientAttribute};

	std::map<Worker_ComponentId, WorkerRequirementSet> ComponentWriteAcl;
	ComponentWriteAcl.emplace(POSITION_COMPONENT_ID, WorkersOnly);

	std::vector<Worker_ComponentData> ComponentDatas(4, Worker_ComponentData{});
	CreatePositionData(ComponentDatas[0], SpatialConstants::LocationToSpatialOSCoordinates(Position));
	CreateMetadataData(ComponentDatas[1], TCHAR_TO_UTF8(*Metadata));
	CreateEntityAclData(ComponentDatas[2], AnyUnrealWorkerOrOwningClient, ComponentWriteAcl);
	CreatePersistenceData(ComponentDatas[3]);

	Worker_EntityId EntityId = Channel->GetEntityId().ToSpatialEntityId();
	return Worker_Connection_SendCreateEntityRequest(Connection, ComponentDatas.size(), ComponentDatas.data(), &EntityId, nullptr);
}

void UDTBManager::Tick()
{
	if (!Connection) return;

	Worker_OpList* OpList = Worker_Connection_GetOpList(Connection, 0);
	for (size_t i = 0; i < OpList->op_count; ++i)
	{
		Worker_Op* Op = &OpList->ops[i];
		switch (Op->op_type)
		{
		case WORKER_OP_TYPE_DISCONNECT:
			UE_LOG(LogTemp, Warning, TEXT("!!! Yo dawg y u disconnect me?! %s"), UTF8_TO_TCHAR(Op->disconnect.reason));
			break;
		case WORKER_OP_TYPE_LOG_MESSAGE:
			UE_LOG(LogTemp, Log, TEXT("!!! Log: %s"), UTF8_TO_TCHAR(Op->log_message.message));
			break;
		case WORKER_OP_TYPE_RESERVE_ENTITY_ID_RESPONSE:
			OnReserveEntityIdResponse(Op->reserve_entity_id_response);
			break;
		case WORKER_OP_TYPE_ADD_COMPONENT:
			break;
		case WORKER_OP_TYPE_CREATE_ENTITY_RESPONSE:
			UE_LOG(LogTemp, Log, TEXT("!!! Create entity response"), UTF8_TO_TCHAR(Op->create_entity_response.message));
			break;
		case WORKER_OP_TYPE_COMMAND_REQUEST:
			OnCommandRequest(Op->command_request);
			break;
		case WORKER_OP_TYPE_COMMAND_RESPONSE:
			OnCommandResponse(Op->command_response);
			break;
		default:
			break;
		}
	}
	Worker_OpList_Destroy(OpList);
}
