// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "EntityRegistry.h"
#include "EngineUtils.h"

DECLARE_LOG_CATEGORY_EXTERN(LogEntityRegistry, Log, All);
DEFINE_LOG_CATEGORY(LogEntityRegistry);

void UEntityRegistry::RegisterEntityBlueprints(const TArray<FString>& BlueprintPaths)
{
  for (auto& Path : BlueprintPaths)
  {
    TArray<UObject*> Assets;
    if (EngineUtils::FindOrLoadAssetsByPath(Path, Assets, EngineUtils::ATL_Class))
    {
      for (auto Asset : Assets)
      {
        UBlueprintGeneratedClass* BlueprintGeneratedClass = Cast<UBlueprintGeneratedClass>(Asset);
        if (BlueprintGeneratedClass != nullptr)
        {
          FString BlueprintName = BlueprintGeneratedClass->GetName().LeftChop(
              2);  // generated blueprint class names end with "_C"
          UE_LOG(LogTemp, Display, TEXT("Registering blueprint in entity spawner with name: %s"),
                 *BlueprintName);
          RegisterEntityClass(BlueprintName, BlueprintGeneratedClass);
        }
        else
        {
          UE_LOG(LogTemp, Warning,
                 TEXT("Found asset in the EntityBlueprints folder which is not a blueprint: %s"),
                 *(Asset->GetFullName()));
        }
      }
    }
    else
    {
      UE_LOG(LogTemp, Warning, TEXT("No assets found in EntityBlueprints folder."));
    }
  }
}

UClass** UEntityRegistry::GetRegisteredEntityClass(const FString& ClassName)
{
  auto ClassToSpawn = ClassMap.Find(ClassName);
  if (ClassToSpawn == nullptr)
  {
    UE_LOG(LogEntityRegistry, Warning, TEXT("No UClass is associated with ClassName('%s')."),
           *ClassName);
    return nullptr;
  }

  return ClassToSpawn;
}

void UEntityRegistry::AddToRegistry(const FEntityId& EntityId, AActor* Actor)
{
  EntityIdToActor.Add(FEntityId(EntityId), Actor);
  ActorToEntityId.Add(Actor, EntityId);
}

void UEntityRegistry::RemoveFromRegistry(const AActor* Actor)
{
  FEntityId EntityId = GetEntityIdFromActor(Actor);

  ActorToEntityId.Remove(Actor);
  EntityComponentCache.Remove(Actor);

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

void UEntityRegistry::RegisterComponent(AActor* Actor, USpatialOsComponent* Component)
{
  auto& ComponentCache = EntityComponentCache.FindOrAdd(Actor);
  check(ComponentCache.Components.Find(Component) == INDEX_NONE);
  ComponentCache.Components.Push(Component);
}

void UEntityRegistry::UnregisterComponent(AActor* Actor, USpatialOsComponent* Component)
{
  if (Component == nullptr)
  {
    UE_LOG(LogEntityRegistry, Error,
           TEXT("UEntityRegistry::UnregisterComponent: Component argument is null."));
    return;
  }

  if (auto ComponentCache = EntityComponentCache.Find(Actor))
  {
    check(ComponentCache->Components.Find(Component) != INDEX_NONE);
    ComponentCache->Components.RemoveSingleSwap(Component);
  }
  else
  {
    UE_LOG(LogEntityRegistry, Error,
           TEXT("UEntityRegistry::UnregisterComponent: Actor is not registered."));
    return;
  }
}

void UEntityRegistry::RegisterEntityClass(const FString& ClassName, UClass* ClassToSpawn)
{
  if (ClassToSpawn == nullptr)
  {
    UE_LOG(LogEntityRegistry, Error,
           TEXT("UEntityRegistry::RegisterEntityClass: ClassToSpawn argument is null."));
    return;
  }

  auto ExistingClass = ClassMap.Find(ClassName);
  if (ExistingClass != nullptr)
  {
    UE_LOG(LogEntityRegistry, Error, TEXT("ClassName '%s' has already been registered to '%s'."),
           *ClassName, *(*ExistingClass)->GetClass()->GetName());
    return;
  }

  ClassMap.Add(ClassName, ClassToSpawn);
}
