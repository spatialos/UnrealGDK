// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"

#include "improbable/UnrealObjectRef.h"

#include "ActorProxyRegistry.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogActorProxyRegistry, Log, All);

UCLASS()
class SPATIALGDK_API UActorProxyRegistry : public UObject
{
	GENERATED_BODY()

public:

	/**
	* Adds a mapping from an FUnrealObjectRef to a AActor to the registry
	*
	* @param ObjectRef the FUnrealObjectRef for the AActor that is added to the registry.
	* @param Actor the AActor instance to be added to the registry.
	**/
	void AddToRegistry(const FUnrealObjectRef& ActorProxyObjectRef, AActor* OwnerActor);

	/**
	* Removes the entry for an AActor in the registry if present in the registry.
	*
	* @param Actor the AActor instance to be removed from the registry.
	**/
	void RemoveFromRegistry(const AActor* Actor);

	/**
	* Removes the entry for an FUnrealObjectRef in the registry if present in the registry.
	*
	* @param ObjectRef the FUnrealObjectRef instance to be removed from the registry.
	**/
	void RemoveFromRegistry(const FUnrealObjectRef& ObjectRef);

	/**
	* Get the FUnrealObjectRef associated with an Unreal AActor.
	* Returns nullptr if no associated FUnrealObjectRef found.
	*
	* @param Actor the AActor for which the FUnrealObjectRef is requested.
	*/
	FUnrealObjectRef GetObjectRefFromActor(const AActor* Actor) const;

	/**
	* Get the AActor associated with an Worker_EntityId.
	* Returns nullptr if no associated AActor found.
	*
	* @param ObjectRef the Worker_EntityId for which the AActor is requested.
	*/
	AActor* GetActorFromObjectRef(const FUnrealObjectRef& ObjectRef) const;

private:

	void RemoveFromRegistryImpl(const AActor* Actor, const FUnrealObjectRef& ObjectRef);

	TMap<FUnrealObjectRef, AActor*> ObjectRefToActor;
	TMap<AActor*, FUnrealObjectRef> ActorToObjectRef;
};
