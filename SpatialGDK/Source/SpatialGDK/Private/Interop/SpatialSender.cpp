// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "Interop/SpatialSender.h"

#include "GameFramework/PlayerController.h"
#include "GameFramework/PlayerState.h"

#include "Engine/Engine.h"
#include "EngineClasses/SpatialActorChannel.h"
#include "EngineClasses/SpatialNetConnection.h"
#include "EngineClasses/SpatialNetDriver.h"
#include "EngineClasses/SpatialPackageMapClient.h"
#include "EngineClasses/SpatialLoadBalanceEnforcer.h"
#include "Interop/Connection/SpatialWorkerConnection.h"
#include "Interop/SpatialDispatcher.h"
#include "Interop/SpatialReceiver.h"
#include "Net/NetworkProfiler.h"
#include "Schema/AlwaysRelevant.h"
#include "Schema/AuthorityIntent.h"
#include "Schema/ClientRPCEndpointLegacy.h"
#include "Schema/Heartbeat.h"
#include "Schema/Interest.h"
#include "Schema/RPCPayload.h"
#include "Schema/ServerRPCEndpointLegacy.h"
#include "Schema/Singleton.h"
#include "Schema/SpawnData.h"
#include "Schema/StandardLibrary.h"
#include "Schema/Tombstone.h"
#include "Schema/UnrealMetadata.h"
#include "SpatialConstants.h"
#include "Utils/SpatialActorGroupManager.h"
#include "Utils/ComponentFactory.h"
#include "Utils/EntityFactory.h"
#include "Utils/InspectionColors.h"
#include "Utils/InterestFactory.h"
#include "Utils/RepLayoutUtils.h"
#include "Utils/SpatialActorUtils.h"
#include "Utils/SpatialDebugger.h"
#include "Utils/SpatialLatencyTracer.h"
#include "Utils/SpatialMetrics.h"

DEFINE_LOG_CATEGORY(LogSpatialSender);

using namespace SpatialGDK;

DECLARE_CYCLE_STAT(TEXT("SendComponentUpdates"), STAT_SpatialSenderSendComponentUpdates, STATGROUP_SpatialNet);
DECLARE_CYCLE_STAT(TEXT("ResetOutgoingUpdate"), STAT_SpatialSenderResetOutgoingUpdate, STATGROUP_SpatialNet);
DECLARE_CYCLE_STAT(TEXT("QueueOutgoingUpdate"), STAT_SpatialSenderQueueOutgoingUpdate, STATGROUP_SpatialNet);
DECLARE_CYCLE_STAT(TEXT("UpdateInterestComponent"), STAT_SpatialSenderUpdateInterestComponent, STATGROUP_SpatialNet);

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

FPendingRPC::FPendingRPC(FPendingRPC&& Other)
	: Offset(Other.Offset)
	, Index(Other.Index)
	, Data(MoveTemp(Other.Data))
	, Entity(Other.Entity)
{
}

void USpatialSender::Init(USpatialNetDriver* InNetDriver, FTimerManager* InTimerManager, SpatialGDK::SpatialRPCService* InRPCService)
{
	NetDriver = InNetDriver;
	StaticComponentView = InNetDriver->StaticComponentView;
	Connection = InNetDriver->Connection;
	Receiver = InNetDriver->Receiver;
	PackageMap = InNetDriver->PackageMap;
	ClassInfoManager = InNetDriver->ClassInfoManager;
	check(InNetDriver->ActorGroupManager.IsValid());
	ActorGroupManager = InNetDriver->ActorGroupManager.Get();
	TimerManager = InTimerManager;
	RPCService = InRPCService;

	OutgoingRPCs.BindProcessingFunction(FProcessRPCDelegate::CreateUObject(this, &USpatialSender::SendRPC));
}

Worker_RequestId USpatialSender::CreateEntity(USpatialActorChannel* Channel)
{
	EntityFactory DataFactory(NetDriver, PackageMap, ClassInfoManager, RPCService);
	TArray<Worker_ComponentData> ComponentDatas = DataFactory.CreateEntityComponents(Channel, OutgoingOnCreateEntityRPCs);

	// If the Actor was loaded rather than dynamically spawned, associate it with its owning sublevel.
	ComponentDatas.Add(CreateLevelComponentData(Channel->Actor));

	Worker_EntityId EntityId = Channel->GetEntityId();
	Worker_RequestId CreateEntityRequestId = Connection->SendCreateEntityRequest(MoveTemp(ComponentDatas), &EntityId);

	return CreateEntityRequestId;
}

Worker_ComponentData USpatialSender::CreateLevelComponentData(AActor* Actor)
{
	UWorld* ActorWorld = Actor->GetTypedOuter<UWorld>();
	if (ActorWorld != NetDriver->World)
	{
		const uint32 ComponentId = ClassInfoManager->GetComponentIdFromLevelPath(ActorWorld->GetOuter()->GetPathName());
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

void USpatialSender::SendAddComponent(USpatialActorChannel* Channel, UObject* Subobject, const FClassInfo& SubobjectInfo)
{
	FRepChangeState SubobjectRepChanges = Channel->CreateInitialRepChangeState(Subobject);
	FHandoverChangeState SubobjectHandoverChanges = Channel->CreateInitialHandoverChangeState(SubobjectInfo);

	ComponentFactory DataFactory(false, NetDriver, USpatialLatencyTracer::GetTracer(Subobject));

	TArray<TraceKey>* TraceKeysPtr = nullptr;
#if TRACE_LIB_ACTIVE
	TArray<TraceKey> TraceKeys;
	TraceKeysPtr = &TraceKeys;
#endif

	TArray<Worker_ComponentData> SubobjectDatas = DataFactory.CreateComponentDatas(Subobject, SubobjectInfo, SubobjectRepChanges, SubobjectHandoverChanges, TraceKeysPtr);

	for (int i = 0; i < SubobjectDatas.Num(); i++)
	{
		Worker_ComponentData& ComponentData = SubobjectDatas[i];
		TraceKey LatencyKey = USpatialLatencyTracer::InvalidTraceKey;

#if TRACE_LIB_ACTIVE
		LatencyKey = TraceKeys[i];
#endif

		Connection->SendAddComponent(Channel->GetEntityId(), &ComponentData, LatencyKey);
	}

	Channel->PendingDynamicSubobjects.Remove(TWeakObjectPtr<UObject>(Subobject));
}

void USpatialSender::GainAuthorityThenAddComponent(USpatialActorChannel* Channel, UObject* Object, const FClassInfo* Info)
{
	const FClassInfo& ActorInfo = ClassInfoManager->GetOrCreateClassInfoByClass(Channel->Actor->GetClass());
	const WorkerAttributeSet WorkerAttribute{ ActorInfo.WorkerType.ToString() };
	const WorkerRequirementSet AuthoritativeWorkerRequirementSet = { WorkerAttribute };

	EntityAcl* EntityACL = StaticComponentView->GetComponentData<EntityAcl>(Channel->GetEntityId());

	TSharedRef<FPendingSubobjectAttachment> PendingSubobjectAttachment = MakeShared<FPendingSubobjectAttachment>();
	PendingSubobjectAttachment->Subobject = Object;
	PendingSubobjectAttachment->Channel = Channel;
	PendingSubobjectAttachment->Info = Info;

	ForAllSchemaComponentTypes([&](ESchemaComponentType Type)
	{
		Worker_ComponentId ComponentId = Info->SchemaComponents[Type];
		if (ComponentId != SpatialConstants::INVALID_COMPONENT_ID)
		{
			// For each valid ComponentId, we need to wait for its authority delegation before
			// adding the subobject.
			PendingSubobjectAttachment->PendingAuthorityDelegations.Add(ComponentId);
			Receiver->PendingEntitySubobjectDelegations.Add(
				MakeTuple(static_cast<Worker_EntityId_Key>(Channel->GetEntityId()), ComponentId),
				PendingSubobjectAttachment);

			EntityACL->ComponentWriteAcl.Add(Info->SchemaComponents[Type], AuthoritativeWorkerRequirementSet);
		}
	});

	Worker_ComponentUpdate Update = EntityACL->CreateEntityAclUpdate();
	Connection->SendComponentUpdate(Channel->GetEntityId(), &Update);
}

void USpatialSender::SendRemoveComponent(Worker_EntityId EntityId, const FClassInfo& Info)
{
	for (Worker_ComponentId SubobjectComponentId : Info.SchemaComponents)
	{
		if (SubobjectComponentId != SpatialConstants::INVALID_COMPONENT_ID)
		{
			NetDriver->Connection->SendRemoveComponent(EntityId, SubobjectComponentId);
		}
	}

	PackageMap->RemoveSubobject(FUnrealObjectRef(EntityId, Info.SchemaComponents[SCHEMA_Data]));
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

	TArray<Worker_ComponentData> Components;
	Components.Add(Position().CreatePositionData());
	Components.Add(Metadata(FString::Format(TEXT("WorkerEntity:{0}"), { Connection->GetWorkerId() })).CreateMetadataData());
	Components.Add(EntityAcl(WorkerIdPermission, ComponentWriteAcl).CreateEntityAclData());
	Components.Add(InterestFactory::CreateServerWorkerInterest().CreateInterestData());

	Worker_RequestId RequestId = Connection->SendCreateEntityRequest(MoveTemp(Components), nullptr);

	CreateEntityDelegate OnCreateWorkerEntityResponse;
	OnCreateWorkerEntityResponse.BindLambda([WeakSender = TWeakObjectPtr<USpatialSender>(this), AttemptCounter](const Worker_CreateEntityResponseOp& Op)
	{
		if (!WeakSender.IsValid())
		{
			return;
		}
		USpatialSender* Sender = WeakSender.Get();

		if (Op.status_code == WORKER_STATUS_CODE_SUCCESS)
		{
			Sender->NetDriver->WorkerEntityId = Op.entity_id;
			return;
		}

		if (Op.status_code != WORKER_STATUS_CODE_TIMEOUT)
		{
			UE_LOG(LogSpatialSender, Error, TEXT("Worker entity creation request failed: \"%s\""),
				UTF8_TO_TCHAR(Op.message));
			return;
		}

		if (AttemptCounter == SpatialConstants::MAX_NUMBER_COMMAND_ATTEMPTS)
		{
			UE_LOG(LogSpatialSender, Error, TEXT("Worker entity creation request timed out too many times. (%u attempts)"),
				SpatialConstants::MAX_NUMBER_COMMAND_ATTEMPTS);
			return;
		}

		UE_LOG(LogSpatialSender, Warning, TEXT("Worker entity creation request timed out and will retry."));
		FTimerHandle RetryTimer;
		Sender->TimerManager->SetTimer(RetryTimer, [WeakSender, AttemptCounter]()
		{
			if (USpatialSender* Sender = WeakSender.Get())
			{
				Sender->CreateServerWorkerEntity(AttemptCounter + 1);
			}
		}, SpatialConstants::GetCommandRetryWaitTimeSeconds(AttemptCounter), false);
	});

	Receiver->AddCreateEntityDelegate(RequestId, OnCreateWorkerEntityResponse);
}

void USpatialSender::ClearPendingRPCs(const Worker_EntityId EntityId)
{
	OutgoingRPCs.DropForEntity(EntityId);
}

bool USpatialSender::ValidateOrExit_IsSupportedClass(const FString& PathName)
{
	// Level blueprint classes could have a PIE prefix, this will remove it.
	FString RemappedPathName = PathName;
	GEngine->NetworkRemapPath(NetDriver, RemappedPathName, false);

	return ClassInfoManager->ValidateOrExit_IsSupportedClass(RemappedPathName);
}

void USpatialSender::SendComponentUpdates(UObject* Object, const FClassInfo& Info, USpatialActorChannel* Channel, const FRepChangeState* RepChanges, const FHandoverChangeState* HandoverChanges)
{
	SCOPE_CYCLE_COUNTER(STAT_SpatialSenderSendComponentUpdates);
	Worker_EntityId EntityId = Channel->GetEntityId();

	UE_LOG(LogSpatialSender, Verbose, TEXT("Sending component update (object: %s, entity: %lld)"), *Object->GetName(), EntityId);

	USpatialLatencyTracer* Tracer = USpatialLatencyTracer::GetTracer(Object);
	ComponentFactory UpdateFactory(Channel->GetInterestDirty(), NetDriver, Tracer);

	TArray<TraceKey>* TraceKeysPtr = nullptr;
#if TRACE_LIB_ACTIVE
	TArray<TraceKey> TraceKeys;
	TraceKeysPtr = &TraceKeys;
#endif

	TArray<Worker_ComponentUpdate> ComponentUpdates = UpdateFactory.CreateComponentUpdates(Object, Info, EntityId, RepChanges, HandoverChanges, TraceKeysPtr);

	for(int i = 0; i < ComponentUpdates.Num(); i++)
	{
		Worker_ComponentUpdate& Update = ComponentUpdates[i];
		TraceKey LatencyKey = USpatialLatencyTracer::InvalidTraceKey;
#if TRACE_LIB_ACTIVE
		checkf(TraceKeys.Num() == ComponentUpdates.Num(), TEXT("Trace keys does not match the component updates for tracing."));
		LatencyKey = TraceKeys[i];
#endif
		if (!NetDriver->StaticComponentView->HasAuthority(EntityId, Update.component_id))
		{
			UE_LOG(LogSpatialSender, Verbose, TEXT("Trying to send component update but don't have authority! Update will be queued and sent when authority gained. Component Id: %d, entity: %lld"), Update.component_id, EntityId);

			// This is a temporary fix. A task to improve this has been created: UNR-955
			// It may be the case that upon resolving a component, we do not have authority to send the update. In this case, we queue the update, to send upon receiving authority.
			// Note: This will break in a multi-worker context, if we try to create an entity that we don't intend to have authority over. For this reason, this fix is only temporary.
			FQueuedUpdate& UpdatesQueuedUntilAuthority = UpdatesQueuedUntilAuthorityMap.FindOrAdd(EntityId);
			UpdatesQueuedUntilAuthority.ComponentUpdates.Add(Update);
#if TRACE_LIB_ACTIVE
			// TODO: Clean this up by creating a composite type which pairs the update with the key UNR-2726
			UpdatesQueuedUntilAuthority.LatencyKeys.Add(LatencyKey);
#endif
			continue;
		}

		Connection->SendComponentUpdate(EntityId, &Update, LatencyKey);
	}
}

// Apply (and clean up) any updates queued, due to being sent previously when they didn't have authority.
void USpatialSender::ProcessUpdatesQueuedUntilAuthority(Worker_EntityId EntityId)
{
	if (FQueuedUpdate* UpdatesQueuedUntilAuthority = UpdatesQueuedUntilAuthorityMap.Find(EntityId))
	{
		TArray<Worker_ComponentUpdate>& Components = UpdatesQueuedUntilAuthority->ComponentUpdates;
#if TRACE_LIB_ACTIVE
		TArray<TraceKey>& LatencyKeys = UpdatesQueuedUntilAuthority->LatencyKeys;
		checkf(Components.Num() == LatencyKeys.Num(), TEXT("Latency key pairs do not match the queued updates."));
#endif

		for (int i = 0; i < UpdatesQueuedUntilAuthority->ComponentUpdates.Num(); i++)
		{
			TraceKey LatencyKey = USpatialLatencyTracer::InvalidTraceKey;

#if TRACE_LIB_ACTIVE
			LatencyKey = LatencyKeys[i];
#endif

			Connection->SendComponentUpdate(EntityId, &Components[i], LatencyKey);			
		}
		UpdatesQueuedUntilAuthorityMap.Remove(EntityId);
	}
}

void USpatialSender::FlushPackedRPCs()
{
	if (RPCsToPack.Num() == 0)
	{
		return;
	}

	// TODO: This could be further optimized for the case when there's only 1 RPC to be sent during this frame
	// by sending it directly to the corresponding entity, without including the EntityId in the payload - UNR-1563.
	for (const auto& It : RPCsToPack)
	{
		Worker_EntityId PlayerControllerEntityId = It.Key;
		const TArray<FPendingRPC>& PendingRPCArray = It.Value;

		Worker_ComponentUpdate ComponentUpdate = {};

		Worker_ComponentId ComponentId = NetDriver->IsServer() ? SpatialConstants::SERVER_RPC_ENDPOINT_COMPONENT_ID_LEGACY : SpatialConstants::CLIENT_RPC_ENDPOINT_COMPONENT_ID_LEGACY;
		ComponentUpdate.component_id = ComponentId;
		ComponentUpdate.schema_type = Schema_CreateComponentUpdate();
		Schema_Object* EventsObject = Schema_GetComponentUpdateEvents(ComponentUpdate.schema_type);

		for (const FPendingRPC& RPC : PendingRPCArray)
		{
			Schema_Object* EventData = Schema_AddObject(EventsObject, SpatialConstants::UNREAL_RPC_ENDPOINT_PACKED_EVENT_ID);

			Schema_AddUint32(EventData, SpatialConstants::UNREAL_RPC_PAYLOAD_OFFSET_ID, RPC.Offset);
			Schema_AddUint32(EventData, SpatialConstants::UNREAL_RPC_PAYLOAD_RPC_INDEX_ID, RPC.Index);
			SpatialGDK::AddBytesToSchema(EventData, SpatialConstants::UNREAL_RPC_PAYLOAD_RPC_PAYLOAD_ID, RPC.Data.GetData(), RPC.Data.Num());
			Schema_AddEntityId(EventData, SpatialConstants::UNREAL_PACKED_RPC_PAYLOAD_ENTITY_ID, RPC.Entity);
		}

		Connection->SendComponentUpdate(PlayerControllerEntityId, &ComponentUpdate);
	}

	RPCsToPack.Empty();
}

void USpatialSender::FlushRPCService()
{
	if (RPCService != nullptr)
	{
		RPCService->PushOverflowedRPCs();

		for (const SpatialRPCService::UpdateToSend& Update : RPCService->GetRPCsAndAcksToSend())
		{
			Connection->SendComponentUpdate(Update.EntityId, &Update.Update);
		}
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

TArray<Worker_InterestOverride> USpatialSender::CreateComponentInterestForActor(USpatialActorChannel* Channel, bool bIsNetOwned)
{
	TArray<Worker_InterestOverride> ComponentInterest;

	const FClassInfo& ActorInfo = ClassInfoManager->GetOrCreateClassInfoByClass(Channel->Actor->GetClass());
	FillComponentInterests(ActorInfo, bIsNetOwned, ComponentInterest);

	// Statically attached subobjects
	for (auto& SubobjectInfoPair : ActorInfo.SubobjectInfo)
	{
		const FClassInfo& SubobjectInfo = SubobjectInfoPair.Value.Get();
		FillComponentInterests(SubobjectInfo, bIsNetOwned, ComponentInterest);
	}

	// Subobjects dynamically created through replication
	for (const auto& Subobject : Channel->CreateSubObjects)
	{
		const FClassInfo& SubobjectInfo = ClassInfoManager->GetOrCreateClassInfoByObject(Subobject);
		FillComponentInterests(SubobjectInfo, bIsNetOwned, ComponentInterest);
	}

	if (GetDefault<USpatialGDKSettings>()->bUseRPCRingBuffers)
	{
		ComponentInterest.Add({ SpatialConstants::CLIENT_ENDPOINT_COMPONENT_ID, bIsNetOwned });
		ComponentInterest.Add({ SpatialConstants::SERVER_ENDPOINT_COMPONENT_ID, bIsNetOwned });
	}
	else
	{
		ComponentInterest.Add({ SpatialConstants::CLIENT_RPC_ENDPOINT_COMPONENT_ID_LEGACY, bIsNetOwned });
		ComponentInterest.Add({ SpatialConstants::SERVER_RPC_ENDPOINT_COMPONENT_ID_LEGACY, bIsNetOwned });
	}

	return ComponentInterest;
}

RPCPayload USpatialSender::CreateRPCPayloadFromParams(UObject* TargetObject, const FUnrealObjectRef& TargetObjectRef, UFunction* Function, void* Params)
{
	const FRPCInfo& RPCInfo = ClassInfoManager->GetRPCInfo(TargetObject, Function);

	FSpatialNetBitWriter PayloadWriter = PackRPCDataToSpatialNetBitWriter(Function, Params);

#if TRACE_LIB_ACTIVE
	return RPCPayload(TargetObjectRef.Offset, RPCInfo.Index, TArray<uint8>(PayloadWriter.GetData(), PayloadWriter.GetNumBytes()), USpatialLatencyTracer::GetTracer(TargetObject)->RetrievePendingTrace(TargetObject, Function));
#else
	return RPCPayload(TargetObjectRef.Offset, RPCInfo.Index, TArray<uint8>(PayloadWriter.GetData(), PayloadWriter.GetNumBytes()));
#endif
}

void USpatialSender::SendComponentInterestForActor(USpatialActorChannel* Channel, Worker_EntityId EntityId, bool bNetOwned)
{
	checkf(!NetDriver->IsServer(), TEXT("Tried to set ComponentInterest on a server-worker. This should never happen!"));

	NetDriver->Connection->SendComponentInterest(EntityId, CreateComponentInterestForActor(Channel, bNetOwned));
}

void USpatialSender::SendComponentInterestForSubobject(const FClassInfo& Info, Worker_EntityId EntityId, bool bNetOwned)
{
	checkf(!NetDriver->IsServer(), TEXT("Tried to set ComponentInterest on a server-worker. This should never happen!"));

	TArray<Worker_InterestOverride> ComponentInterest;
	FillComponentInterests(Info, bNetOwned, ComponentInterest);
	NetDriver->Connection->SendComponentInterest(EntityId, MoveTemp(ComponentInterest));
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

void USpatialSender::SendAuthorityIntentUpdate(const AActor& Actor, VirtualWorkerId NewAuthoritativeVirtualWorkerId)
{
	const Worker_EntityId EntityId = PackageMap->GetEntityIdFromObject(&Actor);
	check(EntityId != SpatialConstants::INVALID_ENTITY_ID);
	check(NetDriver->StaticComponentView->HasAuthority(EntityId, SpatialConstants::AUTHORITY_INTENT_COMPONENT_ID));

	AuthorityIntent* AuthorityIntentComponent = StaticComponentView->GetComponentData<AuthorityIntent>(EntityId);
	check(AuthorityIntentComponent != nullptr);

	if (AuthorityIntentComponent->VirtualWorkerId == NewAuthoritativeVirtualWorkerId)
	{
		// There may be multiple intent updates triggered by a server worker before the Runtime
		// notifies this worker that the authority has changed. Ignore the extra calls here.
		return;
	}

	AuthorityIntentComponent->VirtualWorkerId = NewAuthoritativeVirtualWorkerId;
	UE_LOG(LogSpatialSender, Log, TEXT("(%s) Sending authority intent update for entity id %d. Virtual worker '%d' should become authoritative over %s"),
		*NetDriver->Connection->GetWorkerId(), EntityId, NewAuthoritativeVirtualWorkerId, *GetNameSafe(&Actor));

	// If the SpatialDebugger is enabled, also update the authority intent virtual worker ID and color.
	if (NetDriver->SpatialDebugger != nullptr)
	{
		NetDriver->SpatialDebugger->ActorAuthorityIntentChanged(EntityId, NewAuthoritativeVirtualWorkerId);
	}

	Worker_ComponentUpdate Update = AuthorityIntentComponent->CreateAuthorityIntentUpdate();
	Connection->SendComponentUpdate(EntityId, &Update);

	if (NetDriver->StaticComponentView->HasAuthority(EntityId, SpatialConstants::ENTITY_ACL_COMPONENT_ID))
	{
		// Also notify the enforcer directly on the worker that sends the component update, as the update will short circuit
		NetDriver->LoadBalanceEnforcer->QueueAclAssignmentRequest(EntityId);
	}
}

void USpatialSender::SetAclWriteAuthority(const Worker_EntityId EntityId, const PhysicalWorkerName& DestinationWorkerId)
{
	check(NetDriver);
	check(NetDriver->StaticComponentView->HasAuthority(EntityId, SpatialConstants::ENTITY_ACL_COMPONENT_ID));

	EntityAcl* EntityACL = NetDriver->StaticComponentView->GetComponentData<EntityAcl>(EntityId);
	check(EntityACL);

	const FString& WriteWorkerId = FString::Printf(TEXT("workerId:%s"), *DestinationWorkerId);

	WorkerAttributeSet OwningWorkerAttribute = { WriteWorkerId };

	TArray<Worker_ComponentId> ComponentIds;
	EntityACL->ComponentWriteAcl.GetKeys(ComponentIds);

	for (const Worker_ComponentId& ComponentId : ComponentIds)
	{
		if (ComponentId == SpatialConstants::ENTITY_ACL_COMPONENT_ID ||
			ComponentId == SpatialConstants::HEARTBEAT_COMPONENT_ID ||
			ComponentId == SpatialConstants::GetClientAuthorityComponent(GetDefault<USpatialGDKSettings>()->bUseRPCRingBuffers))
		{
			continue;
		}

		WorkerRequirementSet* RequirementSet = EntityACL->ComponentWriteAcl.Find(ComponentId);
		check(RequirementSet->Num() == 1);
		RequirementSet->Empty();
		RequirementSet->Add(OwningWorkerAttribute);
	}

	UE_LOG(LogSpatialLoadBalanceEnforcer, Verbose, TEXT("(%s) Setting Acl WriteAuth for entity %lld to workerid: %s"), *NetDriver->Connection->GetWorkerId(), EntityId, *DestinationWorkerId);

	Worker_ComponentUpdate Update = EntityACL->CreateEntityAclUpdate();
	NetDriver->Connection->SendComponentUpdate(EntityId, &Update);
}

FRPCErrorInfo USpatialSender::SendRPC(const FPendingRPCParams& Params)
{
	TWeakObjectPtr<UObject> TargetObjectWeakPtr = PackageMap->GetObjectFromUnrealObjectRef(Params.ObjectRef);
	if (!TargetObjectWeakPtr.IsValid())
	{
		// Target object was destroyed before the RPC could be (re)sent
		return FRPCErrorInfo{ nullptr, nullptr, NetDriver->IsServer(), ERPCQueueType::Send, ERPCResult::UnresolvedTargetObject };
	}
	UObject* TargetObject = TargetObjectWeakPtr.Get();

	const FClassInfo& ClassInfo = ClassInfoManager->GetOrCreateClassInfoByObject(TargetObject);
	UFunction* Function = ClassInfo.RPCs[Params.Payload.Index];
	if (Function == nullptr)
	{
		return FRPCErrorInfo{ TargetObject, nullptr, NetDriver->IsServer(), ERPCQueueType::Send, ERPCResult::MissingFunctionInfo };
	}

	ERPCResult Result = SendRPCInternal(TargetObject, Function, Params.Payload);

	return FRPCErrorInfo{ TargetObject, Function, NetDriver->IsServer(), ERPCQueueType::Send, Result };
}

#if !UE_BUILD_SHIPPING
void USpatialSender::TrackRPC(AActor* Actor, UFunction* Function, const RPCPayload& Payload, const ERPCType RPCType)
{
	NETWORK_PROFILER(GNetworkProfiler.TrackSendRPC(Actor, Function, 0, Payload.CountDataBits(), 0, NetDriver->GetSpatialOSNetConnection()));
	NetDriver->SpatialMetrics->TrackSentRPC(Function, RPCType, Payload.PayloadData.Num());
}
#endif

ERPCResult USpatialSender::SendRPCInternal(UObject* TargetObject, UFunction* Function, const RPCPayload& Payload)
{
	USpatialActorChannel* Channel = NetDriver->GetOrCreateSpatialActorChannel(TargetObject);

	if (!Channel)
	{
		UE_LOG(LogSpatialSender, Warning, TEXT("Failed to create an Actor Channel for %s."), *TargetObject->GetName());
		return ERPCResult::NoActorChannel;
	}
	const FRPCInfo& RPCInfo = ClassInfoManager->GetRPCInfo(TargetObject, Function);
	const USpatialGDKSettings* SpatialGDKSettings = GetDefault<USpatialGDKSettings>();

	if (Channel->bCreatingNewEntity && !SpatialGDKSettings->bUseRPCRingBuffers)
	{
		if (Function->HasAnyFunctionFlags(FUNC_NetClient))
		{
			check(NetDriver->IsServer());

			OutgoingOnCreateEntityRPCs.FindOrAdd(TargetObject).RPCs.Add(Payload);
#if !UE_BUILD_SHIPPING
			TrackRPC(Channel->Actor, Function, Payload, RPCInfo.Type);
#endif // !UE_BUILD_SHIPPING
			return ERPCResult::Success;
		}
	}

	Worker_EntityId EntityId = SpatialConstants::INVALID_ENTITY_ID;

	switch (RPCInfo.Type)
	{
	case ERPCType::CrossServer:
	{
		Worker_ComponentId ComponentId = SpatialConstants::GetCrossServerRPCComponent(SpatialGDKSettings->bUseRPCRingBuffers);

		Worker_CommandRequest CommandRequest = CreateRPCCommandRequest(TargetObject, Payload, ComponentId, RPCInfo.Index, EntityId);

		check(EntityId != SpatialConstants::INVALID_ENTITY_ID);
		Worker_RequestId RequestId = Connection->SendCommandRequest(EntityId, &CommandRequest, SpatialConstants::UNREAL_RPC_ENDPOINT_COMMAND_ID);

#if !UE_BUILD_SHIPPING
		TrackRPC(Channel->Actor, Function, Payload, RPCInfo.Type);
#endif // !UE_BUILD_SHIPPING

		if (Function->HasAnyFunctionFlags(FUNC_NetReliable))
		{
			UE_LOG(LogSpatialSender, Verbose, TEXT("Sending reliable command request (entity: %lld, component: %d, function: %s, attempt: 1)"),
				EntityId, CommandRequest.component_id, *Function->GetName());
			Receiver->AddPendingReliableRPC(RequestId, MakeShared<FReliableRPCForRetry>(TargetObject, Function, ComponentId, RPCInfo.Index, Payload.PayloadData, 0));
		}
		else
		{
			UE_LOG(LogSpatialSender, Verbose, TEXT("Sending unreliable command request (entity: %lld, component: %d, function: %s)"),
				EntityId, CommandRequest.component_id, *Function->GetName());
		}

		return ERPCResult::Success;
	}
	case ERPCType::NetMulticast:
	case ERPCType::ClientReliable:
	case ERPCType::ServerReliable:
	case ERPCType::ClientUnreliable:
	case ERPCType::ServerUnreliable:
	{
		FUnrealObjectRef TargetObjectRef = PackageMap->GetUnrealObjectRefFromObject(TargetObject);
		if (TargetObjectRef == FUnrealObjectRef::UNRESOLVED_OBJECT_REF)
		{
			return ERPCResult::UnresolvedTargetObject;
		}

		if (SpatialGDKSettings->bUseRPCRingBuffers && RPCService != nullptr)
		{
			EPushRPCResult Result = RPCService->PushRPC(TargetObjectRef.Entity, RPCInfo.Type, Payload);
#if !UE_BUILD_SHIPPING
			if (Result == EPushRPCResult::Success || Result == EPushRPCResult::QueueOverflowed)
			{
				TrackRPC(Channel->Actor, Function, Payload, RPCInfo.Type);
			}
#endif // !UE_BUILD_SHIPPING

			switch (Result)
			{
			case EPushRPCResult::QueueOverflowed:
				UE_LOG(LogSpatialSender, Log, TEXT("USpatialSender::SendRPCInternal: Ring buffer queue overflowed, queuing RPC locally. Actor: %s, entity: %lld, function: %s"), *TargetObject->GetPathName(), TargetObjectRef.Entity, *Function->GetName());
				break;
			case EPushRPCResult::DropOverflowed:
				UE_LOG(LogSpatialSender, Log, TEXT("USpatialSender::SendRPCInternal: Ring buffer queue overflowed, dropping RPC. Actor: %s, entity: %lld, function: %s"), *TargetObject->GetPathName(), TargetObjectRef.Entity, *Function->GetName());
				break;
			case EPushRPCResult::HasAckAuthority:
				UE_LOG(LogSpatialSender, Warning, TEXT("USpatialSender::SendRPCInternal: Worker has authority over ack component for RPC it is sending. RPC will not be sent. Actor: %s, entity: %lld, function: %s"), *TargetObject->GetPathName(), TargetObjectRef.Entity, *Function->GetName());
				break;
			case EPushRPCResult::NoRingBufferAuthority:
				// TODO: Change engine logic that calls Client RPCs from non-auth servers and change this to error. UNR-2517
				UE_LOG(LogSpatialSender, Log, TEXT("USpatialSender::SendRPCInternal: Failed to send RPC because the worker does not have authority over ring buffer component. Actor: %s, entity: %lld, function: %s"), *TargetObject->GetPathName(), TargetObjectRef.Entity, *Function->GetName());
				break;
			}

			return ERPCResult::Success;
		}

		if (RPCInfo.Type != ERPCType::NetMulticast && !Channel->IsListening())
		{
			// If the Entity endpoint is not yet ready to receive RPCs -
			// treat the corresponding object as unresolved and queue RPC
			// However, it doesn't matter in case of Multicast
			return ERPCResult::SpatialActorChannelNotListening;
		}

		EntityId = TargetObjectRef.Entity;
		check(EntityId != SpatialConstants::INVALID_ENTITY_ID);

		Worker_ComponentId ComponentId = SpatialConstants::RPCTypeToWorkerComponentIdLegacy(RPCInfo.Type);

		bool bCanPackRPC = SpatialGDKSettings->bPackRPCs;
		if (bCanPackRPC && RPCInfo.Type == ERPCType::NetMulticast)
		{
			bCanPackRPC = false;
		}

		if (bCanPackRPC && SpatialGDKSettings->bEnableOffloading)
		{
			if (const AActor* TargetActor = Cast<AActor>(PackageMap->GetObjectFromEntityId(TargetObjectRef.Entity).Get()))
			{
				if (const UNetConnection* OwningConnection = TargetActor->GetNetConnection())
				{
					if (const AActor* ConnectionOwner = OwningConnection->OwningActor)
					{
						if (!ActorGroupManager->IsSameWorkerType(TargetActor, ConnectionOwner))
						{
							UE_LOG(LogSpatialSender, Verbose, TEXT("RPC %s Cannot be packed as TargetActor (%s) and Connection Owner (%s) are on different worker types."),
								*Function->GetName(),
								*TargetActor->GetName(),
								*ConnectionOwner->GetName()
							);
							bCanPackRPC = false;
						}
					}
				}
			}
		}

		if (bCanPackRPC)
		{
			ERPCResult Result = AddPendingRPC(TargetObject, Function, Payload, ComponentId, RPCInfo.Index);
#if !UE_BUILD_SHIPPING
			if (Result == ERPCResult::Success)
			{
				TrackRPC(Channel->Actor, Function, Payload, RPCInfo.Type);
			}
#endif // !UE_BUILD_SHIPPING
			return Result;
		}
		else
		{
			if (!NetDriver->StaticComponentView->HasAuthority(EntityId, ComponentId))
			{
				return ERPCResult::NoAuthority;
			}

			Worker_ComponentUpdate ComponentUpdate = CreateRPCEventUpdate(TargetObject, Payload, ComponentId, RPCInfo.Index);

			Connection->SendComponentUpdate(EntityId, &ComponentUpdate, Payload.Trace);
#if !UE_BUILD_SHIPPING
			TrackRPC(Channel->Actor, Function, Payload, RPCInfo.Type);
#endif // !UE_BUILD_SHIPPING
			return ERPCResult::Success;
		}
	}
	default:
		checkNoEntry();
		return ERPCResult::InvalidRPCType;
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
	UE_LOG(LogSpatialSender, Log, TEXT("Sending create entity request for %s with EntityId %lld"), *Channel->Actor->GetName(), Channel->GetEntityId());

	Worker_RequestId RequestId = CreateEntity(Channel);
	Receiver->AddPendingActorRequest(RequestId, Channel);
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
	ClientRPCEndpointLegacy Endpoint;
	Endpoint.bReady = true;
	Worker_ComponentUpdate Update = Endpoint.CreateRPCEndpointUpdate();
	NetDriver->Connection->SendComponentUpdate(EntityId, &Update);
}

void USpatialSender::SendServerEndpointReadyUpdate(Worker_EntityId EntityId)
{
	ServerRPCEndpointLegacy Endpoint;
	Endpoint.bReady = true;
	Worker_ComponentUpdate Update = Endpoint.CreateRPCEndpointUpdate();
	NetDriver->Connection->SendComponentUpdate(EntityId, &Update);
}

void USpatialSender::ProcessOrQueueOutgoingRPC(const FUnrealObjectRef& InTargetObjectRef, SpatialGDK::RPCPayload&& InPayload)
{
	TWeakObjectPtr<UObject> TargetObjectWeakPtr = PackageMap->GetObjectFromUnrealObjectRef(InTargetObjectRef);
	if (!TargetObjectWeakPtr.IsValid())
	{
		// Target object was destroyed before the RPC could be (re)sent
		return;
	}

	UObject* TargetObject = TargetObjectWeakPtr.Get();
	const FClassInfo& ClassInfo = ClassInfoManager->GetOrCreateClassInfoByObject(TargetObject);
	UFunction* Function = ClassInfo.RPCs[InPayload.Index];
	const FRPCInfo& RPCInfo = ClassInfoManager->GetRPCInfo(TargetObject, Function);

	OutgoingRPCs.ProcessOrQueueRPC(InTargetObjectRef, RPCInfo.Type, MoveTemp(InPayload));

	// Try to send all pending RPCs unconditionally
	OutgoingRPCs.ProcessRPCs();
}

FSpatialNetBitWriter USpatialSender::PackRPCDataToSpatialNetBitWriter(UFunction* Function, void* Parameters) const
{
	FSpatialNetBitWriter PayloadWriter(PackageMap);

	TSharedPtr<FRepLayout> RepLayout = NetDriver->GetFunctionRepLayout(Function);
	RepLayout_SendPropertiesForRPC(*RepLayout, PayloadWriter, Parameters);

	return PayloadWriter;
}

Worker_CommandRequest USpatialSender::CreateRPCCommandRequest(UObject* TargetObject, const RPCPayload& Payload, Worker_ComponentId ComponentId, Schema_FieldId CommandIndex, Worker_EntityId& OutEntityId)
{
	Worker_CommandRequest CommandRequest = {};
	CommandRequest.component_id = ComponentId;
	CommandRequest.command_index = SpatialConstants::UNREAL_RPC_ENDPOINT_COMMAND_ID;
	CommandRequest.schema_type = Schema_CreateCommandRequest();
	Schema_Object* RequestObject = Schema_GetCommandRequestObject(CommandRequest.schema_type);

	FUnrealObjectRef TargetObjectRef(PackageMap->GetUnrealObjectRefFromNetGUID(PackageMap->GetNetGUIDFromObject(TargetObject)));
	ensure(TargetObjectRef != FUnrealObjectRef::UNRESOLVED_OBJECT_REF);

	OutEntityId = TargetObjectRef.Entity;

	RPCPayload::WriteToSchemaObject(RequestObject, TargetObjectRef.Offset, CommandIndex, Payload.PayloadData.GetData(), Payload.PayloadData.Num());

	return CommandRequest;
}

Worker_CommandRequest USpatialSender::CreateRetryRPCCommandRequest(const FReliableRPCForRetry& RPC, uint32 TargetObjectOffset)
{
	Worker_CommandRequest CommandRequest = {};
	CommandRequest.component_id = RPC.ComponentId;
	CommandRequest.command_index = SpatialConstants::UNREAL_RPC_ENDPOINT_COMMAND_ID;
	CommandRequest.schema_type = Schema_CreateCommandRequest();
	Schema_Object* RequestObject = Schema_GetCommandRequestObject(CommandRequest.schema_type);

	RPCPayload::WriteToSchemaObject(RequestObject, TargetObjectOffset, RPC.RPCIndex, RPC.Payload.GetData(), RPC.Payload.Num());

	return CommandRequest;
}

Worker_ComponentUpdate USpatialSender::CreateRPCEventUpdate(UObject* TargetObject, const RPCPayload& Payload, Worker_ComponentId ComponentId, Schema_FieldId EventIndex)
{
	Worker_ComponentUpdate ComponentUpdate = {};

	ComponentUpdate.component_id = ComponentId;
	ComponentUpdate.schema_type = Schema_CreateComponentUpdate();
	Schema_Object* EventsObject = Schema_GetComponentUpdateEvents(ComponentUpdate.schema_type);
	Schema_Object* EventData = Schema_AddObject(EventsObject, SpatialConstants::UNREAL_RPC_ENDPOINT_EVENT_ID);

	FUnrealObjectRef TargetObjectRef(PackageMap->GetUnrealObjectRefFromNetGUID(PackageMap->GetNetGUIDFromObject(TargetObject)));
	ensure(TargetObjectRef != FUnrealObjectRef::UNRESOLVED_OBJECT_REF);

	Payload.WriteToSchemaObject(EventData);

	return ComponentUpdate;
}
ERPCResult USpatialSender::AddPendingRPC(UObject* TargetObject, UFunction* Function, const RPCPayload& Payload, Worker_ComponentId ComponentId, Schema_FieldId RPCIndex)
{
	FUnrealObjectRef TargetObjectRef(PackageMap->GetUnrealObjectRefFromNetGUID(PackageMap->GetNetGUIDFromObject(TargetObject)));
	ensure(TargetObjectRef != FUnrealObjectRef::UNRESOLVED_OBJECT_REF);

	AActor* TargetActor = Cast<AActor>(PackageMap->GetObjectFromEntityId(TargetObjectRef.Entity).Get());
	check(TargetActor != nullptr);
	UNetConnection* OwningConnection = TargetActor->GetNetConnection();
	if (OwningConnection == nullptr)
	{
		UE_LOG(LogSpatialSender, Warning, TEXT("AddPendingRPC: No connection for object %s (RPC %s, actor %s, entity %lld)"),
			*TargetObject->GetName(), *Function->GetName(), *TargetActor->GetName(), TargetObjectRef.Entity);
		return ERPCResult::NoNetConnection;
	}

	APlayerController* Controller = Cast<APlayerController>(OwningConnection->OwningActor);
	if (Controller == nullptr)
	{
		UE_LOG(LogSpatialSender, Warning, TEXT("AddPendingRPC: Connection's owner is not a player controller for object %s (RPC %s, actor %s, entity %lld): connection owner %s"),
			*TargetObject->GetName(), *Function->GetName(), *TargetActor->GetName(), TargetObjectRef.Entity, *OwningConnection->OwningActor->GetName());
		return ERPCResult::NoOwningController;
	}

	USpatialActorChannel* ControllerChannel = NetDriver->GetOrCreateSpatialActorChannel(Controller);
	if (ControllerChannel == nullptr)
	{
		return ERPCResult::NoControllerChannel;
	}

	if (!ControllerChannel->IsListening())
	{
		return ERPCResult::ControllerChannelNotListening;
	}

	FUnrealObjectRef ControllerObjectRef = PackageMap->GetUnrealObjectRefFromObject(Controller);
	ensure(ControllerObjectRef != FUnrealObjectRef::UNRESOLVED_OBJECT_REF);

	TSet<TWeakObjectPtr<const UObject>> UnresolvedObjects;

	FPendingRPC RPC;
	RPC.Offset = TargetObjectRef.Offset;
	RPC.Index = RPCIndex;
	RPC.Data.SetNumUninitialized(Payload.PayloadData.Num());
	FMemory::Memcpy(RPC.Data.GetData(), Payload.PayloadData.GetData(), Payload.PayloadData.Num());
	RPC.Entity = TargetObjectRef.Entity;
	RPCsToPack.FindOrAdd(ControllerObjectRef.Entity).Emplace(MoveTemp(RPC));
	return ERPCResult::Success;
}

void USpatialSender::SendCommandResponse(Worker_RequestId RequestId, Worker_CommandResponse& Response)
{
	Connection->SendCommandResponse(RequestId, &Response);
}

void USpatialSender::SendEmptyCommandResponse(Worker_ComponentId ComponentId, Schema_FieldId CommandIndex, Worker_RequestId RequestId)
{
	Worker_CommandResponse Response = {};
	Response.component_id = ComponentId;
	Response.command_index = CommandIndex;
	Response.schema_type = Schema_CreateCommandResponse();

	Connection->SendCommandResponse(RequestId, &Response);
}

void USpatialSender::SendCommandFailure(Worker_RequestId RequestId, const FString& Message)
{
	Connection->SendCommandFailure(RequestId, Message);
}

// Authority over the ClientRPC Schema component and the Heartbeat component are dictated by the owning connection of a client.
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

	EntityACL->ComponentWriteAcl.Add(SpatialConstants::GetClientAuthorityComponent(GetDefault<USpatialGDKSettings>()->bUseRPCRingBuffers), OwningClientOnly);
	EntityACL->ComponentWriteAcl.Add(SpatialConstants::HEARTBEAT_COMPONENT_ID, OwningClientOnly);
	Worker_ComponentUpdate Update = EntityACL->CreateEntityAclUpdate();

	Connection->SendComponentUpdate(EntityId, &Update);
	return true;
}

void USpatialSender::UpdateInterestComponent(AActor* Actor)
{
	SCOPE_CYCLE_COUNTER(STAT_SpatialSenderUpdateInterestComponent);

	Worker_EntityId EntityId = PackageMap->GetEntityIdFromObject(Actor);
	if (EntityId == SpatialConstants::INVALID_ENTITY_ID)
	{
		UE_LOG(LogSpatialSender, Verbose, TEXT("Attempted to update interest for non replicated actor: %s"), *GetNameSafe(Actor));
		return;
	}

	InterestFactory InterestUpdateFactory(Actor, ClassInfoManager->GetOrCreateClassInfoByObject(Actor), NetDriver->ClassInfoManager, NetDriver->PackageMap);
	Worker_ComponentUpdate Update = InterestUpdateFactory.CreateInterestUpdate();

	Connection->SendComponentUpdate(EntityId, &Update);
}

void USpatialSender::RetireEntity(const Worker_EntityId EntityId)
{
	if (AActor* Actor = Cast<AActor>(PackageMap->GetObjectFromEntityId(EntityId).Get()))
	{
		if (Actor->IsNetStartupActor())
		{
			Receiver->RemoveActor(EntityId);
			// In the case that this is a startup actor, we won't actually delete the entity in SpatialOS.  Instead we'll Tombstone it.
			if (!StaticComponentView->HasComponent(EntityId, SpatialConstants::TOMBSTONE_COMPONENT_ID))
			{
				AddTombstoneToEntity(EntityId);
			}
			else
			{
				UE_LOG(LogSpatialSender, Verbose, TEXT("RetireEntity called on already retired entity: %lld (actor: %s)"), EntityId, *Actor->GetName());
			}
		}
		else
		{
			Connection->SendDeleteEntityRequest(EntityId);
		}
	}
	else
	{
		UE_LOG(LogSpatialSender, Warning, TEXT("RetireEntity: Couldn't get Actor from PackageMap for EntityId: %lld"), EntityId);
	}
}

void USpatialSender::AddTombstoneToEntity(const Worker_EntityId EntityId)
{
	check(NetDriver->StaticComponentView->HasAuthority(EntityId, SpatialConstants::TOMBSTONE_COMPONENT_ID));

	Worker_AddComponentOp AddComponentOp{};
	AddComponentOp.entity_id = EntityId;
	AddComponentOp.data = Tombstone().CreateData();
	Connection->SendAddComponent(EntityId, &AddComponentOp.data);
	StaticComponentView->OnAddComponent(AddComponentOp);

#if WITH_EDITOR
	NetDriver->TrackTombstone(EntityId);
#endif
}
