// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "CAPIPipelineBlock.h"

#include "EngineMinimal.h"
#include "EntityRegistry.h"
#include "GameFramework/PlayerController.h"
#include "SpatialActorChannel.h"
#include "SpatialNetConnection.h"
#include "SpatialNetDriver.h"
#include "SpatialPackageMapClient.h"

template <typename T>
T* GetComponentData(CAPIPipelineBlock& PipelineBlock, Worker_EntityId EntityId)
{
	for (auto& PendingAddComponent : PipelineBlock.PendingAddComponents)
	{
		if (PendingAddComponent.EntityId == EntityId && PendingAddComponent.ComponentId == T::ComponentId)
		{
			return static_cast<T*>(PendingAddComponent.Data.get());
		}
	}

	return nullptr;
}

void CAPIPipelineBlock::EnterCriticalSection()
{
	UE_LOG(LogTemp, Log, TEXT("!!! CAPIPipelineBlock: Entering critical section."));
	check(!bInCriticalSection);
	bInCriticalSection = true;
}

void CAPIPipelineBlock::LeaveCriticalSection()
{
	UE_LOG(LogTemp, Log, TEXT("!!! CAPIPipelineBlock: Leaving critical section."));
	check(bInCriticalSection);

	// Add entities.
	for (auto& PendingAddEntity : PendingAddEntities)
	{
		CreateActor(PendingAddEntity);
	}

	// Remove entities.
	//for (auto& PendingRemoveEntity : PendingRemoveEntities)
	//{
	//	RemoveEntityImpl(PendingRemoveEntity);
	//}

	//NetDriver->GetSpatialInterop()->OnLeaveCriticalSection();

	// Mark that we've left the critical section.
	bInCriticalSection = false;
	PendingAddEntities.Empty();
	PendingAddComponents.Empty();
	//PendingAuthorityChanges.Empty();
	//PendingRemoveComponents.Empty();
	//PendingRemoveEntities.Empty();
}

void CAPIPipelineBlock::AddEntity(Worker_AddEntityOp& Op)
{
	UE_LOG(LogTemp, Log, TEXT("!!! CAPIPipelineBlock: AddEntity: %lld"), Op.entity_id);
	check(bInCriticalSection);

	PendingAddEntities.Emplace(Op.entity_id);
}

void CAPIPipelineBlock::AddComponent(Worker_AddComponentOp& Op)
{
	UE_LOG(LogTemp, Log, TEXT("!!! CAPIPipelineBlock: AddComponent component ID: %u entity ID: %lld inCriticalSection: %d"),
		Op.data.component_id, Op.entity_id, (int)bInCriticalSection);

	std::unique_ptr<ComponentData> Data;

	switch (Op.data.component_id)
	{
	case ENTITY_ACL_COMPONENT_ID:
		Data.reset(new EntityAclData());
		ReadEntityAclData(Op.data, static_cast<EntityAclData&>(*Data));
		break;
	case METADATA_COMPONENT_ID:
		Data.reset(new MetadataData());
		ReadMetadataData(Op.data, static_cast<MetadataData&>(*Data));
		break;
	case POSITION_COMPONENT_ID:
		Data.reset(new PositionData());
		ReadPositionData(Op.data, static_cast<PositionData&>(*Data));
		break;
	case PERSISTENCE_COMPONENT_ID:
		Data.reset(new PersistenceData());
		ReadPersistenceData(Op.data, static_cast<PersistenceData&>(*Data));
		break;
	case UNREAL_METADATA_COMPONENT_ID:
		Data.reset(new UnrealMetadataData());
		ReadUnrealMetadataData(Op.data, static_cast<UnrealMetadataData&>(*Data));
		break;
	default:
		break;
	}

	PendingAddComponents.Emplace(Op.entity_id, Op.data.component_id, std::move(Data));
}

void CAPIPipelineBlock::CreateActor(Worker_EntityId EntityId)
{
	checkf(World, TEXT("We should have a world whilst processing ops."));
	check(NetDriver);
	auto Interop = NetDriver->GetSpatialInterop();
	auto EntityRegistry = NetDriver->GetEntityRegistry();
	check(Interop);
	check(EntityRegistry);

	PositionData* PositionComponent = GetComponentData<PositionData>(*this, EntityId);
	MetadataData* MetadataComponent = GetComponentData<MetadataData>(*this, EntityId);

	if (!PositionComponent || !MetadataComponent)
	{
		return;
	}

	AActor* EntityActor = EntityRegistry->GetActorFromEntityId(FEntityId(EntityId));
	UE_LOG(LogTemp, Log, TEXT("!!! Checked out entity with entity ID %lld"), EntityId);

	// There are 3 main options when we get here with regards to how this entity was created:
	// 1) A SpawnActor() call (through interop) on this worker, which means it already has an actor associated with it.
	//	  This usually happens on the Unreal server only (as servers are the only workers which can spawn actors).
	// 2) A "pure" Spatial create entity request, which means we need to spawn an actor that was manually registered to correspond to it.
	// 3) A SpawnActor() call that was initiated from a different worker, which means we need to find and spawn the corresponding "native" actor that corresponds to it.
	//	  This can happen on either the client (for all actors) or server (for actors which were spawned by a different server worker, or are transitioning).

	if (EntityActor)
	{
		UClass* ActorClass = GetNativeEntityClass(MetadataComponent);

		// Option 1
		UE_LOG(LogTemp, Log, TEXT("!!! Entity for core actor %s has been checked out on the worker which spawned it."), *EntityActor->GetName());

		UnrealMetadataData* UnrealMetadataComponent = GetComponentData<UnrealMetadataData>(*this, EntityId);
		check(UnrealMetadataComponent);

		USpatialPackageMapClient* PackageMap = Cast<USpatialPackageMapClient>(NetDriver->GetSpatialOSNetConnection()->PackageMap);
		check(PackageMap);

		// Cheat a bit by using ::worker::Map
		SubobjectToOffsetMap SubobjectNameToOffset;
		for (auto& KVPair : UnrealMetadataComponent->SubobjectNameToOffset)
		{
			SubobjectNameToOffset.emplace(KVPair.first, KVPair.second);
		}

		FNetworkGUID NetGUID = PackageMap->ResolveEntityActor(EntityActor, FEntityId(EntityId), SubobjectNameToOffset);
		UE_LOG(LogTemp, Log, TEXT("!!! Received create entity response op for %lld"), EntityId);

		// actor channel/entity mapping should be registered by this point
		check(NetDriver->GetSpatialInterop()->GetActorChannelByEntityId(FEntityId(EntityId)));
	}
	else
	{
		UClass* ActorClass = nullptr;
		if (false)//(ActorClass = GetRegisteredEntityClass(MetadataComponent)) != nullptr)
		{
			// Option 2
			UE_LOG(LogTemp, Log, TEXT("!!! Spawning a registered %s"), *ActorClass->GetName());
			//EntityActor = SpawnNewEntity(PositionComponent, ActorClass, false);
			EntityRegistry->AddToRegistry(FEntityId(EntityId), EntityActor);
		}
		else if ((ActorClass = GetNativeEntityClass(MetadataComponent)) != nullptr)
		{
			// Option 3

			// Initial Singleton Actor replication is handled with USpatialInterop::LinkExistingSingletonActors
			//if (NetDriver->IsServer() && Interop->IsSingletonClass(ActorClass))
			//{
			//	return;
			//}

			UNetConnection* Connection = nullptr;
			UnrealMetadataData* UnrealMetadataComponent = GetComponentData<UnrealMetadataData>(*this, EntityId);
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
			EntityRegistry->AddToRegistry(FEntityId(EntityId), EntityActor);

			// Set up actor channel.
			auto PackageMap = Cast<USpatialPackageMapClient>(Connection->PackageMap);
			auto Channel = Cast<USpatialActorChannel>(Connection->CreateChannel(CHTYPE_Actor, NetDriver->IsServer()));
			check(Channel);

			if (bDoingDeferredSpawn)
			{
				auto InitialLocation = CAPIPositionToLocation(PositionComponent->Coords);
				FVector SpawnLocation = FRepMovement::RebaseOntoLocalOrigin(InitialLocation, World->OriginLocation);
				EntityActor->FinishSpawning(FTransform(FRotator::ZeroRotator, SpawnLocation));
			}

			// Cheat a bit by using ::worker::Map
			SubobjectToOffsetMap SubobjectNameToOffset;
			for (auto& KVPair : UnrealMetadataComponent->SubobjectNameToOffset)
			{
				SubobjectNameToOffset.emplace(KVPair.first, KVPair.second);
			}

			PackageMap->ResolveEntityActor(EntityActor, FEntityId(EntityId), SubobjectNameToOffset);
			Channel->SetChannelActor(EntityActor);

			// Apply initial replicated properties.
			// This was moved to after FinishingSpawning because components existing only in blueprints aren't added until spawning is complete
			// Potentially we could split out the initial actor state and the initial component state
			//for (FPendingAddComponentWrapper& PendingAddComponent : PendingAddComponents)
			//{
			//	NetDriver->GetSpatialInterop()->ReceiveAddComponent(Channel, PendingAddComponent.AddComponentOp);
			//}

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
}

UClass* CAPIPipelineBlock::GetNativeEntityClass(MetadataData* MetadataComponent)
{
	FString Metadata = UTF8_TO_TCHAR(MetadataComponent->EntityType.c_str());
	return FindObject<UClass>(ANY_PACKAGE, *Metadata);
}

// Note that in SpatialGDK, this function will not be called on the spawning worker.
// It's only for client, and in the future, other workers.
AActor* CAPIPipelineBlock::SpawnNewEntity(PositionData* PositionComponent, UClass* ActorClass, bool bDeferred)
{
	FVector InitialLocation = CAPIPositionToLocation(PositionComponent->Coords);
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
