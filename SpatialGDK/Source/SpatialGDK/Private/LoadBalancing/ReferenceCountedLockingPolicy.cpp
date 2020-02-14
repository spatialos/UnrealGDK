// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "LoadBalancing/ReferenceCountedLockingPolicy.h"

#include "EngineClasses/AbstractSpatialPackageMapClient.h"
#include "Interop/SpatialStaticComponentView.h"
#include "Schema/AuthorityIntent.h"
#include "Schema/Component.h"
#include "Utils/SpatialActorUtils.h"

#include "Improbable/SpatialEngineDelegates.h"

DEFINE_LOG_CATEGORY(LogReferenceCountedLockingPolicy);

bool UReferenceCountedLockingPolicy::CanAcquireLock(AActor* Actor) const
{
	if (Actor == nullptr)
	{
		UE_LOG(LogReferenceCountedLockingPolicy, Error, TEXT("Failed to lock nullptr actor"));
		return false;
	}

	check(PackageMap.IsValid());
	Worker_EntityId EntityId = PackageMap.Get()->GetEntityIdFromObject(Actor);
	if (EntityId == SpatialConstants::INVALID_ENTITY_ID)
	{
		UE_LOG(LogReferenceCountedLockingPolicy, Error, TEXT("Failed to lock actor without corresponding entity ID. Actor: %s"), *Actor->GetName());
		return false;
	}

	check(StaticComponentView.IsValid());
	const bool bHasAuthority = StaticComponentView.Get()->HasAuthority(EntityId, SpatialConstants::AUTHORITY_INTENT_COMPONENT_ID);
	if (!bHasAuthority)
	{
		UE_LOG(LogReferenceCountedLockingPolicy, Warning, TEXT("Can not lock actor migration. Do not have authority. Actor: %s"), *Actor->GetName());
		return false;
	}

	check(VirtualWorkerTranslator != nullptr);
	const bool bHasAuthorityIntent = VirtualWorkerTranslator->GetLocalVirtualWorkerId() ==
		StaticComponentView->GetComponentData<SpatialGDK::AuthorityIntent>(EntityId)->VirtualWorkerId;
	if (!bHasAuthorityIntent)
	{
		UE_LOG(LogReferenceCountedLockingPolicy, Warning, TEXT("Can not lock actor migration. Authority intent does not match this worker. Actor: %s"), *Actor->GetName());
		return false;
	}
	return true;
}

ActorLockToken UReferenceCountedLockingPolicy::AcquireLock(AActor* Actor, FString DebugString)
{
	if (!CanAcquireLock(Actor))
	{
		UE_LOG(LogReferenceCountedLockingPolicy, Error, TEXT("Called AcquireLock when CanAcquireLock returned false. Actor: %s."), *GetNameSafe(Actor));
		return SpatialConstants::INVALID_ACTOR_LOCK_TOKEN;
	}

	if (MigrationLockElement* ActorLockingState = ActorToLockingState.Find(Actor))
	{
		++ActorLockingState->LockCount;
	}
	else
	{
		// We want to avoid memory leak if a locked actor is deleted.
		// To do this, we register with the Actor OnDestroyed delegate with a function that cleans up the internal map.
		Actor->OnDestroyed.AddDynamic(this, &UReferenceCountedLockingPolicy::OnLockedActorDeleted);

		AActor* HierarchyRoot = SpatialGDK::GetOutermostOwner(Actor);
		UpdateLockedActorHierarchyRootInformation(HierarchyRoot);

		ActorToLockingState.Add(Actor, MigrationLockElement{ 1, HierarchyRoot, [this, Actor]
		{
			Actor->OnDestroyed.RemoveDynamic(this, &UReferenceCountedLockingPolicy::OnLockedActorDeleted);
		} });
	}

	UE_LOG(LogReferenceCountedLockingPolicy, Log, TEXT("Acquiring migration lock. "
		"Actor: %s. Lock name: %s. Token %d: Locks held: %d."), *GetNameSafe(Actor), *DebugString, NextToken, ActorToLockingState.Find(Actor)->LockCount);
	TokenToNameAndActor.Emplace(NextToken, LockNameAndActor{ MoveTemp(DebugString), Actor });
	return NextToken++;
}

bool UReferenceCountedLockingPolicy::ReleaseLock(const ActorLockToken Token)
{
	const LockNameAndActor* NameAndActor = TokenToNameAndActor.Find(Token);
	if (NameAndActor == nullptr)
	{
		UE_LOG(LogReferenceCountedLockingPolicy, Error, TEXT("Called ReleaseLock for unidentified Actor lock token. Token: %d."), Token);
		return false;
	}

	const AActor* Actor = NameAndActor->Actor;
	const FString& Name = NameAndActor->LockName;
	UE_LOG(LogReferenceCountedLockingPolicy, Log, TEXT("Releasing Actor migration lock. Actor: %s. Token: %d. Lock name: %s"), *Actor->GetName(), Token, *Name);

	check(ActorToLockingState.Contains(Actor));

	{
		// Reduce the reference count and erase the entry if reduced to 0.
		auto CountIt = ActorToLockingState.CreateKeyIterator(Actor);
		MigrationLockElement& ActorLockingState = CountIt.Value();
		if (ActorLockingState.LockCount == 1)
		{
			UE_LOG(LogReferenceCountedLockingPolicy, Log, TEXT("Actor migration no longer locked. Actor: %s"), *Actor->GetName());
			ActorLockingState.UnbindActorDeletionDelegateFunc();
			CountIt.RemoveCurrent();

			UpdateReleasedActorHierarchyRootInformation(SpatialGDK::GetOutermostOwner(Actor));
		}
		else
		{
			--ActorLockingState.LockCount;
		}
	}

	TokenToNameAndActor.Remove(Token);

	return true;
}

bool UReferenceCountedLockingPolicy::IsLocked(const AActor* Actor) const
{
	if (IsExplicitlyLocked(Actor))
	{
		return true;
	}

	// If we are the root of a hierarchy tree where some lower hierarchy Actor is locked, then we are locked
	if (LockedHierarchyRootToHierarchyLockCounts.Contains(Actor))
	{
		return true;
	}

	const AActor* HierarchyRoot = SpatialGDK::GetOutermostOwner(Actor);

	// If we have no owner, then we are not locked
	if (HierarchyRoot == nullptr)
	{
		return false;
	}

	// If our hierarchy root is explicitly locked, then we are locked
	if (IsExplicitlyLocked(HierarchyRoot))
	{
		return true;
	}

	// If some Actor in this Actor's hierarchy is locked (our hierarchy root
	// is in the mapping), then we are locked
	if (LockedHierarchyRootToHierarchyLockCounts.Contains(HierarchyRoot))
	{
		return true;
	}

	return false;
}

bool UReferenceCountedLockingPolicy::IsExplicitlyLocked(const AActor* Actor) const
{
	if (Actor == nullptr)
	{
		UE_LOG(LogReferenceCountedLockingPolicy, Warning, TEXT("IsExplicitlyLocked called for nullptr"));
		return false;
	}
	return ActorToLockingState.Contains(Actor);
}

void UReferenceCountedLockingPolicy::OnLockedActorDeleted(AActor* DestroyedActor)
{
	TArray<ActorLockToken> TokensToRemove;
	for (const auto& KeyValuePair : TokenToNameAndActor)
	{
		if (KeyValuePair.Value.Actor == DestroyedActor)
		{
			TokensToRemove.Add(KeyValuePair.Key);
		}
	}
	for (const auto& Token : TokensToRemove)
	{
		TokenToNameAndActor.Remove(Token);
	}
	ActorToLockingState.Remove(DestroyedActor);

	if (AActor* HierarchyRoot = SpatialGDK::GetOutermostOwner(DestroyedActor))
	{
		UpdateReleasedActorHierarchyRootInformation(HierarchyRoot);
	}
}

bool UReferenceCountedLockingPolicy::AcquireLockFromDelegate(AActor* ActorToLock, const FString& DelegateLockIdentifier)
{
	ActorLockToken LockToken = AcquireLock(ActorToLock, DelegateLockIdentifier);
	if (LockToken == SpatialConstants::INVALID_ACTOR_LOCK_TOKEN)
	{
		UE_LOG(LogReferenceCountedLockingPolicy, Error, TEXT("AcquireLock called from engine delegate returned an invalid token"));
		return false;
	}

	check(!DelegateLockingIdentifierToActorLockToken.Contains(DelegateLockIdentifier));
	DelegateLockingIdentifierToActorLockToken.Add(DelegateLockIdentifier, LockToken);
	return true;
}

bool UReferenceCountedLockingPolicy::ReleaseLockFromDelegate(AActor* ActorToRelease, const FString& DelegateLockIdentifier)
{
	if (!DelegateLockingIdentifierToActorLockToken.Contains(DelegateLockIdentifier))
	{
		UE_LOG(LogReferenceCountedLockingPolicy, Error, TEXT("Executed ReleaseLockDelegate for unidentified delegate lock identifier. Token: %s."), *DelegateLockIdentifier);
		return false;
	}
	ActorLockToken LockToken = DelegateLockingIdentifierToActorLockToken.FindAndRemoveChecked(DelegateLockIdentifier);
	bool ReleaseSucceeded = ReleaseLock(LockToken);
	return ReleaseSucceeded;
}

void UReferenceCountedLockingPolicy::OnOwnerUpdated(AActor* ActorChangingOwner)
{
	check(ActorChangingOwner != nullptr);

	// We only care about locked Actors changing owner
	// TODO - accommodate Actors in locked Actor path to hierarchy root changing owner
	if (ActorChangingOwner == nullptr || !IsExplicitlyLocked(ActorChangingOwner))
	{
		return;
	}

	check(ActorToLockingState.Contains(ActorChangingOwner));
	TWeakObjectPtr<AActor> OldHierarchyRoot = ActorToLockingState.Find(ActorChangingOwner)->Root;
	AActor* NewHierarchyRoot = SpatialGDK::GetOutermostOwner(ActorChangingOwner);

	UpdateReleasedActorHierarchyRootInformation(OldHierarchyRoot.Get());
	UpdateLockedActorHierarchyRootInformation(NewHierarchyRoot);
}

void UReferenceCountedLockingPolicy::OnLockedActorOwnerDeleted(AActor* DestroyedActor)
{
	check(DestroyedActor != nullptr);

	// We log an error here because if the locked Actor root was changed before this Actor was destroyed
	// then the OnOwnerUpdated should have unbound this delegate.
	if (!LockedHierarchyRootToHierarchyLockCounts.Contains(DestroyedActor))
	{
		UE_LOG(LogReferenceCountedLockingPolicy, Error, TEXT("Could not find locked Actor root in state in OnDestroyed delegate. Actor: %s"), *DestroyedActor->GetName());
		return;
	}

	LockedHierarchyRootToHierarchyLockCounts.Remove(DestroyedActor);
}

void UReferenceCountedLockingPolicy::UpdateLockedActorHierarchyRootInformation(AActor* LockedRootActor)
{
	// If the Actor is nullptr then we don't need to keep track of anything
	if (LockedRootActor == nullptr)
	{
		return;
	}

	LockedHierarchyRootToHierarchyLockCounts.FindOrAdd(LockedRootActor)++;
	if (!LockedRootActor->OnDestroyed.IsAlreadyBound(this, &UReferenceCountedLockingPolicy::OnLockedActorOwnerDeleted))
	{
		LockedRootActor->OnDestroyed.AddDynamic(this, &UReferenceCountedLockingPolicy::OnLockedActorOwnerDeleted);
	}	
}

void UReferenceCountedLockingPolicy::UpdateReleasedActorHierarchyRootInformation(AActor* ReleasedRootActor)
{
	if (ReleasedRootActor == nullptr)
	{
		return;
	}

	check(LockedHierarchyRootToHierarchyLockCounts.Contains(ReleasedRootActor));

	int32* HierarchyLockCount = LockedHierarchyRootToHierarchyLockCounts.Find(ReleasedRootActor);
	check(*HierarchyLockCount > 0);

	// If the lock count is one, we're releasing the only Actor with this owner, so we can stop caring about it.
	// Otherwise, just decrement it the hierarchy count,
	if (*HierarchyLockCount == 1)
	{
		LockedHierarchyRootToHierarchyLockCounts.Remove(ReleasedRootActor);
		ReleasedRootActor->OnDestroyed.RemoveDynamic(this, &UReferenceCountedLockingPolicy::OnLockedActorDeleted);
	}
	else
	{
		(*HierarchyLockCount)--;
	}
}
