// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "EntityRegistry.h"
#include "Engine.h"

DECLARE_LOG_CATEGORY_EXTERN(LogEntityRegistry, Log, All);
DEFINE_LOG_CATEGORY(LogEntityRegistry);

void UEntityRegistry::AddToRegistry(const FEntityId& EntityId, AActor* Actor)
{
	EntityIdToActor.Add(FEntityId(EntityId), Actor);
	ActorToEntityId.Add(Actor, EntityId);
}

void UEntityRegistry::RemoveFromRegistry(const AActor* Actor)
{
	FEntityId EntityId = GetEntityIdFromActor(Actor);

	ActorToEntityId.Remove(Actor);

	if (EntityId != -1)
	{
		EntityIdToActor.Remove(EntityId);
	}
	else
	{
		UE_LOG(LogEntityRegistry, Warning, TEXT("Couldn't remove Actor from registry: EntityId == -1"));
	}
}

FEntityId UEntityRegistry::GetEntityIdFromActor(const AActor* Actor) const
{
	if (ActorToEntityId.Contains(Actor))
	{
		return *ActorToEntityId.Find(Actor);
	}

	return FEntityId();
}

AActor* UEntityRegistry::GetActorFromEntityId(const FEntityId& EntityId) const
{
	if (EntityIdToActor.Contains(EntityId))
	{
		return *EntityIdToActor.Find(EntityId);
	}

	return nullptr;
}
