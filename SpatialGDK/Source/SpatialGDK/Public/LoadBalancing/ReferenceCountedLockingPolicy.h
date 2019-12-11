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
	virtual bool CanAcquireLock(const AActor* Actor) const override;

	virtual ActorLockToken AcquireLock(const AActor* Actor, FString DebugString = "") override;

	// This should only be called during the lifetime of the locked actor
	virtual void ReleaseLock(ActorLockToken Token) override;

	virtual bool IsLocked(const AActor* Actor) const override;

private:
	struct LockNameAndActor
	{
		FString LockName;
		const AActor* Actor;
	};

	TMap<const AActor*, int32> ActorToReferenceCount;
	TMap<ActorLockToken, LockNameAndActor> TokenToNameAndActor;

	ActorLockToken NextToken = 1;
};
