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
#include "Interop/GlobalStateManager.h"
#include "Interop/SpatialReceiver.h"
#include "LoadBalancing/AbstractLBStrategy.h"
#include "Net/NetworkProfiler.h"
#include "Schema/AuthorityIntent.h"
#include "Schema/ClientRPCEndpointLegacy.h"
#include "Schema/ComponentPresence.h"
#include "Schema/Interest.h"
#include "Schema/RPCPayload.h"
#include "Schema/ServerRPCEndpointLegacy.h"
#include "Schema/ServerWorker.h"
#include "Schema/StandardLibrary.h"
#include "Schema/Tombstone.h"
#include "SpatialConstants.h"
#include "Utils/ComponentFactory.h"
#include "Utils/EntityFactory.h"
#include "Utils/InterestFactory.h"
#include "Utils/RepLayoutUtils.h"
#include "Utils/SpatialActorUtils.h"
#include "Utils/SpatialDebugger.h"
#include "Utils/SpatialLatencyTracer.h"
#include "Utils/SpatialMetrics.h"
#include "Utils/SpatialStatics.h"

DEFINE_LOG_CATEGORY(LogSpatialSender);

using namespace SpatialGDK;

DECLARE_CYCLE_STAT(TEXT("Sender SendComponentUpdates"), STAT_SpatialSenderSendComponentUpdates, STATGROUP_SpatialNet);
DECLARE_CYCLE_STAT(TEXT("Sender ResetOutgoingUpdate"), STAT_SpatialSenderResetOutgoingUpdate, STATGROUP_SpatialNet);
DECLARE_CYCLE_STAT(TEXT("Sender QueueOutgoingUpdate"), STAT_SpatialSenderQueueOutgoingUpdate, STATGROUP_SpatialNet);
DECLARE_CYCLE_STAT(TEXT("Sender UpdateInterestComponent"), STAT_SpatialSenderUpdateInterestComponent, STATGROUP_SpatialNet);
DECLARE_CYCLE_STAT(TEXT("Sender FlushRetryRPCs"), STAT_SpatialSenderFlushRetryRPCs, STATGROUP_SpatialNet);
DECLARE_CYCLE_STAT(TEXT("Sender SendRPC"), STAT_SpatialSenderSendRPC, STATGROUP_SpatialNet);

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

void USpatialSender::Init(USpatialNetDriver* InNetDriver, FTimerManager* InTimerManager, SpatialGDK::SpatialRPCService* InRPCService)
{
	NetDriver = InNetDriver;
	StaticComponentView = InNetDriver->StaticComponentView;
	Connection = InNetDriver->Connection;
	Receiver = InNetDriver->Receiver;
	PackageMap = InNetDriver->PackageMap;
	ClassInfoManager = InNetDriver->ClassInfoManager;
	TimerManager = InTimerManager;
	RPCService = InRPCService;

	OutgoingRPCs.BindProcessingFunction(FProcessRPCDelegate::CreateUObject(this, &USpatialSender::SendRPC));

	// Attempt to send RPCs that might have been queued while waiting for authority over entities this worker created.
	if (GetDefault<USpatialGDKSettings>()->QueuedOutgoingRPCRetryTime > 0.0f)
	{
		PeriodicallyProcessOutgoingRPCs();
	}
}

Worker_RequestId USpatialSender::CreateEntity(USpatialActorChannel* Channel, uint32& OutBytesWritten)
{
	EntityFactory DataFactory(NetDriver, PackageMap, ClassInfoManager, RPCService);
	TArray<FWorkerComponentData> ComponentDatas = DataFactory.CreateEntityComponents(Channel, OutgoingOnCreateEntityRPCs, OutBytesWritten);

	// If the Actor was loaded rather than dynamically spawned, associate it with its owning sublevel.
	ComponentDatas.Add(CreateLevelComponentData(Channel->Actor));

	ComponentDatas.Add(ComponentPresence(EntityFactory::GetComponentPresenceList(ComponentDatas)).CreateComponentPresenceData());

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

void USpatialSender::PeriodicallyProcessOutgoingRPCs()
{
	FTimerHandle Timer;
	TimerManager->SetTimer(Timer, [WeakThis = TWeakObjectPtr<USpatialSender>(this)]()
	{
		if (USpatialSender* SpatialSender = WeakThis.Get())
		{
			SpatialSender->OutgoingRPCs.ProcessRPCs();
		}
	}, GetDefault<USpatialGDKSettings>()->QueuedOutgoingRPCRetryTime, true);
}

void USpatialSender::SendAddComponentForSubobject(USpatialActorChannel* Channel, UObject* Subobject, const FClassInfo& SubobjectInfo, uint32& OutBytesWritten)
{
	FRepChangeState SubobjectRepChanges = Channel->CreateInitialRepChangeState(Subobject);
	FHandoverChangeState SubobjectHandoverChanges = Channel->CreateInitialHandoverChangeState(SubobjectInfo);

	ComponentFactory DataFactory(false, NetDriver, USpatialLatencyTracer::GetTracer(Subobject));

	TArray<FWorkerComponentData> SubobjectDatas = DataFactory.CreateComponentDatas(Subobject, SubobjectInfo, SubobjectRepChanges, SubobjectHandoverChanges, OutBytesWritten);
	SendAddComponents(Channel->GetEntityId(), SubobjectDatas);

	Channel->PendingDynamicSubobjects.Remove(TWeakObjectPtr<UObject>(Subobject));
}

void USpatialSender::SendAddComponents(Worker_EntityId EntityId, TArray<FWorkerComponentData> ComponentDatas)
{
	if (ComponentDatas.Num() == 0)
	{
		return;
	}

	// Update ComponentPresence.
	check(StaticComponentView->HasAuthority(EntityId, SpatialConstants::COMPONENT_PRESENCE_COMPONENT_ID));
	ComponentPresence* Presence = StaticComponentView->GetComponentData<ComponentPresence>(EntityId);
	Presence->AddComponentDataIds(ComponentDatas);
	FWorkerComponentUpdate Update = Presence->CreateComponentPresenceUpdate();
	Connection->SendComponentUpdate(EntityId, &Update);

	for (FWorkerComponentData& ComponentData : ComponentDatas)
	{
		Connection->SendAddComponent(EntityId, &ComponentData);
	}
}

void USpatialSender::GainAuthorityThenAddComponent(USpatialActorChannel* Channel, UObject* Object, const FClassInfo* Info)
{
	Worker_EntityId EntityId = Channel->GetEntityId();

	TSharedRef<FPendingSubobjectAttachment> PendingSubobjectAttachment = MakeShared<FPendingSubobjectAttachment>();
	PendingSubobjectAttachment->Subobject = Object;
	PendingSubobjectAttachment->Info = Info;

	// We collect component IDs related to the dynamic subobject being added to gain authority over.
	TArray<Worker_ComponentId> NewComponentIds;
	ForAllSchemaComponentTypes([&](ESchemaComponentType Type)
	{
		Worker_ComponentId ComponentId = Info->SchemaComponents[Type];
		if (ComponentId != SpatialConstants::INVALID_COMPONENT_ID)
		{
			// For each valid ComponentId, we need to wait for its authority delegation before
			// adding the subobject.
			PendingSubobjectAttachment->PendingAuthorityDelegations.Add(ComponentId);
			Receiver->PendingEntitySubobjectDelegations.Add(
				MakeTuple(static_cast<Worker_EntityId_Key>(EntityId), ComponentId),
				PendingSubobjectAttachment);

			NewComponentIds.Add(ComponentId);
		}
	});

	// If this worker is EntityACL authoritative, we can directly update the component IDs to gain authority over.
	if (StaticComponentView->HasAuthority(EntityId, SpatialConstants::ENTITY_ACL_COMPONENT_ID))
	{
		const WorkerRequirementSet AuthoritativeWorkerRequirementSet = { SpatialConstants::UnrealServerAttributeSet };

		EntityAcl* EntityACL = StaticComponentView->GetComponentData<EntityAcl>(Channel->GetEntityId());
		for (auto& ComponentId : NewComponentIds)
		{
			EntityACL->ComponentWriteAcl.Add(ComponentId, AuthoritativeWorkerRequirementSet);
		}

		FWorkerComponentUpdate Update = EntityACL->CreateEntityAclUpdate();
		Connection->SendComponentUpdate(Channel->GetEntityId(), &Update);
	}

	// Update the ComponentPresence component with the new component IDs. If this worker does not have EntityACL
	// authority, this component is used to inform the enforcer of the component IDs to add to the EntityACL.
	check(StaticComponentView->HasAuthority(EntityId, SpatialConstants::COMPONENT_PRESENCE_COMPONENT_ID));
	ComponentPresence* ComponentPresenceData = StaticComponentView->GetComponentData<ComponentPresence>(EntityId);
	ComponentPresenceData->AddComponentIds(NewComponentIds);
	FWorkerComponentUpdate Update = ComponentPresenceData->CreateComponentPresenceUpdate();
	Connection->SendComponentUpdate(Channel->GetEntityId(), &Update);
}

void USpatialSender::SendRemoveComponentForClassInfo(Worker_EntityId EntityId, const FClassInfo& Info)
{
	TArray<Worker_ComponentId> ComponentsToRemove;
	ComponentsToRemove.Reserve(SCHEMA_Count);
	for (Worker_ComponentId SubobjectComponentId : Info.SchemaComponents)
	{
		if (SubobjectComponentId != SpatialConstants::INVALID_COMPONENT_ID)
		{
			ComponentsToRemove.Add(SubobjectComponentId);
		}
	}

	SendRemoveComponents(EntityId, ComponentsToRemove);

	PackageMap->RemoveSubobject(FUnrealObjectRef(EntityId, Info.SchemaComponents[SCHEMA_Data]));
}

void USpatialSender::SendRemoveComponents(Worker_EntityId EntityId, TArray<Worker_ComponentId> ComponentIds)
{
	check(StaticComponentView->HasAuthority(EntityId, SpatialConstants::COMPONENT_PRESENCE_COMPONENT_ID));
	ComponentPresence* ComponentPresenceData = StaticComponentView->GetComponentData<ComponentPresence>(EntityId);
	ComponentPresenceData->RemoveComponentIds(ComponentIds);
	FWorkerComponentUpdate Update = ComponentPresenceData->CreateComponentPresenceUpdate();
	Connection->SendComponentUpdate(EntityId, &Update);

	for (auto ComponentId : ComponentIds)
	{
		Connection->SendRemoveComponent(EntityId, ComponentId);
	}
}

void USpatialSender::CreateServerWorkerEntity()
{
	RetryServerWorkerEntityCreation(PackageMap->AllocateEntityId(), 1);
}

// Creates an entity authoritative on this server worker, ensuring it will be able to receive updates for the GSM.
void USpatialSender::RetryServerWorkerEntityCreation(Worker_EntityId EntityId, int AttemptCounter)
{
	const WorkerRequirementSet WorkerIdPermission{ { FString::Format(TEXT("workerId:{0}"), { Connection->GetWorkerId() }) } };

	WriteAclMap ComponentWriteAcl;
	ComponentWriteAcl.Add(SpatialConstants::POSITION_COMPONENT_ID, WorkerIdPermission);
	ComponentWriteAcl.Add(SpatialConstants::METADATA_COMPONENT_ID, WorkerIdPermission);
	ComponentWriteAcl.Add(SpatialConstants::ENTITY_ACL_COMPONENT_ID, WorkerIdPermission);
	ComponentWriteAcl.Add(SpatialConstants::INTEREST_COMPONENT_ID, WorkerIdPermission);
	ComponentWriteAcl.Add(SpatialConstants::SERVER_WORKER_COMPONENT_ID, WorkerIdPermission);
	ComponentWriteAcl.Add(SpatialConstants::COMPONENT_PRESENCE_COMPONENT_ID, WorkerIdPermission);

	FString LayerHint = TEXT("");
	FParse::Value(FCommandLine::Get(), TEXT("-LayerHint"), LayerHint);

	TArray<FWorkerComponentData> Components;
	Components.Add(Position().CreatePositionData());
	Components.Add(Metadata(FString::Format(TEXT("WorkerEntity:{0}"), { Connection->GetWorkerId() })).CreateMetadataData());
	Components.Add(EntityAcl(WorkerIdPermission, ComponentWriteAcl).CreateEntityAclData());
	Components.Add(ServerWorker(Connection->GetWorkerId(), false, LayerHint).CreateServerWorkerData());
	check(NetDriver != nullptr);
	// It is unlikely the load balance strategy would be set up at this point, but we call this function again later when it is ready in order
	// to set the interest of the server worker according to the strategy.
	Components.Add(NetDriver->InterestFactory->CreateServerWorkerInterest(NetDriver->LoadBalanceStrategy).CreateInterestData());
	Components.Add(ComponentPresence(EntityFactory::GetComponentPresenceList(Components)).CreateComponentPresenceData());

	const Worker_RequestId RequestId = Connection->SendCreateEntityRequest(MoveTemp(Components), &EntityId);

	CreateEntityDelegate OnCreateWorkerEntityResponse;
	OnCreateWorkerEntityResponse.BindLambda([WeakSender = TWeakObjectPtr<USpatialSender>(this), EntityId, AttemptCounter](const Worker_CreateEntityResponseOp& Op)
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

		// Given the nature of commands, it's possible we have multiple create commands in flight at once. If a command fails where
		// we've already set the worker entity ID locally, this means we already successfully create the entity, so nothing needs doing.
		if (Op.status_code != WORKER_STATUS_CODE_SUCCESS && Sender->NetDriver->WorkerEntityId != SpatialConstants::INVALID_ENTITY_ID)
		{
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
		Sender->TimerManager->SetTimer(RetryTimer, [WeakSender, EntityId, AttemptCounter]()
		{
			if (USpatialSender* SpatialSender = WeakSender.Get())
			{
				SpatialSender->RetryServerWorkerEntityCreation(EntityId, AttemptCounter + 1);
			}
		}, SpatialConstants::GetCommandRetryWaitTimeSeconds(AttemptCounter), false);
	});

	Receiver->AddCreateEntityDelegate(RequestId, MoveTemp(OnCreateWorkerEntityResponse));
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

void USpatialSender::DeleteEntityComponentData(TArray<FWorkerComponentData>& EntityComponents)
{
	for (FWorkerComponentData& Component : EntityComponents)
	{
		Schema_DestroyComponentData(Component.schema_type);
	}

	EntityComponents.Empty();
}

TArray<FWorkerComponentData> USpatialSender::CopyEntityComponentData(const TArray<FWorkerComponentData>& EntityComponents)
{
	TArray<FWorkerComponentData> Copy;
	Copy.Reserve(EntityComponents.Num());
	for (const FWorkerComponentData& Component : EntityComponents)
	{
		Copy.Emplace(Worker_ComponentData{
			Component.reserved,
			Component.component_id,
			Schema_CopyComponentData(Component.schema_type),
			nullptr
		});
	}

	return Copy;
}

void USpatialSender::CreateEntityWithRetries(Worker_EntityId EntityId, FString EntityName, TArray<FWorkerComponentData> EntityComponents)
{
	const Worker_RequestId RequestId = Connection->SendCreateEntityRequest(CopyEntityComponentData(EntityComponents), &EntityId);

	CreateEntityDelegate Delegate;

	Delegate.BindLambda([this, EntityId, Name = MoveTemp(EntityName), Components = MoveTemp(EntityComponents)](const Worker_CreateEntityResponseOp& Op) mutable
	{
		switch (Op.status_code)
		{
		case WORKER_STATUS_CODE_SUCCESS:
			UE_LOG(LogSpatialSender, Log, TEXT("Created entity. "
				"Entity name: %s, entity id: %lld"), *Name, EntityId);
			DeleteEntityComponentData(Components);
			break;
		case WORKER_STATUS_CODE_TIMEOUT:
			UE_LOG(LogSpatialSender, Log, TEXT("Timed out creating entity. Retrying. "
				"Entity name: %s, entity id: %lld"), *Name, EntityId);
			CreateEntityWithRetries(EntityId, MoveTemp(Name), MoveTemp(Components));
			break;
		default:
			UE_LOG(LogSpatialSender, Log, TEXT("Failed to create entity. It might already be created. Not retrying. "
				"Entity name: %s, entity id: %lld"), *Name, EntityId);
			DeleteEntityComponentData(Components);
			break;
		}
	});

	Receiver->AddCreateEntityDelegate(RequestId, MoveTemp(Delegate));
}

void USpatialSender::UpdateServerWorkerEntityInterestAndPosition()
{
	check(Connection != nullptr);
	check(NetDriver != nullptr);
	if (NetDriver->WorkerEntityId == SpatialConstants::INVALID_ENTITY_ID)
	{
		// No worker entity to update.
		return;
	}

	// Update the interest. If it's ready and not null, also adds interest according to the load balancing strategy.
	FWorkerComponentUpdate InterestUpdate = NetDriver->InterestFactory->CreateServerWorkerInterest(NetDriver->LoadBalanceStrategy).CreateInterestUpdate();
	Connection->SendComponentUpdate(NetDriver->WorkerEntityId, &InterestUpdate);

	if (NetDriver->LoadBalanceStrategy != nullptr && NetDriver->LoadBalanceStrategy->IsReady())
	{
		// Also update the position of the worker entity to the centre of the load balancing region.
		SendPositionUpdate(NetDriver->WorkerEntityId, NetDriver->LoadBalanceStrategy->GetWorkerEntityPosition());
	}
}

void USpatialSender::SendComponentUpdates(UObject* Object, const FClassInfo& Info, USpatialActorChannel* Channel, const FRepChangeState* RepChanges, const FHandoverChangeState* HandoverChanges, uint32& OutBytesWritten)
{
	SCOPE_CYCLE_COUNTER(STAT_SpatialSenderSendComponentUpdates);
	Worker_EntityId EntityId = Channel->GetEntityId();

	UE_LOG(LogSpatialSender, Verbose, TEXT("Sending component update (object: %s, entity: %lld)"), *Object->GetName(), EntityId);

	USpatialLatencyTracer* Tracer = USpatialLatencyTracer::GetTracer(Object);
	ComponentFactory UpdateFactory(Channel->GetInterestDirty(), NetDriver, Tracer);

	TArray<FWorkerComponentUpdate> ComponentUpdates = UpdateFactory.CreateComponentUpdates(Object, Info, EntityId, RepChanges, HandoverChanges, OutBytesWritten);

	for(int i = 0; i < ComponentUpdates.Num(); i++)
	{
		FWorkerComponentUpdate& Update = ComponentUpdates[i];
		if (!NetDriver->StaticComponentView->HasAuthority(EntityId, Update.component_id))
		{
			UE_LOG(LogSpatialSender, Verbose, TEXT("Trying to send component update but don't have authority! Update will be queued and sent when authority gained. Component Id: %d, entity: %lld"), Update.component_id, EntityId);

			// This is a temporary fix. A task to improve this has been created: UNR-955
			// It may be the case that upon resolving a component, we do not have authority to send the update. In this case, we queue the update, to send upon receiving authority.
			// Note: This will break in a multi-worker context, if we try to create an entity that we don't intend to have authority over. For this reason, this fix is only temporary.
			TArray<FWorkerComponentUpdate>& UpdatesQueuedUntilAuthority = UpdatesQueuedUntilAuthorityMap.FindOrAdd(EntityId);
			UpdatesQueuedUntilAuthority.Add(Update);
			continue;
		}

		Connection->SendComponentUpdate(EntityId, &Update);
	}
}

// Apply (and clean up) any updates queued, due to being sent previously when they didn't have authority.
void USpatialSender::ProcessUpdatesQueuedUntilAuthority(Worker_EntityId EntityId, Worker_ComponentId ComponentId)
{
	if (TArray<FWorkerComponentUpdate>* UpdatesQueuedUntilAuthority = UpdatesQueuedUntilAuthorityMap.Find(EntityId))
	{
		for (auto It = UpdatesQueuedUntilAuthority->CreateIterator(); It; It++)
		{
			if (ComponentId == It->component_id)
			{
				Connection->SendComponentUpdate(EntityId, &(*It));
				It.RemoveCurrent();
			}
		}

		if (UpdatesQueuedUntilAuthority->Num() == 0)
		{
			UpdatesQueuedUntilAuthorityMap.Remove(EntityId);
		}
	}
}

void USpatialSender::FlushRPCService()
{
	if (RPCService != nullptr)
	{
		RPCService->PushOverflowedRPCs();

		TArray<SpatialRPCService::UpdateToSend> RPCs = RPCService->GetRPCsAndAcksToSend();
		for (SpatialRPCService::UpdateToSend& Update : RPCs)
		{
			Connection->SendComponentUpdate(Update.EntityId, &Update.Update);
		}

		if (RPCs.Num())
		{
			Connection->MaybeFlush();
		}
	}
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

void USpatialSender::SendInterestBucketComponentChange(const Worker_EntityId EntityId, const Worker_ComponentId OldComponent, const Worker_ComponentId NewComponent)
{
	if (OldComponent != SpatialConstants::INVALID_COMPONENT_ID)
	{
		// No loopback, so simulate the operations locally.
		Worker_RemoveComponentOp RemoveOp{};
		RemoveOp.entity_id = EntityId;
		RemoveOp.component_id = OldComponent;
		StaticComponentView->OnRemoveComponent(RemoveOp);

		SendRemoveComponents(EntityId, { OldComponent });
	}

	if (NewComponent != SpatialConstants::INVALID_COMPONENT_ID)
	{
		Worker_AddComponentOp AddOp{};
		AddOp.entity_id = EntityId;
		AddOp.data.component_id = NewComponent;
		AddOp.data.schema_type = nullptr;
		AddOp.data.user_handle = nullptr;

		StaticComponentView->OnAddComponent(AddOp);

		SendAddComponents(EntityId, { ComponentFactory::CreateEmptyComponentData(NewComponent) });
	}
}

void USpatialSender::SendActorTornOffUpdate(Worker_EntityId EntityId, Worker_ComponentId ComponentId)
{
	FWorkerComponentUpdate ComponentUpdate = {};

	ComponentUpdate.component_id = ComponentId;
	ComponentUpdate.schema_type = Schema_CreateComponentUpdate();
	Schema_Object* ComponentObject = Schema_GetComponentUpdateFields(ComponentUpdate.schema_type);

	Schema_AddBool(ComponentObject, SpatialConstants::ACTOR_TEAROFF_ID, 1);

	Connection->SendComponentUpdate(EntityId, &ComponentUpdate);
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

	FWorkerComponentUpdate Update = Position::CreatePositionUpdate(Coordinates::FromFVector(Location));
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

	FWorkerComponentUpdate Update = AuthorityIntentComponent->CreateAuthorityIntentUpdate();
	Connection->SendComponentUpdate(EntityId, &Update);

	// Also notify the enforcer directly on the worker that sends the component update, as the update will short circuit
	NetDriver->LoadBalanceEnforcer->MaybeQueueAclAssignmentRequest(EntityId);
}

void USpatialSender::SetAclWriteAuthority(const SpatialLoadBalanceEnforcer::AclWriteAuthorityRequest& Request)
{
	check(NetDriver);
	check(StaticComponentView->HasComponent(Request.EntityId, SpatialConstants::ENTITY_ACL_COMPONENT_ID));

	const FString& WriteWorkerId = FString::Printf(TEXT("workerId:%s"), *Request.OwningWorkerId);

	const WorkerAttributeSet OwningServerWorkerAttributeSet = { WriteWorkerId };

	EntityAcl* NewAcl = StaticComponentView->GetComponentData<EntityAcl>(Request.EntityId);
	NewAcl->ReadAcl = Request.ReadAcl;

	for (const Worker_ComponentId& ComponentId : Request.ComponentIds)
	{
		if (ComponentId == SpatialConstants::HEARTBEAT_COMPONENT_ID
			|| ComponentId == SpatialConstants::GetClientAuthorityComponent(GetDefault<USpatialGDKSettings>()->UseRPCRingBuffer()))
		{
			NewAcl->ComponentWriteAcl.Add(ComponentId, Request.ClientRequirementSet);
			continue;
		}

		if (ComponentId == SpatialConstants::ENTITY_ACL_COMPONENT_ID)
		{
			NewAcl->ComponentWriteAcl.Add(ComponentId, { SpatialConstants::UnrealServerAttributeSet } );
			continue;
		}

		NewAcl->ComponentWriteAcl.Add(ComponentId, { OwningServerWorkerAttributeSet });
	}

	UE_LOG(LogSpatialLoadBalanceEnforcer, Verbose, TEXT("(%s) Setting Acl WriteAuth for entity %lld to %s"), *NetDriver->Connection->GetWorkerId(), Request.EntityId, *Request.OwningWorkerId);

	FWorkerComponentUpdate Update = NewAcl->CreateEntityAclUpdate();
	NetDriver->Connection->SendComponentUpdate(Request.EntityId, &Update);
}

FRPCErrorInfo USpatialSender::SendRPC(const FPendingRPCParams& Params)
{
	SCOPE_CYCLE_COUNTER(STAT_SpatialSenderSendRPC);

	TWeakObjectPtr<UObject> TargetObjectWeakPtr = PackageMap->GetObjectFromUnrealObjectRef(Params.ObjectRef);
	if (!TargetObjectWeakPtr.IsValid())
	{
		// Target object was destroyed before the RPC could be (re)sent
		return FRPCErrorInfo{ nullptr, nullptr, ERPCResult::UnresolvedTargetObject, ERPCQueueProcessResult::DropEntireQueue };
	}
	UObject* TargetObject = TargetObjectWeakPtr.Get();

	const FClassInfo& ClassInfo = ClassInfoManager->GetOrCreateClassInfoByObject(TargetObject);
	UFunction* Function = ClassInfo.RPCs[Params.Payload.Index];
	if (Function == nullptr)
	{
		return FRPCErrorInfo{ TargetObject, nullptr, ERPCResult::MissingFunctionInfo, ERPCQueueProcessResult::ContinueProcessing };
	}

	USpatialActorChannel* Channel = NetDriver->GetOrCreateSpatialActorChannel(TargetObject);
	if (Channel == nullptr)
	{
		return FRPCErrorInfo{ TargetObject, Function, ERPCResult::NoActorChannel, ERPCQueueProcessResult::DropEntireQueue };
	}

	const FRPCInfo& RPCInfo = ClassInfoManager->GetRPCInfo(TargetObject, Function);
	bool bUseRPCRingBuffer = GetDefault<USpatialGDKSettings>()->UseRPCRingBuffer();

	if (RPCInfo.Type == ERPCType::CrossServer)
	{
		SendCrossServerRPC(TargetObject, Function, Params.Payload, Channel, Params.ObjectRef);
		return FRPCErrorInfo{ TargetObject, Function, ERPCResult::Success };
	}

	if (bUseRPCRingBuffer && RPCService != nullptr)
	{
		if (SendRingBufferedRPC(TargetObject, Function, Params.Payload, Channel, Params.ObjectRef))
		{
			return FRPCErrorInfo{ TargetObject, Function, ERPCResult::Success };
		}
		else
		{
			return FRPCErrorInfo{ TargetObject, Function, ERPCResult::RPCServiceFailure };
		}
	}

	if (Channel->bCreatingNewEntity && Function->HasAnyFunctionFlags(FUNC_NetClient))
	{
		SendOnEntityCreationRPC(TargetObject, Function, Params.Payload, Channel, Params.ObjectRef);
		return FRPCErrorInfo{ TargetObject, Function, ERPCResult::Success };
	}

	return SendLegacyRPC(TargetObject, Function, Params.Payload, Channel, Params.ObjectRef);
}

void USpatialSender::SendOnEntityCreationRPC(UObject* TargetObject, UFunction* Function, const SpatialGDK::RPCPayload& Payload, USpatialActorChannel* Channel, const FUnrealObjectRef& TargetObjectRef)
{
	check(NetDriver->IsServer());

	const FRPCInfo& RPCInfo = ClassInfoManager->GetRPCInfo(TargetObject, Function);

	OutgoingOnCreateEntityRPCs.FindOrAdd(Channel->Actor).RPCs.Add(Payload);
#if !UE_BUILD_SHIPPING
	TrackRPC(Channel->Actor, Function, Payload, RPCInfo.Type);
#endif // !UE_BUILD_SHIPPING
}

void USpatialSender::SendCrossServerRPC(UObject* TargetObject, UFunction* Function, const SpatialGDK::RPCPayload& Payload, USpatialActorChannel* Channel, const FUnrealObjectRef& TargetObjectRef)
{
	const FRPCInfo& RPCInfo = ClassInfoManager->GetRPCInfo(TargetObject, Function);

	Worker_ComponentId ComponentId = SpatialConstants::SERVER_TO_SERVER_COMMAND_ENDPOINT_COMPONENT_ID;

	Worker_EntityId EntityId = SpatialConstants::INVALID_ENTITY_ID;
	Worker_CommandRequest CommandRequest = CreateRPCCommandRequest(TargetObject, Payload, ComponentId, RPCInfo.Index, EntityId);

	check(EntityId != SpatialConstants::INVALID_ENTITY_ID);
	Worker_RequestId RequestId = Connection->SendCommandRequest(EntityId, &CommandRequest, SpatialConstants::UNREAL_RPC_ENDPOINT_COMMAND_ID);

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
#if !UE_BUILD_SHIPPING
	TrackRPC(Channel->Actor, Function, Payload, RPCInfo.Type);
#endif // !UE_BUILD_SHIPPING
}

FRPCErrorInfo USpatialSender::SendLegacyRPC(UObject* TargetObject, UFunction* Function, const RPCPayload& Payload, USpatialActorChannel* Channel, const FUnrealObjectRef& TargetObjectRef)
{
	const FRPCInfo& RPCInfo = ClassInfoManager->GetRPCInfo(TargetObject, Function);

	// Check if the Channel is listening
	if ((RPCInfo.Type != ERPCType::NetMulticast) && !Channel->IsListening())
	{
		// If the Entity endpoint is not yet ready to receive RPCs -
		// treat the corresponding object as unresolved and queue RPC
		// However, it doesn't matter in case of Multicast
		return FRPCErrorInfo{ TargetObject, Function, ERPCResult::SpatialActorChannelNotListening };
	}

	// Check for Authority
	Worker_EntityId EntityId = TargetObjectRef.Entity;
	check(EntityId != SpatialConstants::INVALID_ENTITY_ID);

	Worker_ComponentId ComponentId = SpatialConstants::RPCTypeToWorkerComponentIdLegacy(RPCInfo.Type);
	if (!NetDriver->StaticComponentView->HasAuthority(EntityId, ComponentId))
	{
		ERPCQueueProcessResult QueueProcessResult = ERPCQueueProcessResult::DropEntireQueue;
		if (AActor* TargetActor = Cast<AActor>(TargetObject))
		{
			if (TargetActor->HasAuthority())
			{
				QueueProcessResult = ERPCQueueProcessResult::StopProcessing;
			}
		}
		return FRPCErrorInfo{ TargetObject, Function, ERPCResult::NoAuthority, QueueProcessResult };
	}

	FWorkerComponentUpdate ComponentUpdate = CreateRPCEventUpdate(TargetObject, Payload, ComponentId, RPCInfo.Index);

	Connection->SendComponentUpdate(EntityId, &ComponentUpdate);
	Connection->MaybeFlush();
#if !UE_BUILD_SHIPPING
	TrackRPC(Channel->Actor, Function, Payload, RPCInfo.Type);
#endif // !UE_BUILD_SHIPPING

	return FRPCErrorInfo{ TargetObject, Function, ERPCResult::Success };
}

bool USpatialSender::SendRingBufferedRPC(UObject* TargetObject, UFunction* Function, const SpatialGDK::RPCPayload& Payload, USpatialActorChannel* Channel, const FUnrealObjectRef& TargetObjectRef)
{
	const FRPCInfo& RPCInfo = ClassInfoManager->GetRPCInfo(TargetObject, Function);
	const EPushRPCResult Result = RPCService->PushRPC(TargetObjectRef.Entity, RPCInfo.Type, Payload, Channel->bCreatedEntity);

	if (Result == EPushRPCResult::Success)
	{
		FlushRPCService();
	}

#if !UE_BUILD_SHIPPING
	if (Result == EPushRPCResult::Success || Result == EPushRPCResult::QueueOverflowed)
	{
		TrackRPC(Channel->Actor, Function, Payload, RPCInfo.Type);
	}
#endif // !UE_BUILD_SHIPPING

	switch (Result)
	{
	case EPushRPCResult::QueueOverflowed:
		UE_LOG(LogSpatialSender, Log, TEXT("USpatialSender::SendRingBufferedRPC: Ring buffer queue overflowed, queuing RPC locally. Actor: %s, entity: %lld, function: %s"),
			*TargetObject->GetPathName(), TargetObjectRef.Entity, *Function->GetName());
		return true;
	case EPushRPCResult::DropOverflowed:
		UE_LOG(LogSpatialSender, Log, TEXT("USpatialSender::SendRingBufferedRPC: Ring buffer queue overflowed, dropping RPC. Actor: %s, entity: %lld, function: %s"),
			*TargetObject->GetPathName(), TargetObjectRef.Entity, *Function->GetName());
		return true;
	case EPushRPCResult::HasAckAuthority:
		UE_LOG(LogSpatialSender, Warning,
			TEXT("USpatialSender::SendRingBufferedRPC: Worker has authority over ack component for RPC it is sending. RPC will not be sent. Actor: %s, entity: %lld, function: %s"),
			*TargetObject->GetPathName(), TargetObjectRef.Entity, *Function->GetName());
		return true;
	case EPushRPCResult::NoRingBufferAuthority:
		// TODO: Change engine logic that calls Client RPCs from non-auth servers and change this to error. UNR-2517
		UE_LOG(LogSpatialSender, Log,
			TEXT("USpatialSender::SendRingBufferedRPC: Failed to send RPC because the worker does not have authority over ring buffer component. Actor: %s, entity: %lld, function: %s"),
			*TargetObject->GetPathName(), TargetObjectRef.Entity, *Function->GetName());
		return true;
	case EPushRPCResult::EntityBeingCreated:
		UE_LOG(LogSpatialSender, Log,
			TEXT("USpatialSender::SendRingBufferedRPC: RPC was called between entity creation and initial authority gain, so it will be queued. Actor: %s, entity: %lld, function: %s"),
			*TargetObject->GetPathName(), TargetObjectRef.Entity, *Function->GetName());
		return false;
	default:
		return true;
	}
}

#if !UE_BUILD_SHIPPING
void USpatialSender::TrackRPC(AActor* Actor, UFunction* Function, const RPCPayload& Payload, const ERPCType RPCType)
{
	NETWORK_PROFILER(GNetworkProfiler.TrackSendRPC(Actor, Function, 0, Payload.CountDataBits(), 0, NetDriver->GetSpatialOSNetConnection()));
	NetDriver->SpatialMetrics->TrackSentRPC(Function, RPCType, Payload.PayloadData.Num());
}
#endif

void USpatialSender::EnqueueRetryRPC(TSharedRef<FReliableRPCForRetry> RetryRPC)
{
	RetryRPCs.Add(RetryRPC);
}

void USpatialSender::FlushRetryRPCs()
{
	SCOPE_CYCLE_COUNTER(STAT_SpatialSenderFlushRetryRPCs);

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

void USpatialSender::SendCreateEntityRequest(USpatialActorChannel* Channel, uint32& OutBytesWritten)
{
	UE_LOG(LogSpatialSender, Log, TEXT("Sending create entity request for %s with EntityId %lld, HasAuthority: %d"), *Channel->Actor->GetName(), Channel->GetEntityId(), Channel->Actor->HasAuthority());

	Worker_RequestId RequestId = CreateEntity(Channel, OutBytesWritten);

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
	FWorkerComponentUpdate Update = RPCsOnEntityCreation::CreateClearFieldsUpdate();
	NetDriver->Connection->SendComponentUpdate(EntityId, &Update);
}

void USpatialSender::SendClientEndpointReadyUpdate(Worker_EntityId EntityId)
{
	ClientRPCEndpointLegacy Endpoint;
	Endpoint.bReady = true;
	FWorkerComponentUpdate Update = Endpoint.CreateRPCEndpointUpdate();
	NetDriver->Connection->SendComponentUpdate(EntityId, &Update);
}

void USpatialSender::SendServerEndpointReadyUpdate(Worker_EntityId EntityId)
{
	ServerRPCEndpointLegacy Endpoint;
	Endpoint.bReady = true;
	FWorkerComponentUpdate Update = Endpoint.CreateRPCEndpointUpdate();
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

FWorkerComponentUpdate USpatialSender::CreateRPCEventUpdate(UObject* TargetObject, const RPCPayload& Payload, Worker_ComponentId ComponentId, Schema_FieldId EventIndex)
{
	FWorkerComponentUpdate ComponentUpdate = {};

	ComponentUpdate.component_id = ComponentId;
	ComponentUpdate.schema_type = Schema_CreateComponentUpdate();
	Schema_Object* EventsObject = Schema_GetComponentUpdateEvents(ComponentUpdate.schema_type);
	Schema_Object* EventData = Schema_AddObject(EventsObject, SpatialConstants::UNREAL_RPC_ENDPOINT_EVENT_ID);

	FUnrealObjectRef TargetObjectRef(PackageMap->GetUnrealObjectRefFromNetGUID(PackageMap->GetNetGUIDFromObject(TargetObject)));
	ensure(TargetObjectRef != FUnrealObjectRef::UNRESOLVED_OBJECT_REF);

	Payload.WriteToSchemaObject(EventData);

#if TRACE_LIB_ACTIVE
	ComponentUpdate.Trace = Payload.Trace;
#endif

	return ComponentUpdate;
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
void USpatialSender::UpdateClientAuthoritativeComponentAclEntries(Worker_EntityId EntityId, const FString& OwnerWorkerAttribute)
{
	check(StaticComponentView->HasAuthority(EntityId, SpatialConstants::ENTITY_ACL_COMPONENT_ID));

	WorkerAttributeSet OwningClientAttribute = { OwnerWorkerAttribute };
	WorkerRequirementSet OwningClientOnly = { OwningClientAttribute };

	EntityAcl* EntityACL = StaticComponentView->GetComponentData<EntityAcl>(EntityId);
	EntityACL->ComponentWriteAcl.Add(SpatialConstants::GetClientAuthorityComponent(GetDefault<USpatialGDKSettings>()->UseRPCRingBuffer()), OwningClientOnly);
	EntityACL->ComponentWriteAcl.Add(SpatialConstants::HEARTBEAT_COMPONENT_ID, OwningClientOnly);
	FWorkerComponentUpdate Update = EntityACL->CreateEntityAclUpdate();

	Connection->SendComponentUpdate(EntityId, &Update);
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

	FWorkerComponentUpdate Update = NetDriver->InterestFactory->CreateInterestUpdate(Actor, ClassInfoManager->GetOrCreateClassInfoByObject(Actor), EntityId);

	Connection->SendComponentUpdate(EntityId, &Update);
}

void USpatialSender::RetireEntity(const Worker_EntityId EntityId, bool bIsNetStartupActor)
{
	if (bIsNetStartupActor)
	{
		Receiver->RemoveActor(EntityId);
		// In the case that this is a startup actor, we won't actually delete the entity in SpatialOS.  Instead we'll Tombstone it.
		if (!StaticComponentView->HasComponent(EntityId, SpatialConstants::TOMBSTONE_COMPONENT_ID))
		{
			UE_LOG(LogSpatialSender, Log, TEXT("Adding tombstone to entity: %lld"), EntityId);
			AddTombstoneToEntity(EntityId);
		}
		else
		{
			UE_LOG(LogSpatialSender, Verbose, TEXT("RetireEntity called on already retired entity: %lld"), EntityId);
		}
	}
	else
	{
		// Actor no longer guaranteed to be in package map, but still useful for additional logging info
		AActor* Actor = Cast<AActor>(PackageMap->GetObjectFromEntityId(EntityId));

		UE_LOG(LogSpatialSender, Log, TEXT("Sending delete entity request for %s with EntityId %lld, HasAuthority: %d"), *GetPathNameSafe(Actor), EntityId, Actor != nullptr ? Actor->HasAuthority() : false);
		Connection->SendDeleteEntityRequest(EntityId);
	}
}

void USpatialSender::CreateTombstoneEntity(AActor* Actor)
{
	check(Actor->IsNetStartupActor());

	const Worker_EntityId EntityId = NetDriver->PackageMap->AllocateEntityIdAndResolveActor(Actor);

	EntityFactory DataFactory(NetDriver, PackageMap, ClassInfoManager, RPCService);
	TArray<FWorkerComponentData> Components = DataFactory.CreateTombstoneEntityComponents(Actor);

	Components.Add(CreateLevelComponentData(Actor));

	Components.Add(ComponentPresence(EntityFactory::GetComponentPresenceList(Components)).CreateComponentPresenceData());

	CreateEntityWithRetries(EntityId, Actor->GetName(), MoveTemp(Components));

	UE_LOG(LogSpatialSender, Log, TEXT("Creating tombstone entity for actor. "
		"Actor: %s. Entity ID: %d."), *Actor->GetName(), EntityId);

#if WITH_EDITOR
	NetDriver->TrackTombstone(EntityId);
#endif
}

void USpatialSender::AddTombstoneToEntity(const Worker_EntityId EntityId)
{
	check(NetDriver->StaticComponentView->HasAuthority(EntityId, SpatialConstants::TOMBSTONE_COMPONENT_ID));

	Worker_AddComponentOp AddComponentOp{};
	AddComponentOp.entity_id = EntityId;
	AddComponentOp.data = Tombstone().CreateData();
	SendAddComponents(EntityId, { AddComponentOp.data });
	StaticComponentView->OnAddComponent(AddComponentOp);

#if WITH_EDITOR
	NetDriver->TrackTombstone(EntityId);
#endif
}
