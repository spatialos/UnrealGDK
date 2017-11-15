// Copyright (c) Improbable Worlds Ltd, All Rights Reserved
#include "SpatialShadowActorPipelineBlock.h"

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

void USpatialShadowActorPipelineBlock::Init(UEntityRegistry* Registry)
{
	EntityRegistry = Registry;
}

void USpatialShadowActorPipelineBlock::AddEntity(const worker::AddEntityOp& AddEntityOp)
{
	// Add this to the list of entities waiting to be spawned
	PendingAddEntity.AddUnique(AddEntityOp.EntityId);
	if (NextBlock) {
		NextBlock->AddEntity(AddEntityOp);
	}
}

void USpatialShadowActorPipelineBlock::RemoveEntity(const worker::RemoveEntityOp& RemoveEntityOp)
{
	// Add this to the list of entities waiting to be deleted
	PendingRemoveEntity.AddUnique(RemoveEntityOp.EntityId);
	if (NextBlock) {
		NextBlock->RemoveEntity(RemoveEntityOp);
	}
}

void USpatialShadowActorPipelineBlock::AddComponent(UAddComponentOpWrapperBase* AddComponentOp)
{
	// Store this op to be used later on when setting the initial state of the component
	PendingAddComponentMap.Emplace(FComponentIdentifier{AddComponentOp->EntityId, AddComponentOp->ComponentId}, AddComponentOp);
	if (NextBlock) {
		NextBlock->AddComponent(AddComponentOp);
	}
}

void USpatialShadowActorPipelineBlock::RemoveComponent(const worker::ComponentId ComponentId, const worker::RemoveComponentOp& RemoveComponentOp)
{
	// Add this to the list of components waiting to be disabled
	PendingRemoveComponent.Emplace(FComponentIdentifier{RemoveComponentOp.EntityId, ComponentId});
	if (NextBlock) {
		NextBlock->RemoveComponent(ComponentId, RemoveComponentOp);
	}
}

void USpatialShadowActorPipelineBlock::ChangeAuthority(const worker::ComponentId ComponentId, const worker::AuthorityChangeOp& AuthChangeOp)
{
	// Set the latest authority value for this Component on the owning entity
	PendingAuthorityChange.Emplace(FComponentIdentifier{AuthChangeOp.EntityId, ComponentId}, AuthChangeOp);
	if (NextBlock) {
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
		AActor* PairedEntity = World->GetFirstPlayerController() ? World->GetFirstPlayerController()->GetControlledPawn() : nullptr;//EntityRegistry->GetActorFromEntityId(Entity);

		//UAddComponentOpWrapperBase* PositionBaseComponent = GetPendingAddComponent(Entity, UPositionComponent::ComponentId);
		//UAddComponentOpWrapperBase* MetadataBaseComponent = GetPendingAddComponent(Entity, UMetadataComponent::ComponentId);

		// If we've received the position and metadata components, wait for the _correct_ replicated data components.
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
				if (EntityActor)
				{
					ShadowActors.Add(Entity, EntityActor);
					SpawnedEntities.Add(Entity);
				}
			}
		}
	}

	for (auto& SpawnedEntity : SpawnedEntities)
	{
		PendingAddEntity.Remove(SpawnedEntity);
	}
}

void USpatialShadowActorPipelineBlock::AddComponents(
	const TWeakPtr<worker::View>& InView,
	const TWeakPtr<worker::Connection>& InConnection,
	UCallbackDispatcher* InCallbackDispatcher)
{
/*
	TArray<FComponentIdentifier> InitialisedComponents;

	for (auto& ComponentToAdd : PendingAddComponentMap) {
		AActor* Actor = EntityRegistry->GetActorFromEntityId(ComponentToAdd.Key.EntityId);
		if (Actor) {
			auto ComponentClass = KnownComponents.Find(ComponentToAdd.Key.ComponentId);
			if (ComponentClass) {
				USpatialOsComponent* Component =
					Cast<USpatialOsComponent>(Actor->GetComponentByClass(*ComponentClass));
				if (Component) {
					Component->Init(InConnection, InView, ComponentToAdd.Key.EntityId, InCallbackDispatcher);
					Component->ApplyInitialState(*ComponentToAdd.Value);

					auto QueuedAuthChangeOp = PendingAuthorityChange.Find(ComponentToAdd.Key);
					if (QueuedAuthChangeOp) {
						Component->ApplyInitialAuthority(*QueuedAuthChangeOp);
					}

					InitialisedComponents.Add(ComponentToAdd.Key);
				}
			}
		}
	}

	for (auto& Component : InitialisedComponents) {
		PendingAddComponentMap.Remove(Component);
	}
	*/
}

void USpatialShadowActorPipelineBlock::RemoveComponents(UCallbackDispatcher* InCallbackDispatcher)
{
/*
	for (auto& ComponentToRemove : ComponentsToRemove) {
		const worker::EntityId EntityId = ComponentToRemove.EntityId;
		const worker::ComponentId ComponentId = ComponentToRemove.ComponentId;

		AActor* Actor = EntityRegistry->GetActorFromEntityId(EntityId);
		auto ComponentClass = KnownComponents.Find(ComponentToRemove.ComponentId);

		if (!Actor || !ComponentClass) {
			continue;
		}

		USpatialOsComponent* Component =
			Cast<USpatialOsComponent>(Actor->GetComponentByClass(*ComponentClass));

		if (Component) {
			Component->Disable(EntityId, InCallbackDispatcher);
		}
	}

	ComponentsToRemove.Empty();
	*/
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
	//AddComponents(InView, InConnection, InCallbackDispatcher);
	//RemoveComponents(InCallbackDispatcher);
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

	// Look up entity class.
	//FString EntityTypeString = UTF8_TO_TCHAR(MetadataComponent->Data->entity_type().c_str());

	// Get initial coordinates.
	/*
	auto Coords = PositionComponent->Data->coords();
	FVector InitialTransform =
		USpatialOSConversionFunctionLibrary::SpatialOsCoordinatesToUnrealCoordinates(
			FVector(Coords.x(), Coords.y(), Coords.z()));

		*/
	FVector InitialTransform{0.0f, 0.0f, 0.0f};

	// Spawn shadow actor.
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
