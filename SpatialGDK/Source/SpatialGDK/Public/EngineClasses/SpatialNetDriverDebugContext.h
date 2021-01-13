// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Schema/DebugComponent.h"
#include "Schema/Interest.h"
#include "SpatialCommonTypes.h"

#include "SpatialNetDriverDebugContext.generated.h"

class UDebugLBStrategy;
class USpatialNetDriver;

/*
 * Implement the debug layer from SpatialFunctionalTest, which is enabled by a map flag.
 * The goal is to be able to arbitrarily gain interest and manipulate authority of any actors.
 * This is done by adding an extra debug component to all actors, which will contain tags.
 * All server workers will have interest in all entities with this component, allowing them to inspect tags, and check out extra actors if
 * they need to. Arbitrary authority delegation is achieved by wrapping the NetDriver's load balancing strategy into a debug one, which will
 * inspect tags first. One caveat that all servers must declare the same delegation at the same time (this requirement could be lifted if
 * this object was made to behave like a singleton).
 */

namespace SpatialGDK
{
struct ComponentChange;
}

UCLASS()
class SPATIALGDK_API USpatialNetDriverDebugContext : public UObject
{
	GENERATED_BODY()
public:
	static void EnableDebugSpatialGDK(USpatialNetDriver* NetDriver);
	static void DisableDebugSpatialGDK(USpatialNetDriver* NetDriver);

	// ------ Startup / Shutdown
	void Init(USpatialNetDriver* NetDriver);
	void Cleanup();
	void Reset();

	// ------ Debug Interface

	// Manage tags on the debug component.
	// Add/remove extra interest if tags match extra interest queries
	void AddActorTag(AActor* Actor, FName Tag);
	void RemoveActorTag(AActor* Actor, FName Tag);

	// Manage extra interest.
	// This pushes tags to SemanticInterest, which will be compared to the Actor debug components
	// to see if we should add extra interest queries to the worker's interest.
	void AddInterestOnTag(FName Tag);
	void RemoveInterestOnTag(FName Tag);

	// Pin the given Actor to the current worker by setting the local workerId on its debug component
	void KeepActorOnLocalWorker(AActor* Actor);

	// Manage Actor authority delegation.
	// This pushes an entry to SemanticDelegations, which will be examined by the debug load balancing strategy.
	// For this to work properly, all workers should declare the same set of delegations.
	void DelegateTagToWorker(FName Tag, uint32 WorkerId);
	void RemoveTagDelegation(FName Tag);

	// Used by the debug worker strategy to retrieve
	TOptional<VirtualWorkerId> GetActorHierarchyExplicitDelegation(const AActor* Actor);

	// ----- Utility

	bool IsActorReady(AActor* Actor);

	// ----- NetDriver Integration

	// This will be called from SpatialNetDriver::ServerReplicateActor
	// It will create debug components or update them. It also updates the worker's interest query if needed.
	void AdvanceView();
	void TickServer();

	void ClearNeedEntityInterestUpdate() { bNeedToUpdateInterest = false; }

	SpatialGDK::QueryConstraint ComputeAdditionalEntityQueryConstraint() const;

	UPROPERTY()
	UDebugLBStrategy* DebugStrategy = nullptr;

protected:
	// Called from SpatialReveiver when the corresponding Ops are encountered.
	void AddComponent(Worker_EntityId EntityId);
	void OnComponentChange(Worker_EntityId EntityId, const SpatialGDK::ComponentChange& Change);
	void ApplyComponentUpdate(Worker_EntityId EntityId, Schema_ComponentUpdate* Update);
	void AuthorityLost(Worker_EntityId EntityId);

	struct DebugComponentAuthData
	{
		SpatialGDK::DebugComponent Component;
		Worker_EntityId Entity = SpatialConstants::INVALID_ENTITY_ID;
		bool bAdded = false;
		bool bDirty = false;
	};

	DebugComponentAuthData& GetAuthDebugComponent(AActor* Actor);

	TOptional<VirtualWorkerId> GetActorExplicitDelegation(const AActor* Actor);
	TOptional<VirtualWorkerId> GetActorHierarchyExplicitDelegation_Traverse(const AActor* Actor);

	void AddEntityToWatch(Worker_EntityId);
	void RemoveEntityToWatch(Worker_EntityId);

	bool NeedEntityInterestUpdate() { return bNeedToUpdateInterest; }

	USpatialNetDriver* NetDriver;

	// Collection of actor tag delegations.
	TMap<FName, VirtualWorkerId> SemanticDelegations;

	// Collection of actor tags we should get interest over.
	TSet<FName> SemanticInterest;

	// Debug info for actors. Only keeps entries for Actors we have authority over.
	TMap<AActor*, DebugComponentAuthData> ActorDebugInfo;
	TMap<Worker_EntityId_Key, SpatialGDK::DebugComponent> DebugComponents;

	// Contains a cache of entities computed from the semantic interest.
	TSet<Worker_EntityId_Key> CachedInterestSet;
	bool bNeedToUpdateInterest = false;
};
