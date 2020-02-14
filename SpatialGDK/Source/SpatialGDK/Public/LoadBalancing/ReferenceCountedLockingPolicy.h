// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "AbstractLockingPolicy.h"

#include "Containers/Map.h"
#include "Containers/UnrealString.h"
#include "GameFramework/Actor.h"
#include "UObject/WeakObjectPtr.h"

#include "ReferenceCountedLockingPolicy.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogReferenceCountedLockingPolicy, Log, All)

UCLASS()
class SPATIALGDK_API UReferenceCountedLockingPolicy : public UAbstractLockingPolicy
{
	GENERATED_BODY()

public:
	virtual ActorLockToken AcquireLock(AActor* Actor, FString DebugString = "") override;

	// This should only be called during the lifetime of the locked Actor
	virtual bool ReleaseLock(const ActorLockToken Token) override;

	virtual bool IsLocked(const AActor* Actor) const override;

	virtual void OnOwnerUpdated(AActor* Actor) override;

private:
	struct MigrationLockElement
	{
		int32 LockCount;
		TArray<AActor*> OwnershipPath;
	};

	struct LockNameAndActor
	{
		FString LockName;
		AActor* Actor;
	};

	bool CanAcquireLock(AActor* Actor) const;
	bool IsExplicitlyLocked(const AActor* Actor) const;
	bool IsOnLockedHierarchyPath(const AActor* Actor) const;

	UFUNCTION()
	void OnExplicitlyLockedActorDeleted(AActor* DestroyedActor);

	UFUNCTION()
	void OnOwnershipPathActorDeleted(AActor* DestroyedActorRoot);

	virtual bool AcquireLockFromDelegate(AActor* ActorToLock,    const FString& DelegateLockIdentifier) override;
	virtual bool ReleaseLockFromDelegate(AActor* ActorToRelease, const FString& DelegateLockIdentifier) override;

	void ResetLockedActorOwnershipHierarchyInformation(const AActor* ExplicitlyLockedActor, const AActor* DeletedHierarchyActor = nullptr);
	void AddOwnershipHierarchyPathInformation(const AActor* ExplicitlyLockedActor, TArray<AActor*> OwnershipHierarchyPath);
	void RemoveOwnershipHierarchyPathInformation(TArray<AActor*> OwnershipHierarchyPath);

	TMap<const AActor*, MigrationLockElement> ActorToLockingState;
	TMap<ActorLockToken, LockNameAndActor> TokenToNameAndActor;
	TMap<FString, ActorLockToken> DelegateLockingIdentifierToActorLockToken;
	TMap<AActor*, TArray<const AActor*>> LockedOwnershipPathActorToExplicitlyLockedActors;

	ActorLockToken NextToken = 1;
};
