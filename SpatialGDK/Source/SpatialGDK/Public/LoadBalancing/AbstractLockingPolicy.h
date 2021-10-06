// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "SpatialConstants.h"

#include "GameFramework/Actor.h"
#include "Improbable/SpatialEngineDelegates.h"
#include "Templates/SharedPointer.h"
#include "UObject/WeakObjectPtrTemplates.h"

#include "AbstractLockingPolicy.generated.h"

UCLASS(abstract)
class SPATIALGDK_API UAbstractLockingPolicy : public UObject
{
	GENERATED_BODY()

public:
	virtual void Init(SpatialDelegates::FAcquireLockDelegate& AcquireLockDelegate,
					  SpatialDelegates::FReleaseLockDelegate& ReleaseLockDelegate)
	{
		AcquireLockDelegate.BindUObject(this, &UAbstractLockingPolicy::AcquireLockFromDelegate);
		ReleaseLockDelegate.BindUObject(this, &UAbstractLockingPolicy::ReleaseLockFromDelegate);
	};
	virtual ActorLockToken AcquireLock(AActor* Actor, FString LockName = TEXT(""))
		PURE_VIRTUAL(UAbstractLockingPolicy::AcquireLock, return SpatialConstants::INVALID_ACTOR_LOCK_TOKEN;);
	virtual bool ReleaseLock(const ActorLockToken Token) PURE_VIRTUAL(UAbstractLockingPolicy::ReleaseLock, return false;);
	virtual bool IsLocked(const AActor* Actor) const PURE_VIRTUAL(UAbstractLockingPolicy::IsLocked, return false;);
	virtual int32 GetActorLockCount(const AActor* Actor) const PURE_VIRTUAL(UAbstractLockingPolicy::GetActorLockCount, return 0;);
	virtual void OnOwnerUpdated(const AActor* Actor, const AActor* OldOwner) PURE_VIRTUAL(UAbstractLockingPolicy::OnOwnerUpdated, return;);

private:
	virtual bool AcquireLockFromDelegate(AActor* ActorToLock, const FString& DelegateLockIdentifier)
		PURE_VIRTUAL(UAbstractLockingPolicy::AcquireLockFromDelegate, return false;);
	virtual bool ReleaseLockFromDelegate(AActor* ActorToRelease, const FString& DelegateLockIdentifier)
		PURE_VIRTUAL(UAbstractLockingPolicy::ReleaseLockFromDelegate, return false;);
};
