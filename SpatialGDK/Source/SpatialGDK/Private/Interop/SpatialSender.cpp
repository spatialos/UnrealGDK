// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "Interop/SpatialSender.h"

#include "GameFramework/PlayerController.h"
#include "GameFramework/PlayerState.h"

#include "Engine/Engine.h"
#include "EngineClasses/SpatialActorChannel.h"
#include "EngineClasses/SpatialNetConnection.h"
#include "EngineClasses/SpatialNetDriver.h"
#include "EngineClasses/SpatialPackageMapClient.h"
#include "Interop/Connection/SpatialWorkerConnection.h"
#include "Interop/SpatialDispatcher.h"
#include "Interop/SpatialReceiver.h"
#include "Schema/Heartbeat.h"
#include "Schema/Interest.h"
#include "Schema/RPCPayload.h"
#include "Schema/Singleton.h"
#include "Schema/SpawnData.h"
#include "Schema/StandardLibrary.h"
#include "Schema/UnrealMetadata.h"
#include "SpatialConstants.h"
#include "Utils/ComponentFactory.h"
#include "Utils/InterestFactory.h"
#include "Utils/RepLayoutUtils.h"
#include "Utils/SpatialActorUtils.h"

DEFINE_LOG_CATEGORY(LogSpatialSender);

using namespace SpatialGDK;

DECLARE_CYCLE_STAT(TEXT("SendComponentUpdates"), STAT_SpatialSenderSendComponentUpdates, STATGROUP_SpatialNet);
DECLARE_CYCLE_STAT(TEXT("ResetOutgoingUpdate"), STAT_SpatialSenderResetOutgoingUpdate, STATGROUP_SpatialNet);
DECLARE_CYCLE_STAT(TEXT("QueueOutgoingUpdate"), STAT_SpatialSenderQueueOutgoingUpdate, STATGROUP_SpatialNet);

FPendingRPCParams::FPendingRPCParams(UObject* InTargetObject, UFunction* InFunction, void* InParameters, int InRetryIndex)
	: TargetObject(InTargetObject)
	, Function(InFunction)
	, RetryIndex(InRetryIndex)
	, ReliableRPCIndex(0)
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

FReliableRPCForRetry::FReliableRPCForRetry(UObject* InTargetObject, UFunction* InFunction, Worker_ComponentId InComponentId, Schema_FieldId InRPCIndex, TArray<uint8>&& InPayload, int InRetryIndex)
	: TargetObject(InTargetObject)
	, Function(InFunction)
	, ComponentId(InComponentId)
	, RPCIndex(InRPCIndex)
	, Payload(MoveTemp(InPayload))
	, Attempts(1)
	, RetryIndex(InRetryIndex)
{
}

void USpatialSender::Init(USpatialNetDriver* InNetDriver)
{
	NetDriver = InNetDriver;
	StaticComponentView = InNetDriver->StaticComponentView;
	Connection = InNetDriver->Connection;
	Receiver = InNetDriver->Receiver;
	PackageMap = InNetDriver->PackageMap;
	ClassInfoManager = InNetDriver->ClassInfoManager;
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

	const FClassInfo& Info = ClassInfoManager->GetOrCreateClassInfoByClass(Class);

	WriteAclMap ComponentWriteAcl;
	ComponentWriteAcl.Add(SpatialConstants::POSITION_COMPONENT_ID, ServersOnly);
	ComponentWriteAcl.Add(SpatialConstants::INTEREST_COMPONENT_ID, ServersOnly);
	ComponentWriteAcl.Add(SpatialConstants::SPAWN_DATA_COMPONENT_ID, ServersOnly);
	ComponentWriteAcl.Add(SpatialConstants::ENTITY_ACL_COMPONENT_ID, ServersOnly);
	ComponentWriteAcl.Add(SpatialConstants::SERVER_RPC_ENDPOINT_COMPONENT_ID, ServersOnly);
	ComponentWriteAcl.Add(SpatialConstants::NETMULTICAST_RPCS_COMPONENT_ID, ServersOnly);
	ComponentWriteAcl.Add(SpatialConstants::CLIENT_RPC_ENDPOINT_COMPONENT_ID, OwningClientOnly);

	// If there are pending RPCs, add this component.
	if (OutgoingOnCreateEntityRPCs.Contains(Actor))
	{
		ComponentWriteAcl.Add(SpatialConstants::RPCS_ON_ENTITY_CREATION_ID, ServersOnly);
	}

	// If Actor is a PlayerController, add the heartbeat component.
	if (Actor->IsA<APlayerController>())
	{
		ComponentWriteAcl.Add(SpatialConstants::HEARTBEAT_COMPONENT_ID, OwningClientOnly);
	}

	ForAllSchemaComponentTypes([&](ESchemaComponentType Type)
	{
		Worker_ComponentId ComponentId = Info.SchemaComponents[Type];
		if (ComponentId == SpatialConstants::INVALID_COMPONENT_ID)
		{
			return;
		}

		ComponentWriteAcl.Add(ComponentId, ServersOnly);
	});

	for (auto& SubobjectInfoPair : Info.SubobjectInfo)
	{
		const FClassInfo& SubobjectInfo = SubobjectInfoPair.Value.Get();

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

			ComponentWriteAcl.Add(ComponentId, ServersOnly);
		});
	}

	// We want to have a stably named ref if this is a loaded Actor.
	// We use this to indicate if a new Actor should be created or to link a pre-existing Actor when receiving an AddEntityOp.
	// Previously, IsFullNameStableForNetworking was used but this was only true if bNetLoadOnClient was true.
	TSchemaOption<FUnrealObjectRef> StablyNamedObjectRef;
	TSchemaOption<bool> bNetStartup;
	if (Actor->HasAnyFlags(RF_WasLoaded) || Actor->bNetStartup)
	{
		// Since we've already received the EntityId for this Actor. It is guaranteed to be resolved
		// with the package map by this point
		FUnrealObjectRef OuterObjectRef = PackageMap->GetUnrealObjectRefFromObject(Actor->GetOuter());
		if (OuterObjectRef == FUnrealObjectRef::UNRESOLVED_OBJECT_REF)
		{
			FNetworkGUID NetGUID = PackageMap->ResolveStablyNamedObject(Actor->GetOuter());
			OuterObjectRef = PackageMap->GetUnrealObjectRefFromNetGUID(NetGUID);
		}

		// No path in SpatialOS should contain a PIE prefix.
		FString TempPath = Actor->GetFName().ToString();
		GEngine->NetworkRemapPath(NetDriver, TempPath, false /*bIsReading*/);

		StablyNamedObjectRef = FUnrealObjectRef(0, 0, TempPath, OuterObjectRef, true);
		bNetStartup = Actor->bNetStartup;
	}

	TArray<Worker_ComponentData> ComponentDatas;
	ComponentDatas.Add(Position(Coordinates::FromFVector(Channel->GetActorSpatialPosition(Actor))).CreatePositionData());
	ComponentDatas.Add(Metadata(Class->GetName()).CreateMetadataData());
	ComponentDatas.Add(EntityAcl(ReadAcl, ComponentWriteAcl).CreateEntityAclData());
	ComponentDatas.Add(Persistence().CreatePersistenceData());
	ComponentDatas.Add(SpawnData(Actor).CreateSpawnDataData());
	ComponentDatas.Add(UnrealMetadata(StablyNamedObjectRef, ClientWorkerAttribute, Class->GetPathName(), bNetStartup).CreateUnrealMetadataData());

	if (RPCsOnEntityCreation* QueuedRPCs = OutgoingOnCreateEntityRPCs.Find(Actor))
	{
		if (QueuedRPCs->HasRPCPayloadData())
		{
			ComponentDatas.Add(QueuedRPCs->CreateRPCPayloadData());
		}
		OutgoingOnCreateEntityRPCs.Remove(Actor);
	}

	if (Class->HasAnySpatialClassFlags(SPATIALCLASS_Singleton))
	{
		ComponentDatas.Add(Singleton().CreateSingletonData());
	}

	// If the Actor was loaded rather than dynamically spawned, associate it with its owning sublevel.
	ComponentDatas.Add(CreateLevelComponentData(Actor));

	if (Actor->IsA<APlayerController>())
	{
		ComponentDatas.Add(Heartbeat().CreateHeartbeatData());
	}

	FUnresolvedObjectsMap UnresolvedObjectsMap;
	FUnresolvedObjectsMap HandoverUnresolvedObjectsMap;
	ComponentFactory DataFactory(UnresolvedObjectsMap, HandoverUnresolvedObjectsMap, false, NetDriver);

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

	InterestFactory InterestDataFactory(Actor, Info, NetDriver);
	ComponentDatas.Add(InterestDataFactory.CreateInterestData());

	ComponentDatas.Add(ComponentFactory::CreateEmptyComponentData(SpatialConstants::CLIENT_RPC_ENDPOINT_COMPONENT_ID));
	ComponentDatas.Add(ComponentFactory::CreateEmptyComponentData(SpatialConstants::SERVER_RPC_ENDPOINT_COMPONENT_ID));
	ComponentDatas.Add(ComponentFactory::CreateEmptyComponentData(SpatialConstants::NETMULTICAST_RPCS_COMPONENT_ID));

	for (auto& SubobjectInfoPair : Info.SubobjectInfo)
	{
		FClassInfo& SubobjectInfo = SubobjectInfoPair.Value.Get();

		TWeakObjectPtr<UObject> Subobject = PackageMap->GetObjectFromUnrealObjectRef(FUnrealObjectRef(Channel->GetEntityId(), SubobjectInfoPair.Key));
		if (!Subobject.IsValid())
		{
			UE_LOG(LogSpatialSender, Warning, TEXT("Tried to generate initial replication state for an invalid sub-object (class %s, sub-object %s, actor %s). Object may have been deleted or is PendingKill."), *SubobjectInfo.Class->GetName(), *SubobjectInfo.SubobjectName.ToString(), *Actor->GetName());
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
	}

	Worker_EntityId EntityId = Channel->GetEntityId();
	Worker_RequestId CreateEntityRequestId = Connection->SendCreateEntityRequest(MoveTemp(ComponentDatas), &EntityId);
	PendingActorRequests.Add(CreateEntityRequestId, Channel);

	return CreateEntityRequestId;
}

Worker_ComponentData USpatialSender::CreateLevelComponentData(AActor* Actor)
{
	UWorld* ActorWorld = Actor->GetTypedOuter<UWorld>();
	if (ActorWorld != NetDriver->World)
	{
		const uint32 ComponentId = ClassInfoManager->SchemaDatabase->GetComponentIdFromLevelPath(ActorWorld->GetOuter()->GetPathName());
		if (ComponentId != SpatialConstants::INVALID_COMPONENT_ID)
		{
			return ComponentFactory::CreateEmptyComponentData(ComponentId);
		}
		else
		{
			UE_LOG(LogSpatialSender, Error, TEXT("Could not find Streaming Level Component for Level %s, processing Actor %s. Have you generated schema?"),
				*ActorWorld->GetOuter()->GetPathName(), *Actor->GetPathName());
		}
	}

	return ComponentFactory::CreateEmptyComponentData(SpatialConstants::NOT_STREAMED_COMPONENT_ID);
}

void USpatialSender::SendComponentUpdates(UObject* Object, const FClassInfo& Info, USpatialActorChannel* Channel, const FRepChangeState* RepChanges, const FHandoverChangeState* HandoverChanges)
{
	SCOPE_CYCLE_COUNTER(STAT_SpatialSenderSendComponentUpdates);
	Worker_EntityId EntityId = Channel->GetEntityId();

	UE_LOG(LogSpatialSender, Verbose, TEXT("Sending component update (object: %s, entity: %lld)"), *Object->GetName(), EntityId);

	FUnresolvedObjectsMap UnresolvedObjectsMap;
	FUnresolvedObjectsMap HandoverUnresolvedObjectsMap;
	ComponentFactory UpdateFactory(UnresolvedObjectsMap, HandoverUnresolvedObjectsMap, Channel->GetInterestDirty(), NetDriver);

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
			UE_LOG(LogSpatialSender, Verbose, TEXT("Trying to send component update but don't have authority! Update will be queued and sent when authority gained. Component Id: %d, entity: %lld"), Update.component_id, EntityId);

			// This is a temporary fix. A task to improve this has been created: UNR-955
			// It may be the case that upon resolving a component, we do not have authority to send the update. In this case, we queue the update, to send upon receiving authority.
			// Note: This will break in a multi-worker context, if we try to create an entity that we don't intend to have authority over. For this reason, this fix is only temporary.
			TArray<Worker_ComponentUpdate>& UpdatesQueuedUntilAuthority = UpdatesQueuedUntilAuthorityMap.FindOrAdd(EntityId);
			UpdatesQueuedUntilAuthority.Add(Update);
			continue;
		}

		Connection->SendComponentUpdate(EntityId, &Update);
	}
}

// Apply (and clean up) any updates queued, due to being sent previously when they didn't have authority.
void USpatialSender::ProcessUpdatesQueuedUntilAuthority(Worker_EntityId EntityId)
{
	if (TArray<Worker_ComponentUpdate>* UpdatesQueuedUntilAuthority = UpdatesQueuedUntilAuthorityMap.Find(EntityId))
	{
		for (Worker_ComponentUpdate& Update : *UpdatesQueuedUntilAuthority)
		{
			Connection->SendComponentUpdate(EntityId, &Update);
		}
		UpdatesQueuedUntilAuthorityMap.Remove(EntityId);
	}
}

void FillComponentInterests(const FClassInfo& Info, bool bNetOwned, TArray<Worker_InterestOverride>& ComponentInterest)
{
	if (Info.SchemaComponents[SCHEMA_OwnerOnly] != SpatialConstants::INVALID_COMPONENT_ID)
	{
		Worker_InterestOverride SingleClientInterest = { Info.SchemaComponents[SCHEMA_OwnerOnly], bNetOwned };
		ComponentInterest.Add(SingleClientInterest);
	}

	if (Info.SchemaComponents[SCHEMA_Handover] != SpatialConstants::INVALID_COMPONENT_ID)
	{
		Worker_InterestOverride HandoverInterest = { Info.SchemaComponents[SCHEMA_Handover], false };
		ComponentInterest.Add(HandoverInterest);
	}
}

TArray<Worker_InterestOverride> USpatialSender::CreateComponentInterest(AActor* Actor, bool bIsNetOwned)
{
	TArray<Worker_InterestOverride> ComponentInterest;

	const FClassInfo& ActorInfo = ClassInfoManager->GetOrCreateClassInfoByClass(Actor->GetClass());
	FillComponentInterests(ActorInfo, bIsNetOwned, ComponentInterest);

	for (auto& SubobjectInfoPair : ActorInfo.SubobjectInfo)
	{
		FClassInfo& SubobjectInfo = SubobjectInfoPair.Value.Get();
		FillComponentInterests(SubobjectInfo, bIsNetOwned, ComponentInterest);
	}

	ComponentInterest.Add({ SpatialConstants::CLIENT_RPC_ENDPOINT_COMPONENT_ID, bIsNetOwned });
	ComponentInterest.Add({ SpatialConstants::SERVER_RPC_ENDPOINT_COMPONENT_ID, bIsNetOwned });

	return ComponentInterest;
}

RPCPayload USpatialSender::CreateRPCPayloadFromParams(FPendingRPCParams& RPCParams)
{
	check(RPCParams.TargetObject.IsValid());
	const UObject* Object = RPCParams.TargetObject.Get();
	const FClassInfo& Info = ClassInfoManager->GetOrCreateClassInfoByClass(Object->GetClass());
	const FRPCInfo* RPCInfo = Info.RPCInfoMap.Find(RPCParams.Function);
	FUnrealObjectRef TargetObjectRef(PackageMap->GetUnrealObjectRefFromNetGUID(PackageMap->GetNetGUIDFromObject(Object)));
	check(TargetObjectRef != FUnrealObjectRef::UNRESOLVED_OBJECT_REF);

	TSet<TWeakObjectPtr<const UObject>> UnresolvedObjects;
	FSpatialNetBitWriter PayloadWriter = PackRPCDataToSpatialNetBitWriter(RPCParams.Function, RPCParams.Parameters.GetData(), RPCParams.ReliableRPCIndex, UnresolvedObjects);
	if (UnresolvedObjects.Num() > 0)
	{
		UE_LOG(LogSpatialSender, Warning, TEXT("Some RPC parameters for %s were not resolved."), *RPCParams.Function->GetName());
	}

	return RPCPayload(TargetObjectRef.Offset, RPCInfo->Index, TArray<uint8>(PayloadWriter.GetData(), PayloadWriter.GetNumBytes()));
}

void USpatialSender::SendComponentInterest(AActor* Actor, Worker_EntityId EntityId, bool bNetOwned)
{
	check(!NetDriver->IsServer());

	NetDriver->Connection->SendComponentInterest(EntityId, CreateComponentInterest(Actor, bNetOwned));
}

void USpatialSender::SendPositionUpdate(Worker_EntityId EntityId, const FVector& Location)
{
#if !UE_BUILD_SHIPPING
	if (!NetDriver->StaticComponentView->HasAuthority(EntityId, SpatialConstants::POSITION_COMPONENT_ID))
	{
		UE_LOG(LogSpatialSender, Verbose, TEXT("Trying to send Position component update but don't have authority! Update will not be sent. Entity: %lld"), EntityId);
		return;
	}
#endif

	Worker_ComponentUpdate Update = Position::CreatePositionUpdate(Coordinates::FromFVector(Location));
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

	if (USpatialActorChannel* Channel = NetDriver->GetOrCreateSpatialActorChannel(TargetObject))
	{
		if (Channel->bCreatingNewEntity)
		{
			if (Params->Function->HasAnyFunctionFlags(FUNC_NetMulticast))
			{
				// TODO: UNR-1437 - Add Support for Multicast RPCs on Entity Creation
				UE_LOG(LogSpatialSender, Warning, TEXT("NetMulticast RPC %s triggered on Object %s too close to initial creation."), *Params->Function->GetName(), *TargetObject->GetName());
			}
			check(NetDriver->IsServer());
			check(Params->Function->HasAnyFunctionFlags(FUNC_NetClient | FUNC_NetMulticast));
			check(PackageMap->GetUnrealObjectRefFromObject(TargetObject) != FUnrealObjectRef::UNRESOLVED_OBJECT_REF);

			// This is where we'll serialize this RPC and queue it to be added on entity creation
			OutgoingOnCreateEntityRPCs.FindOrAdd(TargetObject).RPCs.Add(CreateRPCPayloadFromParams(*Params));
			return;
		}
	}
	else
	{
		UE_LOG(LogSpatialSender, Warning, TEXT("Failed to create an Actor Channel for %s."), *TargetObject->GetName());
	}

	if (PackageMap->GetUnrealObjectRefFromObject(TargetObject) == FUnrealObjectRef::UNRESOLVED_OBJECT_REF)
	{
		// This could potentially occur for singletons in multi-worker scenario
		UE_LOG(LogSpatialSender, Verbose, TEXT("Trying to send RPC %s on unresolved Actor %s."), *Params->Function->GetName(), *TargetObject->GetName());
		QueueOutgoingRPC(TargetObject, Params);
		return;
	}

	const FClassInfo& Info = ClassInfoManager->GetOrCreateClassInfoByObject(TargetObject);
	const FRPCInfo* RPCInfo = Info.RPCInfoMap.Find(Params->Function);

	// We potentially have a parent function and need to find the child function.
	// This exists as it's possible in blueprints to explicitly call the parent function.
	if (RPCInfo == nullptr)
	{
		for (auto It = Info.RPCInfoMap.CreateConstIterator(); It; ++It)
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
	case SCHEMA_ClientReliableRPC:
	case SCHEMA_ServerReliableRPC:
	case SCHEMA_CrossServerRPC:
	{
		Worker_ComponentId ComponentId = RPCInfo->Type == SCHEMA_ClientReliableRPC ? SpatialConstants::CLIENT_RPC_ENDPOINT_COMPONENT_ID : SpatialConstants::SERVER_RPC_ENDPOINT_COMPONENT_ID;

		TArray<uint8> Payload;
		Worker_CommandRequest CommandRequest = CreateRPCCommandRequest(TargetObject, Params->Function, Params->Parameters.GetData(), ComponentId, RPCInfo->Index, EntityId, UnresolvedObject, Payload, Params->ReliableRPCIndex);

		if (!UnresolvedObject)
		{
			check(EntityId != SpatialConstants::INVALID_ENTITY_ID);
			Worker_RequestId RequestId = Connection->SendCommandRequest(EntityId, &CommandRequest, SpatialConstants::UNREAL_RPC_ENDPOINT_COMMAND_ID);

			if (Params->Function->HasAnyFunctionFlags(FUNC_NetReliable))
			{
				UE_LOG(LogSpatialSender, Verbose, TEXT("Sending reliable command request (entity: %lld, component: %d, function: %s, attempt: 1)"),
					EntityId, CommandRequest.component_id, *Params->Function->GetName());
				Receiver->AddPendingReliableRPC(RequestId, MakeShared<FReliableRPCForRetry>(TargetObject, Params->Function, ComponentId, RPCInfo->Index, MoveTemp(Payload), Params->RetryIndex));
			}
			else
			{
				UE_LOG(LogSpatialSender, Verbose, TEXT("Sending unreliable command request (entity: %lld, component: %d, function: %s)"),
					EntityId, CommandRequest.component_id, *Params->Function->GetName());
			}
		}
		break;
	}
	case SCHEMA_NetMulticastRPC:
	case SCHEMA_ClientUnreliableRPC:
	case SCHEMA_ServerUnreliableRPC:
	{
		Worker_ComponentId ComponentId;
		if (RPCInfo->Type == SCHEMA_ClientUnreliableRPC)
		{
			ComponentId = SpatialConstants::SERVER_RPC_ENDPOINT_COMPONENT_ID;
		}
		else if (RPCInfo->Type == SCHEMA_ServerUnreliableRPC)
		{
			ComponentId = SpatialConstants::CLIENT_RPC_ENDPOINT_COMPONENT_ID;
		}
		else
		{
			ComponentId = SpatialConstants::NETMULTICAST_RPCS_COMPONENT_ID;
		}

		Worker_ComponentUpdate ComponentUpdate = CreateUnreliableRPCUpdate(TargetObject, Params->Function, Params->Parameters.GetData(), ComponentId, RPCInfo->Index, EntityId, UnresolvedObject);

		if (!UnresolvedObject)
		{
			check(EntityId != SpatialConstants::INVALID_ENTITY_ID);

			if (!NetDriver->StaticComponentView->HasAuthority(EntityId, ComponentUpdate.component_id))
			{
				UE_LOG(LogSpatialSender, Verbose, TEXT("Trying to send RPC %s component update but don't have authority! Update will not be sent. Entity: %lld"), *Params->Function->GetName(), EntityId);
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

void USpatialSender::EnqueueRetryRPC(TSharedRef<FReliableRPCForRetry> RetryRPC)
{
	RetryRPCs.Add(RetryRPC);
}

void USpatialSender::FlushRetryRPCs()
{
	// Retried RPCs are sorted by their index.
	RetryRPCs.Sort([](const TSharedRef<FReliableRPCForRetry>& A, const TSharedRef<FReliableRPCForRetry>& B) { return A->RetryIndex < B->RetryIndex; });
	for (auto& RetryRPC : RetryRPCs)
	{
		RetryReliableRPC(RetryRPC);
	}
	RetryRPCs.Empty();
}

void USpatialSender::RetryReliableRPC(TSharedRef<FReliableRPCForRetry> RetryRPC)
{
	if (!RetryRPC->TargetObject.IsValid())
	{
		// Target object was destroyed before the RPC could be (re)sent
		return;
	}

	UObject* TargetObject = RetryRPC->TargetObject.Get();
	FUnrealObjectRef TargetObjectRef = PackageMap->GetUnrealObjectRefFromObject(TargetObject);
	if (TargetObjectRef == FUnrealObjectRef::UNRESOLVED_OBJECT_REF)
	{
		UE_LOG(LogSpatialSender, Warning, TEXT("Actor %s got unresolved (?) before RPC %s could be retried. This RPC will not be sent."), *TargetObject->GetName(), *RetryRPC->Function->GetName());
		return;
	}

	Worker_CommandRequest CommandRequest = CreateRetryRPCCommandRequest(*RetryRPC, TargetObjectRef.Offset);
	Worker_RequestId RequestId = Connection->SendCommandRequest(TargetObjectRef.Entity, &CommandRequest, SpatialConstants::UNREAL_RPC_ENDPOINT_COMMAND_ID);

	// The number of attempts is used to determine the delay in case the command times out and we need to resend it.
	RetryRPC->Attempts++;
	UE_LOG(LogSpatialSender, Verbose, TEXT("Sending reliable command request (entity: %lld, component: %d, function: %s, attempt: %d)"),
		TargetObjectRef.Entity, RetryRPC->ComponentId, *RetryRPC->Function->GetName(), RetryRPC->Attempts);
	Receiver->AddPendingReliableRPC(RequestId, RetryRPC);
}

void USpatialSender::RegisterChannelForPositionUpdate(USpatialActorChannel* Channel)
{
	ChannelsToUpdatePosition.Add(Channel);
}

void USpatialSender::ProcessPositionUpdates()
{
	for (auto& Channel : ChannelsToUpdatePosition)
	{
		if (Channel.IsValid())
		{
			Channel->UpdateSpatialPosition();
		}
	}

	ChannelsToUpdatePosition.Empty();
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

void USpatialSender::SendRequestToClearRPCsOnEntityCreation(Worker_EntityId EntityId)
{
	Worker_CommandRequest CommandRequest = RPCsOnEntityCreation::CreateClearFieldsCommandRequest();
	NetDriver->Connection->SendCommandRequest(EntityId, &CommandRequest, SpatialConstants::CLEAR_RPCS_ON_ENTITY_CREATION);
}

void USpatialSender::ClearRPCsOnEntityCreation(Worker_EntityId EntityId)
{
	check(NetDriver->IsServer());
	Worker_ComponentUpdate Update = RPCsOnEntityCreation::CreateClearFieldsUpdate();
	NetDriver->Connection->SendComponentUpdate(EntityId, &Update);
}

void USpatialSender::ResetOutgoingUpdate(USpatialActorChannel* DependentChannel, UObject* ReplicatedObject, int16 Handle, bool bIsHandover)
{
	SCOPE_CYCLE_COUNTER(STAT_SpatialSenderResetOutgoingUpdate);

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

	// Remove any references to the unresolved objects.
	// Since these are not dereferenced before removing, it is safe to not check whether the unresolved object is still valid.
	for (TWeakObjectPtr<const UObject>& UnresolvedObject : *Unresolved)
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

void USpatialSender::QueueOutgoingUpdate(USpatialActorChannel* DependentChannel, UObject* ReplicatedObject, int16 Handle, const TSet<TWeakObjectPtr<const UObject>>& UnresolvedObjects, bool bIsHandover)
{
	SCOPE_CYCLE_COUNTER(STAT_SpatialSenderQueueOutgoingUpdate);
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
		// It is expected that this will never be reached. We should never have added an invalid object as an unresolved reference.
		// Check the ComponentFactory.cpp should this ever be triggered.
		checkf(UnresolvedObject.IsValid(), TEXT("Invalid UnresolvedObject passed in to USpatialSender::QueueOutgoingUpdate"));

		FHandleToUnresolved& AnotherHandleToUnresolved = ObjectToUnresolved.FindOrAdd(UnresolvedObject).FindOrAdd(ChannelObjectPair);
		check(!AnotherHandleToUnresolved.Find(Handle));
		AnotherHandleToUnresolved.Add(Handle, Unresolved);

		// Following up on the previous log: listing the unresolved objects
		UE_LOG(LogSpatialSender, Log, TEXT("- %s"), *UnresolvedObject->GetName());

	}
}

void USpatialSender::QueueOutgoingRPC(const UObject* UnresolvedObject, TSharedRef<FPendingRPCParams> Params)
{
	check(UnresolvedObject);
	UE_LOG(LogSpatialSender, Log, TEXT("Added pending outgoing RPC depending on object: %s, target: %s, function: %s"), *UnresolvedObject->GetName(), *Params->TargetObject->GetName(), *Params->Function->GetName());
	OutgoingRPCs.FindOrAdd(UnresolvedObject).Add(Params);
}

FSpatialNetBitWriter USpatialSender::PackRPCDataToSpatialNetBitWriter(UFunction* Function, void* Parameters, int ReliableRPCId, TSet<TWeakObjectPtr<const UObject>>& UnresolvedObjects) const
{
	FSpatialNetBitWriter PayloadWriter(PackageMap, UnresolvedObjects);

	if (GetDefault<USpatialGDKSettings>()->bCheckRPCOrder)
	{
		if (Function->HasAnyFunctionFlags(FUNC_NetReliable) && !Function->HasAnyFunctionFlags(FUNC_NetMulticast))
		{
			PayloadWriter << ReliableRPCId;
		}
	}

	TSharedPtr<FRepLayout> RepLayout = NetDriver->GetFunctionRepLayout(Function);
	RepLayout_SendPropertiesForRPC(*RepLayout, PayloadWriter, Parameters);

	return PayloadWriter;
}

Worker_CommandRequest USpatialSender::CreateRPCCommandRequest(UObject* TargetObject, UFunction* Function, void* Parameters, Worker_ComponentId ComponentId, Schema_FieldId CommandIndex, Worker_EntityId& OutEntityId, const UObject*& OutUnresolvedObject, TArray<uint8>& OutPayload, int ReliableRPCId)
{
	Worker_CommandRequest CommandRequest = {};
	CommandRequest.component_id = ComponentId;
	CommandRequest.schema_type = Schema_CreateCommandRequest(ComponentId, SpatialConstants::UNREAL_RPC_ENDPOINT_COMMAND_ID);
	Schema_Object* RequestObject = Schema_GetCommandRequestObject(CommandRequest.schema_type);

	FUnrealObjectRef TargetObjectRef(PackageMap->GetUnrealObjectRefFromNetGUID(PackageMap->GetNetGUIDFromObject(TargetObject)));
	if (TargetObjectRef == FUnrealObjectRef::UNRESOLVED_OBJECT_REF)
	{
		OutUnresolvedObject = TargetObject;
		Schema_DestroyCommandRequest(CommandRequest.schema_type);
		return CommandRequest;
	}

	OutEntityId = TargetObjectRef.Entity;

	TSet<TWeakObjectPtr<const UObject>> UnresolvedObjects;
	FSpatialNetBitWriter PayloadWriter = PackRPCDataToSpatialNetBitWriter(Function, Parameters, ReliableRPCId, UnresolvedObjects);

	for (TWeakObjectPtr<const UObject> Object : UnresolvedObjects)
	{
		if (Object.IsValid())
		{
			// Take the first unresolved object
			OutUnresolvedObject = Object.Get();
			Schema_DestroyCommandRequest(CommandRequest.schema_type);
			return CommandRequest;
		}
	}

	RPCPayload::WriteToSchemaObject(RequestObject, TargetObjectRef.Offset, CommandIndex, PayloadWriter.GetData(), PayloadWriter.GetNumBytes());
	// Save payload in case we need to retry later.
	OutPayload = TArray<uint8>(PayloadWriter.GetData(), PayloadWriter.GetNumBytes());

	return CommandRequest;
}

Worker_CommandRequest USpatialSender::CreateRetryRPCCommandRequest(const FReliableRPCForRetry& RPC, uint32 TargetObjectOffset)
{
	Worker_CommandRequest CommandRequest = {};
	CommandRequest.component_id = RPC.ComponentId;
	CommandRequest.schema_type = Schema_CreateCommandRequest(RPC.ComponentId, SpatialConstants::UNREAL_RPC_ENDPOINT_COMMAND_ID);
	Schema_Object* RequestObject = Schema_GetCommandRequestObject(CommandRequest.schema_type);

	RPCPayload::WriteToSchemaObject(RequestObject, TargetObjectOffset, RPC.RPCIndex, RPC.Payload.GetData(), RPC.Payload.Num());

	return CommandRequest;
}

Worker_ComponentUpdate USpatialSender::CreateUnreliableRPCUpdate(UObject* TargetObject, UFunction* Function, void* Parameters, Worker_ComponentId ComponentId, Schema_FieldId EventIndex, Worker_EntityId& OutEntityId, const UObject*& OutUnresolvedObject)
{
	Worker_ComponentUpdate ComponentUpdate = {};

	ComponentUpdate.component_id = ComponentId;
	ComponentUpdate.schema_type = Schema_CreateComponentUpdate(ComponentId);
	Schema_Object* EventsObject = Schema_GetComponentUpdateEvents(ComponentUpdate.schema_type);
	Schema_Object* EventData = Schema_AddObject(EventsObject, SpatialConstants::UNREAL_RPC_ENDPOINT_EVENT_ID);

	FUnrealObjectRef TargetObjectRef(PackageMap->GetUnrealObjectRefFromNetGUID(PackageMap->GetNetGUIDFromObject(TargetObject)));
	if (TargetObjectRef == FUnrealObjectRef::UNRESOLVED_OBJECT_REF)
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
	}

	RPCPayload::WriteToSchemaObject(EventData, TargetObjectRef.Offset, EventIndex, PayloadWriter.GetData(), PayloadWriter.GetNumBytes());

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

		const FClassInfo& Info = ClassInfoManager->GetOrCreateClassInfoByObject(ReplicatingObject);

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
			UE_LOG(LogSpatialSender, Verbose, TEXT("Resolving outgoing RPC depending on object: %s, target: %s, function: %s"), *Object->GetName(), *RPCParams->TargetObject->GetName(), *RPCParams->Function->GetName());
			SendRPC(RPCParams);
		}
		OutgoingRPCs.Remove(Object);
	}
}

// Authority over the ClientRPC Schema component is dictated by the owning connection of a client.
// This function updates the authority of that component as the owning connection can change.
bool USpatialSender::UpdateEntityACLs(Worker_EntityId EntityId, const FString& OwnerWorkerAttribute)
{
	EntityAcl* EntityACL = StaticComponentView->GetComponentData<EntityAcl>(EntityId);

	if (EntityACL == nullptr)
	{
		return false;
	}

	if (!NetDriver->StaticComponentView->HasAuthority(EntityId, SpatialConstants::ENTITY_ACL_COMPONENT_ID))
	{
		UE_LOG(LogSpatialSender, Warning, TEXT("Trying to update EntityACL but don't have authority! Update will not be sent. Entity: %lld"), EntityId);
		return false;
	}

	WorkerAttributeSet OwningClientAttribute = { OwnerWorkerAttribute };
	WorkerRequirementSet OwningClientOnly = { OwningClientAttribute };

	EntityACL->ComponentWriteAcl.Add(SpatialConstants::CLIENT_RPC_ENDPOINT_COMPONENT_ID, OwningClientOnly);
	Worker_ComponentUpdate Update = EntityACL->CreateEntityAclUpdate();

	Connection->SendComponentUpdate(EntityId, &Update);
	return true;
}

void USpatialSender::UpdateInterestComponent(AActor* Actor)
{
	InterestFactory InterestUpdateFactory(Actor, ClassInfoManager->GetOrCreateClassInfoByObject(Actor), NetDriver);
	Worker_ComponentUpdate Update = InterestUpdateFactory.CreateInterestUpdate();

	Worker_EntityId EntityId = PackageMap->GetEntityIdFromObject(Actor);
	Connection->SendComponentUpdate(EntityId, &Update);
}
