// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "DTBManager.h"

#include "SpatialActorChannel.h"
#include "SpatialInterop.h"

#include "SpatialGDKEditorToolbarSettings.h"

const Worker_ComponentId SPECIAL_SPAWNER_COMPONENT_ID = 100003;
const Worker_EntityId SPECIAL_SPAWNER_ENTITY_ID = 3;

UDTBManager::UDTBManager()
{
}

ClassInfo* UDTBManager::FindClassInfoByClass(UClass* Class)
{
	for (UClass* CurrentClass = Class; CurrentClass; CurrentClass = CurrentClass->GetSuperClass())
	{
		auto Info = ClassInfoMap.Find(CurrentClass);
		if (Info)
		{
			return Info;
		}
	}
	return nullptr;
}

void UDTBManager::CreateTypebindings()
{
	const USpatialGDKEditorToolbarSettings* SpatialGDKToolbarSettings = GetDefault<USpatialGDKEditorToolbarSettings>();
	TMap<UClass*, TArray<FString>> InteropGeneratedClasses;
	for (auto& Element : SpatialGDKToolbarSettings->InteropCodegenClasses)
	{
		InteropGeneratedClasses.Add(Element.ReplicatedClass, Element.IncludeList);
	}

	int ComponentId = 100010;
	for (auto& ClassHeaderList : InteropGeneratedClasses)
	{
		auto Class = ClassHeaderList.Key;

		auto AddComponentId = [Class, &ComponentId, this]()
		{
			ComponentToClassMap.Add(ComponentId, Class);
			return ComponentId++;
		};

		ClassInfo Info;

		for (TFieldIterator<UFunction> RemoteFunction(Class); RemoteFunction; ++RemoteFunction)
		{
			if (RemoteFunction->FunctionFlags & FUNC_NetClient ||
				RemoteFunction->FunctionFlags & FUNC_NetServer ||
				RemoteFunction->FunctionFlags & FUNC_NetCrossServer ||
				RemoteFunction->FunctionFlags & FUNC_NetMulticast)
			{
				EAlsoRPCType RPCType;
				if (RemoteFunction->FunctionFlags & FUNC_NetClient)
				{
					RPCType = ARPC_Client;
				}
				else if (RemoteFunction->FunctionFlags & FUNC_NetServer)
				{
					RPCType = ARPC_Server;
				}
				else if (RemoteFunction->FunctionFlags & FUNC_NetCrossServer)
				{
					RPCType = ARPC_CrossServer;
				}
				else if (RemoteFunction->FunctionFlags & FUNC_NetMulticast)
				{
					RPCType = ARPC_NetMulticast;
				}
				else
				{
					checkNoEntry();
				}

				auto& RPCArray = Info.RPCs.FindOrAdd(RPCType);

				RPCInfo RPCInfo;
				RPCInfo.Type = RPCType;
				RPCInfo.Index = RPCArray.Num();

				RPCArray.Add(*RemoteFunction);
				Info.RPCInfoMap.Add(*RemoteFunction, RPCInfo);
			}
		}

		Info.SingleClientComponent = AddComponentId();
		Info.MultiClientComponent = AddComponentId();
		Info.HandoverComponent = AddComponentId();
		for (int RPCType = ARPC_Client; RPCType < ARPC_Count; RPCType++)
		{
			Info.RPCComponents[RPCType] = AddComponentId();
		}

		ClassInfoMap.Add(Class, Info);
	}
}

void UDTBManager::InitClient()
{
	if (Connection) return;

	CreateTypebindings();

	Worker_ConnectionParameters Params = Worker_DefaultConnectionParameters();
	Params.worker_type = "CAPIClient";
	Params.network.tcp.multiplex_level = 4;
	//Params.component_vtable_count = 0;
	//Params.component_vtables = Vtables.Vtables;
	//Params.enable_protocol_logging_at_startup = true;
	//Params.protocol_logging.log_prefix = "C:\\workspace\\UnrealGDK\\UnrealGDKStarterProject\\spatial\\logs\\whyyy";
	Worker_ComponentVtable DefaultVtable = {};
	Params.default_component_vtable = &DefaultVtable;

	std::string WorkerId = "CAPIClient" + std::string(TCHAR_TO_UTF8(*FGuid::NewGuid().ToString()));
	auto* ConnectionFuture = Worker_ConnectAsync("localhost", 7777, WorkerId.c_str(), &Params);
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

		// TODO: Remove this
		Worker_EntityId ControllerEntity = 0;
		{
			auto NetGUID = PackageMap->GetNetGUIDFromObject(PipelineBlock.World->GetFirstPlayerController());
			check(NetGUID.IsValid());
			auto ObjectRef = PackageMap->GetUnrealObjectRefFromNetGUID(NetGUID);
			check(ObjectRef != SpatialConstants::UNRESOLVED_OBJECT_REF);
			ControllerEntity = ObjectRef.entity();
		}
		check(ControllerEntity > 0);
		Schema_AddEntityId(RequestObject, 2, ControllerEntity);
		Worker_CommandParameters CommandParams = {};
		Worker_Connection_SendCommandRequest(Connection, SPECIAL_SPAWNER_ENTITY_ID, &CommandRequest, 1, nullptr, &CommandParams);
	}
}

void UDTBManager::InitServer()
{
	if (Connection) return;

	CreateTypebindings();

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

	Worker_CommandResponse Response = {};
	Response.component_id = Op.request.component_id;
	Response.schema_type = Schema_CreateCommandResponse(Op.request.component_id, CommandIndex);

	if (auto ClassPtr = ComponentToClassMap.Find(Op.request.component_id))
	{
		auto Class = *ClassPtr;
		auto Info = FindClassInfoByClass(Class);
		check(Info);

		EAlsoRPCType RPCType = ARPC_Count;
		for (int i = ARPC_Client; i <= ARPC_CrossServer; i++)
		{
			if (Info->RPCComponents[i] == Op.request.component_id)
			{
				RPCType = (EAlsoRPCType)i;
				break;
			}
		}
		check(RPCType <= ARPC_CrossServer);

		auto RPCArray = Info->RPCs.Find(RPCType);
		check(RPCArray);
		check((int)CommandIndex - 1 < RPCArray->Num());

		auto Function = (*RPCArray)[CommandIndex - 1];

		uint8* Parms = (uint8*)FMemory_Alloca(Function->ParmsSize);
		FMemory::Memzero(Parms, Function->ParmsSize);

		UObject* TargetObject = nullptr;
		ReceiveRPCCommandRequest(Op.request, Op.entity_id, Function, PackageMap, Interop->GetNetDriver(), TargetObject, Parms);

		if (TargetObject)
		{
			TargetObject->ProcessEvent(Function, Parms);
		}

		// Destroy the parameters.
		for (TFieldIterator<UProperty> It(Function); It && It->HasAnyPropertyFlags(CPF_Parm); ++It)
		{
			It->DestroyValue_InContainer(Parms);
		}
	}
	else if (Op.request.component_id == SPECIAL_SPAWNER_COMPONENT_ID && CommandIndex == 1)
	{
		auto* Payload = Schema_GetCommandRequestObject(Op.request.schema_type);
		std::string Url = Schema_GetString(Payload, 1);
		UE_LOG(LogTemp, Log, TEXT("!!! Url: %s"), UTF8_TO_TCHAR(Url.c_str()));

		Worker_EntityId OwnerEntity = Schema_GetEntityId(Payload, 2);
		improbable::unreal::UnrealObjectRef OwnerEntityRef = { OwnerEntity, 0, {}, {} };
		auto OwnerController = Cast<APlayerController>(PackageMap->GetObjectFromNetGUID(PackageMap->GetNetGUIDFromUnrealObjectRef(OwnerEntityRef), false));
		check(OwnerController);

		if (OnSpawnRequest)
		{
			AActor* SpawnedActor = OnSpawnRequest();
			ActorToWorkerId.Add(SpawnedActor, UTF8_TO_TCHAR(Op.caller_worker_id));
			SpawnedActor->SetOwner(OwnerController);
		}

		auto* ResponseObject = Schema_GetCommandResponseObject(Response.schema_type);
		Schema_AddBool(ResponseObject, 1, 1);
		Schema_AddBytes(ResponseObject, 2, (const std::uint8_t*)"", 0);
		Schema_AddEntityId(ResponseObject, 3, 0);
	}

	Worker_Connection_SendCommandResponse(Connection, Op.request_id, &Response);
}

void UDTBManager::OnCommandResponse(Worker_CommandResponseOp& Op)
{
	auto CommandIndex = Schema_GetCommandResponseCommandIndex(Op.response.schema_type);
	UE_LOG(LogTemp, Log, TEXT("!!! Received command response (entity: %lld, component: %d, command: %d)"), Op.entity_id, Op.response.component_id, CommandIndex);

	// TODO: Re-send reliable RPCs on timeout
}

void UDTBManager::OnDynamicData(Worker_ComponentData& Data, USpatialActorChannel* Channel, USpatialPackageMapClient* PackageMap)
{
	auto ClassPtr = ComponentToClassMap.Find(Data.component_id);
	checkf(ClassPtr, TEXT("Component %d isn't hand-written and not present in ComponentToClassMap."));
	auto Class = *ClassPtr;
	check(Channel->Actor->IsA(Class));
	auto Info = FindClassInfoByClass(Class);
	check(Info);

	// TODO: Make this work with subobjects
	UObject* TargetObject = Channel->Actor;
	auto ChannelObjectPair = FChannelObjectPair(Channel, TargetObject);

	auto& ObjectReferencesMap = UnresolvedRefsMap.FindOrAdd(ChannelObjectPair);
	TSet<UnrealObjectRef> UnresolvedRefs;

	if (Data.component_id == Info->SingleClientComponent)
	{
		ReadDynamicData(Data, Channel, PackageMap, Interop->GetNetDriver(), AGROUP_SingleClient, ObjectReferencesMap, UnresolvedRefs);
	}
	else if (Data.component_id == Info->MultiClientComponent)
	{
		ReadDynamicData(Data, Channel, PackageMap, Interop->GetNetDriver(), AGROUP_MultiClient, ObjectReferencesMap, UnresolvedRefs);
	}
	else if (Data.component_id == Info->HandoverComponent)
	{
		// TODO: Handover
	}
	else
	{
		UE_LOG(LogTemp, Verbose, TEXT("!!! Skipping because RPC components don't have actual data."));
	}

	QueueIncomingRepUpdates(ChannelObjectPair, ObjectReferencesMap, UnresolvedRefs);
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
	}

	auto ClassPtr = ComponentToClassMap.Find(Op.update.component_id);
	checkf(ClassPtr, TEXT("Component %d isn't hand-written and not present in ComponentToClassMap."));
	auto Class = *ClassPtr;
	auto Info = FindClassInfoByClass(Class);
	check(Info);

	USpatialActorChannel* ActorChannel = Interop->GetActorChannelByEntityId(Op.entity_id);
	bool bIsServer = Interop->GetNetDriver()->IsServer();

	if (Op.update.component_id == Info->SingleClientComponent)
	{
		if (bIsServer)
		{
			UE_LOG(LogTemp, Verbose, TEXT("!!! Skipping SingleClient component because we're a server."));
			return;
		}
		check(ActorChannel);

		HandleComponentUpdate(Op.update, ActorChannel, AGROUP_SingleClient);
	}
	else if (Op.update.component_id == Info->MultiClientComponent)
	{
		if (bIsServer)
		{
			UE_LOG(LogTemp, Verbose, TEXT("!!! Skipping MultiClient component because we're a server."));
			return;
		}
		check(ActorChannel);

		HandleComponentUpdate(Op.update, ActorChannel, AGROUP_MultiClient);
	}
	else if (Op.update.component_id == Info->HandoverComponent)
	{
		if (!bIsServer)
		{
			UE_LOG(LogTemp, Verbose, TEXT("!!! Skipping Handover component because we're a client."));
			return;
		}
		// TODO: Handover
	}
	else if (Op.update.component_id == Info->RPCComponents[ARPC_NetMulticast])
	{
		if (bIsServer)
		{
			UE_LOG(LogTemp, Verbose, TEXT("!!! Skipping MulticastRPC component because we're a server."));
			return;
		}
		check(ActorChannel);
		auto& RPCArray = Info->RPCs.FindChecked(ARPC_NetMulticast);
		ReceiveMulticastUpdate(Op.update, Op.entity_id, RPCArray, PackageMap, Interop->GetNetDriver());
	}
	else
	{
		UE_LOG(LogTemp, Verbose, TEXT("!!! Skipping because it's an empty component update from an RPC component. (most likely as a result of gaining authority)"));
	}
}

void UDTBManager::HandleComponentUpdate(const Worker_ComponentUpdate& ComponentUpdate, USpatialActorChannel* Channel, EAlsoReplicatedPropertyGroup PropertyGroup)
{
	// TODO: Make this work with subobjects
	UObject* TargetObject = Channel->Actor;
	auto ChannelObjectPair = FChannelObjectPair(Channel, TargetObject);

	auto& ObjectReferencesMap = UnresolvedRefsMap.FindOrAdd(ChannelObjectPair);
	TSet<UnrealObjectRef> UnresolvedRefs;
	ReceiveDynamicUpdate(ComponentUpdate, Channel, PackageMap, Interop->GetNetDriver(), PropertyGroup, ObjectReferencesMap, UnresolvedRefs);

	QueueIncomingRepUpdates(ChannelObjectPair, ObjectReferencesMap, UnresolvedRefs);
}

void UDTBManager::QueueIncomingRepUpdates(FChannelObjectPair ChannelObjectPair, const FObjectReferencesMap& ObjectReferencesMap, const TSet<UnrealObjectRef>& UnresolvedRefs)
{
	for (auto& UnresolvedRef : UnresolvedRefs)
	{
		IncomingRefsMap.FindOrAdd(UnresolvedRef).Add(ChannelObjectPair);
	}

	if (ObjectReferencesMap.Num() == 0)
	{
		UnresolvedRefsMap.Remove(ChannelObjectPair);
	}
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

	auto WorkerIdPtr = ActorToWorkerId.Find(Actor);

	auto CreateEntityRequestId = CreateActorEntity(WorkerIdPtr ? *WorkerIdPtr : PlayerWorkerId, Location, PathStr, Channel->GetChangeState(RepChanged, HandoverChanged), Channel);

	UE_LOG(LogTemp, Log, TEXT("!!! Creating entity for actor %s (%lld) using initial changelist. Request ID: %d"),
		*Actor->GetName(), Channel->GetEntityId().ToSpatialEntityId(), CreateEntityRequestId);

	return CreateEntityRequestId;
}

Worker_RequestId UDTBManager::CreateActorEntity(const FString& ClientWorkerId, const FVector& Position, const FString& Metadata, const FPropertyChangeState& InitialChanges, USpatialActorChannel* Channel)
{
	std::string ClientWorkerIdString = TCHAR_TO_UTF8(*ClientWorkerId);

	WorkerAttributeSet WorkerAttribute = {"CAPIWorker"};
	WorkerAttributeSet ClientAttribute = {"CAPIClient"};
	WorkerAttributeSet OwningClientAttribute = {"workerId:" + ClientWorkerIdString};

	WorkerRequirementSet WorkersOnly = {WorkerAttribute};
	WorkerRequirementSet ClientsOnly = {ClientAttribute};
	WorkerRequirementSet OwningClientOnly = {OwningClientAttribute};

	auto Actor = Channel->Actor;

	WorkerRequirementSet AnyUnrealWorkerOrClient = {WorkerAttribute, ClientAttribute};
	WorkerRequirementSet AnyUnrealWorkerOrOwningClient = {WorkerAttribute, OwningClientAttribute};
	WorkerRequirementSet ReadAcl = Actor->IsA<APlayerController>() ? AnyUnrealWorkerOrOwningClient : AnyUnrealWorkerOrClient;

	auto Info = FindClassInfoByClass(Actor->GetClass());
	check(Info);

	std::map<Worker_ComponentId, WorkerRequirementSet> ComponentWriteAcl;
	ComponentWriteAcl.emplace(POSITION_COMPONENT_ID, WorkersOnly);

	ComponentWriteAcl.emplace(Info->SingleClientComponent, WorkersOnly);
	ComponentWriteAcl.emplace(Info->MultiClientComponent, WorkersOnly);
	ComponentWriteAcl.emplace(Info->HandoverComponent, WorkersOnly);
	ComponentWriteAcl.emplace(Info->RPCComponents[ARPC_Client], OwningClientOnly);
	ComponentWriteAcl.emplace(Info->RPCComponents[ARPC_Server], WorkersOnly);
	ComponentWriteAcl.emplace(Info->RPCComponents[ARPC_CrossServer], WorkersOnly);
	ComponentWriteAcl.emplace(Info->RPCComponents[ARPC_NetMulticast], WorkersOnly);

	std::string StaticPath;

	if (Actor->IsFullNameStableForNetworking())
	{
		StaticPath = TCHAR_TO_UTF8(*Actor->GetPathName(Actor->GetWorld()));
	}

	uint32 CurrentOffset = 1;
	std::map<std::string, std::uint32_t> SubobjectNameToOffset;
	ForEachObjectWithOuter(Actor, [&CurrentOffset, &SubobjectNameToOffset](UObject* Object)
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
	ComponentDatas.push_back(CreateEntityAclData(EntityAclData(ReadAcl, ComponentWriteAcl)));
	ComponentDatas.push_back(CreatePersistenceData(PersistenceData()));
	ComponentDatas.push_back(CreateUnrealMetadataData(UnrealMetadataData(StaticPath, ClientWorkerIdString, SubobjectNameToOffset)));

	FUnresolvedObjectsMap UnresolvedObjectsMap;

	ComponentDatas.push_back(CreateDynamicData(Info->SingleClientComponent, InitialChanges, PackageMap, Interop->GetNetDriver(), AGROUP_SingleClient, UnresolvedObjectsMap));
	ComponentDatas.push_back(CreateDynamicData(Info->MultiClientComponent, InitialChanges, PackageMap, Interop->GetNetDriver(), AGROUP_MultiClient, UnresolvedObjectsMap));

	for (auto& HandleUnresolvedObjectsPair : UnresolvedObjectsMap)
	{
		QueueOutgoingRepUpdate(Channel, Actor, HandleUnresolvedObjectsPair.Key, HandleUnresolvedObjectsPair.Value);
	}

	// TODO: Handover
	Worker_ComponentData HandoverData = {};
	HandoverData.component_id = Info->HandoverComponent;
	HandoverData.schema_type = Schema_CreateComponentData(Info->HandoverComponent);
	ComponentDatas.push_back(HandoverData);

	for (int RPCType = 0; RPCType < ARPC_Count; RPCType++)
	{
		Worker_ComponentData RPCData = {};
		RPCData.component_id = Info->RPCComponents[RPCType];
		RPCData.schema_type = Schema_CreateComponentData(Info->RPCComponents[RPCType]);
		ComponentDatas.push_back(RPCData);
	}

	Worker_EntityId EntityId = Channel->GetEntityId().ToSpatialEntityId();
	return Worker_Connection_SendCreateEntityRequest(Connection, ComponentDatas.size(), ComponentDatas.data(), &EntityId, nullptr);
}

void UDTBManager::SendSpatialPositionUpdate(Worker_EntityId EntityId, const FVector& Location)
{
	auto ComponentUpdate = CreatePositionUpdate(LocationToCAPIPosition(Location));

	Worker_Connection_SendComponentUpdate(Connection, EntityId, &ComponentUpdate);
}

void UDTBManager::SendComponentUpdates(UObject* Object, const FPropertyChangeState& Changes, USpatialActorChannel* Channel)
{
	Worker_EntityId EntityId = Channel->GetEntityId().ToSpatialEntityId();
	UE_LOG(LogTemp, Verbose, TEXT("!!! Sending component update (object: %s, entity: %lld)"), *Object->GetName(), EntityId);

	auto Info = FindClassInfoByClass(Object->GetClass());
	check(Info);

	FUnresolvedObjectsMap UnresolvedObjectsMap;

	bool bWroteSingleClient = false;
	auto SingleClientRepDataUpdate = CreateDynamicUpdate(Info->SingleClientComponent, Changes, PackageMap, Interop->GetNetDriver(), AGROUP_SingleClient, UnresolvedObjectsMap, bWroteSingleClient);

	bool bWroteMultiClient = false;
	auto MultiClientRepDataUpdate = CreateDynamicUpdate(Info->MultiClientComponent, Changes, PackageMap, Interop->GetNetDriver(), AGROUP_MultiClient, UnresolvedObjectsMap, bWroteMultiClient);

	for (auto Handle : Changes.RepChanged)
	{
		ResetOutgoingRepUpdate(Channel, Object, Handle);

		if (auto UnresolvedObjects = UnresolvedObjectsMap.Find(Handle))
		{
			QueueOutgoingRepUpdate(Channel, Object, Handle, *UnresolvedObjects);
		}
	}

	if (bWroteSingleClient)
	{
		Worker_Connection_SendComponentUpdate(Connection, EntityId, &SingleClientRepDataUpdate);
	}
	if (bWroteMultiClient)
	{
		Worker_Connection_SendComponentUpdate(Connection, EntityId, &MultiClientRepDataUpdate);
	}
	// TODO: Handover
}

void UDTBManager::SendRPC(UObject* TargetObject, UFunction* Function, void* Parameters)
{
	auto Info = FindClassInfoByClass(TargetObject->GetClass());
	check(Info);

	auto RPCInfo = Info->RPCInfoMap.Find(Function);
	check(RPCInfo);

	switch (RPCInfo->Type)
	{
	case ARPC_Client:
	case ARPC_Server:
	case ARPC_CrossServer:
	{
		Worker_EntityId EntityId = 0;
		auto CommandRequest = CreateRPCCommandRequest(TargetObject, Function, Parameters, PackageMap, Interop->GetNetDriver(), Info->RPCComponents[RPCInfo->Type], RPCInfo->Index + 1, EntityId);
		check(EntityId > 0);
		Worker_CommandParameters CommandParams = {};
		Worker_Connection_SendCommandRequest(Connection, EntityId, &CommandRequest, RPCInfo->Index + 1, nullptr, &CommandParams);
		break;
	}
	case ARPC_NetMulticast:
	{
		Worker_EntityId EntityId = 0;
		auto ComponentUpdate = CreateMulticastUpdate(TargetObject, Function, Parameters, PackageMap, Interop->GetNetDriver(), Info->RPCComponents[RPCInfo->Type], RPCInfo->Index + 1, EntityId);
		check(EntityId > 0);
		Worker_Connection_SendComponentUpdate(Connection, EntityId, &ComponentUpdate);
		break;
	}
	default:
		break;
	}
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
				for (auto& It : ResolvedObjectQueue)
				{
					ResolvePendingOperations_Internal(It.Key, It.Value);
				}
				ResolvedObjectQueue.Empty();
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

void UDTBManager::ResetOutgoingRepUpdate(USpatialActorChannel* DependentChannel, UObject* ReplicatedObject, int16 Handle)
{
	check(DependentChannel);
	check(ReplicatedObject);
	auto ChannelObjectPair = FChannelObjectPair(DependentChannel, ReplicatedObject);

	auto HandleToUnresolved = PropertyToUnresolved.Find(ChannelObjectPair);
	if (!HandleToUnresolved) return;

	auto UnresolvedPtr = HandleToUnresolved->Find(Handle);
	if (!UnresolvedPtr) return;

	FUnresolvedEntry& Unresolved = *UnresolvedPtr;

	check(Unresolved.IsValid());

	UE_LOG(LogTemp, Log, TEXT("!!! Resetting pending outgoing array depending on channel: %s, object: %s, handle: %d."),
		*DependentChannel->GetName(), *ReplicatedObject->GetName(), Handle);

	for (auto UnresolvedObject : *Unresolved)
	{
		auto& ChannelToUnresolved = ObjectToUnresolved.FindChecked(UnresolvedObject);
		auto& OtherHandleToUnresolved = ChannelToUnresolved.FindChecked(ChannelObjectPair);

		OtherHandleToUnresolved.Remove(Handle);
		if (OtherHandleToUnresolved.Num() == 0)
		{
			ChannelToUnresolved.Remove(ChannelObjectPair);
			if (ChannelToUnresolved.Num() == 0)
			{
				ObjectToUnresolved.Remove(UnresolvedObject);
			}
		}
	}

	HandleToUnresolved->Remove(Handle);
	if (HandleToUnresolved->Num() == 0)
	{
		PropertyToUnresolved.Remove(ChannelObjectPair);
	}
}

void UDTBManager::QueueOutgoingRepUpdate(USpatialActorChannel* DependentChannel, UObject* ReplicatedObject, int16 Handle, const TSet<const UObject*>& UnresolvedObjects)
{
	check(DependentChannel);
	check(ReplicatedObject);
	auto ChannelObjectPair = FChannelObjectPair(DependentChannel, ReplicatedObject);

	UE_LOG(LogTemp, Log, TEXT("!!! Added pending outgoing property: channel: %s, object: %s, handle: %d. Depending on objects:"),
		*DependentChannel->GetName(), *ReplicatedObject->GetName(), Handle);

	auto Unresolved = MakeShared<TSet<const UObject*>>();
	*Unresolved = UnresolvedObjects;

	auto& HandleToUnresolved = PropertyToUnresolved.FindOrAdd(ChannelObjectPair);
	check(!HandleToUnresolved.Find(Handle));
	HandleToUnresolved.Add(Handle, Unresolved);

	for (auto UnresolvedObject : UnresolvedObjects)
	{
		auto& AnotherHandleToUnresolved = ObjectToUnresolved.FindOrAdd(UnresolvedObject).FindOrAdd(ChannelObjectPair);
		check(!AnotherHandleToUnresolved.Find(Handle));
		AnotherHandleToUnresolved.Add(Handle, Unresolved);

		// Following up on the previous log: listing the unresolved objects
		UE_LOG(LogTemp, Log, TEXT("!!! %s"), *UnresolvedObject->GetName());
	}
}

void UDTBManager::ResolvePendingOperations(UObject* Object, const UnrealObjectRef& ObjectRef)
{
	if (PipelineBlock.bInCriticalSection)
	{
		ResolvedObjectQueue.Add(TPair<UObject*, UnrealObjectRef>{ Object, ObjectRef });
	}
	else
	{
		ResolvePendingOperations_Internal(Object, ObjectRef);
	}
}

void UDTBManager::ResolvePendingOperations_Internal(UObject* Object, const UnrealObjectRef& ObjectRef)
{
	UE_LOG(LogTemp, Log, TEXT("!!! Resolving pending object refs and RPCs which depend on object: %s %s."), *Object->GetName(), *ObjectRef.ToString());
	ResolveOutgoingOperations(Object);
	ResolveIncomingOperations(Object, ObjectRef);
}

void UDTBManager::ResolveOutgoingOperations(UObject* Object)
{
	auto ChannelToUnresolved = ObjectToUnresolved.Find(Object);
	if (!ChannelToUnresolved) return;

	for (auto& ChannelProperties : *ChannelToUnresolved)
	{
		auto& ChannelObjectPair = ChannelProperties.Key;
		auto DependentChannel = ChannelObjectPair.Key;
		auto ReplicatingObject = ChannelObjectPair.Value;
		auto& HandleToUnresolved = ChannelProperties.Value;

		TArray<uint16> PropertyHandles;

		for (auto& HandleUnresolvedPair : HandleToUnresolved)
		{
			uint16 Handle = HandleUnresolvedPair.Key;
			auto& Unresolved = HandleUnresolvedPair.Value;

			Unresolved->Remove(Object);
			if (Unresolved->Num() == 0)
			{
				PropertyHandles.Add(Handle);

				// Hack to figure out if this property is an array to add extra handles
				if (DependentChannel->IsDynamicArrayHandle(ReplicatingObject, Handle))
				{
					PropertyHandles.Add(0);
					PropertyHandles.Add(0);
				}

				auto& AnotherHandleToUnresolved = PropertyToUnresolved.FindChecked(ChannelObjectPair);
				AnotherHandleToUnresolved.Remove(Handle);
				if (AnotherHandleToUnresolved.Num() == 0)
				{
					PropertyToUnresolved.Remove(ChannelObjectPair);
				}
			}
		}

		if (PropertyHandles.Num() > 0)
		{
			// End with zero to indicate the end of the list of handles.
			PropertyHandles.Add(0);

			Interop->SendSpatialUpdateForObject(DependentChannel, ReplicatingObject, PropertyHandles, TArray<uint16>());
		}
	}

	ObjectToUnresolved.Remove(Object);
}

void UDTBManager::ResolveIncomingOperations(UObject* Object, const UnrealObjectRef& ObjectRef)
{
	auto TargetObjectSet = IncomingRefsMap.Find(ObjectRef);
	if (!TargetObjectSet) return;

	for (auto& ChannelObjectPair : *TargetObjectSet)
	{
		auto UnresolvedRefs = UnresolvedRefsMap.Find(ChannelObjectPair);

		if (!UnresolvedRefs)
		{
			continue;
		}

		auto DependentChannel = ChannelObjectPair.Key;
		auto ReplicatingObject = ChannelObjectPair.Value;

		bool bStillHasUnresolved = false;
		bool bSomeObjectsWereMapped = false;
		TArray<UProperty*> RepNotifies;

		auto& RepLayout = DependentChannel->GetObjectRepLayout(ReplicatingObject);
		auto& ShadowData = DependentChannel->GetObjectStaticBuffer(ReplicatingObject);

		ResolveObjectReferences(RepLayout, ReplicatingObject, *UnresolvedRefs, PackageMap, Interop->GetNetDriver(), ShadowData.GetData(), (uint8*)ReplicatingObject, ShadowData.Num(), RepNotifies, bSomeObjectsWereMapped, bStillHasUnresolved);

		if (bSomeObjectsWereMapped)
		{
			DependentChannel->PostReceiveSpatialUpdate(ReplicatingObject, RepNotifies);
		}

		if (!bStillHasUnresolved)
		{
			UnresolvedRefsMap.Remove(ChannelObjectPair);
		}
	}

	IncomingRefsMap.Remove(ObjectRef);
}
