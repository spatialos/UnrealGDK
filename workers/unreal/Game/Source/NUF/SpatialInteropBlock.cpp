// Copyright (c) Improbable Worlds Ltd, All Rights Reserved
#include "SpatialInteropBlock.h"

#include "SpatialActorChannel.h"
#include "SpatialNetDriver.h"
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
			// We may already have an actor for this entity if it was spawned locally on the server.
			// In this case, we still need to register 
			AActor* EntityActor = EntityRegistry->GetActorFromEntityId(EntityToSpawn);
			USpatialPackageMapClient* PMC = nullptr;

			UE_LOG(LogTemp, Warning, TEXT("Received add entity op for %d"), EntityToSpawn.ToSpatialEntityId());

			if (EntityActor)
			{
				EntityRegistry->AddToRegistry(EntityToSpawn, EntityActor);
				SpawnedEntities.Add(EntityToSpawn);
				SetupComponentInterests(EntityActor, EntityToSpawn, InConnection);

				// register this entity with the PackageMap so that it has a GUID allocated				
				//if (Driver && Driver->ClientConnections.Num() > 0)
				{
					// should provide a better way of getting hold of the SpatialOS client connection 
					PMC = Cast<USpatialPackageMapClient>(Driver->ClientConnections[0]->PackageMap);
					PMC->ResolveEntityActor(EntityActor, EntityToSpawn);
				}
			}
			else
			{
				UClass* ClassToSpawn = GetRegisteredEntityClass(MetadataAddComponentOp);

				if (ClassToSpawn)
				{
					// This means that we had registered this class. We treat this as a "standard" Spatial entity. No actor channel etc.
					//(This assumption might change in the future)
					EntityActor = SpawnNewEntity(PositionAddComponentOp, World, ClassToSpawn);
				}
				else
				{
					ClassToSpawn = GetNativeEntityClass(MetadataAddComponentOp);
					// We are either on a client, or a worker that is not the original spawner of this entity.
					EntityActor = SpawnNewEntity(PositionAddComponentOp, World, ClassToSpawn);
					check(EntityActor);

					//NUF-todo: When we have multiple servers, this won't work. On which connection would we create the channel?
					PMC = Cast<USpatialPackageMapClient>(Driver->ServerConnection->PackageMap);
					USpatialActorChannel* Ch = Cast<USpatialActorChannel>(Driver->ServerConnection->CreateChannel(CHTYPE_Actor, false));
					check(Ch);
					EntityToClientActorChannel.Add(EntityToSpawn, Ch);

					PMC->ResolveEntityActor(EntityActor, EntityToSpawn);
					Ch->SetChannelActor(EntityActor);

					//This is a bit of a hack unfortunately, among the core classes only PlayerController implements this function and it requires
					// a player index. For now we don't support split screen, so the number is always 0.
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
		UE_LOG(LogTemp, Warning, TEXT("SpawnNewEntity() failed: invalid World context"));

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

