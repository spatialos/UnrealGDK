// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "Interop/SpatialReceiver.h"

#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/PlayerState.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"

#include "EngineClasses/SpatialActorChannel.h"
#include "EngineClasses/SpatialFastArrayNetSerialize.h"
#include "EngineClasses/SpatialNetConnection.h"
#include "EngineClasses/SpatialPackageMapClient.h"
#include "EngineClasses/SpatialVirtualWorkerTranslator.h"
#include "EngineClasses/SpatialLoadBalanceEnforcer.h"
#include "Interop/Connection/SpatialWorkerConnection.h"
#include "Interop/GlobalStateManager.h"
#include "Interop/SpatialPlayerSpawner.h"
#include "Interop/SpatialSender.h"
#include "Schema/AlwaysRelevant.h"
#include "Schema/DynamicComponent.h"
#include "Schema/RPCPayload.h"
#include "Schema/SpawnData.h"
#include "Schema/Tombstone.h"
#include "Schema/UnrealMetadata.h"
#include "SpatialConstants.h"
#include "Utils/ComponentReader.h"
#include "Utils/ErrorCodeRemapping.h"
#include "Utils/RepLayoutUtils.h"
#include "Utils/SpatialDebugger.h"
#include "Utils/SpatialMetrics.h"

DEFINE_LOG_CATEGORY(LogSpatialReceiver);

DECLARE_CYCLE_STAT(TEXT("PendingOpsOnChannel"), STAT_SpatialPendingOpsOnChannel, STATGROUP_SpatialNet);

DECLARE_CYCLE_STAT(TEXT("Receiver CritSection"), STAT_ReceiverCritSection, STATGROUP_SpatialNet);
DECLARE_CYCLE_STAT(TEXT("Receiver AddEntity"), STAT_ReceiverAddEntity, STATGROUP_SpatialNet);
DECLARE_CYCLE_STAT(TEXT("Receiver RemoveEntity"), STAT_ReceiverRemoveEntity, STATGROUP_SpatialNet);
DECLARE_CYCLE_STAT(TEXT("Receiver AddComponent"), STAT_ReceiverAddComponent, STATGROUP_SpatialNet);
DECLARE_CYCLE_STAT(TEXT("Receiver ComponentUpdate"), STAT_ReceiverComponentUpdate, STATGROUP_SpatialNet);
DECLARE_CYCLE_STAT(TEXT("Receiver ApplyData"), STAT_ReceiverApplyData, STATGROUP_SpatialNet);
DECLARE_CYCLE_STAT(TEXT("Receiver ApplyHandover"), STAT_ReceiverApplyHandover, STATGROUP_SpatialNet);
DECLARE_CYCLE_STAT(TEXT("Receiver HandleRPC"), STAT_ReceiverHandleRPC, STATGROUP_SpatialNet);
DECLARE_CYCLE_STAT(TEXT("Receiver HandleRPCLegacy"), STAT_ReceiverHandleRPCLegacy, STATGROUP_SpatialNet);
DECLARE_CYCLE_STAT(TEXT("Receiver CommandRequest"), STAT_ReceiverCommandRequest, STATGROUP_SpatialNet);
DECLARE_CYCLE_STAT(TEXT("Receiver CommandResponse"), STAT_ReceiverCommandResponse, STATGROUP_SpatialNet);
DECLARE_CYCLE_STAT(TEXT("Receiver AuthorityChange"), STAT_ReceiverAuthChange, STATGROUP_SpatialNet);
DECLARE_CYCLE_STAT(TEXT("Receiver ReserveEntityIds"), STAT_ReceiverReserveEntityIds, STATGROUP_SpatialNet);
DECLARE_CYCLE_STAT(TEXT("Receiver CreateEntityResponse"), STAT_ReceiverCreateEntityResponse, STATGROUP_SpatialNet);
DECLARE_CYCLE_STAT(TEXT("Receiver EntityQueryResponse"), STAT_ReceiverEntityQueryResponse, STATGROUP_SpatialNet);
using namespace SpatialGDK;

void USpatialReceiver::Init(USpatialNetDriver* InNetDriver, FTimerManager* InTimerManager, SpatialGDK::SpatialRPCService* InRPCService)
{
	NetDriver = InNetDriver;
	StaticComponentView = InNetDriver->StaticComponentView;
	Sender = InNetDriver->Sender;
	PackageMap = InNetDriver->PackageMap;
	ClassInfoManager = InNetDriver->ClassInfoManager;
	GlobalStateManager = InNetDriver->GlobalStateManager;
	LoadBalanceEnforcer = InNetDriver->LoadBalanceEnforcer.Get();
	TimerManager = InTimerManager;
	RPCService = InRPCService;

	IncomingRPCs.BindProcessingFunction(FProcessRPCDelegate::CreateUObject(this, &USpatialReceiver::ApplyRPC));
	PeriodicallyProcessIncomingRPCs();
}

void USpatialReceiver::OnCriticalSection(bool InCriticalSection)
{
	SCOPE_CYCLE_COUNTER(STAT_ReceiverCritSection);
	if (InCriticalSection)
	{
		EnterCriticalSection();
	}
	else
	{
		LeaveCriticalSection();
	}
}

void USpatialReceiver::EnterCriticalSection()
{
	UE_LOG(LogSpatialReceiver, Verbose, TEXT("Entering critical section."));
	check(!bInCriticalSection);
	bInCriticalSection = true;
}

void USpatialReceiver::LeaveCriticalSection()
{
	UE_LOG(LogSpatialReceiver, Verbose, TEXT("Leaving critical section."));
	check(bInCriticalSection);

	for (Worker_EntityId& PendingAddEntity : PendingAddEntities)
	{
		ReceiveActor(PendingAddEntity);
		if (!IsEntityWaitingForAsyncLoad(PendingAddEntity))
		{
			OnEntityAddedDelegate.Broadcast(PendingAddEntity);
		}
	}

	for (Worker_AuthorityChangeOp& PendingAuthorityChange : PendingAuthorityChanges)
	{
		HandleActorAuthority(PendingAuthorityChange);
	}

	// Mark that we've left the critical section.
	bInCriticalSection = false;
	PendingAddEntities.Empty();
	PendingAddComponents.Empty();
	PendingAuthorityChanges.Empty();

	ProcessQueuedResolvedObjects();
}

void USpatialReceiver::OnAddEntity(const Worker_AddEntityOp& Op)
{
	SCOPE_CYCLE_COUNTER(STAT_ReceiverAddEntity);
	UE_LOG(LogSpatialReceiver, Verbose, TEXT("AddEntity: %lld"), Op.entity_id);

	check(bInCriticalSection);

	PendingAddEntities.Emplace(Op.entity_id);
}

void USpatialReceiver::OnAddComponent(const Worker_AddComponentOp& Op)
{
	SCOPE_CYCLE_COUNTER(STAT_ReceiverAddComponent);
	UE_LOG(LogSpatialReceiver, Verbose, TEXT("AddComponent component ID: %u entity ID: %lld"),
		Op.data.component_id, Op.entity_id);

	if (IsEntityWaitingForAsyncLoad(Op.entity_id))
	{
		QueueAddComponentOpForAsyncLoad(Op);
		return;
	}

	switch (Op.data.component_id)
	{
	case SpatialConstants::ENTITY_ACL_COMPONENT_ID:
	case SpatialConstants::METADATA_COMPONENT_ID:
	case SpatialConstants::POSITION_COMPONENT_ID:
	case SpatialConstants::PERSISTENCE_COMPONENT_ID:
	case SpatialConstants::SPAWN_DATA_COMPONENT_ID:
	case SpatialConstants::PLAYER_SPAWNER_COMPONENT_ID:
	case SpatialConstants::SINGLETON_COMPONENT_ID:
	case SpatialConstants::UNREAL_METADATA_COMPONENT_ID:
	case SpatialConstants::INTEREST_COMPONENT_ID:
	case SpatialConstants::NOT_STREAMED_COMPONENT_ID:
	case SpatialConstants::GSM_SHUTDOWN_COMPONENT_ID:
	case SpatialConstants::HEARTBEAT_COMPONENT_ID:
	case SpatialConstants::NETMULTICAST_RPCS_COMPONENT_ID_LEGACY:
	case SpatialConstants::RPCS_ON_ENTITY_CREATION_ID:
	case SpatialConstants::DEBUG_METRICS_COMPONENT_ID:
	case SpatialConstants::ALWAYS_RELEVANT_COMPONENT_ID:
	case SpatialConstants::CLIENT_RPC_ENDPOINT_COMPONENT_ID_LEGACY:
	case SpatialConstants::SERVER_RPC_ENDPOINT_COMPONENT_ID_LEGACY:
	case SpatialConstants::AUTHORITY_INTENT_COMPONENT_ID:
	case SpatialConstants::CLIENT_ENDPOINT_COMPONENT_ID:
	case SpatialConstants::SERVER_ENDPOINT_COMPONENT_ID:
	case SpatialConstants::MULTICAST_RPCS_COMPONENT_ID:
	case SpatialConstants::SPATIAL_DEBUGGING_COMPONENT_ID:
		// Ignore static spatial components as they are managed by the SpatialStaticComponentView.
		return;
	case SpatialConstants::SINGLETON_MANAGER_COMPONENT_ID:
		GlobalStateManager->ApplySingletonManagerData(Op.data);
		GlobalStateManager->LinkAllExistingSingletonActors();
		return;
	case SpatialConstants::DEPLOYMENT_MAP_COMPONENT_ID:
		GlobalStateManager->ApplyDeploymentMapData(Op.data);
		return;
	case SpatialConstants::STARTUP_ACTOR_MANAGER_COMPONENT_ID:
		GlobalStateManager->ApplyStartupActorManagerData(Op.data);
		return;
	case SpatialConstants::TOMBSTONE_COMPONENT_ID:
		RemoveActor(Op.entity_id);
		return;
	case SpatialConstants::DORMANT_COMPONENT_ID:
		if (USpatialActorChannel* Channel = NetDriver->GetActorChannelByEntityId(Op.entity_id))
		{
			NetDriver->AddPendingDormantChannel(Channel);
		}
		else
		{
			// This would normally get registered through the channel cleanup, but we don't have one for this entity
			NetDriver->RegisterDormantEntityId(Op.entity_id);
		}
		return;
	case SpatialConstants::VIRTUAL_WORKER_TRANSLATION_COMPONENT_ID:
		if (NetDriver->VirtualWorkerTranslator.IsValid())
		{
			Schema_Object* ComponentObject = Schema_GetComponentDataFields(Op.data.schema_type);
			NetDriver->VirtualWorkerTranslator->ApplyVirtualWorkerManagerData(ComponentObject);
		}
		return;
	}

	if (Op.data.component_id < SpatialConstants::MAX_RESERVED_SPATIAL_SYSTEM_COMPONENT_ID)
	{
		return;
	}

	if (ClassInfoManager->IsSublevelComponent(Op.data.component_id))
	{
		return;
	}

	if (bInCriticalSection)
	{
		PendingAddComponents.Emplace(Op.entity_id, Op.data.component_id, MakeUnique<DynamicComponent>(Op.data));
	}
	else
	{
		HandleIndividualAddComponent(Op);
	}
}

void USpatialReceiver::OnRemoveEntity(const Worker_RemoveEntityOp& Op)
{
	SCOPE_CYCLE_COUNTER(STAT_ReceiverRemoveEntity);
	if (IsEntityWaitingForAsyncLoad(Op.entity_id))
	{
		// Pretend we never saw this entity.
		EntitiesWaitingForAsyncLoad.Remove(Op.entity_id);
		return;
	}

	RemoveActor(Op.entity_id);
	OnEntityRemovedDelegate.Broadcast(Op.entity_id);
	if (GetDefault<USpatialGDKSettings>()->bUseRPCRingBuffers && RPCService != nullptr)
	{
		RPCService->OnRemoveEntity(Op.entity_id);
	}
}

void USpatialReceiver::OnRemoveComponent(const Worker_RemoveComponentOp& Op)
{
	// We are queuing here because if an Actor is removed from your view, remove component ops will be
	// generated and sent first, and then the RemoveEntityOp will be sent. In this case, we only want
	// to delete the Actor and not delete the subobjects that the RemoveComponent relate to.
	// So we queue RemoveComponentOps then process the RemoveEntityOps normally, and then apply the
	// RemoveComponentOps in ProcessRemoveComponent. Any RemoveComponentOps that relate to delete entities
	// will be dropped in ProcessRemoveComponent.
	QueuedRemoveComponentOps.Add(Op);
}

void USpatialReceiver::FlushRemoveComponentOps()
{
	for (const auto& Op : QueuedRemoveComponentOps)
	{
		ProcessRemoveComponent(Op);
	}

	QueuedRemoveComponentOps.Empty();
}

void USpatialReceiver::RemoveComponentOpsForEntity(Worker_EntityId EntityId)
{
	for (auto& RemoveComponentOp : QueuedRemoveComponentOps)
	{
		if (RemoveComponentOp.entity_id == EntityId)
		{
			// Zero component op to prevent array resize
			RemoveComponentOp = Worker_RemoveComponentOp{};
		}
	}
}

USpatialActorChannel* USpatialReceiver::RecreateDormantSpatialChannel(AActor* Actor, Worker_EntityId EntityID)
{
	// Receive would normally create channel in ReceiveActor - this function is used to recreate the channel after waking up a dormant actor
	USpatialActorChannel* Channel = NetDriver->GetOrCreateSpatialActorChannel(Actor);
	check(!Channel->bCreatingNewEntity);
	check(Channel->GetEntityId() == EntityID);

	NetDriver->UnregisterDormantEntityId(EntityID);

	return Channel;
}

void USpatialReceiver::ProcessRemoveComponent(const Worker_RemoveComponentOp& Op)
{
	if (IsEntityWaitingForAsyncLoad(Op.entity_id))
	{
		QueueRemoveComponentOpForAsyncLoad(Op);
		return;
	}

	if (!StaticComponentView->HasComponent(Op.entity_id, Op.component_id))
	{
		return;
	}

	if (AActor* Actor = Cast<AActor>(PackageMap->GetObjectFromEntityId(Op.entity_id).Get()))
	{
		FUnrealObjectRef ObjectRef(Op.entity_id, Op.component_id);
		if (Op.component_id == SpatialConstants::DORMANT_COMPONENT_ID)
		{
			RecreateDormantSpatialChannel(Actor, Op.entity_id);
		}
		else if (UObject* Object = PackageMap->GetObjectFromUnrealObjectRef(ObjectRef).Get())
		{
			if (USpatialActorChannel* Channel = NetDriver->GetActorChannelByEntityId(Op.entity_id))
			{
				Channel->OnSubobjectDeleted(ObjectRef, Object);

				Actor->OnSubobjectDestroyFromReplication(Object);

				Object->PreDestroyFromReplication();
				Object->MarkPendingKill();

				PackageMap->RemoveSubobject(FUnrealObjectRef(Op.entity_id, Op.component_id));
			}
		}
	}

	StaticComponentView->OnRemoveComponent(Op);
}

void USpatialReceiver::UpdateShadowData(Worker_EntityId EntityId)
{
	USpatialActorChannel* ActorChannel = NetDriver->GetActorChannelByEntityId(EntityId);
	ActorChannel->UpdateShadowData();
}

void USpatialReceiver::OnAuthorityChange(const Worker_AuthorityChangeOp& Op)
{
	SCOPE_CYCLE_COUNTER(STAT_ReceiverAuthChange);
	if (IsEntityWaitingForAsyncLoad(Op.entity_id))
	{
		QueueAuthorityOpForAsyncLoad(Op);
		return;
	}

	if (bInCriticalSection)
	{
		PendingAuthorityChanges.Add(Op);
		return;
	}

	HandleActorAuthority(Op);
}

void USpatialReceiver::HandlePlayerLifecycleAuthority(const Worker_AuthorityChangeOp& Op, APlayerController* PlayerController)
{
	UE_LOG(LogSpatialReceiver, Verbose, TEXT("HandlePlayerLifecycleAuthority for PlayerController %d."), *AActor::GetDebugName(PlayerController));

	// Server initializes heartbeat logic based on its authority over the position component,
	// client does the same for heartbeat component
	if ((NetDriver->IsServer() && Op.component_id == SpatialConstants::POSITION_COMPONENT_ID) ||
		(!NetDriver->IsServer() && Op.component_id == SpatialConstants::HEARTBEAT_COMPONENT_ID))
	{
		if (Op.authority == WORKER_AUTHORITY_AUTHORITATIVE)
		{
			if (USpatialNetConnection* Connection = Cast<USpatialNetConnection>(PlayerController->GetNetConnection()))
			{
				if (NetDriver->IsServer())
				{
					AuthorityPlayerControllerConnectionMap.Add(Op.entity_id, Connection);
				}
				Connection->InitHeartbeat(TimerManager, Op.entity_id);
			}
		}
		else if (Op.authority == WORKER_AUTHORITY_NOT_AUTHORITATIVE)
		{
			if (NetDriver->IsServer())
			{
				AuthorityPlayerControllerConnectionMap.Remove(Op.entity_id);
			}
			if (USpatialNetConnection* Connection = Cast<USpatialNetConnection>(PlayerController->GetNetConnection()))
			{
				Connection->DisableHeartbeat();
			}
		}
	}
}

void USpatialReceiver::HandleActorAuthority(const Worker_AuthorityChangeOp& Op)
{
	StaticComponentView->OnAuthorityChange(Op);

	if (GlobalStateManager->HandlesComponent(Op.component_id))
	{
		GlobalStateManager->AuthorityChanged(Op);
		return;
	}

	if (NetDriver->VirtualWorkerTranslator != nullptr
		&& Op.component_id == SpatialConstants::VIRTUAL_WORKER_TRANSLATION_COMPONENT_ID
		&& Op.authority == WORKER_AUTHORITY_AUTHORITATIVE)
	{
		NetDriver->InitializeVirtualWorkerTranslationManager();
		NetDriver->VirtualWorkerTranslationManager->AuthorityChanged(Op);
	}

	if (LoadBalanceEnforcer != nullptr)
	{
		LoadBalanceEnforcer->AuthorityChanged(Op);
	}

	if (NetDriver->SpatialDebugger != nullptr)
	{
		NetDriver->SpatialDebugger->ActorAuthorityChanged(Op);
	}

	AActor* Actor = Cast<AActor>(NetDriver->PackageMap->GetObjectFromEntityId(Op.entity_id));
	if (Actor == nullptr)
	{
		return;
	}

	if (Op.component_id == SpatialConstants::CLIENT_RPC_ENDPOINT_COMPONENT_ID_LEGACY
		&& Op.authority == WORKER_AUTHORITY_AUTHORITATIVE)
	{
		check(!NetDriver->IsServer());
		if (RPCsOnEntityCreation* QueuedRPCs = StaticComponentView->GetComponentData<RPCsOnEntityCreation>(Op.entity_id))
		{
			if (QueuedRPCs->HasRPCPayloadData())
			{
				ProcessQueuedActorRPCsOnEntityCreation(Actor, *QueuedRPCs);
			}

			Sender->SendRequestToClearRPCsOnEntityCreation(Op.entity_id);
		}
	}

	if (APlayerController* PlayerController = Cast<APlayerController>(Actor))
	{
		HandlePlayerLifecycleAuthority(Op, PlayerController);
	}

	if (NetDriver->IsServer())
	{
		// TODO UNR-955 - Remove this once batch reservation of EntityIds are in.
		if (Op.authority == WORKER_AUTHORITY_AUTHORITATIVE)
		{
			Sender->ProcessUpdatesQueuedUntilAuthority(Op.entity_id);
		}

		// If we became authoritative over the position component. set our role to be ROLE_Authority
		// and set our RemoteRole to be ROLE_AutonomousProxy if the actor has an owning connection.
		// Note: Pawn, PlayerController, and PlayerState for player-owned characters can arrive in
		// any order on non-authoritative servers, so it's possible that we don't yet know if a pawn
		// is player controlled when gaining authority over the pawn and need to wait for the player
		// state. Likewise, it's possible that the player state doesn't have a pointer to its pawn
		// yet, so we need to wait for the pawn to arrive.
		if (Op.component_id == SpatialConstants::POSITION_COMPONENT_ID)
		{
			if (Op.authority == WORKER_AUTHORITY_AUTHORITATIVE)
			{
				const bool bDormantActor = (Actor->NetDormancy >= DORM_DormantAll);

				if (IsValid(NetDriver->GetActorChannelByEntityId(Op.entity_id)) || bDormantActor)
				{
					Actor->Role = ROLE_Authority;
					Actor->RemoteRole = ROLE_SimulatedProxy;

					if (Actor->IsA<APlayerController>())
					{
						Actor->RemoteRole = ROLE_AutonomousProxy;
					}
					else if (APawn* Pawn = Cast<APawn>(Actor))
					{
						// The following check will return false on non-authoritative servers if the PlayerState hasn't been received yet.
						if (Pawn->IsPlayerControlled())
						{
							Pawn->RemoteRole = ROLE_AutonomousProxy;
						}
					}
					else if (const APlayerState* PlayerState = Cast<APlayerState>(Actor))
					{
						// The following check will return false on non-authoritative servers if the Pawn hasn't been received yet.
						if (APawn* PawnFromPlayerState = PlayerState->GetPawn())
						{
							check(PlayerState->bIsABot || PawnFromPlayerState->IsPlayerControlled());
							if (PawnFromPlayerState->IsPlayerControlled())
							{
								PawnFromPlayerState->RemoteRole = ROLE_AutonomousProxy;
							}
						}
					}

					if (!bDormantActor)
					{
						UpdateShadowData(Op.entity_id);
					}

					Actor->OnAuthorityGained();
				}
				else
				{
					UE_LOG(LogSpatialReceiver, Verbose, TEXT("Received authority over actor %s, with entity id %lld, which has no channel. This means it attempted to delete it earlier, when it had no authority. Retrying to delete now."), *Actor->GetName(), Op.entity_id);
					Sender->RetireEntity(Op.entity_id);
				}
			}
			else if (Op.authority == WORKER_AUTHORITY_AUTHORITY_LOSS_IMMINENT)
			{
				Actor->OnAuthorityLossImminent();
			}
			else if (Op.authority == WORKER_AUTHORITY_NOT_AUTHORITATIVE)
			{
				if (USpatialActorChannel* ActorChannel = NetDriver->GetActorChannelByEntityId(Op.entity_id))
				{
					ActorChannel->bCreatedEntity = false;
				}

				Actor->Role = ROLE_SimulatedProxy;
				Actor->RemoteRole = ROLE_Authority;

				Actor->OnAuthorityLost();
			}
		}

		// Subobject Delegation
		TPair<Worker_EntityId_Key, Worker_ComponentId> EntityComponentPair = MakeTuple(static_cast<Worker_EntityId_Key>(Op.entity_id), Op.component_id);
		if (TSharedRef<FPendingSubobjectAttachment>* PendingSubobjectAttachmentPtr = PendingEntitySubobjectDelegations.Find(EntityComponentPair))
		{
			FPendingSubobjectAttachment& PendingSubobjectAttachment = PendingSubobjectAttachmentPtr->Get();

			PendingSubobjectAttachment.PendingAuthorityDelegations.Remove(Op.component_id);

			if (PendingSubobjectAttachment.PendingAuthorityDelegations.Num() == 0)
			{
				if (UObject* Object = PendingSubobjectAttachment.Subobject.Get())
				{
					Sender->SendAddComponent(PendingSubobjectAttachment.Channel, Object, *PendingSubobjectAttachment.Info);
				}
			}

			PendingEntitySubobjectDelegations.Remove(EntityComponentPair);
		}
	}
	else
	{
		if (Op.component_id == SpatialConstants::GetClientAuthorityComponent(GetDefault<USpatialGDKSettings>()->bUseRPCRingBuffers))
		{
			if (USpatialActorChannel* ActorChannel = NetDriver->GetActorChannelByEntityId(Op.entity_id))
			{
				// Soft handover isn't supported currently.
				if (Op.authority != WORKER_AUTHORITY_AUTHORITY_LOSS_IMMINENT)
				{
					ActorChannel->ClientProcessOwnershipChange(Op.authority == WORKER_AUTHORITY_AUTHORITATIVE);
				}
			}

			// If we are a Pawn or PlayerController, our local role should be ROLE_AutonomousProxy. Otherwise ROLE_SimulatedProxy
			if (Actor->IsA<APawn>() || Actor->IsA<APlayerController>())
			{
				Actor->Role = (Op.authority == WORKER_AUTHORITY_AUTHORITATIVE) ? ROLE_AutonomousProxy : ROLE_SimulatedProxy;
			}
		}
	}

	if (Op.component_id == SpatialConstants::CLIENT_ENDPOINT_COMPONENT_ID ||
		Op.component_id == SpatialConstants::SERVER_ENDPOINT_COMPONENT_ID ||
		Op.component_id == SpatialConstants::MULTICAST_RPCS_COMPONENT_ID)
	{
		if (GetDefault<USpatialGDKSettings>()->bUseRPCRingBuffers && RPCService != nullptr)
		{
			if (Op.authority == WORKER_AUTHORITY_AUTHORITATIVE)
			{
				RPCService->OnEndpointAuthorityGained(Op.entity_id, Op.component_id);
				if (Op.component_id != SpatialConstants::MULTICAST_RPCS_COMPONENT_ID)
				{
					RPCService->ExtractRPCsForEntity(Op.entity_id, Op.component_id);
				}
			}
			else if (Op.authority == WORKER_AUTHORITY_NOT_AUTHORITATIVE)
			{
				RPCService->OnEndpointAuthorityLost(Op.entity_id, Op.component_id);
			}
		}
		else
		{
			UE_LOG(LogSpatialReceiver, Error, TEXT("USpatialReceiver::HandleActorAuthority: Gained authority over ring buffer endpoint but ring buffers not enabled! Entity: %lld, Component: %d"), Op.entity_id, Op.component_id);
		}
	}

	if (Op.authority == WORKER_AUTHORITY_AUTHORITATIVE)
	{
		if (Op.component_id == SpatialConstants::CLIENT_RPC_ENDPOINT_COMPONENT_ID_LEGACY)
		{
			Sender->SendClientEndpointReadyUpdate(Op.entity_id);
		}
		if (Op.component_id == SpatialConstants::SERVER_RPC_ENDPOINT_COMPONENT_ID_LEGACY)
		{
			Sender->SendServerEndpointReadyUpdate(Op.entity_id);
		}
	}
}

bool USpatialReceiver::IsReceivedEntityTornOff(Worker_EntityId EntityId)
{
	// Check the pending add components, to find the root component for the received entity.
	for (PendingAddComponentWrapper& PendingAddComponent : PendingAddComponents)
	{
		if (PendingAddComponent.EntityId != EntityId
			|| ClassInfoManager->GetCategoryByComponentId(PendingAddComponent.ComponentId) != SCHEMA_Data)
		{
			continue;
		}

		UClass* Class = ClassInfoManager->GetClassByComponentId(PendingAddComponent.ComponentId);
		if (!Class->IsChildOf<AActor>())
		{
			continue;
		}

		Worker_ComponentData* ComponentData = PendingAddComponent.Data->ComponentData;
		Schema_Object* ComponentObject = Schema_GetComponentDataFields(ComponentData->schema_type);
		return GetBoolFromSchema(ComponentObject, SpatialConstants::ACTOR_TEAROFF_ID);
	}

	return false;
}

void USpatialReceiver::ReceiveActor(Worker_EntityId EntityId)
{
	checkf(NetDriver, TEXT("We should have a NetDriver whilst processing ops."));
	checkf(NetDriver->GetWorld(), TEXT("We should have a World whilst processing ops."));

	SpawnData* SpawnDataComp = StaticComponentView->GetComponentData<SpawnData>(EntityId);
	UnrealMetadata* UnrealMetadataComp = StaticComponentView->GetComponentData<UnrealMetadata>(EntityId);

	if (UnrealMetadataComp == nullptr)
	{
		// Not an Unreal entity
		return;
	}

	const USpatialGDKSettings* SpatialGDKSettings = GetDefault<USpatialGDKSettings>();

	// Check if actor's class is loaded. If not, start async loading it and extract all data and
	// authority ops into a separate entry that will get processed once the loading is finished.
	const FString& ClassPath = UnrealMetadataComp->ClassPath;
	if (SpatialGDKSettings->bAsyncLoadNewClassesOnEntityCheckout && NeedToLoadClass(ClassPath))
	{
		StartAsyncLoadingClass(ClassPath, EntityId);
		return;
	}

	if (SpatialGDKSettings->bUseRPCRingBuffers && RPCService != nullptr)
	{
		RPCService->OnCheckoutEntity(EntityId);
	}

	if (AActor* EntityActor = Cast<AActor>(PackageMap->GetObjectFromEntityId(EntityId)))
	{
		UE_LOG(LogSpatialReceiver, Verbose, TEXT("%s: Entity for actor %s has been checked out on the worker which spawned it or is a singleton linked on this worker. "
			"Entity id: %lld"), *NetDriver->Connection->GetWorkerId(), *EntityActor->GetName(), EntityId);

		// Assume SimulatedProxy until we've been delegated Authority
		bool bAuthority = StaticComponentView->HasAuthority(EntityId, Position::ComponentId);
		EntityActor->Role = bAuthority ? ROLE_Authority : ROLE_SimulatedProxy;
		EntityActor->RemoteRole = bAuthority ? ROLE_SimulatedProxy : ROLE_Authority;
		if (bAuthority)
		{
			if (EntityActor->GetNetConnection() != nullptr || EntityActor->IsA<APawn>())
			{
				EntityActor->RemoteRole = ROLE_AutonomousProxy;
			}
		}

		// If we're a singleton, apply the data, regardless of authority - JIRA: 736
	}
	else
	{
		UE_LOG(LogSpatialReceiver, Verbose, TEXT("%s: Entity has been checked out on the worker which didn't spawn it. "
			"Entity id: %lld"), *NetDriver->Connection->GetWorkerId(), EntityId);

		UClass* Class = UnrealMetadataComp->GetNativeEntityClass();
		if (Class == nullptr)
		{
			UE_LOG(LogSpatialReceiver, Warning, TEXT("The received actor with entity id %lld couldn't be loaded. The actor (%s) will not be spawned."),
				EntityId, *UnrealMetadataComp->ClassPath);
			return;
		}

		// Make sure ClassInfo exists
		const FClassInfo& ActorClassInfo = ClassInfoManager->GetOrCreateClassInfoByClass(Class);

		// If the received actor is torn off, don't bother spawning it.
		// (This is only needed due to the delay between tearoff and deleting the entity. See https://improbableio.atlassian.net/browse/UNR-841)
		if (IsReceivedEntityTornOff(EntityId))
		{
			UE_LOG(LogSpatialReceiver, Verbose, TEXT("The received actor with entity id %lld was already torn off. The actor will not be spawned."), EntityId);
			return;
		}

		EntityActor = TryGetOrCreateActor(UnrealMetadataComp, SpawnDataComp);

		if (EntityActor == nullptr)
		{
			// This could be nullptr if:
			// a stably named actor could not be found
			// the Actor is a singleton that has arrived over the wire before it has been created on this worker
			// the class couldn't be loaded
			return;
		}

		// RemoveActor immediately if we've received the tombstone component.
		if (NetDriver->StaticComponentView->HasComponent(EntityId, SpatialConstants::TOMBSTONE_COMPONENT_ID))
		{
			UE_LOG(LogSpatialReceiver, Verbose, TEXT("The received actor with entity id %lld was tombstoned. The actor will not be spawned."), EntityId);
			// We must first Resolve the EntityId to the Actor in order for RemoveActor to succeed.
			PackageMap->ResolveEntityActor(EntityActor, EntityId);
			RemoveActor(EntityId);
			return;
		}

		UNetConnection* Connection = NetDriver->GetSpatialOSNetConnection();

		if (NetDriver->IsServer())
		{
			if (APlayerController* PlayerController = Cast<APlayerController>(EntityActor))
			{
				// If entity is a PlayerController, create channel on the PlayerController's connection.
				Connection = PlayerController->NetConnection;
			}
		}

		if (Connection == nullptr)
		{
			UE_LOG(LogSpatialReceiver, Error, TEXT("Unable to find SpatialOSNetConnection! Has this worker been disconnected from SpatialOS due to a timeout?"));
			return;
		}

		// Set up actor channel.
		USpatialActorChannel* Channel = Cast<USpatialActorChannel>(Connection->CreateChannelByName(NAME_Actor, NetDriver->IsServer() ? EChannelCreateFlags::OpenedLocally : EChannelCreateFlags::None));

		if (!Channel)
		{
			UE_LOG(LogSpatialReceiver, Warning, TEXT("Failed to create an actor channel when receiving entity %lld. The actor will not be spawned."), EntityId);
			EntityActor->Destroy(true);
			return;
		}

		if (!PackageMap->ResolveEntityActor(EntityActor, EntityId))
		{
			EntityActor->Destroy(true);
			Channel->Close(EChannelCloseReason::Destroyed);
			return;
		}

#if ENGINE_MINOR_VERSION <= 22
		Channel->SetChannelActor(EntityActor);
#else
		Channel->SetChannelActor(EntityActor, ESetChannelActorFlags::None);
#endif

		// Apply initial replicated properties.
		// This was moved to after FinishingSpawning because components existing only in blueprints aren't added until spawning is complete
		// Potentially we could split out the initial actor state and the initial component state
		for (PendingAddComponentWrapper& PendingAddComponent : PendingAddComponents)
		{
			if (ClassInfoManager->IsSublevelComponent(PendingAddComponent.ComponentId))
			{
				continue;
			}

			if (PendingAddComponent.EntityId == EntityId)
			{
				ApplyComponentDataOnActorCreation(EntityId, *PendingAddComponent.Data->ComponentData, *Channel, ActorClassInfo);
			}
		}

		if (!NetDriver->IsServer())
		{
			// Update interest on the entity's components after receiving initial component data (so Role and RemoteRole are properly set).
			// Don't send dynamic interest for this actor if it is otherwise handled by result types.
			if (!SpatialGDKSettings->bEnableClientResultTypes)
			{
				Sender->SendComponentInterestForActor(Channel, EntityId, Channel->IsAuthoritativeClient());
			}

			// This is a bit of a hack unfortunately, among the core classes only PlayerController implements this function and it requires
			// a player index. For now we don't support split screen, so the number is always 0.
			if (EntityActor->IsA(APlayerController::StaticClass()))
			{
				uint8 PlayerIndex = 0;
				// FInBunch takes size in bits not bytes
				FInBunch Bunch(NetDriver->ServerConnection, &PlayerIndex, sizeof(PlayerIndex) * 8);
				EntityActor->OnActorChannelOpen(Bunch, NetDriver->ServerConnection);
			}
			else
			{
				FInBunch Bunch(NetDriver->ServerConnection);
				EntityActor->OnActorChannelOpen(Bunch, NetDriver->ServerConnection);
			}

		}

		// Taken from PostNetInit
		if (NetDriver->GetWorld()->HasBegunPlay() && !EntityActor->HasActorBegunPlay())
		{
			// Whenever we receive an actor over the wire, the expectation is that it is not in an authoritative
			// state.  This is because it should already have had authoritative BeginPlay() called.  If we have
			// authority here, we are calling BeginPlay() with authority on this actor a 2nd time, which is always incorrect.
			check(!EntityActor->HasAuthority());
			EntityActor->DispatchBeginPlay();
		}

		if (EntityActor->GetClass()->HasAnySpatialClassFlags(SPATIALCLASS_Singleton))
		{
			GlobalStateManager->RegisterSingletonChannel(EntityActor, Channel);
		}

		EntityActor->UpdateOverlaps();

		if (StaticComponentView->HasComponent(EntityId, SpatialConstants::DORMANT_COMPONENT_ID))
		{
			NetDriver->AddPendingDormantChannel(Channel);
		}
	}
}

void USpatialReceiver::RemoveActor(Worker_EntityId EntityId)
{
	TWeakObjectPtr<UObject> WeakActor = PackageMap->GetObjectFromEntityId(EntityId);

	// Actor has been destroyed already. Clean up surrounding bookkeeping.
	if (!WeakActor.IsValid())
	{
		DestroyActor(nullptr, EntityId);
		return;
	}

	AActor* Actor = Cast<AActor>(WeakActor.Get());

	UE_LOG(LogSpatialReceiver, Verbose, TEXT("Worker %s Remove Actor: %s %lld"), *NetDriver->Connection->GetWorkerId(), Actor && !Actor->IsPendingKill() ? *Actor->GetName() : TEXT("nullptr"), EntityId);

	// Cleanup pending add components if any exist.
	if (USpatialActorChannel* ActorChannel = NetDriver->GetActorChannelByEntityId(EntityId))
	{
		// If we have any pending subobjects on the channel
		if (ActorChannel->PendingDynamicSubobjects.Num() > 0)
		{
			// Then iterate through all pending subobjects and remove entries relating to this entity.
			for (const auto& Pair : PendingDynamicSubobjectComponents)
			{
				if (Pair.Key.Key == EntityId)
				{
					PendingDynamicSubobjectComponents.Remove(Pair.Key);
				}
			}
		}
	}

	// Actor already deleted (this worker was most likely authoritative over it and deleted it earlier).
	if (Actor == nullptr || Actor->IsPendingKill())
	{
		if (USpatialActorChannel* ActorChannel = NetDriver->GetActorChannelByEntityId(EntityId))
		{
			UE_LOG(LogSpatialReceiver, Warning, TEXT("RemoveActor: actor for entity %lld was already deleted (likely on the authoritative worker) but still has an open actor channel."), EntityId);
			ActorChannel->ConditionalCleanUp(false, EChannelCloseReason::Destroyed);
		}
		return;
	}

	if (USpatialActorChannel* ActorChannel = NetDriver->GetActorChannelByEntityId(EntityId))
	{
		for (UObject* SubObject : ActorChannel->CreateSubObjects)
		{
			if (SubObject)
			{
				FUnrealObjectRef ObjectRef = FUnrealObjectRef::FromObjectPtr(SubObject, Cast<USpatialPackageMapClient>(PackageMap));
				// Unmap this object so we can remap it if it becomes relevant again in the future
				MoveMappedObjectToUnmapped(ObjectRef);
			}
		}

		FUnrealObjectRef ObjectRef = FUnrealObjectRef::FromObjectPtr(Actor, Cast<USpatialPackageMapClient>(PackageMap));
		// Unmap this object so we can remap it if it becomes relevant again in the future
		MoveMappedObjectToUnmapped(ObjectRef);

		for (auto& ChannelRefs : ActorChannel->ObjectReferenceMap)
		{
			CleanupRepStateMap(ChannelRefs.Value);
		}

		ActorChannel->ObjectReferenceMap.Empty();

		// If the entity is to be deleted after having been torn off, ignore the request (but clean up the channel if it has not been cleaned up already).
		if (Actor->GetTearOff())
		{
			ActorChannel->ConditionalCleanUp(false, EChannelCloseReason::TearOff);
			return;
		}
	}

	// Actor is a startup actor that is a part of the level.  If it's not Tombstone'd, then it
	// has just fallen out of our view and we should only remove the entity.
	if (Actor->IsFullNameStableForNetworking() &&
		StaticComponentView->HasComponent(EntityId, SpatialConstants::TOMBSTONE_COMPONENT_ID) == false)
	{
		// We can't call CleanupDeletedEntity here as we need the NetDriver to maintain the EntityId
		// to Actor Channel mapping for the DestroyActor to function correctly
		PackageMap->RemoveEntityActor(EntityId);
		return;
	}

	if (APlayerController* PC = Cast<APlayerController>(Actor))
	{
		// Force APlayerController::DestroyNetworkActorHandled to return false
		PC->Player = nullptr;
	}

	// Workaround for camera loss on handover: prevent UnPossess() (non-authoritative destruction of pawn, while being authoritative over the controller)
	// TODO: Check how AI controllers are affected by this (UNR-430)
	// TODO: This should be solved properly by working sets (UNR-411)
	if (APawn* Pawn = Cast<APawn>(Actor))
	{
		AController* Controller = Pawn->Controller;

		if (Controller && Controller->HasAuthority())
		{
			Pawn->Controller = nullptr;
		}
	}

	if (Actor->GetClass()->HasAnySpatialClassFlags(SPATIALCLASS_Singleton))
	{
		return;
	}

	DestroyActor(Actor, EntityId);
}

void USpatialReceiver::DestroyActor(AActor* Actor, Worker_EntityId EntityId)
{
	// Destruction of actors can cause the destruction of associated actors (eg. Character > Controller). Actor destroy
	// calls will eventually find their way into USpatialActorChannel::DeleteEntityIfAuthoritative() which checks if the entity
	// is currently owned by this worker before issuing an entity delete request. If the associated entity is still authoritative
	// on this server, we need to make sure this worker doesn't issue an entity delete request, as this entity is really
	// transitioning to the same server as the actor we're currently operating on, and is just a few frames behind.
	// We make the assumption that if we're destroying actors here (due to a remove entity op), then this is only due to two
	// situations;
	// 1. Actor's entity has been transitioned to another server
	// 2. The Actor was deleted on another server
	// In neither situation do we want to delete associated entities, so prevent them from being issued.
	// TODO: fix this with working sets (UNR-411)
	NetDriver->StartIgnoringAuthoritativeDestruction();

	// Clean up the actor channel. For clients, this will also call destroy on the actor.
	if (USpatialActorChannel* ActorChannel = NetDriver->GetActorChannelByEntityId(EntityId))
	{
		ActorChannel->ConditionalCleanUp(false, EChannelCloseReason::Destroyed);
	}
	else
	{
		if (NetDriver->IsDormantEntity(EntityId))
		{
			PackageMap->RemoveEntityActor(EntityId);
		}
		else
		{
			UE_LOG(LogSpatialReceiver, Verbose, TEXT("Removing actor as a result of a remove entity op, which has a missing actor channel. Actor: %s EntityId: %lld"), *GetNameSafe(Actor), EntityId);
		}
	}

	// It is safe to call AActor::Destroy even if the destruction has already started.
	if (Actor != nullptr && !Actor->Destroy(true))
	{
		UE_LOG(LogSpatialReceiver, Error, TEXT("Failed to destroy actor in RemoveActor %s %lld"), *Actor->GetName(), EntityId);
	}
	NetDriver->StopIgnoringAuthoritativeDestruction();

	check(PackageMap->GetObjectFromEntityId(EntityId) == nullptr);
}

AActor* USpatialReceiver::TryGetOrCreateActor(UnrealMetadata* UnrealMetadataComp, SpawnData* SpawnDataComp)
{
	if (UnrealMetadataComp->StablyNamedRef.IsSet())
	{
		if (NetDriver->IsServer() || UnrealMetadataComp->bNetStartup.GetValue())
		{
			// This Actor already exists in the map, get it from the package map.
			const FUnrealObjectRef& StablyNamedRef = UnrealMetadataComp->StablyNamedRef.GetValue();
			AActor* StaticActor = Cast<AActor>(PackageMap->GetObjectFromUnrealObjectRef(StablyNamedRef));
			// An unintended side effect of GetObjectFromUnrealObjectRef is that this ref
			// will be registered with this Actor. It can be the case that this Actor is not
			// stably named (due to bNetLoadOnClient = false) so we should let
			// SpatialPackageMapClient::ResolveEntityActor handle it properly.
			PackageMap->UnregisterActorObjectRefOnly(StablyNamedRef);

			return StaticActor;
		}
	}

	return CreateActor(UnrealMetadataComp, SpawnDataComp);
}

// This function is only called for client and server workers who did not spawn the Actor
AActor* USpatialReceiver::CreateActor(UnrealMetadata* UnrealMetadataComp, SpawnData* SpawnDataComp)
{
	UClass* ActorClass = UnrealMetadataComp->GetNativeEntityClass();

	if (ActorClass == nullptr)
	{
		UE_LOG(LogSpatialReceiver, Error, TEXT("Could not load class %s when spawning entity!"), *UnrealMetadataComp->ClassPath);
		return nullptr;
	}

	const bool bIsServer = NetDriver->IsServer();

	// Initial Singleton Actor replication is handled with GlobalStateManager::LinkExistingSingletonActors
	if (bIsServer && ActorClass->HasAnySpatialClassFlags(SPATIALCLASS_Singleton))
	{
		return FindSingletonActor(ActorClass);
	}

	UE_LOG(LogSpatialReceiver, Verbose, TEXT("Spawning a %s whilst checking out an entity."), *ActorClass->GetFullName());

	const bool bCreatingPlayerController = ActorClass->IsChildOf(APlayerController::StaticClass());

	FActorSpawnParameters SpawnInfo;
	SpawnInfo.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	SpawnInfo.bRemoteOwned = true;
	SpawnInfo.bNoFail = true;

	FVector SpawnLocation = FRepMovement::RebaseOntoLocalOrigin(SpawnDataComp->Location, NetDriver->GetWorld()->OriginLocation);

	AActor* NewActor = NetDriver->GetWorld()->SpawnActorAbsolute(ActorClass, FTransform(SpawnDataComp->Rotation, SpawnLocation), SpawnInfo);
	check(NewActor);

	if (bIsServer && bCreatingPlayerController)
	{
		NetDriver->PostSpawnPlayerController(Cast<APlayerController>(NewActor), UnrealMetadataComp->OwnerWorkerAttribute);
	}

	// Imitate the behavior in UPackageMapClient::SerializeNewActor.
	const float Epsilon = 0.001f;
	if (!SpawnDataComp->Velocity.Equals(FVector::ZeroVector, Epsilon))
	{
		NewActor->PostNetReceiveVelocity(SpawnDataComp->Velocity);
	}
	if (!SpawnDataComp->Scale.Equals(FVector::OneVector, Epsilon))
	{
		NewActor->SetActorScale3D(SpawnDataComp->Scale);
	}

	// Don't have authority over Actor until SpatialOS delegates authority
	NewActor->Role = ROLE_SimulatedProxy;
	NewActor->RemoteRole = ROLE_Authority;

	return NewActor;
}

FTransform USpatialReceiver::GetRelativeSpawnTransform(UClass* ActorClass, FTransform SpawnTransform)
{
	FTransform NewTransform = SpawnTransform;
	if (AActor* Template = ActorClass->GetDefaultObject<AActor>())
	{
		if (USceneComponent* TemplateRootComponent = Template->GetRootComponent())
		{
			TemplateRootComponent->UpdateComponentToWorld();
			NewTransform = TemplateRootComponent->GetComponentToWorld().Inverse() * NewTransform;
		}
	}

	return NewTransform;
}

void USpatialReceiver::ApplyComponentDataOnActorCreation(Worker_EntityId EntityId, const Worker_ComponentData& Data, USpatialActorChannel& Channel, const FClassInfo& ActorClassInfo)
{
	AActor* Actor = Channel.GetActor();

	uint32 Offset = 0;
	bool bFoundOffset = ClassInfoManager->GetOffsetByComponentId(Data.component_id, Offset);
	if (!bFoundOffset)
	{
		UE_LOG(LogSpatialReceiver, Warning, TEXT("EntityId %lld, ComponentId %d - Could not find offset for component id when applying component data to Actor %s!"), EntityId, Data.component_id, *Actor->GetPathName());
		return;
	}

	TWeakObjectPtr<UObject> TargetObject = PackageMap->GetObjectFromUnrealObjectRef(FUnrealObjectRef(EntityId, Offset));
	if (!TargetObject.IsValid())
	{
		bool bIsDynamicSubobject = !ActorClassInfo.SubobjectInfo.Contains(Offset);
		if (!bIsDynamicSubobject)
		{
			UE_LOG(LogSpatialReceiver, Verbose, TEXT("Tried to apply component data on actor creation for a static subobject that's been deleted, will skip. Entity: %lld, Component: %d, Actor: %s"), EntityId, Data.component_id, *Actor->GetPathName());
			return;
		}

		// If we can't find this subobject, it's a dynamically attached object. Create it now.
		TargetObject = NewObject<UObject>(Actor, ClassInfoManager->GetClassByComponentId(Data.component_id));

		Actor->OnSubobjectCreatedFromReplication(TargetObject.Get());

		PackageMap->ResolveSubobject(TargetObject.Get(), FUnrealObjectRef(EntityId, Offset));

		Channel.CreateSubObjects.Add(TargetObject.Get());
	}

	ApplyComponentData(Channel, *TargetObject, Data);
}

void USpatialReceiver::HandleIndividualAddComponent(const Worker_AddComponentOp& Op)
{
	uint32 Offset = 0;
	bool bFoundOffset = ClassInfoManager->GetOffsetByComponentId(Op.data.component_id, Offset);
	if (!bFoundOffset)
	{
		UE_LOG(LogSpatialReceiver, Warning, TEXT("EntityId %lld, ComponentId %d - Could not find offset for component id "
			"when receiving dynamic AddComponent."), Op.entity_id, Op.data.component_id);
		return;
	}

	// Object already exists, we can apply data directly.
	if (UObject* Object = PackageMap->GetObjectFromUnrealObjectRef(FUnrealObjectRef(Op.entity_id, Offset)).Get())
	{
		if (USpatialActorChannel* Channel = NetDriver->GetActorChannelByEntityId(Op.entity_id))
		{
			ApplyComponentData(*Channel, *Object, Op.data);
		}
		return;
	}

	const FClassInfo& Info = ClassInfoManager->GetClassInfoByComponentId(Op.data.component_id);
	AActor* Actor = Cast<AActor>(PackageMap->GetObjectFromEntityId(Op.entity_id).Get());
	if (Actor == nullptr)
	{
		UE_LOG(LogSpatialReceiver, Verbose, TEXT("Received an add component op for subobject of type %s on entity %lld but couldn't find Actor!"), *Info.Class->GetName(), Op.entity_id);
		return;
	}

	// Check if this is a static subobject that's been destroyed by the receiver.
	const FClassInfo& ActorClassInfo = ClassInfoManager->GetOrCreateClassInfoByClass(Actor->GetClass());
	bool bIsDynamicSubobject = !ActorClassInfo.SubobjectInfo.Contains(Offset);
	if (!bIsDynamicSubobject)
	{
		UE_LOG(LogSpatialReceiver, Verbose, TEXT("Tried to apply component data on add component for a static subobject that's been deleted, will skip. Entity: %lld, Component: %d, Actor: %s"), Op.entity_id, Op.data.component_id, *Actor->GetPathName());
		return;
	}

	// Otherwise this is a dynamically attached component. We need to make sure we have all related components before creation.
	PendingDynamicSubobjectComponents.Add(MakeTuple(static_cast<Worker_EntityId_Key>(Op.entity_id), Op.data.component_id),
		PendingAddComponentWrapper(Op.entity_id, Op.data.component_id, MakeUnique<DynamicComponent>(Op.data)));

	bool bReadyToCreate = true;
	ForAllSchemaComponentTypes([&](ESchemaComponentType Type)
	{
		Worker_ComponentId ComponentId = Info.SchemaComponents[Type];

		if (ComponentId == SpatialConstants::INVALID_COMPONENT_ID)
		{
			return;
		}

		if (!PendingDynamicSubobjectComponents.Contains(MakeTuple(static_cast<Worker_EntityId_Key>(Op.entity_id), ComponentId)))
		{
			bReadyToCreate = false;
		}
	});

	if (bReadyToCreate)
	{
		AttachDynamicSubobject(Actor, Op.entity_id, Info);
	}
}

void USpatialReceiver::AttachDynamicSubobject(AActor* Actor, Worker_EntityId EntityId, const FClassInfo& Info)
{
	USpatialActorChannel* Channel = NetDriver->GetActorChannelByEntityId(EntityId);
	if (Channel == nullptr)
	{
		UE_LOG(LogSpatialReceiver, Verbose, TEXT("Tried to dynamically attach subobject of type %s to entity %lld but couldn't find Channel!"), *Info.Class->GetName(), EntityId);
		return;
	}

	UObject* Subobject = NewObject<UObject>(Actor, Info.Class.Get());

	Actor->OnSubobjectCreatedFromReplication(Subobject);

	PackageMap->ResolveSubobject(Subobject, FUnrealObjectRef(EntityId, Info.SchemaComponents[SCHEMA_Data]));

	Channel->CreateSubObjects.Add(Subobject);

	ForAllSchemaComponentTypes([&](ESchemaComponentType Type)
	{
		Worker_ComponentId ComponentId = Info.SchemaComponents[Type];

		if (ComponentId == SpatialConstants::INVALID_COMPONENT_ID)
		{
			return;
		}

		TPair<Worker_EntityId_Key, Worker_ComponentId> EntityComponentPair = MakeTuple(static_cast<Worker_EntityId_Key>(EntityId), ComponentId);

		PendingAddComponentWrapper& AddComponent = PendingDynamicSubobjectComponents[EntityComponentPair];
		ApplyComponentData(*Channel, *Subobject, *AddComponent.Data->ComponentData);
		PendingDynamicSubobjectComponents.Remove(EntityComponentPair);
	});

	// Don't send dynamic interest for this subobject if it is otherwise handled by result types.
	if (GetDefault<USpatialGDKSettings>()->bEnableClientResultTypes)
	{
		return;
	}

	// If on a client, we need to set up the proper component interest for the new subobject.
	if (!NetDriver->IsServer())
	{
		Sender->SendComponentInterestForSubobject(Info, EntityId, Channel->IsAuthoritativeClient());
	}
}

struct USpatialReceiver::RepStateUpdateHelper
{
	RepStateUpdateHelper(USpatialActorChannel& Channel, UObject& TargetObject)
		: ObjectPtr(MakeWeakObjectPtr(&TargetObject))
		, ObjectRepState(Channel.ObjectReferenceMap.Find(ObjectPtr))
	{
	}

	~RepStateUpdateHelper()
	{
		check(bUpdatePerfomed);
	}

	FObjectReferencesMap& GetRefMap()
	{
		if (ObjectRepState)
		{
			return ObjectRepState->ReferenceMap;
		}
		return TempRefMap;
	}

	void Update(USpatialReceiver& Receiver, USpatialActorChannel& Channel, UObject& TargetObject, bool bReferencesChanged)
	{
		check(!bUpdatePerfomed);

		if (bReferencesChanged)
		{
			if (ObjectRepState == nullptr && TempRefMap.Num() > 0)
			{
				ObjectRepState = &Channel.ObjectReferenceMap.Add(ObjectPtr, FSpatialObjectRepState(FChannelObjectPair(&Channel, ObjectPtr)));
				ObjectRepState->ReferenceMap = MoveTemp(TempRefMap);
			}

			if (ObjectRepState)
			{
				ObjectRepState->UpdateRefToRepStateMap(Receiver.ObjectRefToRepStateMap);

				if (ObjectRepState->ReferencedObj.Num() == 0)
				{
					Channel.ObjectReferenceMap.Remove(ObjectPtr);
				}
			}
		}
#if DO_CHECK
		bUpdatePerfomed = true;
#endif
	}

	//TSet<FUnrealObjectRef> UnresolvedRefs;
private:
	FObjectReferencesMap TempRefMap;
	TWeakObjectPtr<UObject> ObjectPtr;
	FSpatialObjectRepState* ObjectRepState;
#if DO_CHECK
	bool bUpdatePerfomed = false;
#endif
};

void USpatialReceiver::ApplyComponentData(USpatialActorChannel& Channel, UObject& TargetObject, const Worker_ComponentData& Data)
{
	UClass* Class = ClassInfoManager->GetClassByComponentId(Data.component_id);
	checkf(Class, TEXT("Component %d isn't hand-written and not present in ComponentToClassMap."), Data.component_id);

	ESchemaComponentType ComponentType = ClassInfoManager->GetCategoryByComponentId(Data.component_id);

	if (ComponentType == SCHEMA_Data || ComponentType == SCHEMA_OwnerOnly)
	{
		if (ComponentType == SCHEMA_Data && TargetObject.IsA<UActorComponent>())
		{
			Schema_Object* ComponentObject = Schema_GetComponentDataFields(Data.schema_type);
			bool bReplicates = !!Schema_IndexBool(ComponentObject, SpatialConstants::ACTOR_COMPONENT_REPLICATES_ID, 0);
			if (!bReplicates)
			{
				return;
			}
		}
		RepStateUpdateHelper RepStateHelper(Channel, TargetObject);

		ComponentReader Reader(NetDriver, RepStateHelper.GetRefMap());
		bool bOutReferencesChanged = false;
		Reader.ApplyComponentData(Data, TargetObject, Channel, /* bIsHandover */ false, bOutReferencesChanged);

		RepStateHelper.Update(*this, Channel, TargetObject, bOutReferencesChanged);
	}
	else if (ComponentType == SCHEMA_Handover)
	{
		RepStateUpdateHelper RepStateHelper(Channel, TargetObject);

		ComponentReader Reader(NetDriver, RepStateHelper.GetRefMap());
		bool bOutReferencesChanged = false;
		Reader.ApplyComponentData(Data, TargetObject, Channel, /* bIsHandover */ true, bOutReferencesChanged);

		RepStateHelper.Update(*this, Channel, TargetObject, bOutReferencesChanged);
	}
	else
	{
		UE_LOG(LogSpatialReceiver, Verbose, TEXT("Entity: %d Component: %d - Skipping because RPC components don't have actual data."), Channel.GetEntityId(), Data.component_id);
	}
}

void USpatialReceiver::OnComponentUpdate(const Worker_ComponentUpdateOp& Op)
{
	SCOPE_CYCLE_COUNTER(STAT_ReceiverComponentUpdate);
	if (IsEntityWaitingForAsyncLoad(Op.entity_id))
	{
		QueueComponentUpdateOpForAsyncLoad(Op);
		return;
	}

	switch (Op.update.component_id)
	{
	case SpatialConstants::ENTITY_ACL_COMPONENT_ID:
	case SpatialConstants::METADATA_COMPONENT_ID:
	case SpatialConstants::POSITION_COMPONENT_ID:
	case SpatialConstants::PERSISTENCE_COMPONENT_ID:
	case SpatialConstants::INTEREST_COMPONENT_ID:
	case SpatialConstants::SPAWN_DATA_COMPONENT_ID:
	case SpatialConstants::PLAYER_SPAWNER_COMPONENT_ID:
	case SpatialConstants::SINGLETON_COMPONENT_ID:
	case SpatialConstants::UNREAL_METADATA_COMPONENT_ID:
	case SpatialConstants::NOT_STREAMED_COMPONENT_ID:
	case SpatialConstants::RPCS_ON_ENTITY_CREATION_ID:
	case SpatialConstants::DEBUG_METRICS_COMPONENT_ID:
	case SpatialConstants::ALWAYS_RELEVANT_COMPONENT_ID:
	case SpatialConstants::SPATIAL_DEBUGGING_COMPONENT_ID:
		UE_LOG(LogSpatialReceiver, Verbose, TEXT("Entity: %d Component: %d - Skipping because this is hand-written Spatial component"), Op.entity_id, Op.update.component_id);
		return;
	case SpatialConstants::GSM_SHUTDOWN_COMPONENT_ID:
#if WITH_EDITOR
		GlobalStateManager->OnShutdownComponentUpdate(Op.update);
#endif // WITH_EDITOR
		return;
	case SpatialConstants::HEARTBEAT_COMPONENT_ID:
		OnHeartbeatComponentUpdate(Op);
		return;
	case SpatialConstants::SINGLETON_MANAGER_COMPONENT_ID:
		GlobalStateManager->ApplySingletonManagerUpdate(Op.update);
		GlobalStateManager->LinkAllExistingSingletonActors();
		return;
	case SpatialConstants::DEPLOYMENT_MAP_COMPONENT_ID:
		NetDriver->GlobalStateManager->ApplyDeploymentMapUpdate(Op.update);
		return;
	case SpatialConstants::STARTUP_ACTOR_MANAGER_COMPONENT_ID:
		NetDriver->GlobalStateManager->ApplyStartupActorManagerUpdate(Op.update);
		return;
	case SpatialConstants::CLIENT_RPC_ENDPOINT_COMPONENT_ID_LEGACY:
	case SpatialConstants::SERVER_RPC_ENDPOINT_COMPONENT_ID_LEGACY:
	case SpatialConstants::NETMULTICAST_RPCS_COMPONENT_ID_LEGACY:
		HandleRPCLegacy(Op);
		return;
	case SpatialConstants::AUTHORITY_INTENT_COMPONENT_ID:
		if (NetDriver->IsServer() && (LoadBalanceEnforcer != nullptr))
		{
			LoadBalanceEnforcer->OnAuthorityIntentComponentUpdated(Op);
		}
		return;
	case SpatialConstants::VIRTUAL_WORKER_TRANSLATION_COMPONENT_ID:
		if (NetDriver->VirtualWorkerTranslator.IsValid())
		{
			Schema_Object* ComponentObject = Schema_GetComponentUpdateFields(Op.update.schema_type);
			NetDriver->VirtualWorkerTranslator->ApplyVirtualWorkerManagerData(ComponentObject);
		}
		return;
	case SpatialConstants::CLIENT_ENDPOINT_COMPONENT_ID:
	case SpatialConstants::SERVER_ENDPOINT_COMPONENT_ID:
	case SpatialConstants::MULTICAST_RPCS_COMPONENT_ID:
		HandleRPC(Op);
		return;
	}

	if (Op.update.component_id < SpatialConstants::MAX_RESERVED_SPATIAL_SYSTEM_COMPONENT_ID)
	{
		UE_LOG(LogSpatialReceiver, Verbose, TEXT("Entity: %d Component: %d - Skipping because this is a reserved spatial system component"), Op.entity_id, Op.update.component_id);
		return;
	}

	// If this entity has a Tombstone component, abort all component processing
	if (const Tombstone* TombstoneComponent = StaticComponentView->GetComponentData<Tombstone>(Op.entity_id))
	{
		UE_LOG(LogSpatialReceiver, Warning, TEXT("Received component update for Entity: %lld Component: %d after tombstone marked dead.  Aborting update."), Op.entity_id, Op.update.component_id);
		return;
	}

	if (ClassInfoManager->IsSublevelComponent(Op.update.component_id))
	{
		return;
	}

	USpatialActorChannel* Channel = NetDriver->GetActorChannelByEntityId(Op.entity_id);
	if (Channel == nullptr)
	{
		// If there is no actor channel as a result of the actor being dormant, then assume the actor is about to become active.
		if (const Dormant* DormantComponent = StaticComponentView->GetComponentData<Dormant>(Op.entity_id))
		{
			if (AActor* Actor = Cast<AActor>(PackageMap->GetObjectFromEntityId(Op.entity_id)))
			{
				Channel = RecreateDormantSpatialChannel(Actor, Op.entity_id);
			}
			else
			{
				UE_LOG(LogSpatialReceiver, Warning, TEXT("Worker: %s Dormant actor (entity: %lld) has been deleted on this worker but we have received a component update (id: %d) from the server."), *NetDriver->Connection->GetWorkerId(), Op.entity_id, Op.update.component_id);
				return;
			}
		}
		else
		{
			UE_LOG(LogSpatialReceiver, Log, TEXT("Worker: %s Entity: %lld Component: %d - No actor channel for update. This most likely occured due to the component updates that are sent when authority is lost during entity deletion."), *NetDriver->Connection->GetWorkerId(), Op.entity_id, Op.update.component_id);
			return;
		}
	}

	const FClassInfo& Info = ClassInfoManager->GetClassInfoByComponentId(Op.update.component_id);

	uint32 Offset;
	bool bFoundOffset = ClassInfoManager->GetOffsetByComponentId(Op.update.component_id, Offset);
	if (!bFoundOffset)
	{
		UE_LOG(LogSpatialReceiver, Warning, TEXT("Entity: %d Component: %d - Couldn't find Offset for component id"), Op.entity_id, Op.update.component_id);
		return;
	}

	UObject* TargetObject = nullptr;

	if (Offset == 0)
	{
		TargetObject = Channel->GetActor();
	}
	else
	{
		TargetObject = PackageMap->GetObjectFromUnrealObjectRef(FUnrealObjectRef(Op.entity_id, Offset)).Get();
	}

	if (TargetObject == nullptr)
	{
		UE_LOG(LogSpatialReceiver, Warning, TEXT("Entity: %d Component: %d - Couldn't find target object for update"), Op.entity_id, Op.update.component_id);
		return;
	}

	ESchemaComponentType Category = ClassInfoManager->GetCategoryByComponentId(Op.update.component_id);

	if (Category == ESchemaComponentType::SCHEMA_Data || Category == ESchemaComponentType::SCHEMA_OwnerOnly)
	{
		SCOPE_CYCLE_COUNTER(STAT_ReceiverApplyData);
		ApplyComponentUpdate(Op.update, *TargetObject, *Channel, /* bIsHandover */ false);
	}
	else if (Category == ESchemaComponentType::SCHEMA_Handover)
	{
		SCOPE_CYCLE_COUNTER(STAT_ReceiverApplyHandover);
		if (!NetDriver->IsServer())
		{
			UE_LOG(LogSpatialReceiver, Verbose, TEXT("Entity: %d Component: %d - Skipping Handover component because we're a client."), Op.entity_id, Op.update.component_id);
			return;
		}

		ApplyComponentUpdate(Op.update, *TargetObject, *Channel, /* bIsHandover */ true);
	}
	else
	{
		UE_LOG(LogSpatialReceiver, Verbose, TEXT("Entity: %d Component: %d - Skipping because it's an empty component update from an RPC component. (most likely as a result of gaining authority)"), Op.entity_id, Op.update.component_id);
	}
}

void USpatialReceiver::HandleRPCLegacy(const Worker_ComponentUpdateOp& Op)
{
	SCOPE_CYCLE_COUNTER(STAT_ReceiverHandleRPCLegacy);
	Worker_EntityId EntityId = Op.entity_id;

	// If the update is to the client rpc endpoint, then the handler should have authority over the server rpc endpoint component and vice versa
	// Ideally these events are never delivered to workers which are not able to handle them with clever interest management
	const Worker_ComponentId RPCEndpointComponentId = Op.update.component_id == SpatialConstants::CLIENT_RPC_ENDPOINT_COMPONENT_ID_LEGACY
		? SpatialConstants::SERVER_RPC_ENDPOINT_COMPONENT_ID_LEGACY : SpatialConstants::CLIENT_RPC_ENDPOINT_COMPONENT_ID_LEGACY;

	// Multicast RPCs should be executed by whoever receives them.
	if (Op.update.component_id != SpatialConstants::NETMULTICAST_RPCS_COMPONENT_ID_LEGACY)
	{
		if (!StaticComponentView->HasAuthority(Op.entity_id, RPCEndpointComponentId))
		{
			return;
		}
	}

	// Always process unpacked RPCs since some cannot be packed.
	ProcessRPCEventField(EntityId, Op, RPCEndpointComponentId, /* bPacked */ false);

	if (GetDefault<USpatialGDKSettings>()->bPackRPCs)
	{
		// Only process packed RPCs if packing is enabled
		ProcessRPCEventField(EntityId, Op, RPCEndpointComponentId, /* bPacked */ true);
	}
}

void USpatialReceiver::ProcessRPCEventField(Worker_EntityId EntityId, const Worker_ComponentUpdateOp& Op, Worker_ComponentId RPCEndpointComponentId, bool bPacked)
{
	Schema_Object* EventsObject = Schema_GetComponentUpdateEvents(Op.update.schema_type);
	const Schema_FieldId EventId = bPacked ? SpatialConstants::UNREAL_RPC_ENDPOINT_PACKED_EVENT_ID : SpatialConstants::UNREAL_RPC_ENDPOINT_EVENT_ID;
	uint32 EventCount = Schema_GetObjectCount(EventsObject, EventId);

	for (uint32 i = 0; i < EventCount; i++)
	{
		Schema_Object* EventData = Schema_IndexObject(EventsObject, EventId, i);

		RPCPayload Payload(EventData);

		FUnrealObjectRef ObjectRef(EntityId, Payload.Offset);

		if (bPacked)
		{
			// When packing unreliable RPCs into one update, they also always go through the PlayerController.
			// This means we need to retrieve the actual target Entity ID from the payload.
			if (Op.update.component_id == SpatialConstants::CLIENT_RPC_ENDPOINT_COMPONENT_ID_LEGACY ||
				Op.update.component_id == SpatialConstants::SERVER_RPC_ENDPOINT_COMPONENT_ID_LEGACY)
			{
				ObjectRef.Entity = Schema_GetEntityId(EventData, SpatialConstants::UNREAL_PACKED_RPC_PAYLOAD_ENTITY_ID);


				// In a zoned multiworker scenario we might not have gained authority over the current entity in this bundle in time
				// before processing so don't ApplyRPCs to an entity that we don't have authority over.
				if (!StaticComponentView->HasAuthority(ObjectRef.Entity, RPCEndpointComponentId))
				{
					continue;
				}
			}
		}

		if (UObject* TargetObject = PackageMap->GetObjectFromUnrealObjectRef(ObjectRef).Get())
		{
			ProcessOrQueueIncomingRPC(ObjectRef, MoveTemp(Payload));
		}

	}
}

void USpatialReceiver::HandleRPC(const Worker_ComponentUpdateOp& Op)
{
	SCOPE_CYCLE_COUNTER(STAT_ReceiverHandleRPC);
	if (!GetDefault<USpatialGDKSettings>()->bUseRPCRingBuffers || RPCService == nullptr)
	{
		UE_LOG(LogSpatialReceiver, Error, TEXT("USpatialReceiver::HandleRPC: Received component update on ring buffer component but ring buffers not enabled! Entity: %lld, Component: %d"), Op.entity_id, Op.update.component_id);
		return;
	}

	RPCService->ExtractRPCsForEntity(Op.entity_id, Op.update.component_id);
}

void USpatialReceiver::OnCommandRequest(const Worker_CommandRequestOp& Op)
{
	SCOPE_CYCLE_COUNTER(STAT_ReceiverCommandRequest);
	Schema_FieldId CommandIndex = Op.request.command_index;

	if (IsEntityWaitingForAsyncLoad(Op.entity_id))
	{
		UE_LOG(LogSpatialReceiver, Warning, TEXT("USpatialReceiver::OnCommandRequest: Actor class async loading, cannot handle command. Entity %lld, Class %s"), Op.entity_id, *EntitiesWaitingForAsyncLoad[Op.entity_id].ClassPath);
		Sender->SendCommandFailure(Op.request_id, TEXT("Target actor async loading."));
		return;
	}

	if (Op.request.component_id == SpatialConstants::PLAYER_SPAWNER_COMPONENT_ID && CommandIndex == SpatialConstants::PLAYER_SPAWNER_SPAWN_PLAYER_COMMAND_ID)
	{
		Schema_Object* Payload = Schema_GetCommandRequestObject(Op.request.schema_type);

		// Op.caller_attribute_set has two attributes.
		// 1. The attribute of the worker type
		// 2. The attribute of the specific worker that sent the request
		// We want to give authority to the specific worker, so we grab the second element from the attribute set.
		NetDriver->PlayerSpawner->ReceivePlayerSpawnRequest(Payload, Op.caller_attribute_set.attributes[1], Op.request_id);
		return;
	}
	else if (Op.request.component_id == SpatialConstants::RPCS_ON_ENTITY_CREATION_ID && CommandIndex == SpatialConstants::CLEAR_RPCS_ON_ENTITY_CREATION)
	{
		Sender->ClearRPCsOnEntityCreation(Op.entity_id);
		Sender->SendEmptyCommandResponse(Op.request.component_id, CommandIndex, Op.request_id);
		return;
	}
#if WITH_EDITOR
	else if (Op.request.component_id == SpatialConstants::GSM_SHUTDOWN_COMPONENT_ID && CommandIndex == SpatialConstants::SHUTDOWN_MULTI_PROCESS_REQUEST_ID)
	{
		NetDriver->GlobalStateManager->ReceiveShutdownMultiProcessRequest();
		return;
	}
#endif // WITH_EDITOR
#if !UE_BUILD_SHIPPING
	else if (Op.request.component_id == SpatialConstants::DEBUG_METRICS_COMPONENT_ID)
	{
		switch (CommandIndex)
		{
		case SpatialConstants::DEBUG_METRICS_START_RPC_METRICS_ID:
			NetDriver->SpatialMetrics->OnStartRPCMetricsCommand();
			break;
		case SpatialConstants::DEBUG_METRICS_STOP_RPC_METRICS_ID:
			NetDriver->SpatialMetrics->OnStopRPCMetricsCommand();
			break;
		case SpatialConstants::DEBUG_METRICS_MODIFY_SETTINGS_ID:
		{
			Schema_Object* Payload = Schema_GetCommandRequestObject(Op.request.schema_type);
			NetDriver->SpatialMetrics->OnModifySettingCommand(Payload);
			break;
		}
		default:
			UE_LOG(LogSpatialReceiver, Error, TEXT("Unknown command index for DebugMetrics component: %d, entity: %lld"), CommandIndex, Op.entity_id);
			break;
		}

		Sender->SendEmptyCommandResponse(Op.request.component_id, CommandIndex, Op.request_id);
		return;
	}
#endif // !UE_BUILD_SHIPPING

	Schema_Object* RequestObject = Schema_GetCommandRequestObject(Op.request.schema_type);

	RPCPayload Payload(RequestObject);
	FUnrealObjectRef ObjectRef = FUnrealObjectRef(Op.entity_id, Payload.Offset);
	UObject* TargetObject = PackageMap->GetObjectFromUnrealObjectRef(ObjectRef).Get();
	if (TargetObject == nullptr)
	{
		UE_LOG(LogSpatialReceiver, Warning, TEXT("No target object found for EntityId %d"), Op.entity_id);
		Sender->SendEmptyCommandResponse(Op.request.component_id, CommandIndex, Op.request_id);
		return;
	}

	const FClassInfo& Info = ClassInfoManager->GetOrCreateClassInfoByObject(TargetObject);
	UFunction* Function = Info.RPCs[Payload.Index];
	const FRPCInfo& RPCInfo = ClassInfoManager->GetRPCInfo(TargetObject, Function);

	UE_LOG(LogSpatialReceiver, Verbose, TEXT("Received command request (entity: %lld, component: %d, function: %s)"),
		Op.entity_id, Op.request.component_id, *Function->GetName());

	ProcessOrQueueIncomingRPC(ObjectRef, MoveTemp(Payload));
	Sender->SendEmptyCommandResponse(Op.request.component_id, CommandIndex, Op.request_id);
}

void USpatialReceiver::OnCommandResponse(const Worker_CommandResponseOp& Op)
{
	SCOPE_CYCLE_COUNTER(STAT_ReceiverCommandResponse);
	if (Op.response.component_id == SpatialConstants::PLAYER_SPAWNER_COMPONENT_ID)
	{
		NetDriver->PlayerSpawner->ReceivePlayerSpawnResponse(Op);
		return;
	}

	ReceiveCommandResponse(Op);
}

void USpatialReceiver::FlushRetryRPCs()
{
	Sender->FlushRetryRPCs();
}

void USpatialReceiver::ReceiveCommandResponse(const Worker_CommandResponseOp& Op)
{
	TSharedRef<FReliableRPCForRetry>* ReliableRPCPtr = PendingReliableRPCs.Find(Op.request_id);
	if (ReliableRPCPtr == nullptr)
	{
		// We received a response for some other command, ignore.
		return;
	}

	TSharedRef<FReliableRPCForRetry> ReliableRPC = *ReliableRPCPtr;
	PendingReliableRPCs.Remove(Op.request_id);
	if (Op.status_code != WORKER_STATUS_CODE_SUCCESS)
	{
		bool bCanRetry = false;

		// Only attempt to retry if the error code indicates it makes sense too
		if ((Op.status_code == WORKER_STATUS_CODE_TIMEOUT || Op.status_code == WORKER_STATUS_CODE_NOT_FOUND)
			&& (ReliableRPC->Attempts < SpatialConstants::MAX_NUMBER_COMMAND_ATTEMPTS))
		{
			bCanRetry = true;
		}
		// Don't apply the retry limit on auth lost, as it should eventually succeed
		else if (Op.status_code == WORKER_STATUS_CODE_AUTHORITY_LOST)
		{
			bCanRetry = true;
		}

		if (bCanRetry)
		{
			float WaitTime = SpatialConstants::GetCommandRetryWaitTimeSeconds(ReliableRPC->Attempts);
			UE_LOG(LogSpatialReceiver, Log, TEXT("%s: retrying in %f seconds. Error code: %d Message: %s"),
				*ReliableRPC->Function->GetName(), WaitTime, (int)Op.status_code, UTF8_TO_TCHAR(Op.message));

			if (!ReliableRPC->TargetObject.IsValid())
			{
				UE_LOG(LogSpatialReceiver, Warning, TEXT("%s: target object was destroyed before we could deliver the RPC."),
					*ReliableRPC->Function->GetName());
				return;
			}

			// Queue retry
			FTimerHandle RetryTimer;
			TimerManager->SetTimer(RetryTimer, [WeakSender = TWeakObjectPtr<USpatialSender>(Sender), ReliableRPC]()
			{
				if (USpatialSender* SpatialSender = WeakSender.Get())
				{
					SpatialSender->EnqueueRetryRPC(ReliableRPC);
				}
			}, WaitTime, false);
		}
		else
		{
			UE_LOG(LogSpatialReceiver, Error, TEXT("%s: failed too many times, giving up (%u attempts). Error code: %d Message: %s"),
				*ReliableRPC->Function->GetName(), SpatialConstants::MAX_NUMBER_COMMAND_ATTEMPTS, (int)Op.status_code, UTF8_TO_TCHAR(Op.message));
		}
	}
}

void USpatialReceiver::ApplyComponentUpdate(const Worker_ComponentUpdate& ComponentUpdate, UObject& TargetObject, USpatialActorChannel& Channel, bool bIsHandover)
{
	RepStateUpdateHelper RepStateHelper(Channel, TargetObject);

	ComponentReader Reader(NetDriver, RepStateHelper.GetRefMap());
	bool bOutReferencesChanged = false;
	Reader.ApplyComponentUpdate(ComponentUpdate, TargetObject, Channel, bIsHandover, bOutReferencesChanged);

	// This is a temporary workaround, see UNR-841:
	// If the update includes tearoff, close the channel and clean up the entity.
	if (TargetObject.IsA<AActor>() && ClassInfoManager->GetCategoryByComponentId(ComponentUpdate.component_id) == SCHEMA_Data)
	{
		Schema_Object* ComponentObject = Schema_GetComponentUpdateFields(ComponentUpdate.schema_type);

		// Check if bTearOff has been set to true
		if (GetBoolFromSchema(ComponentObject, SpatialConstants::ACTOR_TEAROFF_ID))
		{
			Channel.ConditionalCleanUp(false, EChannelCloseReason::TearOff);
		}
	}

	RepStateHelper.Update(*this, Channel, TargetObject, bOutReferencesChanged);
}

ERPCResult USpatialReceiver::ApplyRPCInternal(UObject* TargetObject, UFunction* Function, const RPCPayload& Payload, const FString& SenderWorkerId, bool bApplyWithUnresolvedRefs /* = false */)
{
	ERPCResult Result = ERPCResult::Unknown;

	uint8* Parms = (uint8*)FMemory_Alloca(Function->ParmsSize);
	FMemory::Memzero(Parms, Function->ParmsSize);

	TSet<FUnrealObjectRef> UnresolvedRefs;
	TSet<FUnrealObjectRef> MappedRefs;
	RPCPayload PayloadCopy = Payload;
	FSpatialNetBitReader PayloadReader(PackageMap, PayloadCopy.PayloadData.GetData(), PayloadCopy.CountDataBits(), MappedRefs, UnresolvedRefs);

	TSharedPtr<FRepLayout> RepLayout = NetDriver->GetFunctionRepLayout(Function);
	RepLayout_ReceivePropertiesForRPC(*RepLayout, PayloadReader, Parms);

	if ((UnresolvedRefs.Num() == 0) || bApplyWithUnresolvedRefs)
	{
		TargetObject->ProcessEvent(Function, Parms);
		Result = ERPCResult::Success;
	}
	else
	{
		Result = ERPCResult::UnresolvedParameters;
	}

	// Destroy the parameters.
	// warning: highly dependent on UObject::ProcessEvent freeing of parms!
	for (TFieldIterator<UProperty> It(Function); It && It->HasAnyPropertyFlags(CPF_Parm); ++It)
	{
		It->DestroyValue_InContainer(Parms);
	}
	return Result;
}

FRPCErrorInfo USpatialReceiver::ApplyRPC(const FPendingRPCParams& Params)
{
	TWeakObjectPtr<UObject> TargetObjectWeakPtr = PackageMap->GetObjectFromUnrealObjectRef(Params.ObjectRef);
	if (!TargetObjectWeakPtr.IsValid())
	{
		return FRPCErrorInfo{ nullptr, nullptr, NetDriver->IsServer(), ERPCQueueType::Receive, ERPCResult::UnresolvedTargetObject };
	}

	UObject* TargetObject = TargetObjectWeakPtr.Get();
	const FClassInfo& ClassInfo = ClassInfoManager->GetOrCreateClassInfoByObject(TargetObjectWeakPtr.Get());
	UFunction* Function = ClassInfo.RPCs[Params.Payload.Index];
	if (Function == nullptr)
	{
		return FRPCErrorInfo{ TargetObject, nullptr, NetDriver->IsServer(), ERPCQueueType::Receive, ERPCResult::MissingFunctionInfo };
	}

	bool bApplyWithUnresolvedRefs = false;
	const float TimeDiff = (FDateTime::Now() - Params.Timestamp).GetTotalSeconds();
	if (GetDefault<USpatialGDKSettings>()->QueuedIncomingRPCWaitTime < TimeDiff)
	{
		if ((Function->SpatialFunctionFlags & SPATIALFUNC_AllowUnresolvedParameters) == 0)
		{
			UE_LOG(LogSpatialReceiver, Warning, TEXT("Executing RPC %s::%s with unresolved references after %f seconds of queueing"), *TargetObjectWeakPtr->GetName(), *Function->GetName(), TimeDiff);
		}
		bApplyWithUnresolvedRefs = true;
	}

#if TRACE_LIB_ACTIVE
	USpatialLatencyTracer* Tracer = USpatialLatencyTracer::GetTracer(TargetObject);
	Tracer->MarkActiveLatencyTrace(Params.Payload.Trace);
#endif

	ERPCResult Result = ApplyRPCInternal(TargetObject, Function, Params.Payload, FString{}, bApplyWithUnresolvedRefs);

#if TRACE_LIB_ACTIVE
	if (Result == ERPCResult::Success)
	{
		Tracer->EndLatencyTrace(Params.Payload.Trace, TEXT("Unhandled trace - automatically ended"));
	}
	Tracer->MarkActiveLatencyTrace(USpatialLatencyTracer::InvalidTraceKey);
#endif

	return FRPCErrorInfo{ TargetObject, Function, NetDriver->IsServer(), ERPCQueueType::Receive, Result };
}

void USpatialReceiver::OnReserveEntityIdsResponse(const Worker_ReserveEntityIdsResponseOp& Op)
{
	SCOPE_CYCLE_COUNTER(STAT_ReceiverReserveEntityIds);
	if (Op.status_code != WORKER_STATUS_CODE_SUCCESS)
	{
		UE_LOG(LogSpatialReceiver, Warning, TEXT("ReserveEntityIds request failed: request id: %d, message: %s"), Op.request_id, UTF8_TO_TCHAR(Op.message));
	}
	else
	{
		UE_LOG(LogSpatialReceiver, Verbose, TEXT("ReserveEntityIds request succeeded: request id: %d, message: %s"), Op.request_id, UTF8_TO_TCHAR(Op.message));
	}

	if (ReserveEntityIDsDelegate* RequestDelegate = ReserveEntityIDsDelegates.Find(Op.request_id))
	{
		UE_LOG(LogSpatialReceiver, Log, TEXT("Executing ReserveEntityIdsResponse with delegate, request id: %d, first entity id: %lld, message: %s"), Op.request_id, Op.first_entity_id, UTF8_TO_TCHAR(Op.message));
		RequestDelegate->ExecuteIfBound(Op);
		ReserveEntityIDsDelegates.Remove(Op.request_id);
	}
	else
	{
		UE_LOG(LogSpatialReceiver, Warning, TEXT("Received ReserveEntityIdsResponse but with no delegate set, request id: %d, first entity id: %lld, message: %s"), Op.request_id, Op.first_entity_id, UTF8_TO_TCHAR(Op.message));
	}
}

void USpatialReceiver::OnCreateEntityResponse(const Worker_CreateEntityResponseOp& Op)
{
	SCOPE_CYCLE_COUNTER(STAT_ReceiverCreateEntityResponse);
	switch (static_cast<Worker_StatusCode>(Op.status_code))
	{
	case WORKER_STATUS_CODE_SUCCESS:
		UE_LOG(LogSpatialReceiver, Verbose, TEXT("Create entity request succeeded. "
			"Request id: %d, entity id: %lld, message: %s"), Op.request_id, Op.entity_id, UTF8_TO_TCHAR(Op.message));
		break;
	case WORKER_STATUS_CODE_TIMEOUT:
		UE_LOG(LogSpatialReceiver, Verbose, TEXT("Create entity request timed out. "
			"Request id: %d, entity id: %lld, message: %s"), Op.request_id, Op.entity_id, UTF8_TO_TCHAR(Op.message));
		break;
	case WORKER_STATUS_CODE_APPLICATION_ERROR:
		UE_LOG(LogSpatialReceiver, Verbose, TEXT("Create entity request failed. "
			"Either the reservation expired, the entity already existed, or the entity was invalid. "
			"Request id: %d, entity id: %lld, message: %s"), Op.request_id, Op.entity_id, UTF8_TO_TCHAR(Op.message));
		break;
	default:
		UE_LOG(LogSpatialReceiver, Error, TEXT("Create entity request failed. This likely indicates a bug in the Unreal GDK and should be reported. "
			"Request id: %d, entity id: %lld, message: %s"), Op.request_id, Op.entity_id, UTF8_TO_TCHAR(Op.message));
		break;
	}

	if (CreateEntityDelegate* Delegate = CreateEntityDelegates.Find(Op.request_id))
	{
		Delegate->ExecuteIfBound(Op);
		CreateEntityDelegates.Remove(Op.request_id);
	}

	TWeakObjectPtr<USpatialActorChannel> Channel = PopPendingActorRequest(Op.request_id);

	// It's possible for the ActorChannel to have been closed by the time we receive a response. Actor validity is checked within the channel.
	if (Channel.IsValid())
	{
		Channel->OnCreateEntityResponse(Op);
	}
	else if (Channel.IsStale())
	{
		UE_LOG(LogSpatialReceiver, Verbose, TEXT("Received CreateEntityResponse for actor which no longer has an actor channel: "
			"request id: %d, entity id: %lld. This should only happen in the case where we attempt to delete the entity before we have authority. "
			"The entity will therefore be deleted once authority is gained."), Op.request_id, Op.entity_id);
	}
}

void USpatialReceiver::OnEntityQueryResponse(const Worker_EntityQueryResponseOp& Op)
{
	SCOPE_CYCLE_COUNTER(STAT_ReceiverEntityQueryResponse);
	if (Op.status_code != WORKER_STATUS_CODE_SUCCESS)
	{
		UE_LOG(LogSpatialReceiver, Error, TEXT("EntityQuery failed: request id: %d, message: %s"), Op.request_id, UTF8_TO_TCHAR(Op.message));
	}

	if (EntityQueryDelegate* RequestDelegate = EntityQueryDelegates.Find(Op.request_id))
	{
		UE_LOG(LogSpatialReceiver, Verbose, TEXT("Executing EntityQueryResponse with delegate, request id: %d, number of entities: %d, message: %s"), Op.request_id, Op.result_count, UTF8_TO_TCHAR(Op.message));
		RequestDelegate->ExecuteIfBound(Op);
	}
	else
	{
		UE_LOG(LogSpatialReceiver, Warning, TEXT("Received EntityQueryResponse but with no delegate set, request id: %d, number of entities: %d, message: %s"), Op.request_id, Op.result_count, UTF8_TO_TCHAR(Op.message));
	}
}

void USpatialReceiver::AddPendingActorRequest(Worker_RequestId RequestId, USpatialActorChannel* Channel)
{
	PendingActorRequests.Add(RequestId, Channel);
}

void USpatialReceiver::AddPendingReliableRPC(Worker_RequestId RequestId, TSharedRef<FReliableRPCForRetry> ReliableRPC)
{
	PendingReliableRPCs.Add(RequestId, ReliableRPC);
}

void USpatialReceiver::AddEntityQueryDelegate(Worker_RequestId RequestId, EntityQueryDelegate Delegate)
{
	EntityQueryDelegates.Add(RequestId, Delegate);
}

void USpatialReceiver::AddReserveEntityIdsDelegate(Worker_RequestId RequestId, ReserveEntityIDsDelegate Delegate)
{
	ReserveEntityIDsDelegates.Add(RequestId, Delegate);
}

void USpatialReceiver::AddCreateEntityDelegate(Worker_RequestId RequestId, const CreateEntityDelegate& Delegate)
{
	CreateEntityDelegates.Add(RequestId, Delegate);
}

TWeakObjectPtr<USpatialActorChannel> USpatialReceiver::PopPendingActorRequest(Worker_RequestId RequestId)
{
	TWeakObjectPtr<USpatialActorChannel>* ChannelPtr = PendingActorRequests.Find(RequestId);
	if (ChannelPtr == nullptr)
	{
		return nullptr;
	}
	TWeakObjectPtr<USpatialActorChannel> Channel = *ChannelPtr;
	PendingActorRequests.Remove(RequestId);
	return Channel;
}

AActor* USpatialReceiver::FindSingletonActor(UClass* SingletonClass)
{
	TArray<AActor*> FoundActors;
	UGameplayStatics::GetAllActorsOfClass(NetDriver->World, SingletonClass, FoundActors);

	// There should be only one singleton actor per class
	if (FoundActors.Num() == 1)
	{
		return FoundActors[0];
	}

	return nullptr;
}

void USpatialReceiver::ProcessQueuedResolvedObjects()
{
	for (TPair<UObject*, FUnrealObjectRef>& It : ResolvedObjectQueue)
	{
		ResolvePendingOperations_Internal(It.Key, It.Value);
	}
	ResolvedObjectQueue.Empty();
}

void USpatialReceiver::ProcessQueuedActorRPCsOnEntityCreation(AActor* Actor, RPCsOnEntityCreation& QueuedRPCs)
{
	const FClassInfo& Info = ClassInfoManager->GetOrCreateClassInfoByClass(Actor->GetClass());

	for (auto& RPC : QueuedRPCs.RPCs)
	{
		UFunction* Function = Info.RPCs[RPC.Index];
		const FRPCInfo& RPCInfo = ClassInfoManager->GetRPCInfo(Actor, Function);
		const FUnrealObjectRef ObjectRef = PackageMap->GetUnrealObjectRefFromObject(Actor);
		check(ObjectRef != FUnrealObjectRef::UNRESOLVED_OBJECT_REF);

		ProcessOrQueueIncomingRPC(ObjectRef, MoveTemp(RPC));
	}
}

void USpatialReceiver::ResolvePendingOperations(UObject* Object, const FUnrealObjectRef& ObjectRef)
{
	if (bInCriticalSection)
	{
		ResolvedObjectQueue.Add(TPair<UObject*, FUnrealObjectRef>{ Object, ObjectRef });
	}
	else
	{
		ResolvePendingOperations_Internal(Object, ObjectRef);
	}
}

void USpatialReceiver::OnDisconnect(Worker_DisconnectOp& Op)
{
	if (GEngine != nullptr)
	{
		GEngine->BroadcastNetworkFailure(NetDriver->GetWorld(), NetDriver, ENetworkFailure::FromDisconnectOpStatusCode(Op.connection_status_code), UTF8_TO_TCHAR(Op.reason));
	}
}

bool USpatialReceiver::IsPendingOpsOnChannel(USpatialActorChannel& Channel)
{
	SCOPE_CYCLE_COUNTER(STAT_SpatialPendingOpsOnChannel);

	// Don't allow Actors to go dormant if they have any pending operations waiting on their channel

	for (const auto& RefMap : Channel.ObjectReferenceMap)
	{
		if (RefMap.Value.HasUnresolved())
		{
			return true;
		}
	}

	for (const auto& ActorRequest : PendingActorRequests)
	{
		if (ActorRequest.Value == &Channel)
		{
			return true;
		}
	}

	return false;
}


void USpatialReceiver::ClearPendingRPCs(Worker_EntityId EntityId)
{
	IncomingRPCs.DropForEntity(EntityId);
}


void USpatialReceiver::ProcessOrQueueIncomingRPC(const FUnrealObjectRef& InTargetObjectRef, SpatialGDK::RPCPayload InPayload)
{
	TWeakObjectPtr<UObject> TargetObjectWeakPtr = PackageMap->GetObjectFromUnrealObjectRef(InTargetObjectRef);
	if (!TargetObjectWeakPtr.IsValid())
	{
		UE_LOG(LogSpatialReceiver, Verbose, TEXT("The object has been deleted, dropping the RPC"));
		return;
	}

	UObject* TargetObject = TargetObjectWeakPtr.Get();
	const FClassInfo& ClassInfo = ClassInfoManager->GetOrCreateClassInfoByObject(TargetObject);
	UFunction* Function = ClassInfo.RPCs[InPayload.Index];
	const FRPCInfo& RPCInfo = ClassInfoManager->GetRPCInfo(TargetObject, Function);
	ERPCType Type = RPCInfo.Type;

	IncomingRPCs.ProcessOrQueueRPC(InTargetObjectRef, Type, MoveTemp(InPayload));
}

bool USpatialReceiver::OnExtractIncomingRPC(Worker_EntityId EntityId, ERPCType RPCType, const SpatialGDK::RPCPayload& Payload)
{
	ProcessOrQueueIncomingRPC(FUnrealObjectRef(EntityId, Payload.Offset), Payload);

	return true;
}

void USpatialReceiver::ResolvePendingOperations_Internal(UObject* Object, const FUnrealObjectRef& ObjectRef)
{
	UE_LOG(LogSpatialReceiver, Verbose, TEXT("Resolving pending object refs and RPCs which depend on object: %s %s."), *Object->GetName(), *ObjectRef.ToString());

	ResolveIncomingOperations(Object, ObjectRef);
	if (Object->GetClass()->HasAnySpatialClassFlags(SPATIALCLASS_Singleton) && !Object->IsFullNameStableForNetworking())
	{
		// When resolving a singleton, also resolve using class path (in case any properties
		// were set from a server that hasn't resolved the singleton yet)
		FUnrealObjectRef ClassObjectRef = FUnrealObjectRef::GetSingletonClassRef(Object, PackageMap);
		if (ClassObjectRef.IsValid())
		{
			ResolveIncomingOperations(Object, ClassObjectRef);
		}
	}
	// TODO: UNR-1650 We're trying to resolve all queues, which introduces more overhead.
	IncomingRPCs.ProcessRPCs();
}

void USpatialReceiver::ResolveIncomingOperations(UObject* Object, const FUnrealObjectRef& ObjectRef)
{
	// TODO: queue up resolved objects since they were resolved during process ops
	// and then resolve all of them at the end of process ops - UNR:582

	TSet<FChannelObjectPair>* TargetObjectSet = ObjectRefToRepStateMap.Find(ObjectRef);
	if (!TargetObjectSet)
	{
		return;
	}

	UE_LOG(LogSpatialReceiver, Verbose, TEXT("Resolving incoming operations depending on object ref %s, resolved object: %s"), *ObjectRef.ToString(), *Object->GetName());

	for (auto ChannelObjectIter = TargetObjectSet->CreateIterator(); ChannelObjectIter; ++ChannelObjectIter)
	{
		USpatialActorChannel* DependentChannel = ChannelObjectIter->Key.Get();
		if (!DependentChannel)
		{
			ChannelObjectIter.RemoveCurrent();
			continue;
		}

		UObject* ReplicatingObject = ChannelObjectIter->Value.Get();

		if (!ReplicatingObject)
		{
			if (DependentChannel->ObjectReferenceMap.Find(ChannelObjectIter->Value))
			{
				DependentChannel->ObjectReferenceMap.Remove(ChannelObjectIter->Value);
				ChannelObjectIter.RemoveCurrent();
			}
			continue;
		}

		FSpatialObjectRepState* RepState = DependentChannel->ObjectReferenceMap.Find(ChannelObjectIter->Value);
		if (!RepState || !RepState->UnresolvedRefs.Contains(ObjectRef))
		{
			continue;
		}

		// Check whether the resolved object has been torn off, or is on an actor that has been torn off.
		if (AActor* AsActor = Cast<AActor>(ReplicatingObject))
		{
			if (AsActor->GetTearOff())
			{
				UE_LOG(LogSpatialActorChannel, Log, TEXT("Actor to be resolved was torn off, so ignoring incoming operations. Object ref: %s, resolved object: %s"), *ObjectRef.ToString(), *Object->GetName());
				DependentChannel->ObjectReferenceMap.Remove(ChannelObjectIter->Value);
				continue;
			}
		}
		else if (AActor* OuterActor = ReplicatingObject->GetTypedOuter<AActor>())
		{
			if (OuterActor->GetTearOff())
			{
				UE_LOG(LogSpatialActorChannel, Log, TEXT("Owning Actor of the object to be resolved was torn off, so ignoring incoming operations. Object ref: %s, resolved object: %s"), *ObjectRef.ToString(), *Object->GetName());
				DependentChannel->ObjectReferenceMap.Remove(ChannelObjectIter->Value);
				continue;
			}
		}

		bool bSomeObjectsWereMapped = false;
		TArray<UProperty*> RepNotifies;

		FRepLayout& RepLayout = DependentChannel->GetObjectRepLayout(ReplicatingObject);
		FRepStateStaticBuffer& ShadowData = DependentChannel->GetObjectStaticBuffer(ReplicatingObject);

		ResolveObjectReferences(RepLayout, ReplicatingObject, *RepState, RepState->ReferenceMap, ShadowData.GetData(), (uint8*)ReplicatingObject, ReplicatingObject->GetClass()->GetPropertiesSize(), RepNotifies, bSomeObjectsWereMapped);

		if (bSomeObjectsWereMapped)
		{
			DependentChannel->RemoveRepNotifiesWithUnresolvedObjs(RepNotifies, RepLayout, RepState->ReferenceMap, ReplicatingObject);

			UE_LOG(LogSpatialReceiver, Verbose, TEXT("Resolved for target object %s"), *ReplicatingObject->GetName());
			DependentChannel->PostReceiveSpatialUpdate(ReplicatingObject, RepNotifies);
		}

		RepState->UnresolvedRefs.Remove(ObjectRef);
	}
}

void USpatialReceiver::ResolveObjectReferences(FRepLayout& RepLayout, UObject* ReplicatedObject, FSpatialObjectRepState& RepState, FObjectReferencesMap& ObjectReferencesMap, uint8* RESTRICT StoredData, uint8* RESTRICT Data, int32 MaxAbsOffset, TArray<UProperty*>& RepNotifies, bool& bOutSomeObjectsWereMapped)
{
	for (auto It = ObjectReferencesMap.CreateIterator(); It; ++It)
	{
		int32 AbsOffset = It.Key();

		if (AbsOffset >= MaxAbsOffset)
		{
			UE_LOG(LogSpatialReceiver, Log, TEXT("ResolveObjectReferences: Removed unresolved reference: AbsOffset >= MaxAbsOffset: %d"), AbsOffset);
			It.RemoveCurrent();
			continue;
		}

		FObjectReferences& ObjectReferences = It.Value();

		UProperty* Property = ObjectReferences.Property;

		// ParentIndex is -1 for handover properties
		bool bIsHandover = ObjectReferences.ParentIndex == -1;
		FRepParentCmd* Parent = ObjectReferences.ParentIndex >= 0 ? &RepLayout.Parents[ObjectReferences.ParentIndex] : nullptr;

		int32 StoredDataOffset = ObjectReferences.ShadowOffset;

		if (ObjectReferences.Array)
		{
			check(Property->IsA<UArrayProperty>());

			if (!bIsHandover)
			{
				Property->CopySingleValue(StoredData + StoredDataOffset, Data + AbsOffset);
			}

			FScriptArray* StoredArray = bIsHandover ? nullptr : (FScriptArray*)(StoredData + StoredDataOffset);
			FScriptArray* Array = (FScriptArray*)(Data + AbsOffset);

			int32 NewMaxOffset = Array->Num() * Property->ElementSize;

			ResolveObjectReferences(RepLayout, ReplicatedObject, RepState, *ObjectReferences.Array, bIsHandover ? nullptr : (uint8*)StoredArray->GetData(), (uint8*)Array->GetData(), NewMaxOffset, RepNotifies, bOutSomeObjectsWereMapped);
			continue;
		}

		bool bResolvedSomeRefs = false;
		UObject* SinglePropObject = nullptr;
		FUnrealObjectRef SinglePropRef = FUnrealObjectRef::NULL_OBJECT_REF;

		for (auto UnresolvedIt = ObjectReferences.UnresolvedRefs.CreateIterator(); UnresolvedIt; ++UnresolvedIt)
		{
			FUnrealObjectRef& ObjectRef = *UnresolvedIt;

			bool bUnresolved = false;
			UObject* Object = FUnrealObjectRef::ToObjectPtr(ObjectRef, PackageMap, bUnresolved);
			if (!bUnresolved)
			{
				check(Object != nullptr);

				UE_LOG(LogSpatialReceiver, Verbose, TEXT("ResolveObjectReferences: Resolved object ref: Offset: %d, Object ref: %s, PropName: %s, ObjName: %s"), AbsOffset, *ObjectRef.ToString(), *Property->GetNameCPP(), *Object->GetName());

				if (ObjectReferences.bSingleProp)
				{
					SinglePropObject = Object;
					SinglePropRef = ObjectRef;
				}

				UnresolvedIt.RemoveCurrent();

				bResolvedSomeRefs = true;
			}
		}

		if (bResolvedSomeRefs)
		{
			if (!bOutSomeObjectsWereMapped)
			{
				ReplicatedObject->PreNetReceive();
				bOutSomeObjectsWereMapped = true;
			}

			if (Parent && Parent->Property->HasAnyPropertyFlags(CPF_RepNotify))
			{
				Property->CopySingleValue(StoredData + StoredDataOffset, Data + AbsOffset);
			}

			if (ObjectReferences.bSingleProp)
			{
				UObjectPropertyBase* ObjectProperty = Cast<UObjectPropertyBase>(Property);
				check(ObjectProperty);

				ObjectProperty->SetObjectPropertyValue(Data + AbsOffset, SinglePropObject);
				ObjectReferences.MappedRefs.Add(SinglePropRef);
			}
			else if (ObjectReferences.bFastArrayProp)
			{
				TSet<FUnrealObjectRef> NewMappedRefs;
				TSet<FUnrealObjectRef> NewUnresolvedRefs;
				FSpatialNetBitReader ValueDataReader(PackageMap, ObjectReferences.Buffer.GetData(), ObjectReferences.NumBufferBits, NewMappedRefs, NewUnresolvedRefs);

				check(Property->IsA<UArrayProperty>());
				UScriptStruct* NetDeltaStruct = GetFastArraySerializerProperty(Cast<UArrayProperty>(Property));

				FSpatialNetDeltaSerializeInfo::DeltaSerializeRead(NetDriver, ValueDataReader, ReplicatedObject, Parent->ArrayIndex, Parent->Property, NetDeltaStruct);

				ObjectReferences.MappedRefs.Append(NewMappedRefs);
			}
			else
			{
				TSet<FUnrealObjectRef> NewMappedRefs;
				TSet<FUnrealObjectRef> NewUnresolvedRefs;
				FSpatialNetBitReader BitReader(PackageMap, ObjectReferences.Buffer.GetData(), ObjectReferences.NumBufferBits, NewMappedRefs, NewUnresolvedRefs);
				check(Property->IsA<UStructProperty>());

				bool bHasUnresolved = false;
				ReadStructProperty(BitReader, Cast<UStructProperty>(Property), NetDriver, Data + AbsOffset, bHasUnresolved);

				ObjectReferences.MappedRefs.Append(NewMappedRefs);
			}

			if (Parent && Parent->Property->HasAnyPropertyFlags(CPF_RepNotify))
			{
				if (Parent->RepNotifyCondition == REPNOTIFY_Always || !Property->Identical(StoredData + StoredDataOffset, Data + AbsOffset))
				{
					RepNotifies.AddUnique(Parent->Property);
				}
			}
		}
	}
}

void USpatialReceiver::OnHeartbeatComponentUpdate(const Worker_ComponentUpdateOp& Op)
{
	if (!NetDriver->IsServer())
	{
		// Clients can ignore Heartbeat component updates.
		return;
	}

	TWeakObjectPtr<USpatialNetConnection>* ConnectionPtr = AuthorityPlayerControllerConnectionMap.Find(Op.entity_id);
	if (ConnectionPtr == nullptr)
	{
		// Heartbeat component update on a PlayerController that this server does not have authority over.
		// TODO: Disable component interest for Heartbeat components this server doesn't care about - UNR-986
		return;
	}

	if (!ConnectionPtr->IsValid())
	{
		UE_LOG(LogSpatialReceiver, Warning, TEXT("Received heartbeat component update after NetConnection has been cleaned up. PlayerController entity: %lld"), Op.entity_id);
		AuthorityPlayerControllerConnectionMap.Remove(Op.entity_id);
		return;
	}

	USpatialNetConnection* NetConnection = ConnectionPtr->Get();

	Schema_Object* EventsObject = Schema_GetComponentUpdateEvents(Op.update.schema_type);
	uint32 EventCount = Schema_GetObjectCount(EventsObject, SpatialConstants::HEARTBEAT_EVENT_ID);
	if (EventCount > 0)
	{
		if (EventCount > 1)
		{
			UE_LOG(LogSpatialReceiver, Verbose, TEXT("Received multiple heartbeat events in a single component update, entity %lld."), Op.entity_id);
		}

		NetConnection->OnHeartbeat();
	}

	Schema_Object* FieldsObject = Schema_GetComponentUpdateFields(Op.update.schema_type);
	if (Schema_GetBoolCount(FieldsObject, SpatialConstants::HEARTBEAT_CLIENT_HAS_QUIT_ID) > 0 &&
		GetBoolFromSchema(FieldsObject, SpatialConstants::HEARTBEAT_CLIENT_HAS_QUIT_ID))
	{
		// Client has disconnected, let's clean up their connection.
		NetConnection->CleanUp();
		AuthorityPlayerControllerConnectionMap.Remove(Op.entity_id);
	}
}

void USpatialReceiver::PeriodicallyProcessIncomingRPCs()
{
	FTimerHandle IncomingRPCsPeriodicProcessTimer;
	TimerManager->SetTimer(IncomingRPCsPeriodicProcessTimer, [WeakThis = TWeakObjectPtr<USpatialReceiver>(this)]()
	{
		if (USpatialReceiver* SpatialReceiver = WeakThis.Get())
		{
			SpatialReceiver->IncomingRPCs.ProcessRPCs();
		}
	}, GetDefault<USpatialGDKSettings>()->QueuedIncomingRPCWaitTime, true);
}

bool USpatialReceiver::NeedToLoadClass(const FString& ClassPath)
{
	return FindObject<UClass>(nullptr, *ClassPath, false) == nullptr;
}

FString USpatialReceiver::GetPackagePath(const FString& ClassPath)
{
	return FSoftObjectPath(ClassPath).GetLongPackageName();
}

void USpatialReceiver::StartAsyncLoadingClass(const FString& ClassPath, Worker_EntityId EntityId)
{
	FString PackagePath = GetPackagePath(ClassPath);
	FName PackagePathName = *PackagePath;

	bool bAlreadyLoading = AsyncLoadingPackages.Contains(PackagePathName);

	if (IsEntityWaitingForAsyncLoad(EntityId))
	{
		// This shouldn't happen because even if the entity goes out and comes back into view,
		// we would've received a RemoveEntity op that would remove the entry from the map.
		UE_LOG(LogSpatialReceiver, Error, TEXT("USpatialReceiver::ReceiveActor: Checked out entity but it's already waiting for async load! Entity: %lld"), EntityId);
	}

	EntityWaitingForAsyncLoad AsyncLoadEntity;
	AsyncLoadEntity.ClassPath = ClassPath;
	AsyncLoadEntity.InitialPendingAddComponents = ExtractAddComponents(EntityId);
	AsyncLoadEntity.PendingOps = ExtractAuthorityOps(EntityId);

	EntitiesWaitingForAsyncLoad.Emplace(EntityId, MoveTemp(AsyncLoadEntity));
	AsyncLoadingPackages.FindOrAdd(PackagePathName).Add(EntityId);

	UE_LOG(LogSpatialReceiver, Log, TEXT("Async loading package %s for entity %lld. Already loading: %s"), *PackagePath, EntityId, bAlreadyLoading ? TEXT("true") : TEXT("false"));
	if (!bAlreadyLoading)
	{
		LoadPackageAsync(PackagePath, FLoadPackageAsyncDelegate::CreateUObject(this, &USpatialReceiver::OnAsyncPackageLoaded));
	}
}

void USpatialReceiver::OnAsyncPackageLoaded(const FName& PackageName, UPackage* Package, EAsyncLoadingResult::Type Result)
{
	TArray<Worker_EntityId> Entities;
	if (!AsyncLoadingPackages.RemoveAndCopyValue(PackageName, Entities))
	{
		UE_LOG(LogSpatialReceiver, Error, TEXT("USpatialReceiver::OnAsyncPackageLoaded: Package loaded but no entry in AsyncLoadingPackages. Package: %s"), *PackageName.ToString());
		return;
	}

	for (Worker_EntityId Entity : Entities)
	{
		if (IsEntityWaitingForAsyncLoad(Entity))
		{
			UE_LOG(LogSpatialReceiver, Log, TEXT("Finished async loading package %s for entity %lld."), *PackageName.ToString(), Entity);

			// Save critical section if we're in one and restore upon leaving this scope.
			CriticalSectionSaveState CriticalSectionState(*this);

			EntityWaitingForAsyncLoad AsyncLoadEntity = EntitiesWaitingForAsyncLoad.FindAndRemoveChecked(Entity);
			PendingAddEntities.Add(Entity);
			PendingAddComponents = MoveTemp(AsyncLoadEntity.InitialPendingAddComponents);
			LeaveCriticalSection();

			for (QueuedOpForAsyncLoad& Op : AsyncLoadEntity.PendingOps)
			{
				HandleQueuedOpForAsyncLoad(Op);
			}
		}
	}
}

void USpatialReceiver::MoveMappedObjectToUnmapped(const FUnrealObjectRef& Ref)
{
	if (TSet<FChannelObjectPair>* RepStatesWithMappedRef = ObjectRefToRepStateMap.Find(Ref))
	{
		for (const FChannelObjectPair& ChannelObject : *RepStatesWithMappedRef)
		{
			if (USpatialActorChannel* Channel = ChannelObject.Key.Get())
			{
				if (FSpatialObjectRepState* RepState = Channel->ObjectReferenceMap.Find(ChannelObject.Value))
				{
					RepState->MoveMappedObjectToUnmapped(Ref);
				}
			}
		}
	}
}

bool USpatialReceiver::IsEntityWaitingForAsyncLoad(Worker_EntityId Entity)
{
	return EntitiesWaitingForAsyncLoad.Contains(Entity);
}

void USpatialReceiver::QueueAddComponentOpForAsyncLoad(const Worker_AddComponentOp& Op)
{
	EntityWaitingForAsyncLoad& AsyncLoadEntity = EntitiesWaitingForAsyncLoad.FindChecked(Op.entity_id);

	QueuedOpForAsyncLoad NewOp = {};
	NewOp.AcquiredData = Worker_AcquireComponentData(&Op.data);
	NewOp.Op.op_type = WORKER_OP_TYPE_ADD_COMPONENT;
	NewOp.Op.op.add_component.entity_id = Op.entity_id;
	NewOp.Op.op.add_component.data = *NewOp.AcquiredData;
	AsyncLoadEntity.PendingOps.Add(NewOp);
}

void USpatialReceiver::QueueRemoveComponentOpForAsyncLoad(const Worker_RemoveComponentOp& Op)
{
	EntityWaitingForAsyncLoad& AsyncLoadEntity = EntitiesWaitingForAsyncLoad.FindChecked(Op.entity_id);

	QueuedOpForAsyncLoad NewOp = {};
	NewOp.Op.op_type = WORKER_OP_TYPE_REMOVE_COMPONENT;
	NewOp.Op.op.remove_component = Op;
	AsyncLoadEntity.PendingOps.Add(NewOp);
}

void USpatialReceiver::QueueAuthorityOpForAsyncLoad(const Worker_AuthorityChangeOp& Op)
{
	EntityWaitingForAsyncLoad& AsyncLoadEntity = EntitiesWaitingForAsyncLoad.FindChecked(Op.entity_id);

	QueuedOpForAsyncLoad NewOp = {};
	NewOp.Op.op_type = WORKER_OP_TYPE_AUTHORITY_CHANGE;
	NewOp.Op.op.authority_change = Op;
	AsyncLoadEntity.PendingOps.Add(NewOp);
}

void USpatialReceiver::QueueComponentUpdateOpForAsyncLoad(const Worker_ComponentUpdateOp& Op)
{
	EntityWaitingForAsyncLoad& AsyncLoadEntity = EntitiesWaitingForAsyncLoad.FindChecked(Op.entity_id);

	QueuedOpForAsyncLoad NewOp = {};
	NewOp.AcquiredUpdate = Worker_AcquireComponentUpdate(&Op.update);
	NewOp.Op.op_type = WORKER_OP_TYPE_COMPONENT_UPDATE;
	NewOp.Op.op.component_update.entity_id = Op.entity_id;
	NewOp.Op.op.component_update.update = *NewOp.AcquiredUpdate;
	AsyncLoadEntity.PendingOps.Add(NewOp);
}

TArray<PendingAddComponentWrapper> USpatialReceiver::ExtractAddComponents(Worker_EntityId Entity)
{
	TArray<PendingAddComponentWrapper> ExtractedAddComponents;
	TArray<PendingAddComponentWrapper> RemainingAddComponents;

	for (PendingAddComponentWrapper& AddComponent : PendingAddComponents)
	{
		if (AddComponent.EntityId == Entity)
		{
			ExtractedAddComponents.Add(MoveTemp(AddComponent));
		}
		else
		{
			RemainingAddComponents.Add(MoveTemp(AddComponent));
		}
	}
	PendingAddComponents = MoveTemp(RemainingAddComponents);
	return ExtractedAddComponents;
}

TArray<USpatialReceiver::QueuedOpForAsyncLoad> USpatialReceiver::ExtractAuthorityOps(Worker_EntityId Entity)
{
	TArray<QueuedOpForAsyncLoad> ExtractedOps;
	TArray<Worker_AuthorityChangeOp> RemainingOps;

	for (const Worker_AuthorityChangeOp& Op : PendingAuthorityChanges)
	{
		if (Op.entity_id == Entity)
		{
			QueuedOpForAsyncLoad NewOp = {};
			NewOp.Op.op_type = WORKER_OP_TYPE_AUTHORITY_CHANGE;
			NewOp.Op.op.authority_change = Op;
			ExtractedOps.Add(NewOp);
		}
		else
		{
			RemainingOps.Add(Op);
		}
	}
	PendingAuthorityChanges = MoveTemp(RemainingOps);
	return ExtractedOps;
}

void USpatialReceiver::HandleQueuedOpForAsyncLoad(QueuedOpForAsyncLoad& Op)
{
	switch (Op.Op.op_type)
	{
	case WORKER_OP_TYPE_ADD_COMPONENT:
		OnAddComponent(Op.Op.op.add_component);
		Worker_ReleaseComponentData(Op.AcquiredData);
		break;
	case WORKER_OP_TYPE_REMOVE_COMPONENT:
		ProcessRemoveComponent(Op.Op.op.remove_component);
		break;
	case WORKER_OP_TYPE_AUTHORITY_CHANGE:
		OnAuthorityChange(Op.Op.op.authority_change);
		break;
	case WORKER_OP_TYPE_COMPONENT_UPDATE:
		OnComponentUpdate(Op.Op.op.component_update);
		Worker_ReleaseComponentUpdate(Op.AcquiredUpdate);
		break;
	default:
		checkNoEntry();
	}
}

USpatialReceiver::CriticalSectionSaveState::CriticalSectionSaveState(USpatialReceiver& InReceiver)
	: Receiver(InReceiver)
	, bInCriticalSection(InReceiver.bInCriticalSection)
{
	if (bInCriticalSection)
	{
		PendingAddEntities = MoveTemp(Receiver.PendingAddEntities);
		PendingAuthorityChanges = MoveTemp(Receiver.PendingAuthorityChanges);
		PendingAddComponents = MoveTemp(Receiver.PendingAddComponents);
		Receiver.PendingAddEntities.Empty();
		Receiver.PendingAuthorityChanges.Empty();
		Receiver.PendingAddComponents.Empty();
	}
	Receiver.bInCriticalSection = true;
}

USpatialReceiver::CriticalSectionSaveState::~CriticalSectionSaveState()
{
	if (bInCriticalSection)
	{
		Receiver.PendingAddEntities = MoveTemp(PendingAddEntities);
		Receiver.PendingAuthorityChanges = MoveTemp(PendingAuthorityChanges);
		Receiver.PendingAddComponents = MoveTemp(PendingAddComponents);
	}
	Receiver.bInCriticalSection = bInCriticalSection;
}

namespace
{
	FString GetObjectNameFromRepState(const FSpatialObjectRepState& RepState)
	{
		if (UObject* Obj = RepState.GetChannelObjectPair().Value.Get())
		{
			return Obj->GetName();
		}
		return TEXT("<unknown>");
	}
}

void USpatialReceiver::CleanupRepStateMap(FSpatialObjectRepState& RepState)
{
	for (const FUnrealObjectRef& Ref : RepState.ReferencedObj)
	{
		TSet<FChannelObjectPair>* RepStatesWithMappedRef = ObjectRefToRepStateMap.Find(Ref);
		if (ensureMsgf(RepStatesWithMappedRef, TEXT("Ref to entity %lld on object %s is missing its referenced entry in the Ref/RepState map"), Ref.Entity, *GetObjectNameFromRepState(RepState)))
		{
			checkf(RepStatesWithMappedRef->Contains(RepState.GetChannelObjectPair()), TEXT("Ref to entity %lld on object %s is missing its referenced entry in the Ref/RepState map"), Ref.Entity, *GetObjectNameFromRepState(RepState));
			RepStatesWithMappedRef->Remove(RepState.GetChannelObjectPair());
			if (RepStatesWithMappedRef->Num() == 0)
			{
				ObjectRefToRepStateMap.Remove(Ref);
			}
		}
	}
}
