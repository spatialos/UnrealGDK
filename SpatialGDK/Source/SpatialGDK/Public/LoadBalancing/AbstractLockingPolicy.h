#pragma once

#include "GameFramework/Actor.h"

#include "SpatialConstants.h"

#include "AbstractLockingPolicy.generated.h"

UCLASS(abstract)
class SPATIALGDK_API UAbstractLockingPolicy : public UObject
{
	GENERATED_BODY()

public:
	virtual bool CanAcquireLock(const AActor* Actor) const PURE_VIRTUAL(UAbstractLockingPolicy::CanAcquireLock, return false;);
	virtual ActorLockToken AcquireLock(const AActor* Actor, FString LockName = "") PURE_VIRTUAL(UAbstractLockingPolicy::AcquireLock, return SpatialConstants::INVALID_ACTOR_LOCK_TOKEN;);
	virtual void ReleaseLock(ActorLockToken) PURE_VIRTUAL(UAbstractLockingPolicy::ReleaseLock, return;);
	virtual bool IsLocked(const AActor* Actor) const PURE_VIRTUAL(UAbstractLockingPolicy::IsLocked, return false;);
};
