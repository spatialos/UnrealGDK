// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "SpatialInterop.h"

#include "SpatialActorChannel.h"
#include "DTBUtil.h"
#include "AssetRegistryModule.h"
#include "SpatialNetDriver.h"
#include "SpatialPlayerSpawner.h"

#include "CoreTypes/StandardLibrary.h"

USpatialInterop::USpatialInterop()
{
}

void USpatialInterop::Init(USpatialNetDriver* NetDriver)
{
	this->NetDriver = NetDriver;
	Connection = NetDriver->Connection;

	EntityPipeline.Init(this);

	CreateTypebindings();
}

void USpatialInterop::FinishDestroy()
{
	if (Connection)
	{
		Worker_Connection_Destroy(Connection);
		Connection = nullptr;
	}

	Super::FinishDestroy();
}

FClassInfo* USpatialInterop::FindClassInfoByClass(UClass* Class)
{
	for (UClass* CurrentClass = Class; CurrentClass; CurrentClass = CurrentClass->GetSuperClass())
	{
		FClassInfo* Info = ClassInfoMap.Find(CurrentClass);
		if (Info)
		{
			return Info;
		}
	}
	return nullptr;
}

void USpatialInterop::CreateTypebindings()
{
	// Load the asset registry module
	FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>(FName("AssetRegistry"));
	IAssetRegistry& AssetRegistry = AssetRegistryModule.Get();

	// Before running the interop generator, ensure all blueprint classes that have been tagged with 'spatial' are loaded
	TArray<FAssetData> AssetData;
	uint32 SpatialClassFlags = 0;
	AssetRegistry.GetAssetsByClass(UBlueprint::StaticClass()->GetFName(), AssetData, true);
	for (FAssetData& It : AssetData)
	{
		if (It.GetTagValue("SpatialClassFlags", SpatialClassFlags))
		{
			if (SpatialClassFlags & SPATIALCLASS_GenerateTypeBindings)
			{
				FString ObjectPath = It.ObjectPath.ToString() + TEXT("_C");
				UClass* LoadedClass = LoadObject<UClass>(nullptr, *ObjectPath, nullptr, LOAD_EditorOnly, nullptr);
				UE_LOG(LogTemp, Log, TEXT("Found spatial blueprint class `%s`."), *ObjectPath);
				if (LoadedClass == nullptr)
				{
					FMessageDialog::Debugf(FText::FromString(FString::Printf(TEXT("Error: Failed to load blueprint %s."), *ObjectPath)));
				}
			}
		}
	}

	for (TObjectIterator<UClass> It; It; ++It)
	{
		if (It->HasAnySpatialClassFlags(SPATIALCLASS_GenerateTypeBindings) == false)
		{
			continue;
		}

		// Ensure we don't process skeleton or reinitialised classes
		if (It->GetName().StartsWith(TEXT("SKEL_"), ESearchCase::CaseSensitive)
			|| It->GetName().StartsWith(TEXT("REINST_"), ESearchCase::CaseSensitive))
		{
			continue;
		}

		DTBClasses.Add(*It);
	}

	int ComponentId = 100010;
	for (UClass* Class : DTBClasses)
	{
		auto AddComponentId = [Class, &ComponentId, this]()
		{
			ComponentToClassMap.Add(ComponentId, Class);
			return ComponentId++;
		};

		FClassInfo Info;

		for (TFieldIterator<UFunction> RemoteFunction(Class); RemoteFunction; ++RemoteFunction)
		{
			if (RemoteFunction->FunctionFlags & FUNC_NetClient ||
				RemoteFunction->FunctionFlags & FUNC_NetServer ||
				RemoteFunction->FunctionFlags & FUNC_NetCrossServer ||
				RemoteFunction->FunctionFlags & FUNC_NetMulticast)
			{
				ERPCType RPCType;
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

				TArray<UFunction*>& RPCArray = Info.RPCs.FindOrAdd(RPCType);

				FRPCInfo RPCInfo;
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

		if (Class->IsChildOf<AActor>())
		{
			if (AActor* ContainerCDO = Cast<AActor>(Class->GetDefaultObject()))
			{
				TInlineComponentArray<UActorComponent*> NativeComponents;
				ContainerCDO->GetComponents(NativeComponents);

				for (UActorComponent* Component : NativeComponents)
				{
					if (ShouldUseDTB(this, Component->GetClass()))
					{
						Info.ComponentClasses.Add(Component->GetClass());
					}
				}

				// Components that are added in a blueprint won't appear in the CDO.
				if (UBlueprintGeneratedClass* BGC = Cast<UBlueprintGeneratedClass>(Class))
				{
					if (USimpleConstructionScript* SCS = BGC->SimpleConstructionScript)
					{
						for (USCS_Node* Node : SCS->GetAllNodes())
						{
							if (Node->ComponentTemplate == nullptr)
							{
								continue;
							}

							if (ShouldUseDTB(this, Node->ComponentTemplate->GetClass()))
							{
								Info.ComponentClasses.Add(Node->ComponentTemplate->GetClass());
							}
						}
					}
				}
			}
		}

		ClassInfoMap.Add(Class, Info);
	}
}

UObject* USpatialInterop::GetTargetObjectFromChannelAndClass(USpatialActorChannel* Channel, UClass* Class)
{
	UObject* TargetObject = nullptr;

	if (Class->IsChildOf<AActor>())
	{
		check(Channel->Actor->IsA(Class));
		TargetObject = Channel->Actor;
	}
	else if (Class->IsChildOf<UActorComponent>())
	{
		FClassInfo* ActorInfo = FindClassInfoByClass(Channel->Actor->GetClass());
		check(ActorInfo);
		check(ActorInfo->ComponentClasses.Find(Class));
		TArray<UActorComponent*> Components = Channel->Actor->GetComponentsByClass(Class);
		checkf(Components.Num() == 1, TEXT("Multiple replicated components of the same type are currently not supported by Unreal GDK"));
		TargetObject = Components[0];
	}
	else
	{
		checkNoEntry();
	}

	check(TargetObject);
	return TargetObject;
}

bool USpatialInterop::HasComponentAuthority(Worker_EntityId EntityId, Worker_ComponentId ComponentId)
{
	TMap<Worker_ComponentId, Worker_Authority>* EntityAuthority = ComponentAuthorityMap.Find(EntityId);
	if (EntityAuthority)
	{
		Worker_Authority* ComponentAuthority = EntityAuthority->Find(ComponentId);
		if (ComponentAuthority)
		{
			return *ComponentAuthority == WORKER_AUTHORITY_AUTHORITATIVE;
		}
	}
	return false;
}

void USpatialInterop::OnCommandRequest(Worker_CommandRequestOp& Op)
{
	Schema_FieldId CommandIndex = Schema_GetCommandRequestCommandIndex(Op.request.schema_type);
	UE_LOG(LogTemp, Verbose, TEXT("Received command request (entity: %lld, component: %d, command: %d)"), Op.entity_id, Op.request.component_id, CommandIndex);

	if (Op.request.component_id == SpatialConstants::PLAYER_SPAWNER_COMPONENT_ID && CommandIndex == 1)
	{
		Schema_Object* Payload = Schema_GetCommandRequestObject(Op.request.schema_type);
		std::string Message = Schema_GetString(Payload, 1);

		FString URLString = UTF8_TO_TCHAR(Message.c_str());
		URLString.Append(TEXT("?workerId=")).Append(UTF8_TO_TCHAR(Op.caller_worker_id));

		NetDriver->AcceptNewPlayer(FURL(nullptr, *URLString, TRAVEL_Absolute), false);

		Worker_CommandResponse CommandResponse = {};
		CommandResponse.component_id = SpatialConstants::PLAYER_SPAWNER_COMPONENT_ID;
		CommandResponse.schema_type = Schema_CreateCommandResponse(SpatialConstants::PLAYER_SPAWNER_COMPONENT_ID, 1);
		Schema_Object* ResponseObject = Schema_GetCommandResponseObject(CommandResponse.schema_type);
		Schema_AddBool(ResponseObject, 1, true);

		Worker_Connection_SendCommandResponse(NetDriver->Connection, Op.request_id, &CommandResponse);

		//NetDriver->PlayerSpawner->ReceivePlayerSpawnRequest(Message, Op.caller_worker_id, Op.request_id);
		return;
	}

	Worker_CommandResponse Response = {};
	Response.component_id = Op.request.component_id;
	Response.schema_type = Schema_CreateCommandResponse(Op.request.component_id, CommandIndex);

	if (UClass** ClassPtr = ComponentToClassMap.Find(Op.request.component_id))
	{
		UClass* Class = *ClassPtr;
		FClassInfo* Info = FindClassInfoByClass(Class);
		check(Info);

		ERPCType RPCType = ARPC_Count;
		for (int i = ARPC_Client; i <= ARPC_CrossServer; i++)
		{
			if (Info->RPCComponents[i] == Op.request.component_id)
			{
				RPCType = (ERPCType)i;
				break;
			}
		}
		check(RPCType <= ARPC_CrossServer);

		const TArray<UFunction*>* RPCArray = Info->RPCs.Find(RPCType);
		check(RPCArray);
		check((int)CommandIndex - 1 < RPCArray->Num());

		UFunction* Function = (*RPCArray)[CommandIndex - 1];

		uint8* Parms = (uint8*)FMemory_Alloca(Function->ParmsSize);
		FMemory::Memzero(Parms, Function->ParmsSize);

		UObject* TargetObject = nullptr;
		ReceiveRPCCommandRequest(Op.request, Op.entity_id, Function, PackageMap, NetDriver, TargetObject, Parms);

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

	Worker_Connection_SendCommandResponse(Connection, Op.request_id, &Response);
}

void USpatialInterop::OnCommandResponse(Worker_CommandResponseOp& Op)
{
	if (Op.status_code != WORKER_STATUS_CODE_SUCCESS)
	{
		UE_LOG(LogTemp, Warning, TEXT("!!! Command request failed (entity: %lld, request id: %d, error: %s)"), Op.entity_id, Op.request_id, UTF8_TO_TCHAR(Op.message));
		return;
	}

	Schema_FieldId CommandIndex = Schema_GetCommandResponseCommandIndex(Op.response.schema_type);
	UE_LOG(LogTemp, Verbose, TEXT("!!! Received command response (entity: %lld, component: %d, command: %d)"), Op.entity_id, Op.response.component_id, CommandIndex);

	// TODO: Re-send reliable RPCs on timeout
}

void USpatialInterop::OnDynamicData(Worker_EntityId EntityId, Worker_ComponentData& Data, USpatialActorChannel* Channel, USpatialPackageMapClient* PackageMap)
{
	UClass** ClassPtr = ComponentToClassMap.Find(Data.component_id);
	checkf(ClassPtr, TEXT("Component %d isn't hand-written and not present in ComponentToClassMap."));
	UClass* Class = *ClassPtr;

	UObject* TargetObject = GetTargetObjectFromChannelAndClass(Channel, Class);
	FChannelObjectPair ChannelObjectPair(Channel, TargetObject);

	FClassInfo* Info = FindClassInfoByClass(Class);
	check(Info);

	bool bAutonomousProxy = NetDriver->GetNetMode() == NM_Client && HasComponentAuthority(EntityId, Info->RPCComponents[ARPC_Client]);

	if (Data.component_id == Info->SingleClientComponent)
	{
		FObjectReferencesMap& ObjectReferencesMap = UnresolvedRefsMap.FindOrAdd(ChannelObjectPair);
		TSet<UnrealObjectRef> UnresolvedRefs;

		ReadDynamicData(Data, TargetObject, Channel, PackageMap, NetDriver, AGROUP_SingleClient, bAutonomousProxy, ObjectReferencesMap, UnresolvedRefs);

		QueueIncomingRepUpdates(ChannelObjectPair, ObjectReferencesMap, UnresolvedRefs);
	}
	else if (Data.component_id == Info->MultiClientComponent)
	{
		FObjectReferencesMap& ObjectReferencesMap = UnresolvedRefsMap.FindOrAdd(ChannelObjectPair);
		TSet<UnrealObjectRef> UnresolvedRefs;

		ReadDynamicData(Data, TargetObject, Channel, PackageMap, NetDriver, AGROUP_MultiClient, bAutonomousProxy, ObjectReferencesMap, UnresolvedRefs);

		QueueIncomingRepUpdates(ChannelObjectPair, ObjectReferencesMap, UnresolvedRefs);
	}
	else if (Data.component_id == Info->HandoverComponent)
	{
		// TODO: Handover
	}
	else
	{
		UE_LOG(LogTemp, Verbose, TEXT("!!! Skipping because RPC components don't have actual data."));
	}
}

void USpatialInterop::OnComponentUpdate(Worker_ComponentUpdateOp& Op)
{
	UE_LOG(LogTemp, Verbose, TEXT("!!! Received component update (entity: %lld, component: %d)"), Op.entity_id, Op.update.component_id);

	// TODO: Remove this check once we can disable component update short circuiting. This will be exposed in 14.0. See TIG-137.
	if (HasComponentAuthority(Op.entity_id, Op.update.component_id))
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
	case SpatialConstants::PLAYER_SPAWNER_COMPONENT_ID:
	case UNREAL_METADATA_COMPONENT_ID:
		UE_LOG(LogTemp, Verbose, TEXT("!!! Skipping because this is hand-written Spatial component"));
		return;
	}

	UClass** ClassPtr = ComponentToClassMap.Find(Op.update.component_id);
	checkf(ClassPtr, TEXT("Component %d isn't hand-written and not present in ComponentToClassMap."));
	UClass* Class = *ClassPtr;
	FClassInfo* Info = FindClassInfoByClass(Class);
	check(Info);

	bool bAutonomousProxy = NetDriver->GetNetMode() == NM_Client && HasComponentAuthority(Op.entity_id, Info->RPCComponents[ARPC_Client]);

	USpatialActorChannel* ActorChannel = GetActorChannelByEntityId(Op.entity_id);
	bool bIsServer = NetDriver->IsServer();

	if (Op.update.component_id == Info->SingleClientComponent)
	{
		if (bIsServer)
		{
			UE_LOG(LogTemp, Verbose, TEXT("!!! Skipping SingleClient component because we're a server."));
			return;
		}
		check(ActorChannel);

		UObject* TargetObject = GetTargetObjectFromChannelAndClass(ActorChannel, Class);
		HandleComponentUpdate(Op.update, TargetObject, ActorChannel, AGROUP_SingleClient, bAutonomousProxy);
	}
	else if (Op.update.component_id == Info->MultiClientComponent)
	{
		if (bIsServer)
		{
			UE_LOG(LogTemp, Verbose, TEXT("!!! Skipping MultiClient component because we're a server."));
			return;
		}
		check(ActorChannel);

		UObject* TargetObject = GetTargetObjectFromChannelAndClass(ActorChannel, Class);
		HandleComponentUpdate(Op.update, TargetObject, ActorChannel, AGROUP_MultiClient, bAutonomousProxy);
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
		const TArray<UFunction*>& RPCArray = Info->RPCs.FindChecked(ARPC_NetMulticast);
		ReceiveMulticastUpdate(Op.update, Op.entity_id, RPCArray, PackageMap, NetDriver);
	}
	else
	{
		UE_LOG(LogTemp, Verbose, TEXT("!!! Skipping because it's an empty component update from an RPC component. (most likely as a result of gaining authority)"));
	}
}

void USpatialInterop::HandleComponentUpdate(const Worker_ComponentUpdate& ComponentUpdate, UObject* TargetObject, USpatialActorChannel* Channel, EReplicatedPropertyGroup PropertyGroup, bool bAutonomousProxy)
{
	FChannelObjectPair ChannelObjectPair(Channel, TargetObject);

	FObjectReferencesMap& ObjectReferencesMap = UnresolvedRefsMap.FindOrAdd(ChannelObjectPair);
	TSet<UnrealObjectRef> UnresolvedRefs;
	ReceiveDynamicUpdate(ComponentUpdate, TargetObject, Channel, PackageMap, NetDriver, PropertyGroup, bAutonomousProxy, ObjectReferencesMap, UnresolvedRefs);

	QueueIncomingRepUpdates(ChannelObjectPair, ObjectReferencesMap, UnresolvedRefs);
}

void USpatialInterop::QueueIncomingRepUpdates(FChannelObjectPair ChannelObjectPair, const FObjectReferencesMap& ObjectReferencesMap, const TSet<UnrealObjectRef>& UnresolvedRefs)
{
	for (const UnrealObjectRef& UnresolvedRef : UnresolvedRefs)
	{
		UE_LOG(LogTemp, Log, TEXT("!!! Added pending incoming property for object ref: %s, target object: %s"), *UnresolvedRef.ToString(), *ChannelObjectPair.Value->GetName());
		IncomingRefsMap.FindOrAdd(UnresolvedRef).Add(ChannelObjectPair);
	}

	if (ObjectReferencesMap.Num() == 0)
	{
		UnresolvedRefsMap.Remove(ChannelObjectPair);
	}
}

void USpatialInterop::QueueOutgoingRPC(const UObject* UnresolvedObject, UObject* TargetObject, UFunction* Function, void* Parameters)
{
	check(UnresolvedObject);
	UE_LOG(LogTemp, Log, TEXT("!!! Added pending outgoing RPC depending on object: %s, target: %s, function: %s"), *UnresolvedObject->GetName(), *TargetObject->GetName(), *Function->GetName());
	OutgoingRPCs.FindOrAdd(UnresolvedObject).Add(FPendingRPCParams(TargetObject, Function, Parameters));
}

void USpatialInterop::OnAuthorityChange(Worker_AuthorityChangeOp& Op)
{
	ComponentAuthorityMap.FindOrAdd(Op.entity_id).FindOrAdd(Op.component_id) = (Worker_Authority)Op.authority;
	UE_LOG(LogTemp, Log, TEXT("!!! Received authority change (entity: %lld, component: %d, authority: %d)"), Op.entity_id, Op.component_id, (int)Op.authority);
}

void USpatialInterop::SendReserveEntityIdRequest(USpatialActorChannel* Channel)
{
	Worker_RequestId RequestId = Worker_Connection_SendReserveEntityIdRequest(Connection, nullptr);
	AddPendingActorRequest(RequestId, Channel);
	UE_LOG(LogTemp, Log, TEXT("!!! Opened channel for actor with no entity ID. Initiated reserve entity ID. Request id: %d"), RequestId);
}

void USpatialInterop::AddPendingActorRequest(Worker_RequestId RequestId, USpatialActorChannel* Channel)
{
	PendingActorRequests.Emplace(RequestId, Channel);
}

USpatialActorChannel* USpatialInterop::RemovePendingActorRequest(Worker_RequestId RequestId)
{
	USpatialActorChannel** Channel = PendingActorRequests.Find(RequestId);
	if (Channel == nullptr)
	{
		return nullptr;
	}
	PendingActorRequests.Remove(RequestId);
	return *Channel;
}

void USpatialInterop::OnReserveEntityIdResponse(Worker_ReserveEntityIdResponseOp& Op)
{
	if (USpatialActorChannel* Channel = RemovePendingActorRequest(Op.request_id))
	{
		Channel->OnReserveEntityIdResponse(Op);
	}
}

void USpatialInterop::OnCreateEntityResponse(Worker_CreateEntityResponseOp& Op)
{
	if (USpatialActorChannel* Channel = RemovePendingActorRequest(Op.request_id))
	{
		Channel->OnCreateEntityResponse(Op);
	}
}

Worker_RequestId USpatialInterop::SendCreateEntityRequest(USpatialActorChannel* Channel, const FVector& Location, const FString& PlayerWorkerId, const TArray<uint16>& RepChanged, const TArray<uint16>& HandoverChanged)
{
	if (!Connection) return 0;

	AActor* Actor = Channel->Actor;

	FSoftClassPath ActorClassPath(Actor->GetClass());
	FString PathStr = ActorClassPath.ToString();

	Worker_RequestId CreateEntityRequestId = CreateActorEntity(PlayerWorkerId, Location, PathStr, Channel->GetChangeState(RepChanged, HandoverChanged), Channel);

	UE_LOG(LogTemp, Log, TEXT("!!! Creating entity for actor %s (%lld) using initial changelist. Request ID: %d"),
		*Actor->GetName(), Channel->GetEntityId(), CreateEntityRequestId);

	return CreateEntityRequestId;
}

Worker_RequestId USpatialInterop::CreateActorEntity(const FString& ClientWorkerId, const FVector& Position, const FString& Metadata, const FPropertyChangeState& InitialChanges, USpatialActorChannel* Channel)
{
	std::string ClientWorkerIdString = TCHAR_TO_UTF8(*ClientWorkerId);

	WorkerAttributeSet WorkerAttribute = {"UnrealWorker"};
	WorkerAttributeSet ClientAttribute = {"UnrealClient"};
	WorkerAttributeSet OwningClientAttribute = {"workerId:" + ClientWorkerIdString};

	WorkerRequirementSet WorkersOnly = {WorkerAttribute};
	WorkerRequirementSet ClientsOnly = {ClientAttribute};
	WorkerRequirementSet OwningClientOnly = {OwningClientAttribute};

	AActor* Actor = Channel->Actor;

	WorkerRequirementSet AnyUnrealWorkerOrClient = {WorkerAttribute, ClientAttribute};
	WorkerRequirementSet AnyUnrealWorkerOrOwningClient = {WorkerAttribute, OwningClientAttribute};
	WorkerRequirementSet ReadAcl = Actor->IsA<APlayerController>() ? AnyUnrealWorkerOrOwningClient : AnyUnrealWorkerOrClient;

	FClassInfo* Info = FindClassInfoByClass(Actor->GetClass());
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

	for (TSubclassOf<UActorComponent> ComponentClass : Info->ComponentClasses)
	{
		FClassInfo* ComponentInfo = FindClassInfoByClass(ComponentClass);
		check(ComponentInfo);

		ComponentWriteAcl.emplace(ComponentInfo->SingleClientComponent, WorkersOnly);
		ComponentWriteAcl.emplace(ComponentInfo->MultiClientComponent, WorkersOnly);
		// Not adding handover since component's handover properties will be handled by the actor anyway
		ComponentWriteAcl.emplace(ComponentInfo->RPCComponents[ARPC_Client], OwningClientOnly);
		ComponentWriteAcl.emplace(ComponentInfo->RPCComponents[ARPC_Server], WorkersOnly);
		ComponentWriteAcl.emplace(ComponentInfo->RPCComponents[ARPC_CrossServer], WorkersOnly);
		ComponentWriteAcl.emplace(ComponentInfo->RPCComponents[ARPC_NetMulticast], WorkersOnly);
	}

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
	ComponentDatas.push_back(CreatePositionData(Position(LocationToCAPIPosition(Position))));
	ComponentDatas.push_back(CreateMetadataData(Metadata(TCHAR_TO_UTF8(*Metadata))));
	ComponentDatas.push_back(CreateEntityAclData(EntityAcl(ReadAcl, ComponentWriteAcl)));
	ComponentDatas.push_back(CreatePersistenceData(Persistence()));
	ComponentDatas.push_back(CreateUnrealMetadataData(UnrealMetadata(StaticPath, ClientWorkerIdString, SubobjectNameToOffset)));

	FUnresolvedObjectsMap UnresolvedObjectsMap;

	ComponentDatas.push_back(CreateDynamicData(Info->SingleClientComponent, InitialChanges, PackageMap, NetDriver, AGROUP_SingleClient, UnresolvedObjectsMap));
	ComponentDatas.push_back(CreateDynamicData(Info->MultiClientComponent, InitialChanges, PackageMap, NetDriver, AGROUP_MultiClient, UnresolvedObjectsMap));

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

	for (TSubclassOf<UActorComponent> ComponentClass : Info->ComponentClasses)
	{
		FClassInfo* ComponentInfo = FindClassInfoByClass(ComponentClass);
		check(ComponentInfo);

		TArray<UActorComponent*> Components = Actor->GetComponentsByClass(ComponentClass);
		checkf(Components.Num() == 1, TEXT("Multiple replicated components of the same type are currently not supported by Unreal GDK"));
		UActorComponent* Component = Components[0];

		FPropertyChangeState ComponentChanges = Channel->CreateSubobjectChangeState(Component);
		FUnresolvedObjectsMap ComponentUnresolvedObjectsMap;

		ComponentDatas.push_back(CreateDynamicData(ComponentInfo->SingleClientComponent, ComponentChanges, PackageMap, NetDriver, AGROUP_SingleClient, ComponentUnresolvedObjectsMap));
		ComponentDatas.push_back(CreateDynamicData(ComponentInfo->MultiClientComponent, ComponentChanges, PackageMap, NetDriver, AGROUP_MultiClient, ComponentUnresolvedObjectsMap));

		for (auto& HandleUnresolvedObjectsPair : ComponentUnresolvedObjectsMap)
		{
			QueueOutgoingRepUpdate(Channel, Component, HandleUnresolvedObjectsPair.Key, HandleUnresolvedObjectsPair.Value);
		}

		// Not adding handover since component's handover properties will be handled by the actor anyway

		for (int RPCType = 0; RPCType < ARPC_Count; RPCType++)
		{
			Worker_ComponentData RPCData = {};
			RPCData.component_id = ComponentInfo->RPCComponents[RPCType];
			RPCData.schema_type = Schema_CreateComponentData(ComponentInfo->RPCComponents[RPCType]);
			ComponentDatas.push_back(RPCData);
		}
	}

	Worker_EntityId EntityId = Channel->GetEntityId();
	Worker_RequestId CreateEntityRequestId = Worker_Connection_SendCreateEntityRequest(Connection, ComponentDatas.size(), ComponentDatas.data(), &EntityId, nullptr);
	AddPendingActorRequest(CreateEntityRequestId, Channel);

	return CreateEntityRequestId;
}

void USpatialInterop::DeleteEntityIfAuthoritative(Worker_EntityId EntityId)
{
	if (!Connection) return;

	bool bHasAuthority = /*Interop->IsAuthoritativeDestructionAllowed() && */ HasComponentAuthority(EntityId, POSITION_COMPONENT_ID);

	UE_LOG(LogTemp, Log, TEXT("!!! Delete entity request on %lld. Has authority: %d"), EntityId, (int)bHasAuthority);

	// If we have authority and aren't trying to delete a critical entity, delete it
	if (bHasAuthority && !IsCriticalEntity(EntityId))
	{
		Worker_Connection_SendDeleteEntityRequest(Connection, EntityId, nullptr);
		EntityPipeline.CleanupDeletedEntity(EntityId);
	}
}

bool USpatialInterop::IsCriticalEntity(Worker_EntityId EntityId)
{
	// Don't delete if the actor is the spawner
	if (EntityId == SpatialConstants::EntityIds::SPAWNER_ENTITY_ID)
	{
		return true;
	}

	// TODO: Singletons

	return false;
}

void USpatialInterop::SendSpatialPositionUpdate(Worker_EntityId EntityId, const FVector& Location)
{
	Worker_ComponentUpdate ComponentUpdate = CreatePositionUpdate(LocationToCAPIPosition(Location));

	Worker_Connection_SendComponentUpdate(Connection, EntityId, &ComponentUpdate);
}

void USpatialInterop::SendComponentUpdates(UObject* Object, const FPropertyChangeState& Changes, USpatialActorChannel* Channel)
{
	Worker_EntityId EntityId = Channel->GetEntityId();
	UE_LOG(LogTemp, Verbose, TEXT("!!! Sending component update (object: %s, entity: %lld)"), *Object->GetName(), EntityId);

	FClassInfo* Info = FindClassInfoByClass(Object->GetClass());
	check(Info);

	FUnresolvedObjectsMap UnresolvedObjectsMap;

	bool bWroteSingleClient = false;
	Worker_ComponentUpdate SingleClientRepDataUpdate = CreateDynamicUpdate(Info->SingleClientComponent, Changes, PackageMap, NetDriver, AGROUP_SingleClient, UnresolvedObjectsMap, bWroteSingleClient);

	bool bWroteMultiClient = false;
	Worker_ComponentUpdate MultiClientRepDataUpdate = CreateDynamicUpdate(Info->MultiClientComponent, Changes, PackageMap, NetDriver, AGROUP_MultiClient, UnresolvedObjectsMap, bWroteMultiClient);

	for (uint16 Handle : Changes.RepChanged)
	{
		ResetOutgoingRepUpdate(Channel, Object, Handle);

		if (TSet<const UObject*>* UnresolvedObjects = UnresolvedObjectsMap.Find(Handle))
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

void USpatialInterop::SendRPC(UObject* TargetObject, UFunction* Function, void* Parameters, bool bOwnParameters)
{
	FClassInfo* Info = FindClassInfoByClass(TargetObject->GetClass());
	check(Info);

	FRPCInfo* RPCInfo = Info->RPCInfoMap.Find(Function);
	check(RPCInfo);

	Worker_EntityId EntityId = 0;
	const UObject* UnresolvedObject = nullptr;

	switch (RPCInfo->Type)
	{
	case ARPC_Client:
	case ARPC_Server:
	case ARPC_CrossServer:
	{
		Worker_CommandRequest CommandRequest = CreateRPCCommandRequest(TargetObject, Function, Parameters, PackageMap, NetDriver, Info->RPCComponents[RPCInfo->Type], RPCInfo->Index + 1, EntityId, UnresolvedObject);

		if (!UnresolvedObject)
		{
			check(EntityId > 0);
			Worker_CommandParameters CommandParams = {};
			Worker_Connection_SendCommandRequest(Connection, EntityId, &CommandRequest, RPCInfo->Index + 1, nullptr, &CommandParams);
		}
		break;
	}
	case ARPC_NetMulticast:
	{
		Worker_ComponentUpdate ComponentUpdate = CreateMulticastUpdate(TargetObject, Function, Parameters, PackageMap, NetDriver, Info->RPCComponents[RPCInfo->Type], RPCInfo->Index + 1, EntityId, UnresolvedObject);

		if (!UnresolvedObject)
		{
			check(EntityId > 0);
			Worker_Connection_SendComponentUpdate(Connection, EntityId, &ComponentUpdate);
		}
		break;
	}
	default:
		checkNoEntry();
		break;
	}

	if (UnresolvedObject)
	{
		void* NewParameters = Parameters;
		if (!bOwnParameters)
		{
			// Copy parameters
			NewParameters = new uint8[Function->ParmsSize];
			FMemory::Memzero(NewParameters, Function->ParmsSize); // Not sure if needed considering we're copying everything from Parameters

			for (TFieldIterator<UProperty> It(Function); It && It->HasAnyPropertyFlags(CPF_Parm); ++It)
			{
				It->CopyCompleteValue_InContainer(NewParameters, Parameters);
			}
		}

		QueueOutgoingRPC(UnresolvedObject, TargetObject, Function, NewParameters);
	}
	else if (bOwnParameters)
	{
		// Destroy parameters
		for (TFieldIterator<UProperty> It(Function); It && It->HasAnyPropertyFlags(CPF_Parm); ++It)
		{
			It->DestroyValue_InContainer(Parameters);
		}
		delete[] Parameters;
	}
}

void USpatialInterop::ProcessOps(Worker_OpList* OpList)
{
	for (size_t i = 0; i < OpList->op_count; ++i)
	{
		Worker_Op* Op = &OpList->ops[i];
		switch (Op->op_type)
		{
		// Critical Section
		case WORKER_OP_TYPE_CRITICAL_SECTION:
			if (Op->critical_section.in_critical_section)
			{
				EntityPipeline.EnterCriticalSection();
			}
			else
			{
				EntityPipeline.LeaveCriticalSection();
				for (TPair<UObject*, UnrealObjectRef>& It : ResolvedObjectQueue)
				{
					ResolvePendingOperations_Internal(It.Key, It.Value);
				}
				ResolvedObjectQueue.Empty();
			}
			break;

		// Entity Lifetime
		case WORKER_OP_TYPE_ADD_ENTITY:
			UE_LOG(LogTemp, Warning, TEXT("Mine: %d"), Op->add_entity.entity_id)
			EntityPipeline.AddEntity(Op->add_entity);
			break;
		case WORKER_OP_TYPE_REMOVE_ENTITY:
			EntityPipeline.RemoveEntity(Op->remove_entity);
			break;

		// World Command Responses
		case WORKER_OP_TYPE_RESERVE_ENTITY_ID_RESPONSE:
			OnReserveEntityIdResponse(Op->reserve_entity_id_response);
			break;
		case WORKER_OP_TYPE_RESERVE_ENTITY_IDS_RESPONSE:
			break;
		case WORKER_OP_TYPE_CREATE_ENTITY_RESPONSE:
			OnCreateEntityResponse(Op->create_entity_response);
			break;
		case WORKER_OP_TYPE_DELETE_ENTITY_RESPONSE:
			UE_LOG(LogTemp, Log, TEXT("SpatialOS Delete Entity Response: %s"), UTF8_TO_TCHAR(Op->delete_entity_response.message));
			break;
		case WORKER_OP_TYPE_ENTITY_QUERY_RESPONSE:
			break;

		// Components
		case WORKER_OP_TYPE_ADD_COMPONENT:
			EntityPipeline.AddComponent(Op->add_component);
			break;
		case WORKER_OP_TYPE_REMOVE_COMPONENT:
			EntityPipeline.RemoveComponent(Op->remove_component);
			break;
		case WORKER_OP_TYPE_COMPONENT_UPDATE:
			OnComponentUpdate(Op->component_update);
			break;

		// Commands
		case WORKER_OP_TYPE_COMMAND_REQUEST:
			OnCommandRequest(Op->command_request);
			break;
		case WORKER_OP_TYPE_COMMAND_RESPONSE:
			OnCommandResponse(Op->command_response);
			break;

		// Authority Change
		case WORKER_OP_TYPE_AUTHORITY_CHANGE:
			OnAuthorityChange(Op->authority_change);
			break;

		case WORKER_OP_TYPE_FLAG_UPDATE:
			break;
		case WORKER_OP_TYPE_LOG_MESSAGE:
			UE_LOG(LogTemp, Log, TEXT("SpatialOS: %s"), UTF8_TO_TCHAR(Op->log_message.message));
			break;
		case WORKER_OP_TYPE_METRICS:
			break;
		case WORKER_OP_TYPE_DISCONNECT:
			UE_LOG(LogTemp, Warning, TEXT("Disconnecting from SpatialOS"), UTF8_TO_TCHAR(Op->disconnect.reason));
			break;

		default:
			break;
		}
	}
}

void USpatialInterop::ResetOutgoingRepUpdate(USpatialActorChannel* DependentChannel, UObject* ReplicatedObject, int16 Handle)
{
	check(DependentChannel);
	check(ReplicatedObject);
	FChannelObjectPair ChannelObjectPair(DependentChannel, ReplicatedObject);

	FHandleToUnresolved* HandleToUnresolved = PropertyToUnresolved.Find(ChannelObjectPair);
	if (!HandleToUnresolved) return;

	FUnresolvedEntry* UnresolvedPtr = HandleToUnresolved->Find(Handle);
	if (!UnresolvedPtr) return;

	FUnresolvedEntry& Unresolved = *UnresolvedPtr;

	check(Unresolved.IsValid());

	UE_LOG(LogTemp, Log, TEXT("!!! Resetting pending outgoing array depending on channel: %s, object: %s, handle: %d."),
		*DependentChannel->GetName(), *ReplicatedObject->GetName(), Handle);

	for (const UObject* UnresolvedObject : *Unresolved)
	{
		FChannelToHandleToUnresolved& ChannelToUnresolved = ObjectToUnresolved.FindChecked(UnresolvedObject);
		FHandleToUnresolved& OtherHandleToUnresolved = ChannelToUnresolved.FindChecked(ChannelObjectPair);

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

void USpatialInterop::QueueOutgoingRepUpdate(USpatialActorChannel* DependentChannel, UObject* ReplicatedObject, int16 Handle, const TSet<const UObject*>& UnresolvedObjects)
{
	check(DependentChannel);
	check(ReplicatedObject);
	FChannelObjectPair ChannelObjectPair(DependentChannel, ReplicatedObject);

	UE_LOG(LogTemp, Log, TEXT("!!! Added pending outgoing property: channel: %s, object: %s, handle: %d. Depending on objects:"),
		*DependentChannel->GetName(), *ReplicatedObject->GetName(), Handle);

	FUnresolvedEntry Unresolved = MakeShared<TSet<const UObject*>>();
	*Unresolved = UnresolvedObjects;

	FHandleToUnresolved& HandleToUnresolved = PropertyToUnresolved.FindOrAdd(ChannelObjectPair);
	check(!HandleToUnresolved.Find(Handle));
	HandleToUnresolved.Add(Handle, Unresolved);

	for (const UObject* UnresolvedObject : UnresolvedObjects)
	{
		FHandleToUnresolved& AnotherHandleToUnresolved = ObjectToUnresolved.FindOrAdd(UnresolvedObject).FindOrAdd(ChannelObjectPair);
		check(!AnotherHandleToUnresolved.Find(Handle));
		AnotherHandleToUnresolved.Add(Handle, Unresolved);

		// Following up on the previous log: listing the unresolved objects
		UE_LOG(LogTemp, Log, TEXT("!!! %s"), *UnresolvedObject->GetName());
	}
}

void USpatialInterop::ResolvePendingOperations(UObject* Object, const UnrealObjectRef& ObjectRef)
{
	if (EntityPipeline.bInCriticalSection)
	{
		ResolvedObjectQueue.Add(TPair<UObject*, UnrealObjectRef>{ Object, ObjectRef });
	}
	else
	{
		ResolvePendingOperations_Internal(Object, ObjectRef);
	}
}

void USpatialInterop::ResolvePendingOperations_Internal(UObject* Object, const UnrealObjectRef& ObjectRef)
{
	UE_LOG(LogTemp, Log, TEXT("!!! Resolving pending object refs and RPCs which depend on object: %s %s."), *Object->GetName(), *ObjectRef.ToString());
	ResolveOutgoingOperations(Object);
	ResolveIncomingOperations(Object, ObjectRef);
	ResolveOutgoingRPCs(Object);
}

void USpatialInterop::ResolveOutgoingOperations(UObject* Object)
{
	FChannelToHandleToUnresolved* ChannelToUnresolved = ObjectToUnresolved.Find(Object);
	if (!ChannelToUnresolved) return;

	for (auto& ChannelProperties : *ChannelToUnresolved)
	{
		FChannelObjectPair& ChannelObjectPair = ChannelProperties.Key;
		USpatialActorChannel* DependentChannel = ChannelObjectPair.Key;
		UObject* ReplicatingObject = ChannelObjectPair.Value;
		FHandleToUnresolved& HandleToUnresolved = ChannelProperties.Value;

		TArray<uint16> PropertyHandles;

		for (auto& HandleUnresolvedPair : HandleToUnresolved)
		{
			uint16 Handle = HandleUnresolvedPair.Key;
			FUnresolvedEntry& Unresolved = HandleUnresolvedPair.Value;

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

				FHandleToUnresolved& AnotherHandleToUnresolved = PropertyToUnresolved.FindChecked(ChannelObjectPair);
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

			//SendSpatialUpdateForObject(DependentChannel, ReplicatingObject, PropertyHandles, TArray<uint16>());
		}
	}

	ObjectToUnresolved.Remove(Object);
}

void USpatialInterop::ResolveIncomingOperations(UObject* Object, const UnrealObjectRef& ObjectRef)
{
	// TODO: queue up resolved objects since they were resolved during process ops
	// and then resolve all of them at the end of process ops

	TSet<FChannelObjectPair>* TargetObjectSet = IncomingRefsMap.Find(ObjectRef);
	if (!TargetObjectSet) return;

	UE_LOG(LogTemp, Log, TEXT("!!! Resolving incoming operations depending on object ref %s, resolved object: %s"), *ObjectRef.ToString(), *Object->GetName());

	for (FChannelObjectPair& ChannelObjectPair : *TargetObjectSet)
	{
		FObjectReferencesMap* UnresolvedRefs = UnresolvedRefsMap.Find(ChannelObjectPair);

		if (!UnresolvedRefs)
		{
			continue;
		}

		USpatialActorChannel* DependentChannel = ChannelObjectPair.Key;
		UObject* ReplicatingObject = ChannelObjectPair.Value;

		bool bStillHasUnresolved = false;
		bool bSomeObjectsWereMapped = false;
		TArray<UProperty*> RepNotifies;

		FRepLayout& RepLayout = DependentChannel->GetObjectRepLayout(ReplicatingObject);
		FRepStateStaticBuffer& ShadowData = DependentChannel->GetObjectStaticBuffer(ReplicatingObject);

		ResolveObjectReferences(RepLayout, ReplicatingObject, *UnresolvedRefs, PackageMap, NetDriver, ShadowData.GetData(), (uint8*)ReplicatingObject, ShadowData.Num(), RepNotifies, bSomeObjectsWereMapped, bStillHasUnresolved);

		if (bSomeObjectsWereMapped)
		{
			UE_LOG(LogTemp, Log, TEXT("!!! Resolved for target object %s"), *ReplicatingObject->GetName());
			DependentChannel->PostReceiveSpatialUpdate(ReplicatingObject, RepNotifies);
		}

		if (!bStillHasUnresolved)
		{
			UnresolvedRefsMap.Remove(ChannelObjectPair);
		}
	}

	IncomingRefsMap.Remove(ObjectRef);
}

void USpatialInterop::ResolveOutgoingRPCs(UObject* Object)
{
	TArray<FPendingRPCParams>* RPCList = OutgoingRPCs.Find(Object);
	if (RPCList)
	{
		for (FPendingRPCParams& RPCParams : *RPCList)
		{
			// We can guarantee that SendRPC won't populate OutgoingRPCs[Object] whilst we're iterating through it,
			// because Object has been resolved when we call ResolveOutgoingRPCs.
			UE_LOG(LogTemp, Log, TEXT("!!! Resolving outgoing RPC depending on object: %s, target: %s, function: %s"), *Object->GetName(), *RPCParams.TargetObject->GetName(), *RPCParams.Function->GetName());
			SendRPC(RPCParams.TargetObject, RPCParams.Function, RPCParams.Parameters, true);
		}
		OutgoingRPCs.Remove(Object);
	}
}

void USpatialInterop::AddActorChannel(const Worker_EntityId& EntityId, USpatialActorChannel* Channel)
{
	EntityToActorChannel.Add(EntityId, Channel);
}

USpatialActorChannel* USpatialInterop::GetActorChannelByEntityId(const Worker_EntityId& EntityId) const
{
	USpatialActorChannel* const* ActorChannel = EntityToActorChannel.Find(EntityId);

	if (ActorChannel == nullptr)
	{
		// Can't find actor channel for this entity, give up.
		return nullptr;
	}

	return *ActorChannel;
}
