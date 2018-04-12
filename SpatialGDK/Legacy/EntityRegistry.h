#pragma once

#include "SpatialGDKWorkerTypes.h"

#include "Engine.h"
#include "EntityId.h"

#include "EntityRegistry.generated.h"

UCLASS()
class SPATIALGDK_API UEntityRegistry : public UObject
{
	GENERATED_BODY()

  public:
	/**
  * Adds a mapping from an FEntityId to a AActor to the registry
  *
  * @param EntityId the FEntityId for the AActor that is added to the registry.
  * @param Actor the AActor instance to be added to the registry.
  **/
	UFUNCTION(BlueprintCallable, Category = "SpatialOS EntityRegistry")
	void AddToRegistry(const FEntityId& EntityId, AActor* Actor);

	/**
  * Removes the entry for an AActor in the registry if present in the registry.
  *
  * @param Actor the AActor instance to be removed from the registry.
  **/
	UFUNCTION(BlueprintCallable, Category = "SpatialOS EntityRegistry")
	void RemoveFromRegistry(const AActor* Actor);

	/**
  * Get the FEntityId associated with an Unreal AActor.
  * Returns nullptr if no associated FEntityId found.
  *
  * @param Actor the AActor for which the FEntityId is requested.
  */
	UFUNCTION(BlueprintCallable, Category = "SpatialOS EntityRegistry")
	FEntityId GetEntityIdFromActor(const AActor* Actor) const;

	/**
  * Get the AActor associated with an FEntityId.
  * Returns nullptr if no associated AActor found.
  *
  * @param EntityId the FEntityId for which the AActor is requested.
  */
	UFUNCTION(BlueprintCallable, Category = "SpatialOS EntityRegistry")
	AActor* GetActorFromEntityId(const FEntityId& EntityId) const;

  private:
	UPROPERTY()
	TMap<FString, UClass*> ClassMap;
	UPROPERTY()
	TMap<FEntityId, AActor*> EntityIdToActor;
	UPROPERTY()
	TMap<AActor*, FEntityId> ActorToEntityId;
};