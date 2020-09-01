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
 * All server workers will have interest over this component, allowing them to inspect tags, and check out extra actors if they need to.
 * Arbitrary authority delegation is achieved by wrapping the NetDriver's load balancing strategy into a debug one, which will inspect tags
 * first. One caveat that all servers must declare the same delegation at the same time (this requirement could be lifted if this object was
 * made to behave like a singleton).
 */

UCLASS()
class SPATIALGDK_API USpatialNetDriverDebugContext : public UObject
{
	GENERATED_BODY()
public:
	static void EnableDebugSpatialGDK(USpatialNetDriver* NetDriver);
	static void DisableDebugSpatialGDK(USpatialNetDriver* NetDriver);

	// Startup / Shutdown
	void Init(USpatialNetDriver* NetDriver);
	void Cleanup();
	void Reset();

	// Debug Interface
	void AddActorTag(AActor* Actor, FName Tag);
	void RemoveActorTag(AActor* Actor, FName Tag);
	void AddInterestOnTag(FName Tag);
	void RemoveInterestOnTag(FName Tag);
	void KeepActorOnLocalWorker(AActor* Actor);
	void DelegateTagToWorker(FName Tag, uint32 WorkerId);
	void RemoveTagDelegation(FName Tag);
	TOptional<VirtualWorkerId> GetActorHierarchyExplicitDelegation(const AActor* Actor);

	// Utility
	bool IsActorReady(AActor* Actor);

	// NetDriver Integration
	void TickServer();
	void OnDebugComponentUpdateReceived(Worker_EntityId);
	void OnDebugComponentAuthLost(Worker_EntityId EntityId);

	void ClearNeedEntityInterestUpdate() { bNeedToUpdateInterest = false; }

	SpatialGDK::QueryConstraint ComputeAdditionalEntityQueryConstraint() const;

	UPROPERTY()
	UDebugLBStrategy* DebugStrategy = nullptr;

protected:
	struct DebugComponentView
	{
		SpatialGDK::DebugComponent Component;
		Worker_EntityId Entity = SpatialConstants::INVALID_ENTITY_ID;
		bool bAdded = false;
		bool bDirty = false;
	};

	DebugComponentView& GetDebugComponentView(AActor* Actor);

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

	// Debug info for actors
	TMap<AActor*, DebugComponentView> ActorDebugInfo;

	// Contains a cache of entities computed from the semantic interest.
	TSet<Worker_EntityId_Key> CachedInterestSet;
	bool bNeedToUpdateInterest = false;
};
