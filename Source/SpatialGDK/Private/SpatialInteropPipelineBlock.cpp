// Copyright (c) Improbable Worlds Ltd, All Rights Reserved
#include "SpatialInteropPipelineBlock.h"

#include "AddComponentOpWrapperBase.h"
#include "CallbackDispatcher.h"
#include "EngineMinimal.h"
#include "EntityRegistry.h"
#include "GameFramework/PlayerController.h"
#include "SpatialActorChannel.h"
#include "SpatialConstants.h"
#include "SpatialNetConnection.h"
#include "SpatialNetDriver.h"
#include "SpatialPackageMapClient.h"
#include "improbable/view.h"
#include "improbable/worker.h"

#include "MetadataAddComponentOp.h"
#include "MetadataComponent.h"
#include "PositionAddComponentOp.h"
#include "PositionComponent.h"
#include "SpatialConstants.h"
#include "UnrealMetadataAddComponentOp.h"
#include "UnrealMetadataComponent.h"

// TODO(David): Needed for ApplyNetworkMovementMode hack below.
#include "GameFramework/CharacterMovementComponent.h"

DEFINE_LOG_CATEGORY(LogSpatialGDKInteropPipelineBlock);

void USpatialInteropPipelineBlock::Init(UEntityRegistry* Registry, USpatialNetDriver* Driver, UWorld* LoadedWorld)
{
	EntityRegistry = Registry;
	NetDriver = Driver;
	World = LoadedWorld;

	bInCriticalSection = false;

	// Fill KnownComponents.
	for (TObjectIterator<UClass> It; It; ++It)
	{
		if (It->IsChildOf(USpatialOsComponent::StaticClass()) && !It->HasAnyClassFlags(CLASS_Abstract))
		{
			USpatialOsComponent* CDO = Cast<USpatialOsComponent>((*It)->GetDefaultObject());
			KnownComponents.Emplace(CDO->GetComponentId(), *It);
		}
	}
}

void USpatialInteropPipelineBlock::AddEntity(const worker::AddEntityOp& AddEntityOp)
{
	UE_LOG(LogSpatialGDKInteropPipelineBlock, Verbose, TEXT("USpatialInteropPipelineBlock: worker::AddEntityOp %lld"), AddEntityOp.EntityId);
	check(bInCriticalSection);

	PendingAddEntities.Emplace(AddEntityOp.EntityId);

	if (NextBlock)
	{
		NextBlock->AddEntity(AddEntityOp);
	}
}

void USpatialInteropPipelineBlock::RemoveEntity(const worker::RemoveEntityOp& RemoveEntityOp)
{
	UE_LOG(LogSpatialGDKInteropPipelineBlock, Verbose, TEXT("USpatialInteropPipelineBlock: worker::RemoveEntityOp %lld"), RemoveEntityOp.EntityId);

	if (bInCriticalSection)
	{
		PendingRemoveEntities.Emplace(RemoveEntityOp.EntityId);
	}
	else
	{
		RemoveEntityImpl(RemoveEntityOp.EntityId);
	}

	if (NextBlock)
	{
		NextBlock->RemoveEntity(RemoveEntityOp);
	}
}

void USpatialInteropPipelineBlock::AddComponent(UAddComponentOpWrapperBase* AddComponentOp)
{
	UE_LOG(LogSpatialGDKInteropPipelineBlock, Verbose, TEXT("USpatialInteropPipelineBlock: worker::AddComponentOp component ID: %u entity ID: %lld inCriticalSection: %d"),
		AddComponentOp->ComponentId, AddComponentOp->EntityId, (int)bInCriticalSection);

	if (bInCriticalSection)
	{
		FPendingAddComponentWrapper Wrapper;
		Wrapper.EntityComponent = FComponentIdentifier{AddComponentOp->EntityId, AddComponentOp->ComponentId};
		Wrapper.AddComponentOp = AddComponentOp;
		PendingAddComponents.Emplace(Wrapper);
	}
	else
	{
		InitialiseNewComponentImpl(FComponentIdentifier{AddComponentOp->EntityId, AddComponentOp->ComponentId}, AddComponentOp);
	}

	if (NextBlock)
	{
		NextBlock->AddComponent(AddComponentOp);
	}
}

void USpatialInteropPipelineBlock::RemoveComponent(const worker::ComponentId ComponentId,
	const worker::RemoveComponentOp& RemoveComponentOp)
{
	UE_LOG(LogSpatialGDKInteropPipelineBlock, Verbose, TEXT("USpatialInteropPipelineBlock: worker::RemoveComponentOp component ID: %u entity ID: %lld inCriticalSection: %d"),
		ComponentId, RemoveComponentOp.EntityId, (int)bInCriticalSection);

	if (bInCriticalSection)
	{
		PendingRemoveComponents.Emplace(FComponentIdentifier{RemoveComponentOp.EntityId, ComponentId});
	}
	else
	{
		DisableComponentImpl(FComponentIdentifier{RemoveComponentOp.EntityId, ComponentId});
	}

	if (NextBlock)
	{
		NextBlock->RemoveComponent(ComponentId, RemoveComponentOp);
	}
}

void USpatialInteropPipelineBlock::ChangeAuthority(const worker::ComponentId ComponentId,
	const worker::AuthorityChangeOp& AuthChangeOp)
{
	UE_LOG(LogSpatialGDKInteropPipelineBlock, Verbose, TEXT("USpatialInteropPipelineBlock: worker::ChangeAuthorityOp component ID: %u entity ID: %lld inCriticalSection: %d"),
		ComponentId, AuthChangeOp.EntityId, (int)bInCriticalSection);

	// When a component is initialised, the callback dispatcher will automatically deal with authority changes. Therefore, we need
	// to only queue changes if the entity itself has been queued for addition, which can only happen in a critical section.
	if (bInCriticalSection && PendingAddEntities.Contains(FEntityId(AuthChangeOp.EntityId)))
	{
		PendingAuthorityChanges.Emplace(FComponentIdentifier{AuthChangeOp.EntityId, ComponentId}, AuthChangeOp);
	}

	if (NextBlock)
	{
		NextBlock->ChangeAuthority(ComponentId, AuthChangeOp);
	}
}

void USpatialInteropPipelineBlock::EnterCriticalSection()
{
	UE_LOG(LogSpatialGDKInteropPipelineBlock, Verbose, TEXT("USpatialInteropPipelineBlock: Entering critical section."));
	check(!bInCriticalSection);
	bInCriticalSection = true;

	if (NextBlock)
	{
		NextBlock->EnterCriticalSection();
	}
}

void USpatialInteropPipelineBlock::LeaveCriticalSection()
{
	UE_LOG(LogSpatialGDKInteropPipelineBlock, Verbose, TEXT("USpatialInteropPipelineBlock: Leaving critical section."));
	check(bInCriticalSection);

	TSharedPtr<worker::Connection> LockedConnection = NetDriver->GetSpatialOS()->GetConnection().Pin();
	TSharedPtr<worker::View> LockedView = NetDriver->GetSpatialOS()->GetView().Pin();

	// Add entities.
	for (auto& PendingAddEntity : PendingAddEntities)
	{
		AddEntityImpl(PendingAddEntity);
	}

	// Apply queued add component ops and authority change ops.
	for (auto& PendingAddComponent : PendingAddComponents)
	{
		InitialiseNewComponentImpl(PendingAddComponent.EntityComponent, PendingAddComponent.AddComponentOp);
	}

	// Apply queued remove component ops.
	for (auto& PendingRemoveComponent : PendingRemoveComponents)
	{
		DisableComponentImpl(PendingRemoveComponent);
	}

	// Remove entities.
	for (auto& PendingRemoveEntity : PendingRemoveEntities)
	{
		RemoveEntityImpl(PendingRemoveEntity);
	}

	// Mark that we've left the critical section.
	bInCriticalSection = false;
	PendingAddEntities.Empty();
	PendingAddComponents.Empty();
	PendingAuthorityChanges.Empty();
	PendingRemoveComponents.Empty();
	PendingRemoveEntities.Empty();

	if (NextBlock)
	{
		NextBlock->LeaveCriticalSection();
	}
}

void USpatialInteropPipelineBlock::AddEntityImpl(const FEntityId& EntityId)
{
	TSharedPtr<worker::Connection> LockedConnection = NetDriver->GetSpatialOS()->GetConnection().Pin();
	TSharedPtr<worker::View> LockedView = NetDriver->GetSpatialOS()->GetView().Pin();

	// Create / get actor for this entity.
	GetOrCreateActor(LockedConnection, LockedView, EntityId);
}

void USpatialInteropPipelineBlock::InitialiseNewComponentImpl(const FComponentIdentifier& ComponentIdentifier, UAddComponentOpWrapperBase* AddComponentOp)
{
	TSharedPtr<worker::Connection> LockedConnection = NetDriver->GetSpatialOS()->GetConnection().Pin();
	TSharedPtr<worker::View> LockedView = NetDriver->GetSpatialOS()->GetView().Pin();

	UClass* ComponentClass = KnownComponents.FindRef(FComponentId{ComponentIdentifier.ComponentId});

	if (!ComponentClass)
	{
		// The interop system does not register USpatialOSComponents. The code below is for the UnrealSDK flow.
		return;
	}

	// An actor might not be created for a particular entity ID if that entity doesn't have all of the required components.
	AActor* Actor = EntityRegistry->GetActorFromEntityId(ComponentIdentifier.EntityId);
	if (Actor)
	{
		USpatialOsComponent* Component = Cast<USpatialOsComponent>(Actor->GetComponentByClass(ComponentClass));
		if (Component)
		{
			Component->Init(LockedConnection, LockedView, ComponentIdentifier.EntityId, NetDriver->GetSpatialOS()->GetCallbackDispatcher());
			Component->ApplyInitialState(*AddComponentOp);
			worker::AuthorityChangeOp* QueuedAuthChangeOp = PendingAuthorityChanges.Find(ComponentIdentifier);
			if (QueuedAuthChangeOp)
			{
				Component->ApplyInitialAuthority(*QueuedAuthChangeOp);
			}
		}
	}
}

void USpatialInteropPipelineBlock::DisableComponentImpl(const FComponentIdentifier& ComponentIdentifier)
{
	UClass* ComponentClass = KnownComponents.FindRef(FComponentId{ComponentIdentifier.ComponentId});

	if (!ComponentClass)
	{
		// The interop system does not register USpatialOSComponents. The code below is for the UnrealSDK flow.
		return;
	}

	AActor* Actor = EntityRegistry->GetActorFromEntityId(ComponentIdentifier.EntityId);
	if (Actor)
	{
		USpatialOsComponent* Component = Cast<USpatialOsComponent>(Actor->GetComponentByClass(ComponentClass));
		if (Component)
		{
			Component->Disable(ComponentIdentifier.EntityId, NetDriver->GetSpatialOS()->GetCallbackDispatcher());
		}
	}
}

void USpatialInteropPipelineBlock::RemoveEntityImpl(const FEntityId& EntityId)
{
	AActor* Actor = EntityRegistry->GetActorFromEntityId(EntityId);

	// Actor already deleted (this worker was most likely authoritative over it and deleted it earlier).
	if (!Actor || Actor->IsPendingKill())
	{
		return;
	}

	World->DestroyActor(Actor, true);

	CleanupDeletedEntity(EntityId);
}

void USpatialInteropPipelineBlock::CleanupDeletedEntity(const FEntityId& EntityId)
{
	EntityRegistry->RemoveFromRegistry(EntityId);
	NetDriver->GetSpatialInterop()->RemoveActorChannel(EntityId.ToSpatialEntityId());
	auto* PackageMap = Cast<USpatialPackageMapClient>(NetDriver->GetSpatialOSNetConnection()->PackageMap);
	PackageMap->RemoveEntityActor(EntityId);
}

void USpatialInteropPipelineBlock::ProcessOps(const TWeakPtr<SpatialOSView>&, const TWeakPtr<SpatialOSConnection>&, UWorld*, UCallbackDispatcher*)
{
}

AActor* USpatialInteropPipelineBlock::GetOrCreateActor(TSharedPtr<worker::Connection> LockedConnection, TSharedPtr<worker::View> LockedView, const FEntityId& EntityId)
{
	checkf(World, TEXT("We should have a world whilst processing ops."));

	improbable::PositionData* PositionComponent = GetComponentDataFromView<improbable::Position>(LockedView, EntityId);
	improbable::MetadataData* MetadataComponent = GetComponentDataFromView<improbable::Metadata>(LockedView, EntityId);

	if (!PositionComponent || !MetadataComponent)
	{
		return nullptr;
	}

	AActor* EntityActor = EntityRegistry->GetActorFromEntityId(EntityId);
	UE_LOG(LogSpatialGDKInteropPipelineBlock, Log, TEXT("Checked out entity with entity ID %lld"), EntityId.ToSpatialEntityId());

	// There are 3 main options when we get here with regards to how this entity was created:
	// 1) A SpawnActor() call (through interop) on this worker, which means it already has an actor associated with it.
	//	  This usually happens on the Unreal server only (as servers are the only workers which can spawn actors).
	// 2) A "pure" Spatial create entity request, which means we need to spawn an actor that was manually registered to correspond to it.
	// 3) A SpawnActor() call that was initiated from a different worker, which means we need to find and spawn the corresponding "native" actor that corresponds to it.
	//	  This can happen on either the client (for all actors) or server (for actors which were spawned by a different server worker, or are migrated).

	if (EntityActor)
	{
		// Option 1
		UE_LOG(LogSpatialGDKInteropPipelineBlock, Log, TEXT("Entity for core actor %s has been checked out on the worker which spawned it."), *EntityActor->GetName());
		SetupComponentInterests(EntityActor, EntityId, LockedConnection);

		improbable::unreal::UnrealMetadataData* UnrealMetadataComponent = GetComponentDataFromView<improbable::unreal::UnrealMetadata>(LockedView, EntityId);
		check(UnrealMetadataComponent);

		USpatialPackageMapClient* PackageMap = Cast<USpatialPackageMapClient>(NetDriver->GetSpatialOSNetConnection()->PackageMap);
		check(PackageMap);

		FNetworkGUID NetGUID = PackageMap->ResolveEntityActor(EntityActor, EntityId, UnrealMetadataComponent->subobject_name_to_offset());
		UE_LOG(LogSpatialGDKInteropPipelineBlock, Log, TEXT("Received create entity response op for %d"), EntityId.ToSpatialEntityId());
		
		// actor channel/entity mapping should be registered by this point
		check(NetDriver->GetSpatialInterop()->GetActorChannelByEntityId(EntityId.ToSpatialEntityId()));
	}
	else
	{
		UClass* ActorClass = nullptr;
		if ((ActorClass = GetRegisteredEntityClass(MetadataComponent)) != nullptr)
		{
			// Option 2
			UE_LOG(LogSpatialGDKInteropPipelineBlock, Log, TEXT("Spawning a registered %s"), *ActorClass->GetName());
			EntityActor = SpawnNewEntity(PositionComponent, ActorClass, false);
			EntityRegistry->AddToRegistry(EntityId, EntityActor);
		}
		else if ((ActorClass = GetNativeEntityClass(MetadataComponent)) != nullptr)
		{
			// Option 3
			UNetConnection* Connection = nullptr;
			improbable::unreal::UnrealMetadataData* UnrealMetadataComponent = GetComponentDataFromView<improbable::unreal::UnrealMetadata>(LockedView, EntityId);
			check(UnrealMetadataComponent);
			bool bDoingDeferredSpawn = false;

			// If we're checking out a player controller, spawn it via "USpatialNetDriver::AcceptNewPlayer"
			if (NetDriver->IsServer() && ActorClass == APlayerController::StaticClass())
			{
				checkf(!UnrealMetadataComponent->owner_worker_id().empty(), TEXT("A player controller entity must have an owner worker ID."));
				FString URLString = FURL().ToString();
				FString OwnerWorkerId = UTF8_TO_TCHAR(UnrealMetadataComponent->owner_worker_id().data()->c_str());
				URLString += TEXT("?workerId=") + OwnerWorkerId;
				Connection = NetDriver->AcceptNewPlayer(FURL(nullptr, *URLString, TRAVEL_Absolute), true);
				check(Connection);
				EntityActor = Connection->PlayerController;
			}
			else
			{
				// Either spawn the actor or get it from the level if it has a persistent name.
				if (UnrealMetadataComponent->static_path().empty())
				{
					UE_LOG(LogSpatialGDKInteropPipelineBlock, Log, TEXT("Spawning a native dynamic %s whilst checking out an entity."), *ActorClass->GetFullName());
					EntityActor = SpawnNewEntity(PositionComponent, ActorClass, true);
					bDoingDeferredSpawn = true;
				}
				else
				{
					FString FullPath = UTF8_TO_TCHAR(UnrealMetadataComponent->static_path().data()->c_str());
					UE_LOG(LogSpatialGDKInteropPipelineBlock, Log, TEXT("Searching for a native static actor %s of class %s in the persistent level whilst checking out an entity."), *FullPath, *ActorClass->GetName());
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
			auto PackageMap = Cast<USpatialPackageMapClient>(Connection->PackageMap);
			auto Channel = Cast<USpatialActorChannel>(Connection->CreateChannel(CHTYPE_Actor, NetDriver->IsServer()));
			check(Channel);

			PackageMap->ResolveEntityActor(EntityActor, EntityId, UnrealMetadataComponent->subobject_name_to_offset());
			Channel->SetChannelActor(EntityActor);

			// Update interest on the entity's components after receiving initial component data (so Role and RemoteRole are properly set).
			NetDriver->GetSpatialInterop()->SendComponentInterests(Channel, EntityId.ToSpatialEntityId());

			if (bDoingDeferredSpawn)
			{
				auto InitialLocation = SpatialConstants::SpatialOSCoordinatesToLocation(PositionComponent->coords());
				FVector SpawnLocation = FRepMovement::RebaseOntoLocalOrigin(InitialLocation, World->OriginLocation);
				EntityActor->FinishSpawning(FTransform(FRotator::ZeroRotator, SpawnLocation));
			}

			// Apply initial replicated properties.
			// This was moved to after FinishingSpawning because components existing only in blueprints aren't added until spawning is complete
			// Potentially we could split out the initial actor state and the initial component state
			for (FPendingAddComponentWrapper& PendingAddComponent : PendingAddComponents)
			{
				NetDriver->GetSpatialInterop()->ReceiveAddComponent(Channel, PendingAddComponent.AddComponentOp);
			}

			// This is a bit of a hack unfortunately, among the core classes only PlayerController implements this function and it requires
			// a player index. For now we don't support split screen, so the number is always 0.
			if (NetDriver->ServerConnection)
			{
				if (EntityActor->IsA(APlayerController::StaticClass()))
				{
					uint8 PlayerIndex = 0;
					// FInBunch takes size in bits not bytes
					FInBunch Bunch(NetDriver->ServerConnection, &PlayerIndex, sizeof(PlayerIndex)*8);
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
	return EntityActor;
}

// Note that in SpatialGDK, this function will not be called on the spawning worker.
// It's only for client, and in the future, other workers.
AActor* USpatialInteropPipelineBlock::SpawnNewEntity(improbable::PositionData* PositionComponent, UClass* ActorClass, bool bDeferred)
{
	FVector InitialLocation = SpatialConstants::SpatialOSCoordinatesToLocation(PositionComponent->coords());
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

// This is for classes that we register explicitly with Unreal, currently used for "non-native" replication. This logic might change soon.
UClass* USpatialInteropPipelineBlock::GetRegisteredEntityClass(improbable::MetadataData* MetadataComponent)
{
	FString EntityTypeString = UTF8_TO_TCHAR(MetadataComponent->entity_type().c_str());

	// Initially, attempt to find the class that has been registered to the EntityType string
	UClass** RegisteredClass = EntityRegistry->GetRegisteredEntityClass(EntityTypeString);

	// Try without the full path. This should not be necessary, but for now the pawn class needs it.
	if (!RegisteredClass)
	{
		int32 LastSlash = -1;
		if (EntityTypeString.FindLastChar('/', LastSlash))
		{
			RegisteredClass = EntityRegistry->GetRegisteredEntityClass(EntityTypeString.RightChop(LastSlash));
		}
	}
	UClass* ClassToSpawn = RegisteredClass ? *RegisteredClass : nullptr;

	return ClassToSpawn;
}

// This is for classes that we derive from meta name, mainly to spawn the corresponding actors on clients.
UClass* USpatialInteropPipelineBlock::GetNativeEntityClass(improbable::MetadataData* MetadataComponent)
{
	FString Metadata = UTF8_TO_TCHAR(MetadataComponent->entity_type().c_str());
	return FindObject<UClass>(ANY_PACKAGE, *Metadata);	
}

void USpatialInteropPipelineBlock::SetupComponentInterests(AActor* Actor, const FEntityId& EntityId, const TWeakPtr<worker::Connection>& Connection)
{
	TArray<UActorComponent*> SpatialOSComponents = Actor->GetComponentsByClass(USpatialOsComponent::StaticClass());

	worker::Map<worker::ComponentId, worker::InterestOverride> ComponentIdsAndInterestOverrides;

	for (auto Component : SpatialOSComponents)
	{
		USpatialOsComponent* SpatialOsComponent = Cast<USpatialOsComponent>(Component);
		ComponentIdsAndInterestOverrides.emplace(std::make_pair(
			SpatialOsComponent->GetComponentId().ToSpatialComponentId(),
			worker::InterestOverride{/* IsInterested */ true }));
	}

	auto LockedConnection = Connection.Pin();
	if (LockedConnection.IsValid())
	{
		LockedConnection->SendComponentInterest(EntityId.ToSpatialEntityId(), ComponentIdsAndInterestOverrides);
	}
}

