// Copyright (c) Improbable Worlds Ltd, All Rights Reserved
#include "SpatialInteropPipelineBlock.h"

#include "SpatialConstants.h"
#include "SpatialActorChannel.h"
#include "SpatialNetDriver.h"
#include "SpatialNetConnection.h"
#include "SpatialPackageMapClient.h"
#include "AddComponentOpWrapperBase.h"
#include "EngineMinimal.h"
#include "EntityRegistry.h"
#include "GameFramework/PlayerController.h"
#include "improbable/view.h"
#include "improbable/worker.h"

#include "SpatialConstants.h"
#include "improbable/unreal/unreal_metadata.h"
#include "UnrealLevelAddComponentOp.h"

DEFINE_LOG_CATEGORY(LogSpatialGDKInteropPipelineBlock);

void USpatialInteropPipelineBlock::Init(UEntityRegistry* Registry, USpatialNetDriver* Driver, UWorld* LoadedWorld)
{
	EntityRegistry = Registry;
	NetDriver = Driver;
	World = LoadedWorld;

	bInCriticalSection = false;
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

	// Remove entities.
	for (auto& PendingRemoveEntity : PendingRemoveEntities)
	{
		RemoveEntityImpl(PendingRemoveEntity);
	}

	// Mark that we've left the critical section.
	bInCriticalSection = false;
	PendingAddEntities.Empty();
	PendingAddComponents.Empty();
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
	// An actor might not be created for a particular entity ID if that entity doesn't have all of the required components.
	AActor* Actor = EntityRegistry->GetActorFromEntityId(ComponentIdentifier.EntityId);
	if (Actor)
	{
		// Initialise the static objects when we check out the level data component.
		auto LevelAddComponentOp = Cast<UUnrealLevelAddComponentOp>(AddComponentOp);
		if (LevelAddComponentOp)
		{
			check(Actor);
			USpatialNetDriver* Driver = Cast<USpatialNetDriver>(Actor->GetWorld()->GetNetDriver());
			UNetConnection* Connection = Driver->GetSpatialOSNetConnection();
			USpatialPackageMapClient* PMC = Cast<USpatialPackageMapClient>(Connection->PackageMap);
			PMC->RegisterStaticObjects(*LevelAddComponentOp->Data.data());
		}
	}
}

void USpatialInteropPipelineBlock::RemoveEntityImpl(const FEntityId& EntityId)
{
	AActor* Actor = EntityRegistry->GetActorFromEntityId(EntityId);
	if (Actor && !Actor->IsPendingKill())
	{
		EntityRegistry->RemoveFromRegistry(Actor);
		Actor->GetWorld()->DestroyActor(Actor, true);
	}
	NetDriver->GetSpatialInterop()->RemoveActorChannel(EntityId.ToSpatialEntityId());
	auto* PackageMap = Cast<USpatialPackageMapClient>(NetDriver->GetSpatialOSNetConnection()->PackageMap);
	PackageMap->RemoveEntityActor(EntityId);
}

void USpatialInteropPipelineBlock::ProcessOps(const TWeakPtr<SpatialOSView>&, const TWeakPtr<SpatialOSConnection>&, UWorld*)
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

		improbable::unreal::UnrealMetadataData* UnrealMetadataComponent = GetComponentDataFromView<improbable::unreal::UnrealMetadata>(LockedView, EntityId);
		check(UnrealMetadataComponent);

		USpatialPackageMapClient* PackageMap = Cast<USpatialPackageMapClient>(NetDriver->GetSpatialOSNetConnection()->PackageMap);
		check(PackageMap);

		FNetworkGUID NetGUID = PackageMap->ResolveEntityActor(EntityActor, EntityId, UnrealMetadataComponent->subobject_name_to_offset());
		UE_LOG(LogSpatialGDKInteropPipelineBlock, Log, TEXT("Received create entity response op for %d"), EntityId.ToSpatialEntityId());	
	}
	else
	{
		if (UClass* ActorClass = GetNativeEntityClass(MetadataComponent))
		{
			// Option 3
			UNetConnection* Connection = nullptr;
			improbable::unreal::UnrealMetadataData* UnrealMetadataComponent = GetComponentDataFromView<improbable::unreal::UnrealMetadata>(LockedView, EntityId);
			check(UnrealMetadataComponent);

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
					EntityActor = SpawnNewEntity(PositionComponent, ActorClass);
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

			// Inform USpatialInterop of this new actor channel.
			NetDriver->GetSpatialInterop()->AddActorChannel(EntityId.ToSpatialEntityId(), Channel);

			// Apply initial replicated properties.
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
					FInBunch Bunch(NetDriver->ServerConnection, &PlayerIndex, sizeof(PlayerIndex));
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
AActor* USpatialInteropPipelineBlock::SpawnNewEntity(improbable::PositionData* PositionComponent, UClass* ActorClass)
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
		FVector SpawnLocation = FRepMovement::RebaseOntoLocalOrigin(InitialLocation, World->OriginLocation);

		NewActor = World->SpawnActorAbsolute(ActorClass, FTransform(FRotator::ZeroRotator, InitialLocation), SpawnInfo);
		check(NewActor);
	}

	return NewActor;
}

// This is for classes that we derive from meta name, mainly to spawn the corresponding actors on clients.
UClass* USpatialInteropPipelineBlock::GetNativeEntityClass(improbable::MetadataData* MetadataComponent)
{
	FString Metadata = UTF8_TO_TCHAR(MetadataComponent->entity_type().c_str());
	return FindObject<UClass>(ANY_PACKAGE, *Metadata);	
}
