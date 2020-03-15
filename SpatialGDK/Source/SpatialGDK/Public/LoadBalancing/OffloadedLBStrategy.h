// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "LoadBalancing/AbstractLBStrategy.h"

#include "CoreMinimal.h"
#include "Math/Box2D.h"
#include "Math/Vector2D.h"

#include "OffloadedLBStrategy.generated.h"

class SpatialVirtualWorkerTranslator;

DECLARE_LOG_CATEGORY_EXTERN(LogOffloadedLBStrategy, Log, All)

/**
 * A load balancing strategy that ...
 */
UCLASS(Blueprintable)
class SPATIALGDK_API UOffloadedLBStrategy : public UAbstractLBStrategy
{
	GENERATED_BODY()

public:
	UOffloadedLBStrategy();

	/* UAbstractLBStrategy Interface */
	virtual void Init() override;

	virtual TSet<VirtualWorkerId> GetVirtualWorkerIds() const override;

	virtual bool ShouldHaveAuthority(const AActor& Actor) const override;
	virtual VirtualWorkerId WhoShouldHaveAuthority(const AActor& Actor) const override;

	virtual SpatialGDK::QueryConstraint GetWorkerInterestQueryConstraint() const override;

	virtual FVector GetWorkerEntityPosition() const override;
	/* End UAbstractLBStrategy Interface */

	void SetWorkerType(const FName& WorkerTypeIn) { LocalWorkerType = WorkerTypeIn; }

protected:

private:

	FName LocalWorkerType;

	TArray<VirtualWorkerId> VirtualWorkerIds;

	mutable TMap<TSoftClassPtr<AActor>, FName> ClassPathToActorGroup;

	TMap<FName, FName> ActorGroupToWorkerType;

	FName DefaultWorkerType;

	// Returns the first ActorGroup that contains this, or a parent of this class,
	// or the default actor group, if no mapping is found.
	FName GetActorGroupForClass(TSubclassOf<AActor> Class) const;

	// Returns the Server worker type that is authoritative over the ActorGroup
	// that contains this class (or parent class). Returns DefaultWorkerType
	// if no mapping is found.
	FName GetWorkerTypeForClass(TSubclassOf<AActor> Class) const;

	// Returns the Server worker type that is authoritative over this ActorGroup.
	FName GetWorkerTypeForActorGroup(const FName& ActorGroup) const;

	// Returns true if ActorA and ActorB are contained in ActorGroups that are
	// on the same Server worker type.
	bool IsSameWorkerType(const AActor* ActorA, const AActor* ActorB) const;

	/**
	 * Returns true if the current Worker Type owns the Actor Group this Actor belongs to.
	 * Equivalent to World->GetNetMode() != NM_Client when Spatial Networking is disabled.
	 */
		bool IsActorGroupOwnerForActor(const AActor* Actor) const;

	/**
	 * Returns true if the current Worker Type owns the Actor Group this Actor Class belongs to.
	 * Equivalent to World->GetNetMode() != NM_Client when Spatial Networking is disabled.
	 */
		 bool IsActorGroupOwnerForClass(const TSubclassOf<AActor> ActorClass) const;

	/**
	 * Returns true if the current Worker Type owns this Actor Group.
	 * Equivalent to World->GetNetMode() != NM_Client when Spatial Networking is disabled.
	 */
		 bool IsActorGroupOwner(const FName ActorGroup) const;

	/**
	 * Returns the ActorGroup this Actor belongs to.
	 */
		 FName GetActorGroupForActor(const AActor* Actor) const;


		 FName GetCurrentWorkerType() const;
};
