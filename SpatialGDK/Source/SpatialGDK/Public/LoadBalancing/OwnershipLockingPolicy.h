// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "AbstractLockingPolicy.h"

#include "Containers/Map.h"
#include "Containers/UnrealString.h"
#include "GameFramework/Actor.h"
#include "UObject/WeakObjectPtr.h"

#include "OwnershipLockingPolicy.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogOwnershipLockingPolicy, Log, All)

UCLASS()
class SPATIALGDK_API UOwnershipLockingPolicy : public UAbstractLockingPolicy
{
	GENERATED_BODY()

public:
	virtual ActorLockToken AcquireLock(AActor* Actor, FString DebugString) override;
	// This should only be called during the lifetime of the locked Actor
	virtual bool ReleaseLock(const ActorLockToken Token) override;
	virtual bool IsLocked(const AActor* Actor) const override;
	virtual int32 GetActorLockCount(const AActor* Actor) const override;
	virtual void OnOwnerUpdated(const AActor* Actor, const AActor* OldOwner) override;

private:
	struct MigrationLockElement
	{
		int32 LockCount;
		AActor* HierarchyRoot;
	};

	struct LockNameAndActor
	{
		const FString LockName;
		AActor* Actor;
	};

	static bool CanAcquireLock(const AActor* Actor);
	bool IsExplicitlyLocked(const AActor* Actor) const;
	bool IsLockedHierarchyRoot(const AActor* Actor) const;

	UFUNCTION()
	void OnExplicitlyLockedActorDeleted(AActor* DestroyedActor);

	UFUNCTION()
	void OnHierarchyRootActorDeleted(AActor* DestroyedActorRoot);

	virtual bool AcquireLockFromDelegate(AActor* ActorToLock, const FString& DelegateLockIdentifier) override;
	virtual bool ReleaseLockFromDelegate(AActor* ActorToRelease, const FString& DelegateLockIdentifier) override;

	void RecalculateAllExplicitlyLockedActorsInThisHierarchy(const AActor* HierarchyRoot);
	void RecalculateLockedActorOwnershipHierarchyInformation(const AActor* ExplicitlyLockedActor);
	void AddOwnershipHierarchyRootInformation(AActor* HierarchyRoot, const AActor* ExplicitlyLockedActor);
	void RemoveOwnershipHierarchyRootInformation(AActor* HierarchyRoot, const AActor* ExplicitlyLockedActor);

	TMap<const AActor*, MigrationLockElement> ActorToLockingState;
	TMap<ActorLockToken, LockNameAndActor> TokenToNameAndActor;
	TMap<FString, ActorLockToken> DelegateLockingIdentifierToActorLockToken;
	TMap<const AActor*, TArray<const AActor*>> LockedOwnershipRootActorToExplicitlyLockedActors;

	ActorLockToken NextToken = 1;
};
