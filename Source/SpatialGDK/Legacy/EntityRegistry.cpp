// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "EntityRegistry.h"
#include "EngineUtils.h"

DECLARE_LOG_CATEGORY_EXTERN(LogEntityRegistry, Log, All);
DEFINE_LOG_CATEGORY(LogEntityRegistry);

void UEntityRegistry::AddToRegistry(const worker::EntityId& EntityId, AActor* Actor)
{
  EntityIdToActor.Add(worker::EntityId(EntityId), Actor);
  ActorToEntityId.Add(Actor, EntityId);
}

void UEntityRegistry::RemoveFromRegistry(const AActor* Actor)
{
  worker::EntityId EntityId = GetEntityIdFromActor(Actor);
  RemoveFromRegistryImpl(Actor, EntityId);
}

void UEntityRegistry::RemoveFromRegistry(const worker::EntityId& EntityId)
{
  AActor* Actor = GetActorFromEntityId(EntityId);
  RemoveFromRegistryImpl(Actor, EntityId);
}

void UEntityRegistry::RemoveFromRegistryImpl(const AActor* Actor, const worker::EntityId& EntityId)
{
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

worker::EntityId UEntityRegistry::GetEntityIdFromActor(const AActor* Actor) const
{
  if (ActorToEntityId.Contains(Actor))
  {
    return *ActorToEntityId.Find(Actor);
  }

  return worker::EntityId();
}

AActor* UEntityRegistry::GetActorFromEntityId(const worker::EntityId& EntityId) const
{
  if (EntityIdToActor.Contains(EntityId))
  {
    return *EntityIdToActor.Find(EntityId);
  }

  return nullptr;
}
