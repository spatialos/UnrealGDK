// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "AbstractLockingPolicy.h"
#include "Containers/Map.h"
#include "Containers/UnrealString.h"
#include "GameFramework/Actor.h"

#include "ReferenceCountedLockingPolicy.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogReferenceCountedLockingPolicy, Log, All)

UCLASS()
class SPATIALGDK_API UReferenceCountedLockingPolicy : public UAbstractLockingPolicy
{
	GENERATED_BODY()

public:
	virtual ActorLockToken AcquireLock(AActor* Actor, FString DebugString = "") override;

	// This should only be called during the lifetime of the locked actor
	virtual void ReleaseLock(ActorLockToken Token) override;

	virtual bool IsLocked(const AActor* Actor) const override;

private:
	struct LockingState
	{
		int32 LockCount;
		
		TFunction<void()> UnbindActorDeletionDelegateFunc;
	};

	struct LockNameAndActor
	{
		FString LockName;
		const AActor* Actor;
	};

	UFUNCTION()
	void OnLockedActorDeleted(AActor* DestroyedActor);

	bool CanAcquireLock(AActor* Actor) const;

	TMap<const AActor*, LockingState> ActorToLockingState;
	TMap<ActorLockToken, LockNameAndActor> TokenToNameAndActor;

	ActorLockToken NextToken = 1;
};
