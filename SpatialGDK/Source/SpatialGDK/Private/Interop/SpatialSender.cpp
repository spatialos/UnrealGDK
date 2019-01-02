// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "Interop/SpatialSender.h"

#include "GameFramework/PlayerController.h"
#include "GameFramework/PlayerState.h"

#include "EngineClasses/SpatialActorChannel.h"
#include "EngineClasses/SpatialNetBitWriter.h"
#include "EngineClasses/SpatialNetConnection.h"
#include "EngineClasses/SpatialNetDriver.h"
#include "EngineClasses/SpatialPackageMapClient.h"
#include "Interop/Connection/SpatialWorkerConnection.h"
#include "Interop/SpatialReceiver.h"
#include "Interop/SpatialDispatcher.h"
#include "Schema/Interest.h"
#include "Schema/Singleton.h"
#include "Schema/SpawnData.h"
#include "Schema/StandardLibrary.h"
#include "Schema/UnrealMetadata.h"
#include "SpatialConstants.h"
#include "Utils/ComponentFactory.h"
#include "Utils/EntityRegistry.h"
#include "Utils/RepLayoutUtils.h"

DEFINE_LOG_CATEGORY(LogSpatialSender);

using namespace improbable;

FPendingRPCParams::FPendingRPCParams(UObject* InTargetObject, UFunction* InFunction, void* InParameters)
	: TargetObject(InTargetObject)
	, Function(InFunction)
	, Attempts(0)
{
	Parameters.SetNumZeroed(Function->ParmsSize);

	for (TFieldIterator<UProperty> It(Function); It && It->HasAnyPropertyFlags(CPF_Parm); ++It)
	{
		It->InitializeValue_InContainer(Parameters.GetData());
		It->CopyCompleteValue_InContainer(Parameters.GetData(), InParameters);
	}
}

FPendingRPCParams::~FPendingRPCParams()
{
	for (TFieldIterator<UProperty> It(Function); It && It->HasAnyPropertyFlags(CPF_Parm); ++It)
	{
		It->DestroyValue_InContainer(Parameters.GetData());
	}
}

void USpatialSender::Init(USpatialNetDriver* InNetDriver)
{
	NetDriver = InNetDriver;
	StaticComponentView = InNetDriver->StaticComponentView;
	Connection = InNetDriver->Connection;
	Receiver = InNetDriver->Receiver;
	PackageMap = InNetDriver->PackageMap;
	TypebindingManager = InNetDriver->TypebindingManager;
}

Worker_RequestId USpatialSender::CreateEntity(USpatialActorChannel* Channel)
{
	AActor* Actor = Channel->Actor;
	UClass* Class = Actor->GetClass();

	FString ClientWorkerAttribute = GetOwnerWorkerAttribute(Actor);

	WorkerAttributeSet ServerAttribute = { SpatialConstants::ServerWorkerType };
	WorkerAttributeSet ClientAttribute = { SpatialConstants::ClientWorkerType };
	WorkerAttributeSet OwningClientAttribute = { ClientWorkerAttribute };

	WorkerRequirementSet ServersOnly = { ServerAttribute };
	WorkerRequirementSet ClientsOnly = { ClientAttribute };
	WorkerRequirementSet OwningClientOnly = { OwningClientAttribute };

	WorkerRequirementSet AnyUnrealServerOrClient = { ServerAttribute, ClientAttribute };
	WorkerRequirementSet AnyUnrealServerOrOwningClient = { ServerAttribute, OwningClientAttribute };

	WorkerRequirementSet ReadAcl;
	if (Class->HasAnySpatialClassFlags(SPATIALCLASS_ServerOnly))
	{
		ReadAcl = ServersOnly;
	}
	else if (Actor->IsA<APlayerController>())
	{
		ReadAcl = AnyUnrealServerOrOwningClient;
	}
	else
	{
		ReadAcl = AnyUnrealServerOrClient;
	}

	FClassInfo* Info = TypebindingManager->FindClassInfoByClass(Class);
	check(Info);

	WriteAclMap ComponentWriteAcl;
	ComponentWriteAcl.Add(SpatialConstants::POSITION_COMPONENT_ID, ServersOnly);
	ComponentWriteAcl.Add(SpatialConstants::INTEREST_COMPONENT_ID, ServersOnly);
	ComponentWriteAcl.Add(SpatialConstants::SPAWN_DATA_COMPONENT_ID, ServersOnly);
	ComponentWriteAcl.Add(SpatialConstants::ENTITY_ACL_COMPONENT_ID, ServersOnly);

	ForAllSchemaComponentTypes([&](ESchemaComponentType Type)
	{
		Worker_ComponentId ComponentId = Info->SchemaComponents[Type];
		if (ComponentId == SpatialConstants::INVALID_COMPONENT_ID)
		{
			return;
		}

		WorkerRequirementSet& RequirementSet = Type == SCHEMA_ClientRPC ? OwningClientOnly : ServersOnly;
		ComponentWriteAcl.Add(ComponentId, RequirementSet);
	});

	for (auto& SubobjectInfoPair : Info->SubobjectInfo)
	{
		const FClassInfo& SubobjectInfo = *SubobjectInfoPair.Value;

		// Static subobjects aren't guaranteed to exist on actor instances, check they are present before adding write acls
		TWeakObjectPtr<UObject> Subobject = PackageMap->GetObjectFromUnrealObjectRef(FUnrealObjectRef(Channel->GetEntityId(), SubobjectInfoPair.Key));
		if (!Subobject.IsValid())
		{
			continue;
		}

		ForAllSchemaComponentTypes([&](ESchemaComponentType Type)
		{
			Worker_ComponentId ComponentId = SubobjectInfo.SchemaComponents[Type];
			if (ComponentId == SpatialConstants::INVALID_COMPONENT_ID)
			{
				return;
			}

			WorkerRequirementSet& RequirementSet = Type == SCHEMA_ClientRPC ? OwningClientOnly : ServersOnly;
			ComponentWriteAcl.Add(ComponentId, RequirementSet);
		});
	}

	TArray<Worker_ComponentData> ComponentDatas;
	ComponentDatas.Add(improbable::Position(improbable::Coordinates::FromFVector(Channel->GetActorSpatialPosition(Actor))).CreatePositionData());
	ComponentDatas.Add(improbable::Metadata(Class->GetName()).CreateMetadataData());
	ComponentDatas.Add(improbable::EntityAcl(ReadAcl, ComponentWriteAcl).CreateEntityAclData());
	ComponentDatas.Add(improbable::Persistence().CreatePersistenceData());
	ComponentDatas.Add(improbable::SpawnData(Actor).CreateSpawnDataData());
	ComponentDatas.Add(improbable::UnrealMetadata({}, ClientWorkerAttribute, Class->GetPathName()).CreateUnrealMetadataData());
	ComponentDatas.Add(improbable::Interest().CreateInterestData());

	if (Class->HasAnySpatialClassFlags(SPATIALCLASS_Singleton))
	{
		ComponentDatas.Add(improbable::Singleton().CreateSingletonData());
	}

	FUnresolvedObjectsMap UnresolvedObjectsMap;
	FUnresolvedObjectsMap HandoverUnresolvedObjectsMap;
	ComponentFactory DataFactory(UnresolvedObjectsMap, HandoverUnresolvedObjectsMap, NetDriver);

	FRepChangeState InitialRepChanges = Channel->CreateInitialRepChangeState(Actor);
	FHandoverChangeState InitialHandoverChanges = Channel->CreateInitialHandoverChangeState(Info);

	TArray<Worker_ComponentData> DynamicComponentDatas = DataFactory.CreateComponentDatas(Actor, Info, InitialRepChanges, InitialHandoverChanges);
	ComponentDatas.Append(DynamicComponentDatas);

	for (auto& HandleUnresolvedObjectsPair : UnresolvedObjectsMap)
	{
		QueueOutgoingUpdate(Channel, Actor, HandleUnresolvedObjectsPair.Key, HandleUnresolvedObjectsPair.Value, /* bIsHandover */ false);
	}

	for (auto& HandleUnresolvedObjectsPair : HandoverUnresolvedObjectsMap)
	{
		QueueOutgoingUpdate(Channel, Actor, HandleUnresolvedObjectsPair.Key, HandleUnresolvedObjectsPair.Value, /* bIsHandover */ true);
	}

	for (int32 RPCType = SCHEMA_FirstRPC; RPCType <= SCHEMA_LastRPC; RPCType++)
	{
		if (Info->SchemaComponents[RPCType] != SpatialConstants::INVALID_COMPONENT_ID)
		{
			ComponentDatas.Add(ComponentFactory::CreateEmptyComponentData(Info->SchemaComponents[RPCType]));
		}
	}

	for (auto& SubobjectInfoPair : Info->SubobjectInfo)
	{
		FClassInfo* SubobjectInfo = SubobjectInfoPair.Value.Get();

		TWeakObjectPtr<UObject> Subobject = PackageMap->GetObjectFromUnrealObjectRef(FUnrealObjectRef(Channel->GetEntityId(), SubobjectInfoPair.Key));
		if (!Subobject.IsValid())
		{
			UE_LOG(LogSpatialSender, Error, TEXT("Tried to generate initial replication state for an invalid sub-object. Object may have been deleted or is PendingKill."));
			continue;
		}

		FRepChangeState SubobjectRepChanges = Channel->CreateInitialRepChangeState(Subobject);
		FHandoverChangeState SubobjectHandoverChanges = Channel->CreateInitialHandoverChangeState(SubobjectInfo);

		// Reset unresolved objects so they can be filled again by DataFactory
		UnresolvedObjectsMap.Empty();
		HandoverUnresolvedObjectsMap.Empty();

		TArray<Worker_ComponentData> ActorSubobjectDatas = DataFactory.CreateComponentDatas(Subobject.Get(), SubobjectInfo, SubobjectRepChanges, SubobjectHandoverChanges);
		ComponentDatas.Append(ActorSubobjectDatas);

		for (auto& HandleUnresolvedObjectsPair : UnresolvedObjectsMap)
		{
			QueueOutgoingUpdate(Channel, Subobject.Get(), HandleUnresolvedObjectsPair.Key, HandleUnresolvedObjectsPair.Value, /* bIsHandover */ false);
		}

		for (auto& HandleUnresolvedObjectsPair : HandoverUnresolvedObjectsMap)
		{
			QueueOutgoingUpdate(Channel, Subobject.Get(), HandleUnresolvedObjectsPair.Key, HandleUnresolvedObjectsPair.Value, /* bIsHandover */ true);
		}

		for (int32 RPCType = SCHEMA_ClientRPC; RPCType < SCHEMA_Count; RPCType++)
		{
			if (SubobjectInfo->SchemaComponents[RPCType] != SpatialConstants::INVALID_COMPONENT_ID)
			{
				ComponentDatas.Add(ComponentFactory::CreateEmptyComponentData(SubobjectInfo->SchemaComponents[RPCType]));
			}
		}
	}

	Worker_EntityId EntityId = Channel->GetEntityId();
	Worker_RequestId CreateEntityRequestId = Connection->SendCreateEntityRequest(ComponentDatas.Num(), ComponentDatas.GetData(), &EntityId);
	PendingActorRequests.Add(CreateEntityRequestId, Channel);

	return CreateEntityRequestId;
}

void USpatialSender::SendComponentUpdates(UObject* Object, FClassInfo* Info, USpatialActorChannel* Channel, const FRepChangeState* RepChanges, const FHandoverChangeState* HandoverChanges)
{
	Worker_EntityId EntityId = Channel->GetEntityId();

	UE_LOG(LogSpatialSender, Verbose, TEXT("Sending component update (object: %s, entity: %lld)"), *Object->GetName(), EntityId);

	FUnresolvedObjectsMap UnresolvedObjectsMap;
	FUnresolvedObjectsMap HandoverUnresolvedObjectsMap;
	ComponentFactory UpdateFactory(UnresolvedObjectsMap, HandoverUnresolvedObjectsMap, NetDriver);

	TArray<Worker_ComponentUpdate> ComponentUpdates = UpdateFactory.CreateComponentUpdates(Object, Info, EntityId, RepChanges, HandoverChanges);

	if (RepChanges)
	{
		for (uint16 Handle : RepChanges->RepChanged)
		{
			if (Handle > 0)
			{
				ResetOutgoingUpdate(Channel, Object, Handle, /* bIsHandover */ false);

				if (TSet<TWeakObjectPtr<const UObject>>* UnresolvedObjects = UnresolvedObjectsMap.Find(Handle))
				{
					QueueOutgoingUpdate(Channel, Object, Handle, *UnresolvedObjects, /* bIsHandover */ false);
				}
			}
		}
	}

	if (HandoverChanges)
	{
		for (uint16 Handle : *HandoverChanges)
		{
			ResetOutgoingUpdate(Channel, Object, Handle, /* bIsHandover */ true);

			if (TSet<TWeakObjectPtr<const UObject>>* UnresolvedObjects = HandoverUnresolvedObjectsMap.Find(Handle))
			{
				QueueOutgoingUpdate(Channel, Object, Handle, *UnresolvedObjects, /* bIsHandover */ true);
			}
		}
	}

	for (Worker_ComponentUpdate& Update : ComponentUpdates)
	{
		if (!NetDriver->StaticComponentView->HasAuthority(EntityId, Update.component_id))
		{
			UE_LOG(LogSpatialSender, Warning, TEXT("Trying to send component update but don't have authority! Update will not be sent. Component Id: %d, entity: %lld"), Update.component_id, EntityId);
			continue;
		}

		Connection->SendComponentUpdate(EntityId, &Update);
	}
}


void FillComponentInterests(FClassInfo* Info, bool bNetOwned, TArray<Worker_InterestOverride>& ComponentInterest)
{
	if (Info->SchemaComponents[SCHEMA_OwnerOnly] != SpatialConstants::INVALID_COMPONENT_ID)
	{
		Worker_InterestOverride SingleClientInterest = { Info->SchemaComponents[SCHEMA_OwnerOnly], bNetOwned };
		ComponentInterest.Add(SingleClientInterest);
	}

	if (Info->SchemaComponents[SCHEMA_Handover] != SpatialConstants::INVALID_COMPONENT_ID)
	{
		Worker_InterestOverride HandoverInterest = { Info->SchemaComponents[SCHEMA_Handover], false };
		ComponentInterest.Add(HandoverInterest);
	}
}

TArray<Worker_InterestOverride> USpatialSender::CreateComponentInterest(AActor* Actor)
{
	TArray<Worker_InterestOverride> ComponentInterest;

	// This effectively checks whether the actor is owned by our PlayerController
	bool bNetOwned = Actor->GetNetConnection() != nullptr;

	FClassInfo* ActorInfo = TypebindingManager->FindClassInfoByClass(Actor->GetClass());
	FillComponentInterests(ActorInfo, bNetOwned, ComponentInterest);

	for (auto& SubobjectInfoPair : ActorInfo->SubobjectInfo)
	{
		FClassInfo* SubobjectInfo = SubobjectInfoPair.Value.Get();
		FillComponentInterests(SubobjectInfo, bNetOwned, ComponentInterest);
	}

	return ComponentInterest;
}

void USpatialSender::SendComponentInterest(AActor* Actor, Worker_EntityId EntityId)
{
	check(!NetDriver->IsServer());

	NetDriver->Connection->SendComponentInterest(EntityId, CreateComponentInterest(Actor));
}

void USpatialSender::SendPositionUpdate(Worker_EntityId EntityId, const FVector& Location)
{
#if !UE_BUILD_SHIPPING
	if (!NetDriver->StaticComponentView->HasAuthority(EntityId, SpatialConstants::POSITION_COMPONENT_ID))
	{
		UE_LOG(LogSpatialSender, Warning, TEXT("Trying to send Position component update but don't have authority! Update will not be sent. Entity: %lld"), EntityId);
		return;
	}
#endif

	Worker_ComponentUpdate Update = improbable::Position::CreatePositionUpdate(improbable::Coordinates::FromFVector(Location));
	Connection->SendComponentUpdate(EntityId, &Update);
}

void USpatialSender::SendRPC(TSharedRef<FPendingRPCParams> Params)
{
	if (!Params->TargetObject.IsValid())
	{
		// Target object was destroyed before the RPC could be (re)sent
		return;
	}

	UObject* TargetObject = Params->TargetObject.Get();
	if (PackageMap->GetUnrealObjectRefFromObject(TargetObject) == SpatialConstants::UNRESOLVED_OBJECT_REF)
	{
		UE_LOG(LogSpatialSender, Verbose, TEXT("Trying to send RPC %s on unresolved Actor %s."), *Params->Function->GetName(), *TargetObject->GetName());
		QueueOutgoingRPC(TargetObject, Params);
		return;
	}

	FClassInfo* Info = TypebindingManager->FindClassInfoByObject(TargetObject);

	if (Info == nullptr)
	{
		UE_LOG(LogSpatialSender, Warning, TEXT("Trying to send RPC %s on unsupported Actor %s."), *Params->Function->GetName(), *TargetObject->GetName());
		return;
	}

	const FRPCInfo* RPCInfo = Info->RPCInfoMap.Find(Params->Function);

	// We potentially have a parent function and need to find the child function.
	// This exists as it's possible in blueprints to explicitly call the parent function.
	if (RPCInfo == nullptr)
	{
		for (auto It = Info->RPCInfoMap.CreateConstIterator(); It; ++It)
		{
			if (It.Key()->GetName() == Params->Function->GetName())
			{
				// Matching child function found. Use this for the remote function call.
				RPCInfo = &It.Value();
				break;
			}
		}
	}

	check(RPCInfo);

	Worker_EntityId EntityId = SpatialConstants::INVALID_ENTITY_ID;
	const UObject* UnresolvedObject = nullptr;

	switch (RPCInfo->Type)
	{
	case SCHEMA_ClientRPC:
	case SCHEMA_ServerRPC:
	case SCHEMA_CrossServerRPC:
	{
		Worker_CommandRequest CommandRequest = CreateRPCCommandRequest(TargetObject, Params->Function, Params->Parameters.GetData(), Info->SchemaComponents[RPCInfo->Type], RPCInfo->Index + 1, EntityId, UnresolvedObject);

		if (!UnresolvedObject)
		{
			check(EntityId != SpatialConstants::INVALID_ENTITY_ID);
			Worker_RequestId RequestId = Connection->SendCommandRequest(EntityId, &CommandRequest, RPCInfo->Index + 1);

			if (Params->Function->HasAnyFunctionFlags(FUNC_NetReliable))
			{
				// The number of attempts is used to determine the delay in case the command times out and we need to resend it.
				Params->Attempts++;
				Receiver->AddPendingReliableRPC(RequestId, Params);
			}
		}
		break;
	}
	case SCHEMA_NetMulticastRPC:
	{
		Worker_ComponentUpdate ComponentUpdate = CreateMulticastUpdate(TargetObject, Params->Function, Params->Parameters.GetData(), Info->SchemaComponents[RPCInfo->Type], RPCInfo->Index + 1, EntityId, UnresolvedObject);

		if (!UnresolvedObject)
		{
			check(EntityId != SpatialConstants::INVALID_ENTITY_ID);

			if (!NetDriver->StaticComponentView->HasAuthority(EntityId, ComponentUpdate.component_id))
			{
				UE_LOG(LogSpatialSender, Warning, TEXT("Trying to send MulticastRPC component update but don't have authority! Update will not be sent. Entity: %lld"), EntityId);
				return;
			}

			Connection->SendComponentUpdate(EntityId, &ComponentUpdate);
		}
		break;
	}
	default:
		checkNoEntry();
		break;
	}

	if (UnresolvedObject)
	{
		QueueOutgoingRPC(UnresolvedObject, Params);
	}
}

void USpatialSender::EnqueueRPC(TSharedRef<FPendingRPCParams> Params)
{
	QueuedRPCs.Push(Params);
}

void USpatialSender::FlushQueuedRPCs()
{
	// Retried RPCs are popped from a stack to reverse their order.
	// This is doe to undo the reversal of ordering caused by the TimerManager class.
	while (QueuedRPCs.Num() > 0)
	{
		SendRPC(QueuedRPCs.Pop());
	}
}

void USpatialSender::SendReserveEntityIdRequest(USpatialActorChannel* Channel)
{
	UE_LOG(LogSpatialSender, Log, TEXT("Sending reserve entity Id request for %s"), *Channel->Actor->GetName());
	Worker_RequestId RequestId = Connection->SendReserveEntityIdRequest();
	Receiver->AddPendingActorRequest(RequestId, Channel);
}

void USpatialSender::SendCreateEntityRequest(USpatialActorChannel* Channel)
{
	UE_LOG(LogSpatialSender, Log, TEXT("Sending create entity request for %s"), *Channel->Actor->GetName());

	FSoftClassPath ActorClassPath(Channel->Actor->GetClass());

	Worker_RequestId RequestId = CreateEntity(Channel);
	Receiver->AddPendingActorRequest(RequestId, Channel);
}

void USpatialSender::SendDeleteEntityRequest(Worker_EntityId EntityId)
{
	Connection->SendDeleteEntityRequest(EntityId);
}

void USpatialSender::ResetOutgoingUpdate(USpatialActorChannel* DependentChannel, UObject* ReplicatedObject, int16 Handle, bool bIsHandover)
{
	check(DependentChannel);
	check(ReplicatedObject);
	const FChannelObjectPair ChannelObjectPair(DependentChannel, ReplicatedObject);

	// Choose the correct container based on whether it's handover or not
	FChannelToHandleToUnresolved& PropertyToUnresolved = bIsHandover ? HandoverPropertyToUnresolved : RepPropertyToUnresolved;
	FOutgoingRepUpdates& ObjectToUnresolved = bIsHandover ? HandoverObjectToUnresolved : RepObjectToUnresolved;

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

	UE_LOG(LogSpatialSender, Log, TEXT("Resetting pending outgoing array depending on channel: %s, object: %s, handle: %d."),
		*DependentChannel->GetName(), *ReplicatedObject->GetName(), Handle);

	for (TWeakObjectPtr<const UObject>& UnresolvedObject : *Unresolved)
	{
		if (UnresolvedObject.IsValid())
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
		else
		{
			// If the object is no longer valid (may have been deleted or IsPendingKill) then remove it from the UnresolvedObjects.
			Unresolved->Remove(UnresolvedObject);
			// TODO: UNR-814 Also remove it from other maps which reference the object by handle etc.
		}
	}

	HandleToUnresolved->Remove(Handle);
	if (HandleToUnresolved->Num() == 0)
	{
		PropertyToUnresolved.Remove(ChannelObjectPair);
	}
}

void USpatialSender::QueueOutgoingUpdate(USpatialActorChannel* DependentChannel, UObject* ReplicatedObject, int16 Handle, const TSet<TWeakObjectPtr<const UObject>>& UnresolvedObjects, bool bIsHandover)
{
	check(DependentChannel);
	check(ReplicatedObject);
	FChannelObjectPair ChannelObjectPair(DependentChannel, ReplicatedObject);

	UE_LOG(LogSpatialSender, Log, TEXT("Added pending outgoing property: channel: %s, object: %s, handle: %d. Depending on objects:"),
		*DependentChannel->GetName(), *ReplicatedObject->GetName(), Handle);

	// Choose the correct container based on whether it's handover or not
	FChannelToHandleToUnresolved& PropertyToUnresolved = bIsHandover ? HandoverPropertyToUnresolved : RepPropertyToUnresolved;
	FOutgoingRepUpdates& ObjectToUnresolved = bIsHandover ? HandoverObjectToUnresolved : RepObjectToUnresolved;

	FUnresolvedEntry Unresolved = MakeShared<TSet<TWeakObjectPtr<const UObject>>>();
	*Unresolved = UnresolvedObjects;

	FHandleToUnresolved& HandleToUnresolved = PropertyToUnresolved.FindOrAdd(ChannelObjectPair);
	if (HandleToUnresolved.Find(Handle))
	{
		HandleToUnresolved.Remove(Handle);
	}
	HandleToUnresolved.Add(Handle, Unresolved);

	for (const TWeakObjectPtr<const UObject>& UnresolvedObject : UnresolvedObjects)
	{
		if (UnresolvedObject.IsValid())
		{
			FHandleToUnresolved& AnotherHandleToUnresolved = ObjectToUnresolved.FindOrAdd(UnresolvedObject).FindOrAdd(ChannelObjectPair);
			check(!AnotherHandleToUnresolved.Find(Handle));
			AnotherHandleToUnresolved.Add(Handle, Unresolved);

			// Following up on the previous log: listing the unresolved objects
			UE_LOG(LogSpatialSender, Log, TEXT("- %s"), *UnresolvedObject->GetName());
		}
		else
		{
			// If the object is no longer valid (may have been deleted or IsPendingKill) then remove it from the UnresolvedObjects.
			Unresolved->Remove(UnresolvedObject);
			// TODO: UNR-814 Also remove it from other maps which reference the object by handle etc.
		}
	}
}

void USpatialSender::QueueOutgoingRPC(const UObject* UnresolvedObject, TSharedRef<FPendingRPCParams> Params)
{
	check(UnresolvedObject);
	UE_LOG(LogSpatialSender, Log, TEXT("Added pending outgoing RPC depending on object: %s, target: %s, function: %s"), *UnresolvedObject->GetName(), *Params->TargetObject->GetName(), *Params->Function->GetName());
	OutgoingRPCs.FindOrAdd(UnresolvedObject).Add(Params);
}

Worker_CommandRequest USpatialSender::CreateRPCCommandRequest(UObject* TargetObject, UFunction* Function, void* Parameters, Worker_ComponentId ComponentId, Schema_FieldId CommandIndex, Worker_EntityId& OutEntityId, const UObject*& OutUnresolvedObject)
{
	Worker_CommandRequest CommandRequest = {};
	CommandRequest.component_id = ComponentId;
	CommandRequest.schema_type = Schema_CreateCommandRequest(ComponentId, CommandIndex);
	Schema_Object* RequestObject = Schema_GetCommandRequestObject(CommandRequest.schema_type);

	FUnrealObjectRef TargetObjectRef(PackageMap->GetUnrealObjectRefFromNetGUID(PackageMap->GetNetGUIDFromObject(TargetObject)));
	if (TargetObjectRef == SpatialConstants::UNRESOLVED_OBJECT_REF)
	{
		OutUnresolvedObject = TargetObject;
		Schema_DestroyCommandRequest(CommandRequest.schema_type);
		return CommandRequest;
	}

	OutEntityId = TargetObjectRef.Entity;

	TSet<TWeakObjectPtr<const UObject>> UnresolvedObjects;
	FSpatialNetBitWriter PayloadWriter(PackageMap, UnresolvedObjects);

	TSharedPtr<FRepLayout> RepLayout = NetDriver->GetFunctionRepLayout(Function);
	RepLayout_SendPropertiesForRPC(*RepLayout, PayloadWriter, Parameters);

	for (TWeakObjectPtr<const UObject> Object : UnresolvedObjects)
	{
		if (Object.IsValid())
		{
			// Take the first unresolved object
			OutUnresolvedObject = Object.Get();
			Schema_DestroyCommandRequest(CommandRequest.schema_type);
			return CommandRequest;
		}
		else
		{
			// If the object is no longer valid (may have been deleted or IsPendingKill) then remove it from the UnresolvedObjects.
			UnresolvedObjects.Remove(Object);
			// TODO: UNR-814 Also remove it from other maps which reference the object by handle etc.
		}
	}

	AddPayloadToSchema(RequestObject, 1, PayloadWriter);

	return CommandRequest;
}

Worker_ComponentUpdate USpatialSender::CreateMulticastUpdate(UObject* TargetObject, UFunction* Function, void* Parameters, Worker_ComponentId ComponentId, Schema_FieldId EventIndex, Worker_EntityId& OutEntityId, const UObject*& OutUnresolvedObject)
{
	Worker_ComponentUpdate ComponentUpdate = {};

	ComponentUpdate.component_id = ComponentId;
	ComponentUpdate.schema_type = Schema_CreateComponentUpdate(ComponentId);
	Schema_Object* EventsObject = Schema_GetComponentUpdateEvents(ComponentUpdate.schema_type);
	Schema_Object* EventData = Schema_AddObject(EventsObject, EventIndex);

	FUnrealObjectRef TargetObjectRef(PackageMap->GetUnrealObjectRefFromNetGUID(PackageMap->GetNetGUIDFromObject(TargetObject)));
	if (TargetObjectRef == SpatialConstants::UNRESOLVED_OBJECT_REF)
	{
		OutUnresolvedObject = TargetObject;
		Schema_DestroyComponentUpdate(ComponentUpdate.schema_type);
		return ComponentUpdate;
	}

	OutEntityId = TargetObjectRef.Entity;

	TSet<TWeakObjectPtr<const UObject>> UnresolvedObjects;
	FSpatialNetBitWriter PayloadWriter(PackageMap, UnresolvedObjects);

	TSharedPtr<FRepLayout> RepLayout = NetDriver->GetFunctionRepLayout(Function);
	RepLayout_SendPropertiesForRPC(*RepLayout, PayloadWriter, Parameters);

	for (TWeakObjectPtr<const UObject> Object : UnresolvedObjects)
	{
		if (Object.IsValid())
		{
			// Take the first unresolved object
			OutUnresolvedObject = Object.Get();
			Schema_DestroyComponentUpdate(ComponentUpdate.schema_type);
			return ComponentUpdate;
		}
		else
		{
			// If the object is no longer valid (may have been deleted or IsPendingKill) then remove it from the UnresolvedObjects.
			UnresolvedObjects.Remove(Object);
			// TODO: UNR-814 Also remove it from other maps which reference the object by handle etc.
		}
	}

	AddPayloadToSchema(EventData, 1, PayloadWriter);

	return ComponentUpdate;
}

void USpatialSender::SendCommandResponse(Worker_RequestId request_id, Worker_CommandResponse& Response)
{
	Connection->SendCommandResponse(request_id, &Response);
}

void USpatialSender::ResolveOutgoingOperations(UObject* Object, bool bIsHandover)
{
	// Choose the correct container based on whether it's handover or not
	FChannelToHandleToUnresolved& PropertyToUnresolved = bIsHandover ? HandoverPropertyToUnresolved : RepPropertyToUnresolved;
	FOutgoingRepUpdates& ObjectToUnresolved = bIsHandover ? HandoverObjectToUnresolved : RepObjectToUnresolved;

	FChannelToHandleToUnresolved* ChannelToUnresolved = ObjectToUnresolved.Find(Object);
	if (!ChannelToUnresolved)
	{
		return;
	}

	for (auto& ChannelProperties : *ChannelToUnresolved)
	{
		FChannelObjectPair& ChannelObjectPair = ChannelProperties.Key;
		if (!ChannelObjectPair.Key.IsValid() || !ChannelObjectPair.Value.IsValid())
		{
			continue;
		}

		USpatialActorChannel* DependentChannel = ChannelObjectPair.Key.Get();
		UObject* ReplicatingObject = ChannelObjectPair.Value.Get();
		FHandleToUnresolved& HandleToUnresolved = ChannelProperties.Value;

		FClassInfo* Info = TypebindingManager->FindClassInfoByObject(ReplicatingObject);
		if (Info == nullptr)
		{
			continue;
		}

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
				if (!bIsHandover && DependentChannel->IsDynamicArrayHandle(ReplicatingObject, Handle))
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
			if (bIsHandover)
			{
				SendComponentUpdates(ReplicatingObject, Info, DependentChannel, nullptr, &PropertyHandles);
			}
			else
			{
				// End with zero to indicate the end of the list of handles.
				PropertyHandles.Add(0);
				FRepChangeState RepChangeState = { PropertyHandles, DependentChannel->GetObjectRepLayout(ReplicatingObject) };
				SendComponentUpdates(ReplicatingObject, Info, DependentChannel, &RepChangeState, nullptr);
			}
		}
	}

	ObjectToUnresolved.Remove(Object);
}

void USpatialSender::ResolveOutgoingRPCs(UObject* Object)
{
	TArray<TSharedRef<FPendingRPCParams>>* RPCList = OutgoingRPCs.Find(Object);
	if (RPCList)
	{
		for (TSharedRef<FPendingRPCParams>& RPCParams : *RPCList)
		{
			if (!RPCParams->TargetObject.IsValid())
			{
				// The target object was destroyed before we could send the RPC.
				continue;
			}

			// We can guarantee that SendRPC won't populate OutgoingRPCs[Object] whilst we're iterating through it,
			// because Object has been resolved when we call ResolveOutgoingRPCs.
			UE_LOG(LogSpatialSender, Log, TEXT("Resolving outgoing RPC depending on object: %s, target: %s, function: %s"), *Object->GetName(), *RPCParams->TargetObject->GetName(), *RPCParams->Function->GetName());
			SendRPC(RPCParams);
		}
		OutgoingRPCs.Remove(Object);
	}
}

FString USpatialSender::GetOwnerWorkerAttribute(AActor* Actor)
{
	// If we don't have an owning connection, there is no assoicated client
	if (Actor->GetNetConnection() == nullptr)
	{
		return FString();
	}

	if (APlayerController* PlayerController = Actor->GetNetConnection()->PlayerController)
	{
		if (APlayerState* PlayerState = PlayerController->PlayerState)
		{
			// If the player state is resolved, the UniqueId is set to be the owning attribute - USpatialNetDriver::AcceptNewPlayer
			if (PlayerState->UniqueId.IsValid())
			{
				return PlayerState->UniqueId.ToString();
			}
			else
			{
				// If the UniqueId is invalid, get the owning attribute from the PlayerController's EntityACL.
				Worker_EntityId PlayerControllerEntityId = NetDriver->GetEntityRegistry()->GetEntityIdFromActor(PlayerController);
				if (PlayerControllerEntityId == 0)
				{
					return FString();
				}

				improbable::EntityAcl* EntityACL = StaticComponentView->GetComponentData<improbable::EntityAcl>(PlayerControllerEntityId);

				FClassInfo* Info = TypebindingManager->FindClassInfoByClass(PlayerController->GetClass());

				WorkerRequirementSet ClientRPCRequirementSet = EntityACL->ComponentWriteAcl[Info->SchemaComponents[SCHEMA_ClientRPC]];
				WorkerAttributeSet ClientRPCAttributeSet = ClientRPCRequirementSet[0];
				return ClientRPCAttributeSet[0];
			}
		}
	}

	return FString();
}

// Authority over the ClientRPC Schema component is dictated by the owning connection of a client.
// This function updates the authority of that component as the owning connection can change.
bool USpatialSender::UpdateEntityACLs(AActor* Actor, Worker_EntityId EntityId)
{
	improbable::EntityAcl* EntityACL = StaticComponentView->GetComponentData<improbable::EntityAcl>(EntityId);

	if (EntityACL == nullptr)
	{
		return false;
	}

	if (!NetDriver->StaticComponentView->HasAuthority(EntityId, SpatialConstants::ENTITY_ACL_COMPONENT_ID))
	{
		UE_LOG(LogSpatialSender, Warning, TEXT("Trying to update EntityACL but don't have authority! Update will not be sent. Entity: %lld"), EntityId);
		return false;
	}

	FClassInfo* Info = TypebindingManager->FindClassInfoByClass(Actor->GetClass());
	check(Info);

	FString OwnerWorkerAttribute = GetOwnerWorkerAttribute(Actor);
	WorkerAttributeSet OwningClientAttribute = { OwnerWorkerAttribute };
	WorkerRequirementSet OwningClientOnly = { OwningClientAttribute };

	EntityACL->ComponentWriteAcl.Add(Info->SchemaComponents[SCHEMA_ClientRPC], OwningClientOnly);

	for (auto& SubobjectInfoPair : Info->SubobjectInfo)
	{
		FClassInfo& SubobjectInfo = *SubobjectInfoPair.Value;

		if (SubobjectInfo.SchemaComponents[SCHEMA_ClientRPC] != SpatialConstants::INVALID_COMPONENT_ID)
		{
			EntityACL->ComponentWriteAcl.Add(SubobjectInfo.SchemaComponents[SCHEMA_ClientRPC], OwningClientOnly);
		}
	}

	Worker_ComponentUpdate Update = EntityACL->CreateEntityAclUpdate();

	Connection->SendComponentUpdate(EntityId, &Update);
	return true;
}
