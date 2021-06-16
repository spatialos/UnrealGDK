// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"

#include "SpatialCommonTypes.h"

#include "Containers/Map.h"

class USpatialNetDriver;

namespace SpatialGDK
{
class FSubView;
struct SpatialDebugging;

class SpatialDebuggerSystem
{
public:
	SpatialDebuggerSystem(USpatialNetDriver* InNetDriver, const FSubView& InSubView);

	void Advance();

	void UpdateSpatialDebuggingData(FSpatialEntityId EntityId, const AActor& Actor);

	void ActorAuthorityIntentChanged(FSpatialEntityId EntityId, VirtualWorkerId NewIntentVirtualWorkerId) const;

	DECLARE_MULTICAST_DELEGATE_OneParam(FSpatialDebuggerActorAddedDelegate, AActor*);
	FSpatialDebuggerActorAddedDelegate OnEntityActorAddedDelegate;

	TOptional<SpatialDebugging> GetDebuggingData(FSpatialEntityId Entity) const;
	AActor* GetActor(FSpatialEntityId EntityId) const;

	typedef TMap<Worker_EntityId_Key, TWeakObjectPtr<AActor>> FEntityToActorMap;

	const Worker_EntityId_Key* GetActorEntityId(AActor* Actor) const;
	const FEntityToActorMap& GetActors() const;

private:
	void OnEntityAdded(FSpatialEntityId AddedEntityId);
	void OnEntityRemoved(FSpatialEntityId RemovedEntityId);
	void ActorAuthorityGained(FSpatialEntityId EntityId) const;

	static constexpr int ENTITY_ACTOR_MAP_RESERVATION_COUNT = 512;

	// These mappings are maintained independently on each client
	// Mapping of the entities a client has checked out
	FEntityToActorMap EntityActorMapping;

	TWeakObjectPtr<USpatialNetDriver> NetDriver;
	const FSubView* SubView;
};
} // namespace SpatialGDK
