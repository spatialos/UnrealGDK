// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "LoadBalancing/ReferenceCountedLockingPolicy.h"
#include "GameFramework/Actor.h"

#include "EngineClasses/SpatialNetDriver.h"
#include "EngineClasses/SpatialPackageMapClient.h"
#include "Interop/SpatialStaticComponentView.h"
#include "Schema/AuthorityIntent.h"

DEFINE_LOG_CATEGORY(LogReferenceCountedLockingPolicy);

ActorLockToken UReferenceCountedLockingPolicy::AcquireLock(const AActor* Actor)
{
	check(Actor != nullptr);
	check(CanAcquireLock(Actor))
	++ActorToReferenceCount.FindOrAdd(Actor);
	TokenToNameAndActor.Emplace(NextToken, LockNameAndActor{ FString(), Actor });
	UE_LOG(LogReferenceCountedLockingPolicy, Log, TEXT("Acquiring migration lock. Actor: %s. Locks held: %d."), *Actor->GetName(), *ActorToReferenceCount.Find(Actor));
	return NextToken++;
}

ActorLockToken UReferenceCountedLockingPolicy::AcquireLock(const AActor* Actor, FString DebugString)
{
	check(Actor != nullptr);
	check(CanAcquireLock(Actor))
	++ActorToReferenceCount.FindOrAdd(Actor);
	UE_LOG(LogReferenceCountedLockingPolicy, Log, TEXT("Acquiring migration lock. "
		"Actor: %s. Lock name: %s. Token %d: Locks held: %d."), *Actor->GetName(), *DebugString, NextToken, *ActorToReferenceCount.Find(Actor));
	TokenToNameAndActor.Emplace(NextToken, LockNameAndActor{ MoveTemp(DebugString), Actor });
	return NextToken++;
}

bool UReferenceCountedLockingPolicy::CanAcquireLock(const AActor* Actor) const
{
	const auto* NetDriver = Cast<USpatialNetDriver>(Actor->GetWorld()->GetNetDriver());
	const auto EntityId = NetDriver->PackageMap->GetEntityIdFromObject(Actor);

	const bool HasAuthIntent = NetDriver->VirtualWorkerTranslator->GetLocalVirtualWorkerId() ==
			NetDriver->StaticComponentView->GetComponentData<SpatialGDK::AuthorityIntent>(EntityId)->VirtualWorkerId;
	const bool HasAuth = NetDriver->StaticComponentView->GetAuthority(EntityId, SpatialConstants::AUTHORITY_INTENT_COMPONENT_ID) == WORKER_AUTHORITY_AUTHORITATIVE;
	if (!HasAuth)
	{
		UE_LOG(LogReferenceCountedLockingPolicy, Log, TEXT("Can not lock actor migration. Do not have authority. Actor: %s"), *Actor->GetName());
	}
	if (!HasAuthIntent)
	{
		UE_LOG(LogReferenceCountedLockingPolicy, Log, TEXT("Can not lock actor migration. Authority intent does not match this worker. Actor: %s"), *Actor->GetName());
	}
	return HasAuthIntent && HasAuth;
}

void UReferenceCountedLockingPolicy::ReleaseLock(ActorLockToken Token)
{
	const auto NameAndActor = TokenToNameAndActor.FindAndRemoveChecked(Token);
	const AActor* Actor = NameAndActor.Actor;
	const FString& Name = NameAndActor.LockName;
	UE_LOG(LogReferenceCountedLockingPolicy, Log, TEXT("Releasing actor migration lock. Actor: %s. Token: %d. Lock name: %s"), *Actor->GetName(), Token, *Name);

	check(ActorToReferenceCount.Contains(Actor));

	{
		// Reduce the reference count and erase the entry if reduced to 0.
		auto CountIt = ActorToReferenceCount.CreateKeyIterator(Actor);
		auto& Count = CountIt.Value();
		if (Count == 1)
		{
			UE_LOG(LogReferenceCountedLockingPolicy, Log, TEXT("Actor migration no longer lock. Actor: %s"), *Actor->GetName(), *Name);
			CountIt.RemoveCurrent();
		}
		else
		{
			--Count;
		}
	}
}

bool UReferenceCountedLockingPolicy::IsLocked(const AActor* Actor) const
{
	check(Actor != nullptr);
	return ActorToReferenceCount.Contains(Actor);
}
