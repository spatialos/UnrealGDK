// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "SpatialConstants.h"

#include <improbable/c_worker.h>

#include "EntityRegistry.generated.h"

UCLASS()
class SPATIALGDK_API UEntityRegistry : public UObject
{
	GENERATED_BODY()

public:

	/**
	* Adds a mapping from an Worker_EntityId to a AActor to the registry
	*
	* @param EntityId the Worker_EntityId for the AActor that is added to the registry.
	* @param Actor the AActor instance to be added to the registry.
	**/
	void AddToRegistry(const Worker_EntityId& EntityId, AActor* Actor);

	/**
	* Removes the entry for an AActor in the registry if present in the registry.
	*
	* @param Actor the AActor instance to be removed from the registry.
	**/
	void RemoveFromRegistry(const AActor* Actor);
	void RemoveFromRegistry(const Worker_EntityId& EntityId);

	/**
	* Get the Worker_EntityId associated with an Unreal AActor.
	* Returns nullptr if no associated Worker_EntityId found.
	*
	* @param Actor the AActor for which the Worker_EntityId is requested.
	*/
	Worker_EntityId GetEntityIdFromActor(const AActor* Actor) const;

	/**
	* Get the AActor associated with an Worker_EntityId.
	* Returns nullptr if no associated AActor found.
	*
	* @param EntityId the Worker_EntityId for which the AActor is requested.
	*/
	AActor* GetActorFromEntityId(const Worker_EntityId& EntityId) const;

private:

	void RemoveFromRegistryImpl(const AActor* Actor, const Worker_EntityId& EntityId);

	TMap<Worker_EntityId, AActor*> EntityIdToActor;
	TMap<AActor*, Worker_EntityId> ActorToEntityId;
};
