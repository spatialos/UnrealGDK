// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Schema/DebugComponent.h"
#include "Schema/Interest.h"
#include "SpatialCommonTypes.h"

#include "SpatialNetDriverDebugContext.generated.h"

class UDebugLBStrategy;
class USpatialNetDriver;

UCLASS()
class USpatialNetDriverDebugContext : public UObject
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
	void DelegateTagToWorker(FName Tag, int32 WorkerId);
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
		bool bUpdated = false;
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
