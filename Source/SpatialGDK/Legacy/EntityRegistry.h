#pragma once

#include "SpatialGDKWorkerTypes.h"

#include "Engine.h"
#include "EntityId.h"

#include "EntityRegistry.generated.h"

class USpatialOsComponent;

USTRUCT()
struct FSpatialComponentList
{
	GENERATED_BODY()

	UPROPERTY()
	TArray<USpatialOsComponent*> Components;
};

UCLASS()
class SPATIALGDK_API UEntityRegistry : public UObject
{
	GENERATED_BODY()

  public:
	/**
	* Finds all Blueprint assets present in the paths specified in the list and registers
	* them in the registry so that they can be used as templates for instantiation
	* When an entity is added.
	*
	* @param BlueprintPaths list containing all Blueprint paths that will be scanned for Blueprint
	* assets.
	*/
	UFUNCTION(BlueprintCallable, Category = "SpatialOS EntityRegistry")
	void RegisterEntityBlueprints(const TArray<FString>& BlueprintPaths);

	/**
	* Returns the UClass type for a specified class name if the UClass
	* type has been registered to the specified class name.
	*
	* @param ClassName from which a UClass is acquired.
	**/
	UClass** GetRegisteredEntityClass(const FString& ClassName);

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

	/**
	* Adds a mapping from an AActor to a USpatialOsComponent to the registry. This cache is
	* then used to update components within SpatialOSComponentUpdater.
	*
	* @param Actor the owning AActor for the USpatialOsComponent that is added to the registry.
	* @param Component the USpatialOsComponent owner by the Actor instance.
	**/
	void RegisterComponent(AActor* Actor, USpatialOsComponent* Component);

	/**
	* Removes the entry for an Actor/Component pair if present in the registry.
	*
	* @param Actor the AActor that owns the Component.
	* @param Component the USpatialOsComponent which will be removed from the registry.
	**/
	void UnregisterComponent(AActor* Actor, USpatialOsComponent* Component);

  private:
	UFUNCTION(BlueprintCallable, Category = "SpatialOS EntityRegistry")
	void RegisterEntityClass(const FString& ClassName, UClass* ClassToSpawn);

	UPROPERTY()
	TMap<FString, UClass*> ClassMap;
	UPROPERTY()
	TMap<FEntityId, AActor*> EntityIdToActor;
	UPROPERTY()
	TMap<AActor*, FEntityId> ActorToEntityId;
	UPROPERTY()
	TMap<AActor*, FSpatialComponentList> EntityComponentCache;

	friend class USpatialOsComponentUpdater;
};