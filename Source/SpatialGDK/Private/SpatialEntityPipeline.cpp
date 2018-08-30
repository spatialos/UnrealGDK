// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "SpatialReceiver.h"

#include "EngineMinimal.h"
#include "EntityRegistry.h"
#include "GameFramework/PlayerController.h"
#include "SpatialActorChannel.h"
#include "SpatialNetConnection.h"
#include "SpatialNetDriver.h"
#include "SpatialPackageMapClient.h"
#include "SpatialInterop.h"

#include "CoreTypes/UnrealMetadata.h"
#include "CoreTypes/DynamicComponent.h"

template <typename T>
T* GetComponentData(USpatialEntityPipeline& PipelineBlock, Worker_EntityId EntityId)
{
	for (PendingAddComponentWrapper& PendingAddComponent : PipelineBlock.PendingAddComponents)
	{
		if (PendingAddComponent.EntityId == EntityId && PendingAddComponent.ComponentId == T::ComponentId)
		{
			return static_cast<T*>(PendingAddComponent.Data.get());
		}
	}

	return nullptr;
}

void USpatialEntityPipeline::Init(USpatialNetDriver* NetDriver)
{
	this->NetDriver = NetDriver;
	this->World = NetDriver->GetWorld();
}

void USpatialEntityPipeline::OnCriticalSection(bool InCriticalSection)
{
	if (InCriticalSection)
	{
		EnterCriticalSection();
	}
	else
	{
		LeaveCriticalSection();
	}
}

void USpatialEntityPipeline::EnterCriticalSection()
{
	UE_LOG(LogTemp, Log, TEXT("!!! CAPIPipelineBlock: Entering critical section."));
	check(!bInCriticalSection);
	bInCriticalSection = true;
}

void USpatialEntityPipeline::LeaveCriticalSection()
{
	UE_LOG(LogTemp, Log, TEXT("!!! CAPIPipelineBlock: Leaving critical section."));
	check(bInCriticalSection);

	// Add entities.
	for (Worker_EntityId& PendingAddEntity : PendingAddEntities)
	{
		CreateActor(PendingAddEntity);
	}

	// Remove entities.
	for (Worker_EntityId& PendingRemoveEntity : PendingRemoveEntities)
	{
		RemoveActor(PendingRemoveEntity);
	}

	// Mark that we've left the critical section.
	bInCriticalSection = false;
	PendingAddEntities.Empty();
	PendingAddComponents.Empty();
	PendingRemoveEntities.Empty();

	for (TPair<UObject*, UnrealObjectRef>& It : ResolvedObjectQueue)
	{
		ResolvePendingOperations_Internal(It.Key, It.Value);
	}
	ResolvedObjectQueue.Empty();
}

void USpatialEntityPipeline::OnAddEntity(Worker_AddEntityOp& Op)
{
	UE_LOG(LogTemp, Log, TEXT("PipelineBlock: AddEntity: %lld"), Op.entity_id);
	check(bInCriticalSection);

	PendingAddEntities.Emplace(Op.entity_id);
}

void USpatialEntityPipeline::OnAddComponent(Worker_AddComponentOp& Op)
{
	UE_LOG(LogTemp, Log, TEXT("!!! CAPIPipelineBlock: AddComponent component ID: %u entity ID: %lld inCriticalSection: %d"),
		Op.data.component_id, Op.entity_id, (int)bInCriticalSection);

	std::shared_ptr<Component> Data;

	switch (Op.data.component_id)
	{
	case ENTITY_ACL_COMPONENT_ID:
		Data = std::make_shared<Component>(EntityAcl(Op.data));
		break;
	case METADATA_COMPONENT_ID:
		Data = std::make_shared<Component>(Metadata(Op.data));
		break;
	case POSITION_COMPONENT_ID:
		Data = std::make_shared<Component>(Position(Op.data));
		break;
	case PERSISTENCE_COMPONENT_ID:
		Data = std::make_shared<Component>(Persistence(Op.data));
		break;
	case UNREAL_METADATA_COMPONENT_ID:
		Data = std::make_shared<Component>(UnrealMetadata(Op.data));
		break;
	default:
		Data = std::make_shared<Component>(DynamicComponent(Op.data));
		break;
	}

	PendingAddComponents.Emplace(Op.entity_id, Op.data.component_id, Data);
}

void USpatialEntityPipeline::OnRemoveEntity(Worker_RemoveEntityOp& Op)
{
	UE_LOG(LogTemp, Log, TEXT("CAPIPipelineBlock: RemoveEntity: %lld"), Op.entity_id);

	if (bInCriticalSection)
	{
		PendingRemoveEntities.Emplace(Op.entity_id);
	}
	else
	{
		RemoveActor(Op.entity_id);
	}
}

void USpatialEntityPipeline::OnComponentUpdate(Worker_ComponentUpdateOp& Op)
{
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

	UClass* Class = TypebindingManager->FindClassByComponentId(Op.update.component_id);
	checkf(Class, TEXT("Component %d isn't hand-written and not present in ComponentToClassMap."));
	FClassInfo* Info = TypebindingManager->FindClassInfoByClass(Class);
	check(Info);

	bool bAutonomousProxy = NetDriver->GetNetMode() == NM_Client && HasComponentAuthority(Op.entity_id, Info->RPCComponents[RPC_Client]);

	USpatialActorChannel* ActorChannel = GetActorChannelByEntityId(Op.entity_id);
	bool bIsServer = NetDriver->IsServer();

	if (Op.update.component_id == Info->SingleClientComponent)
	{
		check(ActorChannel);

		UObject* TargetObject = GetTargetObjectFromChannelAndClass(ActorChannel, Class);
		ApplyComponentUpdate(Op.update, TargetObject, ActorChannel, GROUP_SingleClient, bAutonomousProxy);
	}
	else if (Op.update.component_id == Info->MultiClientComponent)
	{
		check(ActorChannel);

		UObject* TargetObject = GetTargetObjectFromChannelAndClass(ActorChannel, Class);
		ApplyComponentUpdate(Op.update, TargetObject, ActorChannel, GROUP_MultiClient, bAutonomousProxy);
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
	else if (Op.update.component_id == Info->RPCComponents[RPC_NetMulticast])
	{
		check(ActorChannel);
		const TArray<UFunction*>& RPCArray = Info->RPCs.FindChecked(RPC_NetMulticast);
		ReceiveMulticastUpdate(Op.update, Op.entity_id, RPCArray, PackageMap, NetDriver);
	}
	else
	{
		UE_LOG(LogTemp, Verbose, TEXT("Skipping because it's an empty component update from an RPC component. (most likely as a result of gaining authority)"));
	}
}

void USpatialEntityPipeline::ApplyComponentUpdate(const Worker_ComponentUpdate& ComponentUpdate, UObject* TargetObject, USpatialActorChannel* Channel, EReplicatedPropertyGroup PropertyGroup, bool bAutonomousProxy)
{
	FChannelObjectPair ChannelObjectPair(Channel, TargetObject);

	FObjectReferencesMap& ObjectReferencesMap = UnresolvedRefsMap.FindOrAdd(ChannelObjectPair);
	TSet<UnrealObjectRef> UnresolvedRefs;
	ReceiveDynamicUpdate(ComponentUpdate, TargetObject, Channel, PackageMap, NetDriver, PropertyGroup, bAutonomousProxy, ObjectReferencesMap, UnresolvedRefs);

	QueueIncomingRepUpdates(ChannelObjectPair, ObjectReferencesMap, UnresolvedRefs);
}

void USpatialEntityPipeline::CreateActor(Worker_EntityId EntityId)
{
	checkf(World, TEXT("We should have a world whilst processing ops."));
	check(NetDriver);

	UEntityRegistry* EntityRegistry = NetDriver->GetEntityRegistry();
	check(EntityRegistry);

	Position* PositionComponent = GetComponentData<Position>(*this, EntityId);
	Metadata* MetadataComponent = GetComponentData<Metadata>(*this, EntityId);

	AActor* EntityActor = EntityRegistry->GetActorFromEntityId(EntityId);
	UE_LOG(LogTemp, Log, TEXT("!!! Checked out entity with entity ID %lld"), EntityId);

	if (EntityActor)
	{
		UClass* ActorClass = GetNativeEntityClass(MetadataComponent);

		// Option 1
		UE_LOG(LogTemp, Log, TEXT("Entity for core actor %s has been checked out on the worker which spawned it."), *EntityActor->GetName());

		UnrealMetadata* UnrealMetadataComponent = GetComponentData<UnrealMetadata>(*this, EntityId);
		check(UnrealMetadataComponent);

		USpatialPackageMapClient* PackageMap = Cast<USpatialPackageMapClient>(NetDriver->GetSpatialOSNetConnection()->PackageMap);
		check(PackageMap);

		SubobjectToOffsetMap SubobjectNameToOffset;
		for (auto& KVPair : UnrealMetadataComponent->SubobjectNameToOffset)
		{
			SubobjectNameToOffset.Add(UTF8_TO_TCHAR(KVPair.first.c_str()), KVPair.second);
		}

		FNetworkGUID NetGUID = PackageMap->ResolveEntityActor(EntityActor, EntityId, SubobjectNameToOffset);
		UE_LOG(LogTemp, Log, TEXT("Received create entity response op for %lld"), EntityId);
	}
	else
	{
		UClass* ActorClass = GetNativeEntityClass(MetadataComponent);

		if(ActorClass == nullptr)
		{
			return;
		}

		// Initial Singleton Actor replication is handled with USpatialInterop::LinkExistingSingletonActors
		//if (NetDriver->IsServer() && Interop->IsSingletonClass(ActorClass))
		//{
		//	return;
		//}

		UNetConnection* Connection = nullptr;
		UnrealMetadata* UnrealMetadataComponent = GetComponentData<UnrealMetadata>(*this, EntityId);
		check(UnrealMetadataComponent);
		bool bDoingDeferredSpawn = false;

		// If we're checking out a player controller, spawn it via "USpatialNetDriver::AcceptNewPlayer"
		if (NetDriver->IsServer() && ActorClass->IsChildOf(APlayerController::StaticClass()))
		{
			checkf(!UnrealMetadataComponent->OwnerWorkerId.empty(), TEXT("A player controller entity must have an owner worker ID."));
			FString URLString = FURL().ToString();
			FString OwnerWorkerId = UTF8_TO_TCHAR(UnrealMetadataComponent->OwnerWorkerId.c_str());
			URLString += TEXT("?workerId=") + OwnerWorkerId;
			Connection = NetDriver->AcceptNewPlayer(FURL(nullptr, *URLString, TRAVEL_Absolute), true);
			check(Connection);
			EntityActor = Connection->PlayerController;
		}
		else
		{
			// Either spawn the actor or get it from the level if it has a persistent name.
			if (UnrealMetadataComponent->StaticPath.empty())
			{
				UE_LOG(LogTemp, Log, TEXT("!!! Spawning a native dynamic %s whilst checking out an entity."), *ActorClass->GetFullName());
				EntityActor = SpawnNewEntity(PositionComponent, ActorClass, true);
				bDoingDeferredSpawn = true;
			}
			else
			{
				FString FullPath = UTF8_TO_TCHAR(UnrealMetadataComponent->StaticPath.c_str());
				UE_LOG(LogTemp, Log, TEXT("!!! Searching for a native static actor %s of class %s in the persistent level whilst checking out an entity."), *FullPath, *ActorClass->GetName());
				EntityActor = FindObject<AActor>(World, *FullPath);
			}
			check(EntityActor);

			// Get the net connection for this actor.
			if (NetDriver->IsServer())
			{
				// TODO(David): Currently, we just create an actor channel on the "catch-all" connection, then create a new actor channel once we check out the player controller
				// and create a new connection. This is fine due to lazy actor channel creation in USpatialNetDriver::ServerReplicateActors. However, the "right" thing to do
				// would be to make sure to create anything which depends on the PlayerController _after_ the PlayerController's connection is set up so we can use the right
				// one here.
				Connection = NetDriver->GetSpatialOSNetConnection();
			}
			else
			{
				Connection = NetDriver->GetSpatialOSNetConnection();
			}
		}

		// Add to entity registry. 
		EntityRegistry->AddToRegistry(EntityId, EntityActor);

		// Set up actor channel.
		USpatialPackageMapClient* PackageMap = Cast<USpatialPackageMapClient>(Connection->PackageMap);
		USpatialActorChannel* Channel = Cast<USpatialActorChannel>(Connection->CreateChannel(CHTYPE_Actor, NetDriver->IsServer()));
		check(Channel);

		if (bDoingDeferredSpawn)
		{
			FVector InitialLocation = Coordinates::ToFVector(PositionComponent->Coords);
			FVector SpawnLocation = FRepMovement::RebaseOntoLocalOrigin(InitialLocation, World->OriginLocation);
			EntityActor->FinishSpawning(FTransform(FRotator::ZeroRotator, SpawnLocation));
		}

		SubobjectToOffsetMap SubobjectNameToOffset;
		for (auto& KVPair : UnrealMetadataComponent->SubobjectNameToOffset)
		{
			SubobjectNameToOffset.Add(UTF8_TO_TCHAR(KVPair.first.c_str()), KVPair.second);
		}

		PackageMap->ResolveEntityActor(EntityActor, EntityId, SubobjectNameToOffset);
		Channel->SetChannelActor(EntityActor);

		// Apply initial replicated properties.
		// This was moved to after FinishingSpawning because components existing only in blueprints aren't added until spawning is complete
		// Potentially we could split out the initial actor state and the initial component state
		for (PendingAddComponentWrapper& PendingAddComponent : PendingAddComponents)
		{
			if (PendingAddComponent.EntityId == EntityId && PendingAddComponent.Data && PendingAddComponent.Data->bIsDynamic)
			{
				ApplyComponentData(EntityId, *static_cast<DynamicComponent*>(PendingAddComponent.Data.get())->Data, Channel, PackageMap);
			}
		}

		// Update interest on the entity's components after receiving initial component data (so Role and RemoteRole are properly set).
		//NetDriver->GetSpatialInterop()->SendComponentInterests(Channel, EntityId.ToSpatialEntityId());

		// This is a bit of a hack unfortunately, among the core classes only PlayerController implements this function and it requires
		// a player index. For now we don't support split screen, so the number is always 0.
		if (NetDriver->ServerConnection)
		{
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

			// Call PostNetInit on client only.
			EntityActor->PostNetInit();
		}
	}
}

void USpatialEntityPipeline::RemoveActor(Worker_EntityId EntityId)
{
	AActor* Actor = NetDriver->GetEntityRegistry()->GetActorFromEntityId(EntityId);

	UE_LOG(LogTemp, Log, TEXT("CAPIPipelineBlock: Remove Actor: %s %lld"), Actor ? *Actor->GetName() : TEXT("nullptr"), EntityId);

	// Actor already deleted (this worker was most likely authoritative over it and deleted it earlier).
	if (!Actor || Actor->IsPendingKill())
	{
		CleanupDeletedEntity(EntityId);
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
	//NetDriver->GetSpatialInterop()->StartIgnoringAuthoritativeDestruction();
	if (!World->DestroyActor(Actor, true))
	{
		UE_LOG(LogTemp, Error, TEXT("World->DestroyActor failed on RemoveActor %s %lld"), *Actor->GetName(), EntityId);
	}
	//NetDriver->GetSpatialInterop()->StopIgnoringAuthoritativeDestruction();

	CleanupDeletedEntity(EntityId);
}

void USpatialEntityPipeline::CleanupDeletedEntity(Worker_EntityId EntityId)
{
	NetDriver->GetEntityRegistry()->RemoveFromRegistry(EntityId);
	Cast<USpatialPackageMapClient>(NetDriver->GetSpatialOSNetConnection()->PackageMap)->RemoveEntityActor(EntityId);
}

UClass* USpatialEntityPipeline::GetNativeEntityClass(Metadata* MetadataComponent)
{
	FString Metadata = UTF8_TO_TCHAR(MetadataComponent->EntityType.c_str());
	return FindObject<UClass>(ANY_PACKAGE, *Metadata);
}

// Note that in SpatialGDK, this function will not be called on the spawning worker.
// It's only for client, and in the future, other workers.
AActor* USpatialEntityPipeline::SpawnNewEntity(Position* PositionComponent, UClass* ActorClass, bool bDeferred)
{
	FVector InitialLocation = Coordinates::ToFVector(PositionComponent->Coords);
	AActor* NewActor = nullptr;
	if (ActorClass)
	{
		//bRemoteOwned needs to be public in source code. This might be a controversial change.
		FActorSpawnParameters SpawnInfo;
		SpawnInfo.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		SpawnInfo.bRemoteOwned = !NetDriver->IsServer();
		SpawnInfo.bNoFail = true;
		// We defer the construction in the GDK pipeline to allow initialization of replicated properties first.
		SpawnInfo.bDeferConstruction = bDeferred;

		FVector SpawnLocation = FRepMovement::RebaseOntoLocalOrigin(InitialLocation, World->OriginLocation);

		NewActor = World->SpawnActorAbsolute(ActorClass, FTransform(FRotator::ZeroRotator, SpawnLocation), SpawnInfo);
		check(NewActor);
	}

	return NewActor;
}

void USpatialEntityPipeline::ApplyComponentData(Worker_EntityId EntityId, Worker_ComponentData& Data, USpatialActorChannel* Channel, USpatialPackageMapClient* PackageMap)
{
	UClass* Class= TypebindingManager->FindClassByComponentId(Data.component_id);
	checkf(Class, TEXT("Component %d isn't hand-written and not present in ComponentToClassMap."));

	UObject* TargetObject = GetTargetObjectFromChannelAndClass(Channel, Class);
	FChannelObjectPair ChannelObjectPair(Channel, TargetObject);

	FClassInfo* Info = TypebindingManager->FindClassInfoByClass(Class);
	check(Info);

	bool bAutonomousProxy = NetDriver->GetNetMode() == NM_Client && HasComponentAuthority(EntityId, Info->RPCComponents[RPC_Client]);

	if (Data.component_id == Info->SingleClientComponent)
	{
		FObjectReferencesMap& ObjectReferencesMap = UnresolvedRefsMap.FindOrAdd(ChannelObjectPair);
		TSet<UnrealObjectRef> UnresolvedRefs;

		ReadDynamicData(Data, TargetObject, Channel, PackageMap, NetDriver, GROUP_SingleClient, bAutonomousProxy, ObjectReferencesMap, UnresolvedRefs);

		QueueIncomingRepUpdates(ChannelObjectPair, ObjectReferencesMap, UnresolvedRefs);
	}
	else if (Data.component_id == Info->MultiClientComponent)
	{
		FObjectReferencesMap& ObjectReferencesMap = UnresolvedRefsMap.FindOrAdd(ChannelObjectPair);
		TSet<UnrealObjectRef> UnresolvedRefs;

		ReadDynamicData(Data, TargetObject, Channel, PackageMap, NetDriver, GROUP_MultiClient, bAutonomousProxy, ObjectReferencesMap, UnresolvedRefs);

		QueueIncomingRepUpdates(ChannelObjectPair, ObjectReferencesMap, UnresolvedRefs);
	}
	else if (Data.component_id == Info->HandoverComponent)
	{
		// TODO: Handover
	}
	else
	{
		UE_LOG(LogTemp, Verbose, TEXT("Skipping because RPC components don't have actual data."));
	}
}

UObject* USpatialEntityPipeline::GetTargetObjectFromChannelAndClass(USpatialActorChannel* Channel, UClass* Class)
{
	UObject* TargetObject = nullptr;

	if (Class->IsChildOf<AActor>())
	{
		check(Channel->Actor->IsA(Class));
		TargetObject = Channel->Actor;
	}
	else if (Class->IsChildOf<UActorComponent>())
	{
		FClassInfo* ActorInfo = TypebindingManager->FindClassInfoByClass(Channel->Actor->GetClass());
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
