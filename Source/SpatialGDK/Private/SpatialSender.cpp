
#include "SpatialSender.h"
#include "SpatialActorChannel.h"

#include "CoreTypes/StandardLibrary.h"
#include "CoreTypes/UnrealMetadata.h"
#include "Utils/RepLayoutUtils.h"

#include <vector>
#include "SpatialMemoryWriter.h"

void USpatialSender::Init()
{

}

Worker_RequestId USpatialSender::CreateEntity(const FString& ClientWorkerId, const FVector& Location, const FString& EntityType, const FPropertyChangeState& InitialChanges, USpatialActorChannel* Channel)
{
	AActor* Actor = Channel->Actor;
	std::string ClientWorkerIdString = TCHAR_TO_UTF8(*ClientWorkerId);

	WorkerAttributeSet WorkerAttribute = { "UnrealWorker" };
	WorkerAttributeSet ClientAttribute = { "UnrealClient" };
	WorkerAttributeSet OwningClientAttribute = { "workerId:" + ClientWorkerIdString };

	WorkerRequirementSet WorkersOnly = { WorkerAttribute };
	WorkerRequirementSet ClientsOnly = { ClientAttribute };
	WorkerRequirementSet OwningClientOnly = { OwningClientAttribute };

	WorkerRequirementSet AnyUnrealWorkerOrClient = { WorkerAttribute, ClientAttribute };
	WorkerRequirementSet AnyUnrealWorkerOrOwningClient = { WorkerAttribute, OwningClientAttribute };
	WorkerRequirementSet ReadAcl = Actor->IsA<APlayerController>() ? AnyUnrealWorkerOrOwningClient : AnyUnrealWorkerOrClient;

	FClassInfo* Info = TypebindingManager->FindClassInfoByClass(Actor->GetClass());
	check(Info);

	std::map<Worker_ComponentId, WorkerRequirementSet> ComponentWriteAcl;
	ComponentWriteAcl.emplace(POSITION_COMPONENT_ID, WorkersOnly);
	ComponentWriteAcl.emplace(Info->SingleClientComponent, WorkersOnly);
	ComponentWriteAcl.emplace(Info->MultiClientComponent, WorkersOnly);
	ComponentWriteAcl.emplace(Info->HandoverComponent, WorkersOnly);
	ComponentWriteAcl.emplace(Info->RPCComponents[RPC_Client], OwningClientOnly);
	ComponentWriteAcl.emplace(Info->RPCComponents[RPC_Server], WorkersOnly);
	ComponentWriteAcl.emplace(Info->RPCComponents[RPC_CrossServer], WorkersOnly);
	ComponentWriteAcl.emplace(Info->RPCComponents[RPC_NetMulticast], WorkersOnly);

	for (TSubclassOf<UActorComponent> ComponentClass : Info->ComponentClasses)
	{
		FClassInfo* ComponentInfo = TypebindingManager->FindClassInfoByClass(ComponentClass);
		check(ComponentInfo);

		ComponentWriteAcl.emplace(ComponentInfo->SingleClientComponent, WorkersOnly);
		ComponentWriteAcl.emplace(ComponentInfo->MultiClientComponent, WorkersOnly);
		// Not adding handover since component's handover properties will be handled by the actor anyway
		ComponentWriteAcl.emplace(ComponentInfo->RPCComponents[RPC_Client], OwningClientOnly);
		ComponentWriteAcl.emplace(ComponentInfo->RPCComponents[RPC_Server], WorkersOnly);
		ComponentWriteAcl.emplace(ComponentInfo->RPCComponents[RPC_CrossServer], WorkersOnly);
		ComponentWriteAcl.emplace(ComponentInfo->RPCComponents[RPC_NetMulticast], WorkersOnly);
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
	ComponentDatas.push_back(Position(Coordinates::FromFVector(Location)).CreatePositionData());
	ComponentDatas.push_back(Metadata(TCHAR_TO_UTF8(*EntityType)).CreateMetadataData());
	ComponentDatas.push_back(EntityAcl(ReadAcl, ComponentWriteAcl).CreateEntityAclData());
	ComponentDatas.push_back(Persistence().CreatePersistenceData());
	ComponentDatas.push_back(UnrealMetadata(StaticPath, ClientWorkerIdString, SubobjectNameToOffset).CreateUnrealMetadataData());

	std::vector<Worker_ComponentData> DynamicComponentDatas = DataFactory->CreateComponentDatas(Actor, InitialChanges);
	ComponentDatas.insert(ComponentDatas.end(), DynamicComponentDatas.begin(), DynamicComponentDatas.end());

	FUnresolvedObjectsMap UnresolvedObjectsMap = DataFactory->ConsumePendingUnresolvedObjectsMap();

	for (auto& HandleUnresolvedObjectsPair : UnresolvedObjectsMap)
	{
		QueueOutgoingRepUpdate(Channel, Actor, HandleUnresolvedObjectsPair.Key, HandleUnresolvedObjectsPair.Value);
	}

	// TODO: Handover
	Worker_ComponentData HandoverData = {};
	HandoverData.component_id = Info->HandoverComponent;
	HandoverData.schema_type = Schema_CreateComponentData(Info->HandoverComponent);
	ComponentDatas.push_back(HandoverData);

	for (int RPCType = 0; RPCType < RPC_Count; RPCType++)
	{
		Worker_ComponentData RPCData = {};
		RPCData.component_id = Info->RPCComponents[RPCType];
		RPCData.schema_type = Schema_CreateComponentData(Info->RPCComponents[RPCType]);
		ComponentDatas.push_back(RPCData);
	}

	for (TSubclassOf<UActorComponent> ComponentClass : Info->ComponentClasses)
	{
		FClassInfo* ComponentInfo = TypebindingManager->FindClassInfoByClass(ComponentClass);
		check(ComponentInfo);

		TArray<UActorComponent*> Components = Actor->GetComponentsByClass(ComponentClass);
		checkf(Components.Num() == 1, TEXT("Multiple replicated components of the same type are currently not supported by Unreal GDK"));
		UActorComponent* Component = Components[0];

		FPropertyChangeState ComponentChanges = Channel->CreateSubobjectChangeState(Component);

		std::vector<Worker_ComponentData> ActorComponentDatas = DataFactory->CreateComponentDatas(Component, ComponentChanges);
		ComponentDatas.insert(ComponentDatas.end(), DynamicComponentDatas.begin(), DynamicComponentDatas.end());

		FUnresolvedObjectsMap ComponentUnresolvedObjectsMap = DataFactory->ConsumePendingUnresolvedObjectsMap();

		for (auto& HandleUnresolvedObjectsPair : ComponentUnresolvedObjectsMap)
		{
			QueueOutgoingRepUpdate(Channel, Component, HandleUnresolvedObjectsPair.Key, HandleUnresolvedObjectsPair.Value);
		}

		// Not adding handover since component's handover properties will be handled by the actor anyway

		for (int RPCType = 0; RPCType < RPC_Count; RPCType++)
		{
			Worker_ComponentData RPCData = {};
			RPCData.component_id = ComponentInfo->RPCComponents[RPCType];
			RPCData.schema_type = Schema_CreateComponentData(ComponentInfo->RPCComponents[RPCType]);
			ComponentDatas.push_back(RPCData);
		}
	}

	Worker_EntityId EntityId = Channel->GetEntityId();
	Worker_RequestId CreateEntityRequestId = Worker_Connection_SendCreateEntityRequest(Connection, ComponentDatas.size(), ComponentDatas.data(), &EntityId, nullptr);
	PendingActorRequests.Add(CreateEntityRequestId, Channel);

	return CreateEntityRequestId;
}

void USpatialSender::SendComponentUpdate(UObject* Object, USpatialActorChannel* Channel, const FPropertyChangeState& Changes)
{
	Worker_EntityId EntityId = Channel->GetEntityId();

	UE_LOG(LogTemp, Verbose, TEXT("Sending component update (object: %s, entity: %lld)"), *Object->GetName(), EntityId);

	FClassInfo* Info = TypebindingManager->FindClassInfoByClass(Object->GetClass());
	check(Info);

	std::vector<Worker_ComponentUpdate> ComponentUpdates = UpdateFactory->CreateComponentUpdates(Object, Changes);
	FUnresolvedObjectsMap UnresolvedObjectsMap = UpdateFactory->ConsumePendingUnresolvedObjectsMap();

	for (uint16 Handle : Changes.RepChanged)
	{
		ResetOutgoingRepUpdate(Channel, Object, Handle);

		if (TSet<const UObject*>* UnresolvedObjects = UnresolvedObjectsMap.Find(Handle))
		{
			QueueOutgoingRepUpdate(Channel, Object, Handle, *UnresolvedObjects);
		}
	}

	for(Worker_ComponentUpdate& Update : ComponentUpdates)
	{
		Worker_Connection_SendComponentUpdate(Connection, EntityId, &Update);
	}
}

void USpatialSender::SendPositionUpdate(Worker_EntityId EntityId, const FVector& Location)
{
	Coordinates Coords = Coordinates::FromFVector(Location);
	Worker_Connection_SendComponentUpdate(Connection, EntityId, &Position::CreatePositionUpdate(Coords));
}

void USpatialSender::SendRPC(UObject* TargetObject, UFunction* Function, void* Parameters, bool bOwnParameters)
{
	FClassInfo* Info = TypebindingManager->FindClassInfoByClass(TargetObject->GetClass());
	check(Info);

	FRPCInfo* RPCInfo = Info->RPCInfoMap.Find(Function);
	check(RPCInfo);

	Worker_EntityId EntityId = 0;
	const UObject* UnresolvedObject = nullptr;

	switch (RPCInfo->Type)
	{
	case RPC_Client:
	case RPC_Server:
	case RPC_CrossServer:
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
	case RPC_NetMulticast:
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

void USpatialSender::ResetOutgoingRepUpdate(USpatialActorChannel* DependentChannel, UObject* ReplicatedObject, int16 Handle)
{
	check(DependentChannel);
	check(ReplicatedObject);
	FChannelObjectPair ChannelObjectPair(DependentChannel, ReplicatedObject);

	FHandleToUnresolved* HandleToUnresolved = PropertyToUnresolved.Find(ChannelObjectPair);
	if (HandleToUnresolved == nullptr)
	{
		return;
	}

	FUnresolvedEntry* UnresolvedPtr = HandleToUnresolved->Find(Handle);
	if (UnresolvedPtr == nullptr)
	{
		return;
	}

	FUnresolvedEntry& Unresolved = *UnresolvedPtr;

	check(Unresolved.IsValid());

	UE_LOG(LogTemp, Log, TEXT("Resetting pending outgoing array depending on channel: %s, object: %s, handle: %d."),
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

void USpatialSender::QueueOutgoingRepUpdate(USpatialActorChannel* DependentChannel, UObject* ReplicatedObject, int16 Handle, const TSet<const UObject*>& UnresolvedObjects)
{
	check(DependentChannel);
	check(ReplicatedObject);
	FChannelObjectPair ChannelObjectPair(DependentChannel, ReplicatedObject);

	UE_LOG(LogTemp, Log, TEXT("Added pending outgoing property: channel: %s, object: %s, handle: %d. Depending on objects:"),
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
		UE_LOG(LogTemp, Log, TEXT("%s"), *UnresolvedObject->GetName());
	}
}

void USpatialSender::QueueOutgoingRPC(const UObject* UnresolvedObject, UObject* TargetObject, UFunction* Function, void* Parameters)
{
	check(UnresolvedObject);
	UE_LOG(LogTemp, Log, TEXT("Added pending outgoing RPC depending on object: %s, target: %s, function: %s"), *UnresolvedObject->GetName(), *TargetObject->GetName(), *Function->GetName());
	OutgoingRPCs.FindOrAdd(UnresolvedObject).Add(FPendingRPCParams(TargetObject, Function, Parameters));
}

Worker_CommandRequest USpatialSender::CreateRPCCommandRequest(UObject* TargetObject, UFunction* Function, void* Parameters, Worker_ComponentId ComponentId, Schema_FieldId CommandIndex, Worker_EntityId& OutEntityId, const UObject*& OutUnresolvedObject)
{
	Worker_CommandRequest CommandRequest = {};
	CommandRequest.component_id = ComponentId;
	CommandRequest.schema_type = Schema_CreateCommandRequest(ComponentId, CommandIndex);
	Schema_Object* RequestObject = Schema_GetCommandRequestObject(CommandRequest.schema_type);

	UnrealObjectRef TargetObjectRef(PackageMap->GetUnrealObjectRefFromNetGUID(PackageMap->GetNetGUIDFromObject(TargetObject)));
	if (TargetObjectRef == UNRESOLVED_OBJECT_REF)
	{
		OutUnresolvedObject = TargetObject;
		Schema_DestroyCommandRequest(CommandRequest.schema_type);
		return CommandRequest;
	}

	Schema_AddUint32(RequestObject, 1, TargetObjectRef.Offset);
	OutEntityId = TargetObjectRef.Entity;

	TSet<const UObject*> UnresolvedObjects;
	FSpatialNetBitWriter PayloadWriter(PackageMap, UnresolvedObjects);

	TSharedPtr<FRepLayout> RepLayout = NetDriver->GetFunctionRepLayout(Function);
	RepLayout_SendPropertiesForRPC(*RepLayout, PayloadWriter, Parameters);

	for (const UObject* Object : UnresolvedObjects)
	{
		// Take the first unresolved object
		OutUnresolvedObject = Object;
		Schema_DestroyCommandRequest(CommandRequest.schema_type);
		return CommandRequest;
	}

	Schema_AddString(RequestObject, 2, std::string(reinterpret_cast<char*>(PayloadWriter.GetData()), PayloadWriter.GetNumBytes()));

	return CommandRequest;
}

Worker_ComponentUpdate USpatialSender::CreateMulticastUpdate(UObject* TargetObject, UFunction* Function, void* Parameters, Worker_ComponentId ComponentId, Schema_FieldId EventIndex, Worker_EntityId& OutEntityId, const UObject*& OutUnresolvedObject)
{
	Worker_ComponentUpdate ComponentUpdate = {};

	ComponentUpdate.component_id = ComponentId;
	ComponentUpdate.schema_type = Schema_CreateComponentUpdate(ComponentId);
	Schema_Object* EventsObject = Schema_GetComponentUpdateEvents(ComponentUpdate.schema_type);
	Schema_Object* EventData = Schema_AddObject(EventsObject, EventIndex);

	UnrealObjectRef TargetObjectRef(PackageMap->GetUnrealObjectRefFromNetGUID(PackageMap->GetNetGUIDFromObject(TargetObject)));
	if (TargetObjectRef == UNRESOLVED_OBJECT_REF)
	{
		OutUnresolvedObject = TargetObject;
		Schema_DestroyComponentUpdate(ComponentUpdate.schema_type);
		return ComponentUpdate;
	}

	Schema_AddUint32(EventData, 1, TargetObjectRef.Offset);
	OutEntityId = TargetObjectRef.Entity;

	TSet<const UObject*> UnresolvedObjects;
	FSpatialNetBitWriter PayloadWriter(PackageMap, UnresolvedObjects);

	TSharedPtr<FRepLayout> RepLayout = NetDriver->GetFunctionRepLayout(Function);
	RepLayout_SendPropertiesForRPC(*RepLayout, PayloadWriter, Parameters);

	for (const UObject* Object : UnresolvedObjects)
	{
		// Take the first unresolved object
		OutUnresolvedObject = Object;
		Schema_DestroyComponentUpdate(ComponentUpdate.schema_type);
		return ComponentUpdate;
	}

	Schema_AddString(EventData, 2, std::string(reinterpret_cast<char*>(PayloadWriter.GetData()), PayloadWriter.GetNumBytes()));

	return ComponentUpdate;
}
