// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "Containers/Map.h"
#include "Containers/UnrealString.h"
#include "GameFramework/Actor.h"

#include "AbstractLockingPolicy.h"
#include "ReferenceCountedLockingPolicy.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogReferenceCountedLockingPolicy, Log, All)

UCLASS()
class SPATIALGDK_API UReferenceCountedLockingPolicy : public UAbstractLockingPolicy
{
	GENERATED_BODY()

public:
	virtual ActorLockToken AcquireLock(const AActor* Actor) override;
	virtual ActorLockToken AcquireLock(const AActor* Actor, FString DebugString) override;

	virtual void ReleaseLock(ActorLockToken Token) override;

	virtual bool IsLocked(const AActor* Actor) const override;

	virtual bool CanAcquireLock(const AActor* Actor) const override;

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
