// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "Utils/EntityRegistry.h"

#include "EngineUtils.h"

DECLARE_LOG_CATEGORY_EXTERN(LogEntityRegistry, Log, All);
DEFINE_LOG_CATEGORY(LogEntityRegistry);

void UEntityRegistry::AddToRegistry(const Worker_EntityId& EntityId, AActor* Actor)
{
	EntityIdToActor.Add(EntityId, Actor);
	ActorToEntityId.Add(Actor, EntityId);
}

void UEntityRegistry::RemoveFromRegistry(const AActor* Actor)
{
	Worker_EntityId EntityId = GetEntityIdFromActor(Actor);
	RemoveFromRegistryImpl(Actor, EntityId);
}

void UEntityRegistry::RemoveFromRegistry(const Worker_EntityId& EntityId)
{
	AActor* Actor = GetActorFromEntityId(EntityId);
	RemoveFromRegistryImpl(Actor, EntityId);
}

void UEntityRegistry::RemoveFromRegistryImpl(const AActor* Actor, const Worker_EntityId& EntityId)
{
	if (Actor)
	{
		if (ActorToEntityId.Contains(Actor))
		{
			ActorToEntityId.Remove(Actor);
		}
		else
		{
			UE_LOG(LogEntityRegistry, Warning, TEXT("Tried to remove actor %s from registry but it wasn't there"), *Actor->GetFullName());
		}
	}
	else
	{
		UE_LOG(LogEntityRegistry, Warning, TEXT("Couldn't remove Actor from registry: Actor == nullptr"));
	}

	if (EntityId != -1)
	{
		if (EntityIdToActor.Contains(EntityId))
		{
			EntityIdToActor.Remove(EntityId);
		}
		else
		{
			UE_LOG(LogEntityRegistry, Warning, TEXT("Tried to remove entity ID %lld from registry but it wasn't there"), EntityId);
		}
	}
	else
	{
		UE_LOG(LogEntityRegistry, Warning, TEXT("Couldn't remove Actor from registry: EntityId == -1"));
	}
}

Worker_EntityId UEntityRegistry::GetEntityIdFromActor(const AActor* Actor) const
{
	if (ActorToEntityId.Contains(Actor))
	{
		return *ActorToEntityId.Find(Actor);
	}

	return Worker_EntityId();
}

AActor* UEntityRegistry::GetActorFromEntityId(const Worker_EntityId& EntityId) const
{
	if (EntityIdToActor.Contains(EntityId))
	{
		return *EntityIdToActor.Find(EntityId);
	}

	return nullptr;
}
