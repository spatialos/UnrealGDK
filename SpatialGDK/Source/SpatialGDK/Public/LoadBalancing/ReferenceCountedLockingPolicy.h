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
		TWeakObjectPtr<AActor> Root;
		TFunction<void()> UnbindActorDeletionDelegateFunc;
	};

	struct LockNameAndActor
	{
		FString LockName;
		const AActor* Actor;
	};

	bool IsExplicitlyLocked(const AActor* Actor) const;

	UFUNCTION()
	void OnLockedActorDeleted(AActor* DestroyedActor);

	UFUNCTION()
	void OnLockedActorOwnerDeleted(AActor* DestroyedActorRoot);

	bool CanAcquireLock(AActor* Actor) const;

	virtual bool AcquireLockFromDelegate(AActor* ActorToLock,    const FString& DelegateLockIdentifier) override;
	virtual bool ReleaseLockFromDelegate(AActor* ActorToRelease, const FString& DelegateLockIdentifier) override;

	void UpdateLockedActorHierarchyRootInformation(AActor* LockedRootActor);
	void UpdateReleasedActorHierarchyRootInformation(AActor* ReleasedRootActor);

	TMap<const AActor*, MigrationLockElement> ActorToLockingState;
	TMap<ActorLockToken, LockNameAndActor> TokenToNameAndActor;
	TMap<FString, ActorLockToken> DelegateLockingIdentifierToActorLockToken;
	TMap<AActor*, int32> LockedHierarchyRootToHierarchyLockCounts;

	ActorLockToken NextToken = 1;
};
