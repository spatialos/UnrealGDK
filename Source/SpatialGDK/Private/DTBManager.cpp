// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "DTBManager.h"

const int SPECIAL_SPAWNER_COMPONENT_ID = 100003;
const int SPECIAL_SPAWNER_ENTITY_ID = 3;

UDTBManager::UDTBManager()
{
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
		Schema_AddBytes(RequestObject, 1, (const uint8_t*)"Yo dawg", 7);
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

void OnCommandRequest(Worker_Connection* Connection, Worker_CommandRequestOp& Op)
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

void OnCommandResponse(Worker_Connection* Connection, Worker_CommandResponseOp& Op)
{
	auto CommandIndex = Schema_GetCommandResponseCommandIndex(Op.response.schema_type);
	UE_LOG(LogTemp, Log, TEXT("!!! Received command response (entity: %lld, component: %d, command: %d)"), Op.entity_id, Op.response.component_id, CommandIndex);
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
		case WORKER_OP_TYPE_ADD_COMPONENT:
			break;
		case WORKER_OP_TYPE_COMMAND_REQUEST:
			OnCommandRequest(Connection, Op->command_request);
			break;
		case WORKER_OP_TYPE_COMMAND_RESPONSE:
			OnCommandResponse(Connection, Op->command_response);
			break;
		default:
			break;
		}
	}
	Worker_OpList_Destroy(OpList);
}
