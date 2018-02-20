// Copyright (c) Improbable Worlds Ltd, All Rights Reserved
#include "SpatialInteropPipelineBlock.h"

#include "SpatialActorChannel.h"
#include "SpatialNetDriver.h"
#include "SpatialNetConnection.h"
#include "SpatialPackageMapClient.h"
#include "AddComponentOpWrapperBase.h"
#include "CallbackDispatcher.h"
#include "EngineMinimal.h"
#include "EntityRegistry.h"
#include "GameFramework/PlayerController.h"
#include "SpatialOSConversionFunctionLibrary.h"
#include "improbable/view.h"
#include "improbable/worker.h"

#include "PositionAddComponentOp.h"
#include "PositionComponent.h"
#include "MetadataAddComponentOp.h"
#include "MetadataComponent.h"
#include "UnrealMetadataAddComponentOp.h"
#include "UnrealMetadataComponent.h"
#include "UnrealLevelComponent.h"

void USpatialInteropPipelineBlock::Init(UEntityRegistry* Registry)
{
	EntityRegistry = Registry;
}

void USpatialInteropPipelineBlock::AddEntity(const worker::AddEntityOp& AddEntityOp)
{
	// Add this to the list of entities waiting to be spawned
	EntitiesToSpawn.AddUnique(AddEntityOp.EntityId);
	if (NextBlock)
	{
		NextBlock->AddEntity(AddEntityOp);
	}
}

void USpatialInteropPipelineBlock::RemoveEntity(const worker::RemoveEntityOp& RemoveEntityOp)
{
	// Add this to the list of entities waiting to be deleted
	EntitiesToRemove.AddUnique(RemoveEntityOp.EntityId);
	if (NextBlock)
	{
		NextBlock->RemoveEntity(RemoveEntityOp);
	}
}

void USpatialInteropPipelineBlock::AddComponent(UAddComponentOpWrapperBase* AddComponentOp)
{
	// Store this op to be used later on when setting the initial state of the component
	ComponentsToAdd.Emplace(FComponentIdentifier{ AddComponentOp->EntityId, AddComponentOp->ComponentId }, AddComponentOp);
	if (NextBlock)
	{
		NextBlock->AddComponent(AddComponentOp);
	}
}

void USpatialInteropPipelineBlock::RemoveComponent(const worker::ComponentId ComponentId,
	const worker::RemoveComponentOp& RemoveComponentOp)
{
	// Add this to the list of components waiting to be disabled
	ComponentsToRemove.Emplace(FComponentIdentifier{ RemoveComponentOp.EntityId, ComponentId });
	if (NextBlock)
	{
		NextBlock->RemoveComponent(ComponentId, RemoveComponentOp);
	}
}

void USpatialInteropPipelineBlock::ChangeAuthority(const worker::ComponentId ComponentId,
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

void USpatialInteropPipelineBlock::AddEntities(UWorld* World,
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
		UPositionAddComponentOp* PositionAddComponentOp = GetPendingAddComponent<UPositionAddComponentOp, UPositionComponent>(EntityToSpawn);
		UMetadataAddComponentOp* MetadataAddComponentOp = GetPendingAddComponent<UMetadataAddComponentOp, UMetadataComponent>(EntityToSpawn);

		// Only spawn entities for which we have received Position and Metadata components
		if (PositionAddComponentOp && MetadataAddComponentOp)
		{
			USpatialNetDriver* Driver = Cast<USpatialNetDriver>(World->GetNetDriver());
			AActor* EntityActor = EntityRegistry->GetActorFromEntityId(EntityToSpawn);

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
					// We need to wait for UnrealMetadataComponent here.
					UUnrealMetadataAddComponentOp* UnrealMetadataAddComponentOp = GetPendingAddComponent<UUnrealMetadataAddComponentOp, UUnrealMetadataComponent>(EntityToSpawn);
					if (!UnrealMetadataAddComponentOp)
					{
						continue;
					}

					// Option 3
					ClassToSpawn = GetNativeEntityClass(MetadataAddComponentOp);
					improbable::unreal::UnrealMetadataData& UnrealMetadata = *(*UnrealMetadataAddComponentOp).Data.data();
					if (UnrealMetadata.static_path().empty())
					{
						UE_LOG(LogSpatialOSNUF, Log, TEXT("Does not exist, attempting to spawn a native %s"), *ClassToSpawn->GetName());
						EntityActor = SpawnNewEntity(PositionAddComponentOp, World, ClassToSpawn);
					}
					else
					{
						FString FullPath = UTF8_TO_TCHAR(UnrealMetadata.static_path().data()->c_str());
						UE_LOG(LogSpatialOSNUF, Log, TEXT("Attempting to find object %s of class %s"), *FullPath, *ClassToSpawn->GetName());
						EntityActor = FindObject<AActor>(World, *FullPath);
					}
					check(EntityActor);
					EntityRegistry->AddToRegistry(EntityToSpawn, EntityActor);

					//todo-giray: When we have multiple servers, this won't work. On which connection would we create the channel?
					UNetConnection* Connection = Driver->GetSpatialOSNetConnection();

					USpatialPackageMapClient* PackageMap = Cast<USpatialPackageMapClient>(Connection->PackageMap);
					USpatialActorChannel* Channel = Cast<USpatialActorChannel>(Connection->CreateChannel(CHTYPE_Actor, false));

					check(Channel);
					
					PackageMap->ResolveEntityActor(EntityActor, EntityToSpawn);
					Channel->SetChannelActor(EntityActor);

					// This is a bit of a hack unfortunately, among the core classes only PlayerController implements this function and it requires
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

					// Inform USpatialInterop of this new client facing actor channel.
					Driver->GetSpatialInterop()->AddActorChannel_Client(EntityToSpawn.ToSpatialEntityId(), Channel);
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

void USpatialInteropPipelineBlock::AddComponents(const TWeakPtr<worker::View>& InView,
	const TWeakPtr<worker::Connection>& InConnection,
	UCallbackDispatcher* InCallbackDispatcher)
{
	TArray<FComponentIdentifier> InitialisedComponents;

	// Go through the add component ops that have been queued
	// If the entity the component belongs to is already spawned, create component & apply initial state.
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

					UUnrealLevelComponent* PackageMapComponent = Cast<UUnrealLevelComponent>(Component);
					if (PackageMapComponent)
					{
						USpatialNetDriver* Driver = Cast<USpatialNetDriver>(Actor->GetWorld()->GetNetDriver());
						UNetConnection* Connection = Driver->GetSpatialOSNetConnection();
						USpatialPackageMapClient* PMC = Cast<USpatialPackageMapClient>(Connection->PackageMap);
						UUnrealLevelAddComponentOp* DerivedOp = Cast<UUnrealLevelAddComponentOp>(ComponentToAdd.Value);
						PMC->RegisterStaticObjects(*DerivedOp->Data.data());
					}
				}
			}
		}
	}

	for (auto& Component : InitialisedComponents)
	{
		ComponentsToAdd.Remove(Component);
	}
}

void USpatialInteropPipelineBlock::RemoveComponents(UCallbackDispatcher* InCallbackDispatcher)
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

void USpatialInteropPipelineBlock::RemoveEntities(UWorld* World)
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

void USpatialInteropPipelineBlock::ProcessOps(const TWeakPtr<worker::View>& InView,
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
AActor* USpatialInteropPipelineBlock::SpawnNewEntity(
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
		//bRemoteOwned needs to be public in source code. This might be a controversial change.
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

// This is for classes that we register explicitly with Unreal, currently used for "non-native" replication. This logic might change soon.
UClass* USpatialInteropPipelineBlock::GetRegisteredEntityClass(UMetadataAddComponentOp* MetadataComponent)
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

// This is for classes that we derive from meta name, mainly to spawn the corresponding actors on clients.
UClass* USpatialInteropPipelineBlock::GetNativeEntityClass(UMetadataAddComponentOp* MetadataComponent)
{
	FString Metadata = UTF8_TO_TCHAR(MetadataComponent->Data->entity_type().c_str());

	// This is all horrendous, but assume it's a full CDO path if not registered	
	return FindObject<UClass>(ANY_PACKAGE, *Metadata);	
}

void USpatialInteropPipelineBlock::SetupComponentInterests(
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

