// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "Utils/LockingStatics.h"

#include "EngineClasses/SpatialNetDriver.h"
#include "LoadBalancing/AbstractLockingPolicy.h"
#include "SpatialConstants.h"
#include "Utils/SpatialStatics.h"

#include "Containers/UnrealString.h"

DEFINE_LOG_CATEGORY(LogLocking);

namespace
{
bool CanProcessActor(const AActor* Actor)
{
	if (Actor == nullptr)
	{
		UE_LOG(LogLocking, Error, TEXT("Calling locking API functions on nullptr Actor is invalid."));
		return false;
	}

	const UNetDriver* NetDriver = Actor->GetWorld()->GetNetDriver();
	if (!NetDriver->IsServer())
	{
		UE_LOG(LogLocking, Error, TEXT("Calling locking API functions on a client is invalid. Actor: %s"), *GetNameSafe(Actor));
		return false;
	}

	if (!Actor->HasAuthority())
	{
		UE_LOG(LogLocking, Error, TEXT("Calling locking API functions on a non-auth Actor is invalid. Actor: %s."),
			*GetNameSafe(Actor));
		return false;
	}

	if (!USpatialStatics::IsSpatialMultiWorkerEnabled(Actor->GetWorld()))
	{
		return false;
	}

	return true;
}
} // anonymous namespace

ActorLockToken ULockingStatics::AcquireLock(AActor* Actor)
{
	if (!CanProcessActor(Actor))
	{
		return SpatialConstants::INVALID_ACTOR_LOCK_TOKEN;
	}

	UAbstractLockingPolicy* LockingPolicy = Cast<USpatialNetDriver>(Actor->GetWorld()->GetNetDriver())->LockingPolicy;

	const uint32 NewLockCount = LockingPolicy->GetActorLockCount(Actor) + 1;

	const ActorLockToken LockToken = LockingPolicy->AcquireLock(Actor,
		FString::Printf(TEXT("Actor %s locked. Now locked %d times."), *Actor->GetName(), NewLockCount));

	UE_LOG(LogLocking, Verbose, TEXT("LockingComponent called AcquireLock. Actor: %s. Token: %lld. New lock count: %d"),
		*Actor->GetName(), LockToken, NewLockCount);

	return LockToken;
}

bool ULockingStatics::IsLocked(const AActor* Actor)
{
	if (!CanProcessActor(Actor))
	{
		return false;
	}

	const USpatialNetDriver* NetDriver = Cast<USpatialNetDriver>(Actor->GetWorld()->GetNetDriver());

	return NetDriver->LockingPolicy->IsLocked(Actor);
}

void ULockingStatics::ReleaseLock(const AActor* Actor, ActorLockToken LockToken)
{
	if (!CanProcessActor(Actor))
	{
		return;
	}

	UAbstractLockingPolicy* LockingPolicy = Cast<USpatialNetDriver>(Actor->GetWorld()->GetNetDriver())->LockingPolicy;
	LockingPolicy->ReleaseLock(LockToken);

    UE_LOG(LogLocking, Verbose, TEXT("LockingComponent called ReleaseLock. Actor: %s. Token: %lld. Resulting lock count: %d"),
        *Actor->GetName(), LockToken, LockingPolicy->GetActorLockCount(Actor));
}
