// Copyright (c) Improbable Worlds Ltd, All Rights Reserved
#include "SpatialInteropBlock.h"

#include "SpatialActorChannel.h"
#include "SpatialNetDriver.h"
#include "SpatialNetConnection.h"
#include "SpatialPackageMapClient.h"
#include "AddComponentOpWrapperBase.h"
#include "CallbackDispatcher.h"
#include "EngineMinimal.h"
#include "EntityRegistry.h"
#include "MetadataAddComponentOp.h"
#include "MetadataComponent.h"
#include "GameFramework/PlayerController.h"
#include "PositionAddComponentOp.h"
#include "PositionComponent.h"
#include "SpatialOSConversionFunctionLibrary.h"
#include "improbable/view.h"
#include "improbable/worker.h"

void USpatialInteropBlock::Init(UEntityRegistry* Registry)
{
	EntityRegistry = Registry;
}

void USpatialInteropBlock::AddEntity(const worker::AddEntityOp& AddEntityOp)
{
	// Add this to the list of entities waiting to be spawned
	EntitiesToSpawn.AddUnique(AddEntityOp.EntityId);
	if (NextBlock)
	{
		NextBlock->AddEntity(AddEntityOp);
	}
}

void USpatialInteropBlock::RemoveEntity(const worker::RemoveEntityOp& RemoveEntityOp)
{
	// Add this to the list of entities waiting to be deleted
	EntitiesToRemove.AddUnique(RemoveEntityOp.EntityId);
	if (NextBlock)
	{
		NextBlock->RemoveEntity(RemoveEntityOp);
	}
}

void USpatialInteropBlock::AddComponent(UAddComponentOpWrapperBase* AddComponentOp)
{
	// Store this op to be used later on when setting the initial state of the component
	ComponentsToAdd.Emplace(FComponentIdentifier{ AddComponentOp->EntityId, AddComponentOp->ComponentId }, AddComponentOp);
	if (NextBlock)
	{
		NextBlock->AddComponent(AddComponentOp);
	}
}

void USpatialInteropBlock::RemoveComponent(const worker::ComponentId ComponentId,
	const worker::RemoveComponentOp& RemoveComponentOp)
{
	// Add this to the list of components waiting to be disabled
	ComponentsToRemove.Emplace(FComponentIdentifier{ RemoveComponentOp.EntityId, ComponentId });
	if (NextBlock)
	{
		NextBlock->RemoveComponent(ComponentId, RemoveComponentOp);
	}
}

void USpatialInteropBlock::ChangeAuthority(const worker::ComponentId ComponentId,
	const worker::AuthorityChangeOp& AuthChangeOp)
{
	// Set the latest authority value for this Component on the owning entity
	ComponentAuthorities.Emplace(FComponentIdentifier{ AuthChangeOp.EntityId, ComponentId },
		AuthChangeOp);
	if (NextBlock)
	{
		NextBlock->ChangeAuthority(ComponentId, AuthChangeOp);	
	}
}

void USpatialInteropBlock::AddEntities(UWorld* World,
	const TWeakPtr<worker::Connection>& InConnection)
{
	if (World == nullptr)
	{
		UE_LOG(LogSpatialOSNUF, Error, TEXT("Not adding entities because the world is NULL"));
		return;
	}

	TArray<FEntityId> SpawnedEntities;

	for (auto& EntityToSpawn : EntitiesToSpawn)
	{
		UAddComponentOpWrapperBase** PositionBaseComponent = ComponentsToAdd.Find(
			FComponentIdentifier{ EntityToSpawn.ToSpatialEntityId(), UPositionComponent::ComponentId });
		UAddComponentOpWrapperBase** MetadataBaseComponent = ComponentsToAdd.Find(
			FComponentIdentifier{ EntityToSpawn.ToSpatialEntityId(), UMetadataComponent::ComponentId });

		// Only spawn entities for which we have received Position and Metadata components
		if ((PositionBaseComponent && (*PositionBaseComponent)->IsValidLowLevel()) &&
			(MetadataBaseComponent && (*MetadataBaseComponent)->IsValidLowLevel()))
		{
			// Retrieve the EntityType string from the Metadata component
			UMetadataAddComponentOp* MetadataAddComponentOp =
				Cast<UMetadataAddComponentOp>(*MetadataBaseComponent);
			UPositionAddComponentOp* PositionAddComponentOp =
				Cast<UPositionAddComponentOp>(*PositionBaseComponent);

			USpatialNetDriver* Driver = Cast<USpatialNetDriver>(World->GetNetDriver());
			AActor* EntityActor = EntityRegistry->GetActorFromEntityId(EntityToSpawn);
			USpatialPackageMapClient* PMC = nullptr;

			UE_LOG(LogSpatialOSNUF, Warning, TEXT("Received add entity op for %d"), EntityToSpawn.ToSpatialEntityId());

			// There are 3 main options when we get here with regards to how this entity was created:
			// 1) A SpawnActor() call (through interop) on this worker, which means it already has an actor associated with it.
			// 2) A "pure" Spatial create entity request, which means we need to spawn an actor that was manually registered to correspond to it.
			// 3) A SpawnActor() call that was initiated from a different worker, which means we need to find and spawn the corresponding "native" actor that corresponds to it. 

			if (EntityActor)
			{
				// Option 1
				UE_LOG(LogSpatialOSNUF, Log, TEXT("Entity for core actor %s has been checked out on the originating worker."), *EntityActor->GetName());
				SetupComponentInterests(EntityActor, EntityToSpawn, InConnection);
			}
			else
			{
				UClass* ClassToSpawn = GetRegisteredEntityClass(MetadataAddComponentOp);

				if (ClassToSpawn)
				{
					// Option 2
					UE_LOG(LogSpatialOSNUF, Log, TEXT("Attempting to spawn a registered %s"), *ClassToSpawn->GetName());
					EntityActor = SpawnNewEntity(PositionAddComponentOp, World, ClassToSpawn);
					EntityRegistry->AddToRegistry(EntityToSpawn, EntityActor);
				}
				else
				{
					// Option 3
					ClassToSpawn = GetNativeEntityClass(MetadataAddComponentOp);
					UE_LOG(LogSpatialOSNUF, Log, TEXT("Attempting to spawn a native %s"), *ClassToSpawn->GetName());
					EntityActor = SpawnNewEntity(PositionAddComponentOp, World, ClassToSpawn);
					check(EntityActor);
					EntityRegistry->AddToRegistry(EntityToSpawn, EntityActor);

					//todo-giray: When we have multiple servers, this won't work. On which connection would we create the channel?
					USpatialActorChannel* Ch = nullptr;
					UNetConnection* Connection = Driver->GetSpatialOSNetConnection();
					if (Connection == nullptr)
					{
						check(Driver->GetNetMode() == NM_Client);
						Connection = Driver->ServerConnection;
					}

					PMC = Cast<USpatialPackageMapClient>(Connection->PackageMap);
					Ch = Cast<USpatialActorChannel>(Connection->CreateChannel(CHTYPE_Actor, false));
					
					check(Ch);
					Driver->GetSpatialUpdateInterop()->AddClientActorChannel(EntityToSpawn.ToSpatialEntityId(), Ch);
					
					PMC->ResolveEntityActor(EntityActor, EntityToSpawn);
					Ch->SetChannelActor(EntityActor);
					Driver->GetSpatialUpdateInterop()->SetComponentInterests(Ch, EntityToSpawn.ToSpatialEntityId());

					//This is a bit of a hack unfortunately, among the core classes only PlayerController implements this function and it requires
					// a player index. For now we don't support split screen, so the number is always 0.
					if (Driver->ServerConnection)
					{
						if (EntityActor->IsA(APlayerController::StaticClass()))
						{
							uint8 PlayerIndex = 0;
							FInBunch Bunch(Driver->ServerConnection, &PlayerIndex, sizeof(PlayerIndex));
							EntityActor->OnActorChannelOpen(Bunch, Driver->ServerConnection);
						}
						else
						{
							FInBunch Bunch(Driver->ServerConnection);
							EntityActor->OnActorChannelOpen(Bunch, Driver->ServerConnection);
						}
					}
				}
				EntityActor->PostNetInit();				
			}
			SpawnedEntities.Add(EntityToSpawn);
		}
	}

	for (auto& SpawnedEntity : SpawnedEntities)
	{
		EntitiesToSpawn.Remove(SpawnedEntity);
	}
}

void USpatialInteropBlock::AddComponents(const TWeakPtr<worker::View>& InView,
	const TWeakPtr<worker::Connection>& InConnection,
	UCallbackDispatcher* InCallbackDispatcher)
{
	TArray<FComponentIdentifier> InitialisedComponents;

	// Go through the add component ops that have been queued
	// If the entity the component belongs to is already spawned, create component & apply initial
	// state.
	// If not, try again later
	for (auto& ComponentToAdd : ComponentsToAdd)
	{
		AActor* Actor = EntityRegistry->GetActorFromEntityId(ComponentToAdd.Key.EntityId);
		if (Actor)
		{
			auto ComponentClass = KnownComponents.Find(ComponentToAdd.Key.ComponentId);
			if (ComponentClass)
			{
				USpatialOsComponent* Component =
					Cast<USpatialOsComponent>(Actor->GetComponentByClass(*ComponentClass));
				if (Component)
				{
					Component->Init(InConnection, InView, ComponentToAdd.Key.EntityId, InCallbackDispatcher);
					Component->ApplyInitialState(*ComponentToAdd.Value);

					auto QueuedAuthChangeOp = ComponentAuthorities.Find(ComponentToAdd.Key);
					if (QueuedAuthChangeOp)
					{
						Component->ApplyInitialAuthority(*QueuedAuthChangeOp);
					}

					InitialisedComponents.Add(ComponentToAdd.Key);
				}
			}
		}
	}

	for (auto& Component : InitialisedComponents)
	{
		ComponentsToAdd.Remove(Component);
	}
}

void USpatialInteropBlock::RemoveComponents(UCallbackDispatcher* InCallbackDispatcher)
{
	for (auto& ComponentToRemove : ComponentsToRemove)
	{
		const worker::EntityId EntityId = ComponentToRemove.EntityId;
		const worker::ComponentId ComponentId = ComponentToRemove.ComponentId;

		AActor* Actor = EntityRegistry->GetActorFromEntityId(EntityId);
		auto ComponentClass = KnownComponents.Find(ComponentToRemove.ComponentId);

		if (!Actor || !ComponentClass)
		{
			continue;
		}

		USpatialOsComponent* Component =
			Cast<USpatialOsComponent>(Actor->GetComponentByClass(*ComponentClass));

		if (Component)
		{
			Component->Disable(EntityId, InCallbackDispatcher);
		}
	}

	ComponentsToRemove.Empty();
}

void USpatialInteropBlock::RemoveEntities(UWorld* World)
{
	if (World == nullptr)
	{
		return;
	}

	for (auto& EntityToRemove : EntitiesToRemove)
	{
		auto ActorToRemove = EntityRegistry->GetActorFromEntityId(EntityToRemove);

		if (ActorToRemove && !ActorToRemove->IsPendingKill() && World)
		{
			EntityRegistry->RemoveFromRegistry(ActorToRemove);
			World->DestroyActor(ActorToRemove);
		}
	}

	EntitiesToRemove.Empty();
}

void USpatialInteropBlock::ProcessOps(const TWeakPtr<worker::View>& InView,
	const TWeakPtr<worker::Connection>& InConnection,
	UWorld* World, UCallbackDispatcher* InCallbackDispatcher)
{
	//todo-giray: This approach is known to cause reordering of ops which can lead to a memory leak.
	// A fix will be included in a future version of UnrealSDK, which we should integrate.
	AddEntities(World, InConnection);
	AddComponents(InView, InConnection, InCallbackDispatcher);
	RemoveComponents(InCallbackDispatcher);
	RemoveEntities(World);
}

// Note that in NUF, this function will not be called on the spawning worker.
// It's only for client, and in the future, other workers.
AActor* USpatialInteropBlock::SpawnNewEntity(
	UPositionAddComponentOp* PositionComponent,
	UWorld* World,
	UClass* ClassToSpawn)
{
	if (World == nullptr)
	{
		UE_LOG(LogSpatialOSNUF, Warning, TEXT("SpawnNewEntity() failed: invalid World context"));

		return nullptr;
	}
	auto Coords = PositionComponent->Data->coords();
	FVector InitialLocation =
		USpatialOSConversionFunctionLibrary::SpatialOsCoordinatesToUnrealCoordinates(
			FVector(Coords.x(), Coords.y(), Coords.z()));

	AActor* NewActor = nullptr;
	if (ClassToSpawn)
	{
		//NUF-sourcechange: bRemoteOwned needs to be public. This might be a controversial change.
		FActorSpawnParameters SpawnInfo;
		SpawnInfo.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		SpawnInfo.bRemoteOwned = true;
		SpawnInfo.bNoFail = true;
		FVector SpawnLocation = FRepMovement::RebaseOntoLocalOrigin(InitialLocation, World->OriginLocation);
		NewActor = World->SpawnActorAbsolute(ClassToSpawn, FTransform(FRotator::ZeroRotator, InitialLocation), SpawnInfo);
			
		check(NewActor);

		TArray<UActorComponent*> SpatialOSComponents =
			NewActor->GetComponentsByClass(USpatialOsComponent::StaticClass());

		for (auto Component : SpatialOSComponents)
		{
			USpatialOsComponent* SpatialOSComponent = Cast<USpatialOsComponent>(Component);
			KnownComponents.Emplace(SpatialOSComponent->GetComponentId().ToSpatialComponentId(),
				Component->GetClass());
		}
	}
		
	return NewActor;
}

//This is for classes that we register explicitly with Unreal, currently used for "non-native" replication. This logic might change soon.
UClass* USpatialInteropBlock::GetRegisteredEntityClass(UMetadataAddComponentOp* MetadataComponent)
{
	FString EntityTypeString = UTF8_TO_TCHAR(MetadataComponent->Data->entity_type().c_str());

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

//This is for classes that we derive from meta name, mainly to spawn the corresponding actors on clients.
UClass* USpatialInteropBlock::GetNativeEntityClass(UMetadataAddComponentOp* MetadataComponent)
{
	FString EntityTypeString = UTF8_TO_TCHAR(MetadataComponent->Data->entity_type().c_str());

	// This is all horrendous, but assume it's a full CDO path if not registered	
	return FindObject<UClass>(ANY_PACKAGE, *EntityTypeString);	
}

void USpatialInteropBlock::SetupComponentInterests(
	AActor* Actor, const FEntityId& EntityId, const TWeakPtr<worker::Connection>& Connection)
{
	TArray<UActorComponent*> SpatialOSComponents =
		Actor->GetComponentsByClass(USpatialOsComponent::StaticClass());

	worker::Map<worker::ComponentId, worker::InterestOverride> ComponentIdsAndInterestOverrides;

	for (auto Component : SpatialOSComponents)
	{
		USpatialOsComponent* SpatialOsComponent = Cast<USpatialOsComponent>(Component);
		ComponentIdsAndInterestOverrides.emplace(
			std::make_pair(SpatialOsComponent->GetComponentId().ToSpatialComponentId(),
				worker::InterestOverride{/* IsInterested */ true }));
	}

	auto LockedConnection = Connection.Pin();
	if (LockedConnection.IsValid())
	{
		LockedConnection->SendComponentInterest(EntityId.ToSpatialEntityId(),
			ComponentIdsAndInterestOverrides);
	}
}

