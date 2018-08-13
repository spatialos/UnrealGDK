// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "DTBManager.h"

#include "SpatialActorChannel.h"
#include "SpatialInterop.h"
#include "SchemaHelpers.h"

const Worker_ComponentId SPECIAL_SPAWNER_COMPONENT_ID = 100003;
const Worker_EntityId SPECIAL_SPAWNER_ENTITY_ID = 3;

UDTBManager::UDTBManager()
{
}

void UDTBManager::InitClient()
{
	if (Connection) return;

	Worker_ConnectionParameters Params = Worker_DefaultConnectionParameters();
	Params.worker_type = "CAPIClient";
	Params.network.tcp.multiplex_level = 4;
	//Params.component_vtable_count = 0;
	//Params.component_vtables = Vtables.Vtables;
	//Params.enable_protocol_logging_at_startup = true;
	//Params.protocol_logging.log_prefix = "C:\\workspace\\UnrealGDK\\UnrealGDKStarterProject\\spatial\\logs\\whyyy";
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

bool UDTBManager::DTBHasComponentAuthority(Worker_EntityId EntityId, Worker_ComponentId ComponentId)
{
	auto EntityAuthority = ComponentAuthorityMap.Find(EntityId);
	if (EntityAuthority)
	{
		auto ComponentAuthority = EntityAuthority->Find(ComponentId);
		if (ComponentAuthority)
		{
			return *ComponentAuthority == WORKER_AUTHORITY_AUTHORITATIVE;
		}
	}
	return false;
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
		Schema_AddBytes(ResponseObject, 2, (const std::uint8_t*)"", 0);
		Schema_AddEntityId(ResponseObject, 3, 0);
		Worker_Connection_SendCommandResponse(Connection, Op.request_id, &Response);
	}
}

void UDTBManager::OnCommandResponse(Worker_CommandResponseOp& Op)
{
	auto CommandIndex = Schema_GetCommandResponseCommandIndex(Op.response.schema_type);
	UE_LOG(LogTemp, Log, TEXT("!!! Received command response (entity: %lld, component: %d, command: %d)"), Op.entity_id, Op.response.component_id, CommandIndex);
}

void UDTBManager::OnComponentUpdate(Worker_ComponentUpdateOp& Op)
{
	UE_LOG(LogTemp, Verbose, TEXT("!!! Received component update (entity: %lld, component: %d)"), Op.entity_id, Op.update.component_id);

	// TODO: Remove this check once we can disable component update short circuiting. This will be exposed in 14.0. See TIG-137.
	if (DTBHasComponentAuthority(Op.entity_id, Op.update.component_id))
	{
		UE_LOG(LogTemp, Verbose, TEXT("!!! Skipping because we sent this update"));
		return;
	}

	switch (Op.update.component_id)
	{
	case ENTITY_ACL_COMPONENT_ID:
	case METADATA_COMPONENT_ID:
	case POSITION_COMPONENT_ID:
	case PERSISTENCE_COMPONENT_ID:
	case SPECIAL_SPAWNER_COMPONENT_ID:
	case UNREAL_METADATA_COMPONENT_ID:
		UE_LOG(LogTemp, Verbose, TEXT("!!! Skipping because this is hand-written Spatial component"));
		return;

	// TEMP
	case MULTI_REP_COMPONENT_ID:
		if (Interop->GetNetDriver()->GetNetMode() != NM_Client)
		{
			UE_LOG(LogTemp, Verbose, TEXT("!!! Skipping MultiRep component because we're a server"));
			return;
		}
		break;
	case CLIENT_RPCS_COMPONENT_ID:
		if (Interop->GetNetDriver()->GetNetMode() == NM_Client)
		{
			UE_LOG(LogTemp, Verbose, TEXT("!!! Skipping ClientRPCs component because we're a client"));
			return;
		}
		break;
	// TEMP
	}

	USpatialActorChannel* ActorChannel = Interop->GetActorChannelByEntityId(Op.entity_id);
	check(ActorChannel);
	ReceiveDynamicUpdate(Op.update, ActorChannel, PackageMap);
}

void UDTBManager::OnAuthorityChange(Worker_AuthorityChangeOp& Op)
{
	ComponentAuthorityMap.FindOrAdd(Op.entity_id).FindOrAdd(Op.component_id) = (Worker_Authority)Op.authority;
	UE_LOG(LogTemp, Log, TEXT("!!! Received authority change (entity: %lld, component: %d, authority: %d)"), Op.entity_id, Op.component_id, (int)Op.authority);
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

	std::string ClientWorkerIdString = "CAPIClient42";//TCHAR_TO_UTF8(*ClientWorkerId);

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

	// TEMP
	ComponentWriteAcl.emplace(MULTI_REP_COMPONENT_ID, WorkersOnly);
	ComponentWriteAcl.emplace(CLIENT_RPCS_COMPONENT_ID, OwningClientOnly);
	// TEMP

	std::string StaticPath;

	if (Channel->Actor->IsFullNameStableForNetworking())
	{
		StaticPath = TCHAR_TO_UTF8(*Channel->Actor->GetPathName(Channel->Actor->GetWorld()));
	}

	uint32 CurrentOffset = 1;
	std::map<std::string, std::uint32_t> SubobjectNameToOffset;
	ForEachObjectWithOuter(Channel->Actor, [&CurrentOffset, &SubobjectNameToOffset](UObject* Object)
	{
		// Objects can only be allocated NetGUIDs if this is true.
		if (Object->IsSupportedForNetworking() && !Object->IsPendingKill() && !Object->IsEditorOnly())
		{
			SubobjectNameToOffset.emplace(TCHAR_TO_UTF8(*(Object->GetName())), CurrentOffset);
			CurrentOffset++;
		}
	});

	std::vector<Worker_ComponentData> ComponentDatas;
	ComponentDatas.push_back(CreatePositionData(PositionData(LocationToCAPIPosition(Position))));
	ComponentDatas.push_back(CreateMetadataData(MetadataData(TCHAR_TO_UTF8(*Metadata))));
	ComponentDatas.push_back(CreateEntityAclData(EntityAclData(AnyUnrealWorkerOrOwningClient, ComponentWriteAcl)));
	ComponentDatas.push_back(CreatePersistenceData(PersistenceData()));
	ComponentDatas.push_back(CreateUnrealMetadataData(UnrealMetadataData(StaticPath, ClientWorkerIdString, SubobjectNameToOffset)));

	// MultiClientRepData
	ComponentDatas.push_back(CreateDynamicData(MULTI_REP_COMPONENT_ID, InitialChanges, PackageMap));

	// TEMP
	// ClientRPCs
	Worker_ComponentData ClientRPCsData = {};
	ClientRPCsData.component_id = CLIENT_RPCS_COMPONENT_ID;
	ClientRPCsData.schema_type = Schema_CreateComponentData(CLIENT_RPCS_COMPONENT_ID);
	ComponentDatas.push_back(ClientRPCsData);
	// TEMP

	Worker_EntityId EntityId = Channel->GetEntityId().ToSpatialEntityId();
	return Worker_Connection_SendCreateEntityRequest(Connection, ComponentDatas.size(), ComponentDatas.data(), &EntityId, nullptr);
}

void UDTBManager::SendSpatialPositionUpdate(Worker_EntityId EntityId, const FVector& Location)
{
	auto ComponentUpdate = CreatePositionUpdate(LocationToCAPIPosition(Location));

	Worker_Connection_SendComponentUpdate(Connection, EntityId, &ComponentUpdate);
}

void UDTBManager::SendComponentUpdates(const FPropertyChangeState& Changes, USpatialActorChannel* Channel)
{
	Worker_EntityId EntityId = Channel->GetEntityId().ToSpatialEntityId();
	UE_LOG(LogTemp, Verbose, TEXT("!!! Sending component update (actor: %s, entity: %lld)"), *Channel->Actor->GetName(), EntityId);

	auto MultiClientRepDataUpdate = CreateDynamicUpdate(MULTI_REP_COMPONENT_ID, Changes, PackageMap);

	Worker_Connection_SendComponentUpdate(Connection, EntityId, &MultiClientRepDataUpdate);
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
		case WORKER_OP_TYPE_FLAG_UPDATE:
			break;
		case WORKER_OP_TYPE_LOG_MESSAGE:
			UE_LOG(LogTemp, Log, TEXT("!!! Log: %s"), UTF8_TO_TCHAR(Op->log_message.message));
			break;
		case WORKER_OP_TYPE_METRICS:
			break;
		case WORKER_OP_TYPE_CRITICAL_SECTION:
			if (Op->critical_section.in_critical_section)
			{
				PipelineBlock.EnterCriticalSection();
			}
			else
			{
				PipelineBlock.LeaveCriticalSection();
			}
			break;
		case WORKER_OP_TYPE_ADD_ENTITY:
			PipelineBlock.AddEntity(Op->add_entity);
			break;
		case WORKER_OP_TYPE_REMOVE_ENTITY:
			//PipelineBlock.RemoveEntity(Op->remove_entity);
			break;
		case WORKER_OP_TYPE_RESERVE_ENTITY_ID_RESPONSE:
			OnReserveEntityIdResponse(Op->reserve_entity_id_response);
			break;
		case WORKER_OP_TYPE_RESERVE_ENTITY_IDS_RESPONSE:
			break;
		case WORKER_OP_TYPE_CREATE_ENTITY_RESPONSE:
			UE_LOG(LogTemp, Log, TEXT("!!! Create entity response"), UTF8_TO_TCHAR(Op->create_entity_response.message));
			break;
		case WORKER_OP_TYPE_DELETE_ENTITY_RESPONSE:
			break;
		case WORKER_OP_TYPE_ENTITY_QUERY_RESPONSE:
			break;
		case WORKER_OP_TYPE_ADD_COMPONENT:
			PipelineBlock.AddComponent(Op->add_component);
			break;
		case WORKER_OP_TYPE_REMOVE_COMPONENT:
			//PipelineBlock.RemoveComponent(Op->remove_component);
			break;
		case WORKER_OP_TYPE_AUTHORITY_CHANGE:
			OnAuthorityChange(Op->authority_change);
			break;
		case WORKER_OP_TYPE_COMPONENT_UPDATE:
			OnComponentUpdate(Op->component_update);
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
