// Copyright (c) Improbable Worlds Ltd, All Rights Reserved
#include "SpatialShadowActorPipelineBlock.h"

#include "AddComponentOpWrapperBase.h"
#include "CallbackDispatcher.h"
#include "EngineMinimal.h"
#include "EntityRegistry.h"
#include "SpatialNetDriver.h"
#include "SpatialPackageMapClient.h"
#include "MetadataAddComponentOp.h"
#include "MetadataComponent.h"
#include "PackageMapComponent.h"
#include "PositionAddComponentOp.h"
#include "PositionComponent.h"
#include "SpatialOSConversionFunctionLibrary.h"
#include "improbable/view.h"
#include "improbable/worker.h"

struct FMockExportFlags
{
	union
	{
		struct
		{
			uint8 bHasPath : 1;
			uint8 bNoLoad : 1;
			uint8 bHasNetworkChecksum : 1;
		};

		uint8	Value;
	};

	FMockExportFlags(){ Value = 0; }
};

void USpatialShadowActorPipelineBlock::Init(UEntityRegistry* Registry)
{
	EntityRegistry = Registry;
}

void USpatialShadowActorPipelineBlock::AddEntity(const worker::AddEntityOp& AddEntityOp)
{
	// Add this to the list of entities waiting to be spawned
	PendingAddEntity.AddUnique(AddEntityOp.EntityId);
	if (NextBlock)
	{
		NextBlock->AddEntity(AddEntityOp);
	}
}

void USpatialShadowActorPipelineBlock::RemoveEntity(const worker::RemoveEntityOp& RemoveEntityOp)
{
	// Add this to the list of entities waiting to be deleted
	PendingRemoveEntity.AddUnique(RemoveEntityOp.EntityId);
	if (NextBlock)
	{
		NextBlock->RemoveEntity(RemoveEntityOp);
	}
}

void USpatialShadowActorPipelineBlock::AddComponent(UAddComponentOpWrapperBase* AddComponentOp)
{
	// Store this op to be used later on when setting the initial state of the component
	PendingAddComponentMap.Emplace(FComponentIdentifier{AddComponentOp->EntityId, AddComponentOp->ComponentId}, AddComponentOp);
	if (NextBlock)
	{
		NextBlock->AddComponent(AddComponentOp);
	}
}

void USpatialShadowActorPipelineBlock::RemoveComponent(const worker::ComponentId ComponentId, const worker::RemoveComponentOp& RemoveComponentOp)
{
	// Add this to the list of components waiting to be disabled
	PendingRemoveComponent.Emplace(FComponentIdentifier{RemoveComponentOp.EntityId, ComponentId});
	if (NextBlock)
	{
		NextBlock->RemoveComponent(ComponentId, RemoveComponentOp);
	}
}

void USpatialShadowActorPipelineBlock::ChangeAuthority(const worker::ComponentId ComponentId, const worker::AuthorityChangeOp& AuthChangeOp)
{
	// Set the latest authority value for this Component on the owning entity
	PendingAuthorityChange.Emplace(FComponentIdentifier{AuthChangeOp.EntityId, ComponentId}, AuthChangeOp);
	if (NextBlock)
	{
		NextBlock->ChangeAuthority(ComponentId, AuthChangeOp);
	}
}

ASpatialShadowActor* USpatialShadowActorPipelineBlock::GetShadowActor(const FEntityId& EntityId) const
{
	ASpatialShadowActor* const* Value = ShadowActors.Find(EntityId);
	return Value ? *Value : nullptr;
}

void USpatialShadowActorPipelineBlock::ReplicateShadowActorChanges(float DeltaTime)
{
	for (auto& Actor : ShadowActors)
	{
		Actor.Value->ReplicatedData->ReplicateChanges(DeltaTime);
	}
}

void USpatialShadowActorPipelineBlock::AddEntities(
	UWorld* World,
	const TWeakPtr<worker::View>& InView,
	const TWeakPtr<worker::Connection>& InConnection,
	UCallbackDispatcher* InCallbackDispatcher)
{
	TArray<FEntityId> SpawnedEntities;

	if (World == nullptr)
	{
		return;
	}

	// We can only spawn an entity if it exists in the entity registry.
	for (auto& Entity : PendingAddEntity)
	{
		// Wait for the "real" entity to be checked out.
		// TODO: For now, just use the first player controllers pawn.
		AActor* PairedEntity = World->GetFirstPlayerController() ? World->GetFirstPlayerController()->GetPawn() : nullptr;//EntityRegistry->GetActorFromEntityId(Entity);
		if (PairedEntity)
		{
			// Retrieve the EntityType string from the Metadata component.
			UClass* EntityClass = PairedEntity->GetClass();
			// TODO: Hardcoded to ACharacter for now.
			EntityClass = ACharacter::StaticClass();
			FString EntityType = EntityClass->GetName();

			// Wait for the Unreal<EntityType>ReplicatedData and Unreal<EntityType>CompleteData components.
			// TODO: Look up ComponentId's from EntityClass/EntityType
			UAddComponentOpWrapperBase* ReplicatedDataComponent = GetPendingAddComponent(Entity, UUnrealACharacterReplicatedDataComponent::ComponentId);
			UAddComponentOpWrapperBase* CompleteDataComponent = GetPendingAddComponent(Entity, UUnrealACharacterCompleteDataComponent::ComponentId);

			if (ReplicatedDataComponent && CompleteDataComponent)
			{
				ASpatialShadowActor* EntityActor = TrySpawnShadowActor(
					Entity,
					EntityClass,
					ReplicatedDataComponent,
					CompleteDataComponent,
					World,
					InView,
					InConnection,
					InCallbackDispatcher);
				EntityActor->PairedActor = PairedEntity;
				if (EntityActor)
				{
					ShadowActors.Add(Entity, EntityActor);
					SpawnedEntities.Add(Entity);
				}
			}

			// Hardcoding to PackageMap entityId for now
			if (Entity.ToSpatialEntityId() == 3)
			{
				bool bPackageMapImported = false;
				UAddComponentOpWrapperBase* PackageMapComponent = GetPendingAddComponent(Entity, UPackageMapComponent::ComponentId);
				if (PackageMapComponent)
				{
					USpatialNetDriver* Driver = Cast<USpatialNetDriver>(GetOuter());
					if (Driver->ClientConnections.Num() > 0)
					{
						USpatialPackageMapClient* PMC = Cast<USpatialPackageMapClient>(Driver->ClientConnections[0]->PackageMap);
						if (PMC)
						{
							UPackageMapAddComponentOp* Op = Cast<UPackageMapAddComponentOp>(PackageMapComponent);
							worker::Map<std::uint32_t, std::string> PackageMap = Op->Data->id_to_path_map();
							for (auto It = PackageMap.begin();
								It != PackageMap.end();
								It++)
							{
								// Can directly register this object with the PackageMap using the GUID we've received as 
								// we know it is a static object and that this GUID is unique

								FNetworkGUID NetGUID(It->first);
								FString Path(It->second.c_str());
								PMC->ResolveStaticObjectGUID(NetGUID, Path);
							}

							SpawnedEntities.Add(Entity);
						}
					}
				}
			}
		}
	}

	for (auto& SpawnedEntity : SpawnedEntities)
	{
		PendingAddEntity.Remove(SpawnedEntity);
	}
}

void USpatialShadowActorPipelineBlock::RemoveEntities(UWorld* World)
{
	for (auto& EntityToRemove : PendingRemoveEntity)
	{
		ASpatialShadowActor** ActorPtr = ShadowActors.Find(EntityToRemove);
		if (ActorPtr)
		{
			World->DestroyActor(*ActorPtr);


			ShadowActors.Remove(EntityToRemove);
		}
	}
	PendingRemoveEntity.Empty();
}

void USpatialShadowActorPipelineBlock::ProcessOps(
	const TWeakPtr<worker::View>& InView,
	const TWeakPtr<worker::Connection>& InConnection,
	UWorld* World,
	UCallbackDispatcher* InCallbackDispatcher)
{
	AddEntities(World, InView, InConnection, InCallbackDispatcher);
	RemoveEntities(World);
}

UAddComponentOpWrapperBase* USpatialShadowActorPipelineBlock::GetPendingAddComponent(
	const FEntityId& EntityId, const worker::ComponentId& ComponentId)
{
	UAddComponentOpWrapperBase** BaseWrapper = PendingAddComponentMap.Find(FComponentIdentifier{EntityId.ToSpatialEntityId(), ComponentId});
	return (BaseWrapper && (*BaseWrapper)->IsValidLowLevel()) ? *BaseWrapper : nullptr;
}

ASpatialShadowActor* USpatialShadowActorPipelineBlock::TrySpawnShadowActor(
	const FEntityId& EntityId,
	UClass* EntityClass,
	UAddComponentOpWrapperBase* ReplicatedDataComponent,
	UAddComponentOpWrapperBase* CompleteDataComponent,
	UWorld* World,
	const TWeakPtr<worker::View>& InView,
	const TWeakPtr<worker::Connection>& InConnection,
	UCallbackDispatcher* InCallbackDispatcher)
{
	if (!World)
	{
		UE_LOG(LogTemp, Warning, TEXT("TrySpawnShadowActor() failed: invalid World context"));
		return nullptr;
	}

	// Spawn shadow actor.
	FVector InitialTransform{ 0.0f, 0.0f, 0.0f };
	auto NewActor = World->SpawnActor<ASpatialShadowActor>(ASpatialShadowActor::StaticClass(), InitialTransform, FRotator::ZeroRotator, FActorSpawnParameters());

	// Initialise replicated and complete data.
	NewActor->ReplicatedData->Init(InConnection, InView, EntityId.ToSpatialEntityId(), InCallbackDispatcher);
	NewActor->ReplicatedData->ApplyInitialState(*ReplicatedDataComponent);
	if (PendingAuthorityChange.Find(FComponentIdentifier{EntityId.ToSpatialEntityId(), UUnrealACharacterReplicatedDataComponent::ComponentId})) {
		auto AuthorityChange = PendingAuthorityChange.FindAndRemoveChecked(FComponentIdentifier{EntityId.ToSpatialEntityId(), UUnrealACharacterReplicatedDataComponent::ComponentId});
		NewActor->ReplicatedData->ApplyInitialAuthority(AuthorityChange);
	}
	NewActor->CompleteData->Init(InConnection, InView, EntityId.ToSpatialEntityId(), InCallbackDispatcher);
	NewActor->CompleteData->ApplyInitialState(*CompleteDataComponent);
	if (PendingAuthorityChange.Find(FComponentIdentifier{EntityId.ToSpatialEntityId(), UUnrealACharacterCompleteDataComponent::ComponentId})) {
		auto AuthorityChange = PendingAuthorityChange.FindAndRemoveChecked(FComponentIdentifier{EntityId.ToSpatialEntityId(), UUnrealACharacterCompleteDataComponent::ComponentId});
		NewActor->CompleteData->ApplyInitialAuthority(AuthorityChange);
	}

	return NewActor;
}
