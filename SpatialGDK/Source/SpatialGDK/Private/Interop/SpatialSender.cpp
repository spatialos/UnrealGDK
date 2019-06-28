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
#include "Schema/ClientRPCEndpoint.h"
#include "Schema/Heartbeat.h"
#include "Schema/Interest.h"
#include "Schema/RPCPayload.h"
#include "Schema/ServerRPCEndpoint.h"
#include "Schema/Singleton.h"
#include "Schema/SpawnData.h"
#include "Schema/StandardLibrary.h"
#include "Schema/UnrealMetadata.h"
#include "SpatialConstants.h"
#include "Utils/ComponentFactory.h"
#include "Utils/InterestFactory.h"
#include "Utils/RepLayoutUtils.h"
#include "Utils/SpatialActorUtils.h"
#include "Utils/SpatialMetrics.h"

DEFINE_LOG_CATEGORY(LogSpatialSender);

using namespace SpatialGDK;

DECLARE_CYCLE_STAT(TEXT("SendComponentUpdates"), STAT_SpatialSenderSendComponentUpdates, STATGROUP_SpatialNet);
DECLARE_CYCLE_STAT(TEXT("ResetOutgoingUpdate"), STAT_SpatialSenderResetOutgoingUpdate, STATGROUP_SpatialNet);
DECLARE_CYCLE_STAT(TEXT("QueueOutgoingUpdate"), STAT_SpatialSenderQueueOutgoingUpdate, STATGROUP_SpatialNet);

FReliableRPCForRetry::FReliableRPCForRetry(UObject* InTargetObject, UFunction* InFunction, Worker_ComponentId InComponentId, Schema_FieldId InRPCIndex, const TArray<uint8>& InPayload, int InRetryIndex)
	: TargetObject(InTargetObject)
	, Function(InFunction)
	, ComponentId(InComponentId)
	, RPCIndex(InRPCIndex)
	, Payload(InPayload)
	, Attempts(1)
	, RetryIndex(InRetryIndex)
{
}

FPendingUnreliableRPC::FPendingUnreliableRPC(FPendingUnreliableRPC&& Other)
	: Offset(Other.Offset)
	, Index(Other.Index)
	, Data(MoveTemp(Other.Data))
	, Entity(Other.Entity)
{
}

void USpatialSender::Init(USpatialNetDriver* InNetDriver, FTimerManager* InTimerManager)
{
	NetDriver = InNetDriver;
	StaticComponentView = InNetDriver->StaticComponentView;
	Connection = InNetDriver->Connection;
	Receiver = InNetDriver->Receiver;
	PackageMap = InNetDriver->PackageMap;
	ClassInfoManager = InNetDriver->ClassInfoManager;
	TimerManager = InTimerManager;
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
#if !UE_BUILD_SHIPPING
		ComponentWriteAcl.Add(SpatialConstants::DEBUG_METRICS_COMPONENT_ID, ServersOnly);
#endif // !UE_BUILD_SHIPPING
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
#if !UE_BUILD_SHIPPING
		ComponentDatas.Add(ComponentFactory::CreateEmptyComponentData(SpatialConstants::DEBUG_METRICS_COMPONENT_ID));
#endif // !UE_BUILD_SHIPPING
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

	ComponentDatas.Add(ClientRPCEndpoint().CreateRPCEndpointData());
	ComponentDatas.Add(ServerRPCEndpoint().CreateRPCEndpointData());
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

// Creates an entity authoritative on this server worker, ensuring it will be able to receive updates for the GSM.
void USpatialSender::CreateServerWorkerEntity(int AttemptCounter)
{
	const WorkerRequirementSet WorkerIdPermission{ { FString::Format(TEXT("workerId:{0}"), { Connection->GetWorkerId() }) } };

	WriteAclMap ComponentWriteAcl;
	ComponentWriteAcl.Add(SpatialConstants::POSITION_COMPONENT_ID, WorkerIdPermission);
	ComponentWriteAcl.Add(SpatialConstants::METADATA_COMPONENT_ID, WorkerIdPermission);
	ComponentWriteAcl.Add(SpatialConstants::ENTITY_ACL_COMPONENT_ID, WorkerIdPermission);
	ComponentWriteAcl.Add(SpatialConstants::INTEREST_COMPONENT_ID, WorkerIdPermission);

	QueryConstraint Constraint;
	// Ensure server worker receives the GSM entity
	Constraint.EntityIdConstraint = SpatialConstants::INITIAL_GLOBAL_STATE_MANAGER_ENTITY_ID;

	Query Query;
	Query.Constraint = Constraint;
	Query.FullSnapshotResult = true;

	ComponentInterest Queries;
	Queries.Queries.Add(Query);

	Interest Interest;
	Interest.ComponentInterestMap.Add(SpatialConstants::POSITION_COMPONENT_ID, Queries);

	TArray<Worker_ComponentData> Components;
	Components.Add(Position().CreatePositionData());
	Components.Add(Metadata(FString::Format(TEXT("WorkerEntity:{0}"), { Connection->GetWorkerId() })).CreateMetadataData());
	Components.Add(EntityAcl(WorkerIdPermission, ComponentWriteAcl).CreateEntityAclData());
	Components.Add(Interest.CreateInterestData());

	Worker_RequestId RequestId = Connection->SendCreateEntityRequest(MoveTemp(Components), nullptr);

	CreateEntityDelegate OnCreateWorkerEntityResponse;
	OnCreateWorkerEntityResponse.BindLambda([this, AttemptCounter](const Worker_CreateEntityResponseOp& Op)
	{
		if (Op.status_code == WORKER_STATUS_CODE_SUCCESS)
		{
			return;
		}

		if (Op.status_code != WORKER_STATUS_CODE_TIMEOUT)
		{
			UE_LOG(LogSpatialOSNetDriver, Error, TEXT("Worker entity creation request failed: \"%s\""),
				UTF8_TO_TCHAR(Op.message));
			return;
		}

		if (AttemptCounter == SpatialConstants::MAX_NUMBER_COMMAND_ATTEMPTS)
		{
			UE_LOG(LogSpatialOSNetDriver, Error, TEXT("Worker entity creation request timed out too many times. (%u attempts)"),
				SpatialConstants::MAX_NUMBER_COMMAND_ATTEMPTS);
			return;
		}

		UE_LOG(LogSpatialOSNetDriver, Warning, TEXT("Worker entity creation request timed out and will retry."));
		FTimerHandle RetryTimer;
		TWeakObjectPtr<USpatialSender> WeakThis(this);
		TimerManager->SetTimer(RetryTimer, [WeakThis, AttemptCounter]()
		{
			if (USpatialSender* Sender = WeakThis.Get())
			{
				Sender->CreateServerWorkerEntity(AttemptCounter + 1);
			}
		}, SpatialConstants::GetCommandRetryWaitTimeSeconds(AttemptCounter), false);
	});
	Receiver->AddCreateEntityDelegate(RequestId, OnCreateWorkerEntityResponse);
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

void USpatialSender::FlushPackedUnreliableRPCs()
{
	if (UnreliableRPCs.Num() == 0)
	{
		return;
	}

	// TODO: This could be further optimized for the case when there's only 1 RPC to be sent during this frame
	// by sending it directly to the corresponding entity, without including the EntityId in the payload - UNR-1563.
	for (const auto& It : UnreliableRPCs)
	{
		Worker_EntityId PlayerControllerEntityId = It.Key;
		const TArray<FPendingUnreliableRPC>& PendingRPCArray = It.Value;

		Worker_ComponentUpdate ComponentUpdate = {};

		Worker_ComponentId ComponentId = NetDriver->IsServer() ? SpatialConstants::SERVER_RPC_ENDPOINT_COMPONENT_ID : SpatialConstants::CLIENT_RPC_ENDPOINT_COMPONENT_ID;
		ComponentUpdate.component_id = ComponentId;
		ComponentUpdate.schema_type = Schema_CreateComponentUpdate(ComponentId);
		Schema_Object* EventsObject = Schema_GetComponentUpdateEvents(ComponentUpdate.schema_type);

		for (const FPendingUnreliableRPC& RPC : PendingRPCArray)
		{
			Schema_Object* EventData = Schema_AddObject(EventsObject, SpatialConstants::UNREAL_RPC_ENDPOINT_PACKED_EVENT_ID);

			Schema_AddUint32(EventData, SpatialConstants::UNREAL_RPC_PAYLOAD_OFFSET_ID, RPC.Offset);
			Schema_AddUint32(EventData, SpatialConstants::UNREAL_RPC_PAYLOAD_RPC_INDEX_ID, RPC.Index);
			SpatialGDK::AddBytesToSchema(EventData, SpatialConstants::UNREAL_RPC_PAYLOAD_RPC_PAYLOAD_ID, RPC.Data.GetData(), RPC.Data.Num());
			Schema_AddEntityId(EventData, SpatialConstants::UNREAL_PACKED_RPC_PAYLOAD_ENTITY_ID, RPC.Entity);
		}

		Connection->SendComponentUpdate(PlayerControllerEntityId, &ComponentUpdate);
	}

	UnreliableRPCs.Empty();
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

RPCPayload USpatialSender::CreateRPCPayloadFromParams(UObject* TargetObject, UFunction* Function, int ReliableRPCIndex, void* Params, TSet<TWeakObjectPtr<const UObject>>& UnresolvedObjects)
{
	const FRPCInfo RPCInfo = ClassInfoManager->GetRPCInfo(TargetObject, Function);
	FUnrealObjectRef TargetObjectRef(PackageMap->GetUnrealObjectRefFromNetGUID(PackageMap->GetNetGUIDFromObject(TargetObject)));
	if(TargetObjectRef == FUnrealObjectRef::UNRESOLVED_OBJECT_REF)
	{
		UnresolvedObjects.Add(TargetObject);
	}

	FSpatialNetBitWriter PayloadWriter = PackRPCDataToSpatialNetBitWriter(Function, Params, ReliableRPCIndex, UnresolvedObjects);
	if (UnresolvedObjects.Num() > 0)
	{
		UE_LOG(LogSpatialSender, Warning, TEXT("Some RPC parameters for %s were not resolved."), *Function->GetName());
	}

	return RPCPayload(TargetObjectRef.Offset, RPCInfo.Index, TArray<uint8>(PayloadWriter.GetData(), PayloadWriter.GetNumBytes()));
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

bool USpatialSender::SendRPC(FPendingRPCParamsPtr Params)
{
	if (!Params->TargetObject.IsValid())
	{
		// Target object was destroyed before the RPC could be (re)sent
		return false;
	}
	UObject* TargetObject = Params->TargetObject.Get();

	if (USpatialActorChannel* Channel = NetDriver->GetOrCreateSpatialActorChannel(TargetObject))
	{
		if (Channel->bCreatingNewEntity)
		{
			if (Params->Function->HasAnyFunctionFlags(FUNC_NetMulticast))
			{
				// TODO(Alex): Should we queue and send multicast as usual instead of trying to pack into RPCOnEntityCreation?
				// TODO: UNR-1437 - Add Support for Multicast RPCs on Entity Creation
				UE_LOG(LogSpatialSender, Warning, TEXT("NetMulticast RPC %s triggered on Object %s too close to initial creation."), *Params->Function->GetName(), *TargetObject->GetName());
			}
			check(NetDriver->IsServer());
			check(Params->Function->HasAnyFunctionFlags(FUNC_NetClient | FUNC_NetMulticast));
			check(PackageMap->GetUnrealObjectRefFromObject(TargetObject) != FUnrealObjectRef::UNRESOLVED_OBJECT_REF);

			// This is where we'll serialize this RPC and queue it to be added on entity creation
			TSet<TWeakObjectPtr<const UObject>> UnresolvedObjects;
			OutgoingOnCreateEntityRPCs.FindOrAdd(TargetObject).RPCs.Add(Params->Payload);
			return true;
		}
	}
	else
	{
		UE_LOG(LogSpatialSender, Warning, TEXT("Failed to create an Actor Channel for %s."), *TargetObject->GetName());
	}

	const FRPCInfo RPCInfo = ClassInfoManager->GetRPCInfo(TargetObject, Params->Function);

	Worker_EntityId EntityId = SpatialConstants::INVALID_ENTITY_ID;

	switch (RPCInfo.Type)
	{
	case SCHEMA_CrossServerRPC:
	{
		Worker_ComponentId ComponentId = SchemaComponentTypeToWorkerComponentId(RPCInfo.Type);

		const UObject* UnresolvedObject = nullptr;
		Worker_CommandRequest CommandRequest = CreateRPCCommandRequest(TargetObject, Params->Payload, ComponentId, RPCInfo.Index, EntityId, UnresolvedObject);

		if (UnresolvedObject)
		{
			return false;
		}

		check(EntityId != SpatialConstants::INVALID_ENTITY_ID);
		Worker_RequestId RequestId = Connection->SendCommandRequest(EntityId, &CommandRequest, SpatialConstants::UNREAL_RPC_ENDPOINT_COMMAND_ID);

#if !UE_BUILD_SHIPPING
		NetDriver->SpatialMetrics->TrackSentRPC(Params->Function, RPCInfo.Type, Params->Payload.PayloadData.Num());
#endif // !UE_BUILD_SHIPPING		

		if (Params->Function->HasAnyFunctionFlags(FUNC_NetReliable))
		{
			UE_LOG(LogSpatialSender, Verbose, TEXT("Sending reliable command request (entity: %lld, component: %d, function: %s, attempt: 1)"),
				EntityId, CommandRequest.component_id, *Params->Function->GetName());
			Receiver->AddPendingReliableRPC(RequestId, MakeShared<FReliableRPCForRetry>(TargetObject, Params->Function, ComponentId, RPCInfo.Index, Params->Payload.PayloadData, 0));
		}
		else
		{
			UE_LOG(LogSpatialSender, Verbose, TEXT("Sending unreliable command request (entity: %lld, component: %d, function: %s)"),
				EntityId, CommandRequest.component_id, *Params->Function->GetName());
		}

		return true;
	}
	case SCHEMA_NetMulticastRPC:
	case SCHEMA_ClientReliableRPC:
	case SCHEMA_ServerReliableRPC:
	case SCHEMA_ClientUnreliableRPC:
	case SCHEMA_ServerUnreliableRPC:
	{
		FUnrealObjectRef TargetObjectRef = PackageMap->GetUnrealObjectRefFromObject(TargetObject);
		if (TargetObjectRef == FUnrealObjectRef::UNRESOLVED_OBJECT_REF)
		{
			return false;
		}

		EntityId = TargetObjectRef.Entity;
		check(EntityId != SpatialConstants::INVALID_ENTITY_ID);

		Worker_ComponentId ComponentId = SchemaComponentTypeToWorkerComponentId(RPCInfo.Type);

		if (GetDefault<USpatialGDKSettings>()->bPackRPCs
			&& RPCInfo.Type != SCHEMA_NetMulticastRPC)
		{
			const UObject* UnresolvedObject = nullptr;
			AddPendingUnreliableRPC(TargetObject, Params, ComponentId, RPCInfo.Index, UnresolvedObject);
#if !UE_BUILD_SHIPPING
			NetDriver->SpatialMetrics->TrackSentRPC(Params->Function, RPCInfo.Type, Params->Payload.PayloadData.Num());
#endif // !UE_BUILD_SHIPPING
			return true;
		}
		else
		{
			if (!NetDriver->StaticComponentView->HasAuthority(EntityId, ComponentId))
			{
				return false;
			}

			if (RPCInfo.Type != SCHEMA_NetMulticastRPC && !NetDriver->IsEntityListening(EntityId))
			{
				// If the Entity endpoint is not yet ready to receive RPCs -
				// treat the corresponding object as unresolved and queue RPC
				// However, it doesn't matter in case of Multicast
				return false;
			}

			const UObject* UnresolvedParameter = nullptr;
			Worker_ComponentUpdate ComponentUpdate = CreateRPCEventUpdate(TargetObject, Params->Payload, ComponentId, RPCInfo.Index, UnresolvedParameter);

			if (UnresolvedParameter)
			{
				return false;
			}

			Connection->SendComponentUpdate(EntityId, &ComponentUpdate);
#if !UE_BUILD_SHIPPING
			NetDriver->SpatialMetrics->TrackSentRPC(Params->Function, RPCInfo.Type, Params->Payload.PayloadData.Num());
#endif // !UE_BUILD_SHIPPING
			return true;
		}
	}
	default:
		checkNoEntry();
		return false;
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

void USpatialSender::SendClientEndpointReadyUpdate(Worker_EntityId EntityId)
{
	ClientRPCEndpoint Endpoint;
	Endpoint.bReady = true;
	Worker_ComponentUpdate Update = Endpoint.CreateRPCEndpointUpdate();
	NetDriver->Connection->SendComponentUpdate(EntityId, &Update);
}

void USpatialSender::SendServerEndpointReadyUpdate(Worker_EntityId EntityId)
{
	ServerRPCEndpoint Endpoint;
	Endpoint.bReady = true;
	Worker_ComponentUpdate Update = Endpoint.CreateRPCEndpointUpdate();
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

void USpatialSender::QueueOutgoingRPC(FPendingRPCParamsPtr Params)
{
	if (!Params->TargetObject.IsValid())
	{
		// Target object was destroyed before the RPC could be (re)sent
		return;
	}
	UObject* TargetObject = Params->TargetObject.Get();
	const FRPCInfo RPCInfo = ClassInfoManager->GetRPCInfo(TargetObject, Params->Function);

	OutgoingRPCs.QueueRPC(Params, RPCInfo.Type);
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

Worker_CommandRequest USpatialSender::CreateRPCCommandRequest(UObject* TargetObject, const RPCPayload& Payload, Worker_ComponentId ComponentId, Schema_FieldId CommandIndex, Worker_EntityId& OutEntityId, const UObject*& OutUnresolvedObject)
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

	RPCPayload::WriteToSchemaObject(RequestObject, TargetObjectRef.Offset, CommandIndex, Payload.PayloadData.GetData(), Payload.PayloadData.Num());

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

Worker_ComponentUpdate USpatialSender::CreateRPCEventUpdate(UObject* TargetObject, const RPCPayload& Payload, Worker_ComponentId ComponentId, Schema_FieldId EventIndex, const UObject*& OutUnresolvedObject)
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

	RPCPayload::WriteToSchemaObject(EventData, Payload.Offset, Payload.Index, Payload.PayloadData.GetData(), Payload.PayloadData.Num());

	return ComponentUpdate;
}

void USpatialSender::AddPendingUnreliableRPC(UObject* TargetObject, FPendingRPCParamsPtr Parameters, Worker_ComponentId ComponentId, Schema_FieldId RPCIndex, const UObject*& OutUnresolvedObject)
{
	FUnrealObjectRef TargetObjectRef(PackageMap->GetUnrealObjectRefFromNetGUID(PackageMap->GetNetGUIDFromObject(TargetObject)));
	if (TargetObjectRef == FUnrealObjectRef::UNRESOLVED_OBJECT_REF)
	{
		OutUnresolvedObject = TargetObject;
		return;
	}

	AActor* TargetActor = Cast<AActor>(PackageMap->GetObjectFromEntityId(TargetObjectRef.Entity).Get());
	check(TargetActor != nullptr);
	UNetConnection* OwningConnection = TargetActor->GetNetConnection();
	if (OwningConnection == nullptr)
	{
		UE_LOG(LogSpatialSender, Warning, TEXT("AddPendingUnreliableRPC: No connection for object %s (RPC %s, actor %s, entity %lld)"),
			*TargetObject->GetName(), *Parameters->Function->GetName(), *TargetActor->GetName(), TargetObjectRef.Entity);
		return;
	}

	APlayerController* Controller = Cast<APlayerController>(OwningConnection->OwningActor);
	if (Controller == nullptr)
	{
		UE_LOG(LogSpatialSender, Warning, TEXT("AddPendingUnreliableRPC: Connection's owner is not a player controller for object %s (RPC %s, actor %s, entity %lld): connection owner %s"),
			*TargetObject->GetName(), *Parameters->Function->GetName(), *TargetActor->GetName(), TargetObjectRef.Entity, *OwningConnection->OwningActor->GetName());
		return;
	}

	FUnrealObjectRef ControllerObjectRef = PackageMap->GetUnrealObjectRefFromObject(Controller);
	if (ControllerObjectRef == FUnrealObjectRef::UNRESOLVED_OBJECT_REF)
	{
		OutUnresolvedObject = Controller;
		return;
	}

	TSet<TWeakObjectPtr<const UObject>> UnresolvedObjects;

	FPendingUnreliableRPC RPC;
	RPC.Offset = TargetObjectRef.Offset;
	RPC.Index = RPCIndex;
	RPC.Data.SetNumUninitialized(Parameters->Payload.PayloadData.Num());
	FMemory::Memcpy(RPC.Data.GetData(), Parameters->Payload.PayloadData.GetData(), Parameters->Payload.PayloadData.Num());
	RPC.Entity = TargetObjectRef.Entity;
	UnreliableRPCs.FindOrAdd(ControllerObjectRef.Entity).Emplace(MoveTemp(RPC));
}

void USpatialSender::SendCommandResponse(Worker_RequestId request_id, Worker_CommandResponse& Response)
{
	Connection->SendCommandResponse(request_id, &Response);
}

void USpatialSender::SendEmptyCommandResponse(Worker_ComponentId ComponentId, Schema_FieldId CommandIndex, Worker_RequestId RequestId)
{
	Worker_CommandResponse Response = {};
	Response.component_id = ComponentId;
	Response.schema_type = Schema_CreateCommandResponse(ComponentId, CommandIndex);

	Connection->SendCommandResponse(RequestId, &Response);
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

void USpatialSender::SendOutgoingRPCs()
{
	FProcessRPCDelegate Delegate;
	Delegate.BindUObject(this, &USpatialSender::SendRPC);
	OutgoingRPCs.ProcessRPCs(Delegate);
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
