// Copyright (c) Improbable Worlds Ltd, All Rights Reserved
#include "SpatialInteropPipelineBlock.h"

#include "SpatialConstants.h"
#include "SpatialActorChannel.h"
#include "SpatialNetDriver.h"
#include "SpatialNetConnection.h"
#include "SpatialPackageMapClient.h"
#include "AddComponentOpWrapperBase.h"
#include "CallbackDispatcher.h"
#include "EngineMinimal.h"
#include "EntityRegistry.h"
#include "GameFramework/PlayerController.h"
#include "improbable/view.h"
#include "improbable/worker.h"

#include "PositionAddComponentOp.h"
#include "PositionComponent.h"
#include "MetadataAddComponentOp.h"
#include "MetadataComponent.h"
#include "UnrealMetadataAddComponentOp.h"
#include "UnrealMetadataComponent.h"
#include "UnrealLevelComponent.h"

DEFINE_LOG_CATEGORY(LogSpatialOSInteropPipelineBlock);

void USpatialInteropPipelineBlock::Init(UEntityRegistry* Registry, USpatialNetDriver* Driver)
{
	EntityRegistry = Registry;
	NetDriver = Driver;

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
	UE_LOG(LogSpatialOSInteropPipelineBlock, Verbose, TEXT("USpatialInteropPipelineBlock: worker::AddEntityOp %llu"), AddEntityOp.EntityId);
	check(bInCriticalSection);

	PendingAddEntities.Emplace(AddEntityOp.EntityId);

	if (NextBlock)
	{
		NextBlock->AddEntity(AddEntityOp);
	}
}

void USpatialInteropPipelineBlock::RemoveEntity(const worker::RemoveEntityOp& RemoveEntityOp)
{
	UE_LOG(LogSpatialOSInteropPipelineBlock, Verbose, TEXT("USpatialInteropPipelineBlock: worker::RemoveEntityOp %llu"), RemoveEntityOp.EntityId);

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
	UE_LOG(LogSpatialOSInteropPipelineBlock, Verbose, TEXT("USpatialInteropPipelineBlock: worker::AddComponentOp component ID: %u entity ID: %llu inCriticalSection: %d"),
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
	UE_LOG(LogSpatialOSInteropPipelineBlock, Verbose, TEXT("USpatialInteropPipelineBlock: worker::RemoveComponentOp component ID: %u entity ID: %llu inCriticalSection: %d"),
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
	UE_LOG(LogSpatialOSInteropPipelineBlock, Verbose, TEXT("USpatialInteropPipelineBlock: worker::ChangeAuthorityOp component ID: %u entity ID: %llu inCriticalSection: %d"),
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
	UE_LOG(LogSpatialOSInteropPipelineBlock, Verbose, TEXT("USpatialInteropPipelineBlock: Entering critical section."));
	check(!bInCriticalSection);
	bInCriticalSection = true;

	if (NextBlock)
	{
		NextBlock->EnterCriticalSection();
	}
}

void USpatialInteropPipelineBlock::LeaveCriticalSection()
{
	UE_LOG(LogSpatialOSInteropPipelineBlock, Verbose, TEXT("USpatialInteropPipelineBlock: Leaving critical section. Creating entity."));
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
	check(ComponentClass);

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

void USpatialInteropPipelineBlock::DisableComponentImpl(const FComponentIdentifier& ComponentIdentifier)
{
	UClass* ComponentClass = KnownComponents.FindRef(FComponentId{ComponentIdentifier.ComponentId});
	check(ComponentClass);

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
	if (Actor && !Actor->IsPendingKill())
	{
		EntityRegistry->RemoveFromRegistry(Actor);
		Actor->GetWorld()->DestroyActor(Actor);
	}
}


void USpatialInteropPipelineBlock::ProcessOps(const TWeakPtr<SpatialOSView>& InView,
	const TWeakPtr<SpatialOSConnection>& InConnection, UWorld* World,
	UCallbackDispatcher* CallbackDispatcher)
{
}

AActor* USpatialInteropPipelineBlock::GetOrCreateActor(TSharedPtr<worker::Connection> LockedConnection, TSharedPtr<worker::View> LockedView, const FEntityId& EntityId)
{
	UWorld* World = GEngine->GetWorldFromContextObject(NetDriver, EGetWorldErrorMode::LogAndReturnNull);
	checkf(World, TEXT("We should have a world whilst processing ops."));

	improbable::PositionData* PositionComponent = GetComponentDataFromView<improbable::Position>(LockedView, EntityId);
	improbable::MetadataData* MetadataComponent = GetComponentDataFromView<improbable::Metadata>(LockedView, EntityId);

	if (!PositionComponent || !MetadataComponent)
	{
		return nullptr;
	}

	AActor* EntityActor = EntityRegistry->GetActorFromEntityId(EntityId);
	UE_LOG(LogSpatialOSInteropPipelineBlock, Log, TEXT("Checked out entity with entity ID %llu"), EntityId.ToSpatialEntityId());

	// There are 3 main options when we get here with regards to how this entity was created:
	// 1) A SpawnActor() call (through interop) on this worker, which means it already has an actor associated with it.
	//	  This usually happens on the Unreal server only (as servers are the only workers which can spawn actors).
	// 2) A "pure" Spatial create entity request, which means we need to spawn an actor that was manually registered to correspond to it.
	// 3) A SpawnActor() call that was initiated from a different worker, which means we need to find and spawn the corresponding "native" actor that corresponds to it.
	//	  This can happen on either the client (for all actors) or server (for actors which were spawned by a different server worker, or are migrated).

	if (EntityActor)
	{
		// Option 1
		UE_LOG(LogSpatialOSInteropPipelineBlock, Log, TEXT("Entity for core actor %s has been checked out on the worker which spawned it."), *EntityActor->GetName());
		SetupComponentInterests(EntityActor, EntityId, LockedConnection);
	}
	else
	{
		UClass* ActorClass = GetRegisteredEntityClass(MetadataComponent);
		if (ActorClass)
		{
			// Option 2
			UE_LOG(LogSpatialOSInteropPipelineBlock, Log, TEXT("Spawning a registered %s"), *ActorClass->GetName());
			EntityActor = SpawnNewEntity(PositionComponent, World, ActorClass);
			EntityRegistry->AddToRegistry(EntityId, EntityActor);
		}
		else
		{
			// Option 3
			UNetConnection* Connection = nullptr;
			ActorClass = GetNativeEntityClass(MetadataComponent);
			improbable::unreal::UnrealMetadataData* UnrealMetadataComponent = GetComponentDataFromView<improbable::unreal::UnrealMetadata>(LockedView, EntityId);
			std::string str;
			str.empty();

			// If we're checking out a player controller, spawn it via "USpatialNetDriver::AcceptNewPlayer"
			if (NetDriver->IsServer() && ActorClass == APlayerController::StaticClass())
			{
				checkf(!UnrealMetadataComponent->owner_worker_id().empty(), TEXT("A player controller entity must have an owner worker ID."));
				FString URLString = FURL().ToString();
				FString OwnerWorkerId = UTF8_TO_TCHAR(UnrealMetadataComponent->owner_worker_id().data()->c_str());
				URLString += TEXT("?workerId=") + OwnerWorkerId;
				Connection = NetDriver->AcceptNewPlayer(FURL(nullptr, *URLString, TRAVEL_Absolute));
				check(Connection);
				EntityActor = Connection->PlayerController;
			}
			else
			{
				// Either spawn the actor or get it from the level if it has a persistent name.
				if (UnrealMetadataComponent->static_path().empty())
				{
					UE_LOG(LogSpatialOSInteropPipelineBlock, Log, TEXT("Spawning a native dynamic %s whilst checking out an entity."), *ActorClass->GetFullName());
					EntityActor = SpawnNewEntity(PositionComponent, World, ActorClass);
				}
				else
				{
					FString FullPath = UTF8_TO_TCHAR(UnrealMetadataComponent->static_path().data()->c_str());
					UE_LOG(LogSpatialOSInteropPipelineBlock, Log, TEXT("Searching for a native static actor %s of class %s in the persistent level whilst checking out an entity."), *FullPath, *ActorClass->GetName());
					EntityActor = FindObject<AActor>(World, *FullPath);
				}
				check(EntityActor);

				// Get the net connection for this actor.
				if (NetDriver->IsServer())
				{
					//todo-giray: When we have multiple servers, this won't work. On which connection would we create the channel?
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
			auto Channel = Cast<USpatialActorChannel>(Connection->CreateChannel(CHTYPE_Actor, false));
			check(Channel);

			PackageMap->ResolveEntityActor(EntityActor, EntityId);
			Channel->SetChannelActor(EntityActor);

			// Inform USpatialInterop of this new actor channel.
			NetDriver->GetSpatialInterop()->AddActorChannel(EntityId.ToSpatialEntityId(), Channel);

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

// Note that in NUF, this function will not be called on the spawning worker.
// It's only for client, and in the future, other workers.
AActor* USpatialInteropPipelineBlock::SpawnNewEntity(improbable::PositionData* PositionComponent, UWorld* World, UClass* ActorClass)
{
	FVector InitialLocation = SpatialConstants::SpatialOSCoordinatesToLocation(PositionComponent->coords());
	AActor* NewActor = nullptr;
	if (ActorClass)
	{
		//bRemoteOwned needs to be public in source code. This might be a controversial change.
		FActorSpawnParameters SpawnInfo;
		SpawnInfo.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		SpawnInfo.bRemoteOwned = true;
		SpawnInfo.bNoFail = true;
		FVector SpawnLocation = FRepMovement::RebaseOntoLocalOrigin(InitialLocation, World->OriginLocation);
		NewActor = World->SpawnActorAbsolute(ActorClass, FTransform(FRotator::ZeroRotator, InitialLocation), SpawnInfo);

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

