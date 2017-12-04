// Copyright (c) Improbable Worlds Ltd, All Rights Reserved
#include "SpatialInteropBlock.h"
#include "SpatialNetDriver.h"
#include "SpatialPackageMapClient.h"
#include "AddComponentOpWrapperBase.h"
#include "CallbackDispatcher.h"
#include "EngineMinimal.h"
#include "EntityRegistry.h"
#include "MetadataAddComponentOp.h"
#include "MetadataComponent.h"
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


			// We may already have an actor for this entity if it was spawned locally on the server.
			// In this case, we still need to register 
			AActor* EntityActor = EntityRegistry->GetActorFromEntityId(EntityToSpawn);
			if (EntityActor == nullptr)
			{
				// we don't already have a actor for this entity, so we need to spawn one
				EntityActor = SpawnNewEntity(MetadataAddComponentOp, PositionAddComponentOp, World);
			}
	
			// could still be null if SpawnActor failed
			if (EntityActor)
			{
				EntityRegistry->AddToRegistry(EntityToSpawn, EntityActor);
				SpawnedEntities.Add(EntityToSpawn);
				SetupComponentInterests(EntityActor, EntityToSpawn, InConnection);
			
				// register this entity with the PackageMap so that it has a GUID allocated
				USpatialNetDriver* Driver = Cast<USpatialNetDriver>(GetOuter());
				if (Driver && Driver->ClientConnections.Num() > 0)
				{
					// should provide a better way of getting hold of the SpatialOS client connection 
					USpatialPackageMapClient* PMC = Cast<USpatialPackageMapClient>(Driver->ClientConnections[0]->PackageMap);
					if (PMC)
					{
						PMC->ResolveEntityActor(EntityActor, EntityToSpawn);
					}
				}

			}
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

AActor* USpatialInteropBlock::SpawnNewEntity(UMetadataAddComponentOp* MetadataComponent,
	UPositionAddComponentOp* PositionComponent,
	UWorld* World)
{
	if (World)
	{
		auto Coords = PositionComponent->Data->coords();
		FVector InitialTransform =
			USpatialOSConversionFunctionLibrary::SpatialOsCoordinatesToUnrealCoordinates(
				FVector(Coords.x(), Coords.y(), Coords.z()));

		FString EntityTypeString = UTF8_TO_TCHAR(MetadataComponent->Data->entity_type().c_str());

		// Initially, attempt to find the class that has been registered to the EntityType string
		UClass** RegisteredClass = EntityRegistry->GetRegisteredEntityClass(EntityTypeString);
		UClass* ClassToSpawn = RegisteredClass ? *RegisteredClass : nullptr;

		// This is all horrendous, but assume it's a full CDO path if not registered
		if(!ClassToSpawn)
		{
			ClassToSpawn = FindObject<UClass>(ANY_PACKAGE, *EntityTypeString);				
		}

		AActor* NewActor = nullptr;
		if (ClassToSpawn)
		{
			NewActor = World->SpawnActor<AActor>(ClassToSpawn, InitialTransform,
				FRotator::ZeroRotator, FActorSpawnParameters());

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
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("SpawnNewEntity() failed: invalid World context"));

		return nullptr;
	}
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

