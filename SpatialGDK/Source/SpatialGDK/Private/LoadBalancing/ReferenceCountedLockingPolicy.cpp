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
		if (!Actor->OnDestroyed.IsAlreadyBound(this, &UReferenceCountedLockingPolicy::OnExplicitlyLockedActorDeleted))
		{
			Actor->OnDestroyed.AddDynamic(this, &UReferenceCountedLockingPolicy::OnExplicitlyLockedActorDeleted);
		}

		TArray<AActor*> OwnershipHierarchyPath = SpatialGDK::GetOwnershipHierarchyPath(Actor);
		AddOwnershipHierarchyPathInformation(Actor, OwnershipHierarchyPath);

		ActorToLockingState.Add(Actor, MigrationLockElement{ 1, OwnershipHierarchyPath });
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

	AActor* Actor = NameAndActor->Actor;
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
			Actor->OnDestroyed.RemoveDynamic(this, &UReferenceCountedLockingPolicy::OnExplicitlyLockedActorDeleted);
			RemoveOwnershipHierarchyPathInformation(Actor, ActorLockingState.OwnershipPath);
			CountIt.RemoveCurrent();
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
	if (Actor == nullptr)
	{
		UE_LOG(LogReferenceCountedLockingPolicy, Warning, TEXT("IsLocked called for nullptr"));
		return false;
	}

	// Is this Actor explicitly locked or on a locked hierarchy ownership path
	if (IsExplicitlyLocked(Actor) || IsOnLockedHierarchyPath(Actor))
	{
		return true;
	}

	// Is the hierarchy root of this Actor explicitly locked or on a locked hierarchy ownership path
	TArray<AActor*> OwnershipHierarchyPath = SpatialGDK::GetOwnershipHierarchyPath(Actor);
	if (OwnershipHierarchyPath.Num() > 0)
	{
		AActor* HierarchyRoot = OwnershipHierarchyPath[0];
		return IsExplicitlyLocked(HierarchyRoot) || IsOnLockedHierarchyPath(HierarchyRoot);
	}

	return false;
}

bool UReferenceCountedLockingPolicy::IsExplicitlyLocked(const AActor* Actor) const
{
	return ActorToLockingState.Contains(Actor);
}

bool UReferenceCountedLockingPolicy::IsOnLockedHierarchyPath(const AActor* Actor) const
{
	return LockedOwnershipPathActorToExplicitlyLockedActors.Contains(Actor);
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

	const bool bActorExplicitlyLocked = IsExplicitlyLocked(ActorChangingOwner);
	const bool bActorIsOnLockedHierarchyPath = IsOnLockedHierarchyPath(ActorChangingOwner);

	// We only care about Actors changing owner that are some part of a locked hierarchy
	if (!bActorExplicitlyLocked && !bActorIsOnLockedHierarchyPath)
	{
		return;
	}

	// If an explicitly locked Actor is changing owner
	if (bActorExplicitlyLocked)
	{
		ResetLockedActorOwnershipHierarchyInformation(ActorChangingOwner);
	}

	// If an Actor inside the hierarchy is changing owner to some Actor, we need to find which explicitly locked Actors
	// have this Actor on their hierarchy path, and update their hierarchy information accordingly
	if (bActorIsOnLockedHierarchyPath)
	{
		TArray<const AActor*>& ExplicitlyLockedActorsWithThisActorOnPath = LockedOwnershipPathActorToExplicitlyLockedActors.FindChecked(ActorChangingOwner);
		for (const AActor* LockedActor : ExplicitlyLockedActorsWithThisActorOnPath)
		{
			ResetLockedActorOwnershipHierarchyInformation(LockedActor);
		}
	}
}

void UReferenceCountedLockingPolicy::OnExplicitlyLockedActorDeleted(AActor* DestroyedActor)
{
	//	Find all tokens for this Actor and unlock
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

	//	Delete Actor from local mapping
	MigrationLockElement ActorLockingState = ActorToLockingState.FindAndRemoveChecked(DestroyedActor);

	// Update ownership path Actor mapping to remove this Actor
	RemoveOwnershipHierarchyPathInformation(DestroyedActor, ActorLockingState.OwnershipPath);
}

void UReferenceCountedLockingPolicy::OnOwnershipPathActorDeleted(AActor* DestroyedOwnershipPathActor)
{
	check(LockedOwnershipPathActorToExplicitlyLockedActors.Contains(DestroyedOwnershipPathActor));

	// For all explicitly locked Actors where this Actor is on the ownership path, recalculate the
	// ownership path information to account for this Actor's deletion.
	TArray<const AActor*> ExplicitlyLockedActorsWithThisActorInOwnershipPath = LockedOwnershipPathActorToExplicitlyLockedActors.FindAndRemoveChecked(DestroyedOwnershipPathActor);
	for (const AActor* ExplicitlyLockedActor : ExplicitlyLockedActorsWithThisActorInOwnershipPath)
	{
		check(ActorToLockingState.Contains(ExplicitlyLockedActor));
		ResetLockedActorOwnershipHierarchyInformation(ExplicitlyLockedActor);
	}

	check(!LockedOwnershipPathActorToExplicitlyLockedActors.Contains(DestroyedOwnershipPathActor));
}

void UReferenceCountedLockingPolicy::ResetLockedActorOwnershipHierarchyInformation(const AActor* ExplicitlyLockedActor, const AActor* DeletedHierarchyActor)
{
	// For the old ownership path, update ownership path Actor mapping to explicitly locked Actors to remove this Actor.
	TArray<AActor*> OldOwnershipHierarchyPath = ActorToLockingState.FindChecked(ExplicitlyLockedActor).OwnershipPath;
	RemoveOwnershipHierarchyPathInformation(ExplicitlyLockedActor, OldOwnershipHierarchyPath);

	// For the new ownership path, update ownership path Actor mapping to explicitly locked Actors to include this Actor.
	TArray<AActor*> NewOwnershipHierarchyPath = SpatialGDK::GetOwnershipHierarchyPath(ExplicitlyLockedActor, DeletedHierarchyActor);
	ActorToLockingState.FindChecked(ExplicitlyLockedActor).OwnershipPath = NewOwnershipHierarchyPath;
	AddOwnershipHierarchyPathInformation(ExplicitlyLockedActor, NewOwnershipHierarchyPath);
}

void UReferenceCountedLockingPolicy::RemoveOwnershipHierarchyPathInformation(const AActor* ExplicitlyLockedActor, TArray<AActor*> OwnershipHierarchyPath)
{
	for (AActor* OwnershipPathActor : OwnershipHierarchyPath)
	{
		// Find Actors in this root Actor's hierarchy which are explicitly locked.
		TArray<const AActor*>& ExplicitlyLockedActorsWithThisActorOnPath = LockedOwnershipPathActorToExplicitlyLockedActors.FindOrAdd(OwnershipPathActor);
		check(ExplicitlyLockedActorsWithThisActorOnPath.Num() > 0);

		// If there's only one explicitly locked Actor in the hierarchy, we're removing the only Actor with this owner,
		// so we can stop caring about the root itself. Otherwise, just remove the specific Actor entry it the root's list.
		if (ExplicitlyLockedActorsWithThisActorOnPath.Num() == 1)
		{
			LockedOwnershipPathActorToExplicitlyLockedActors.Remove(OwnershipPathActor);
			OwnershipPathActor->OnDestroyed.RemoveDynamic(this, &UReferenceCountedLockingPolicy::OnOwnershipPathActorDeleted);
		}
		else
		{
			ExplicitlyLockedActorsWithThisActorOnPath.Remove(ExplicitlyLockedActor);
		}
	}
}

void UReferenceCountedLockingPolicy::AddOwnershipHierarchyPathInformation(const AActor* ExplicitlyLockedActor, TArray<AActor*> OwnershipHierarchyPath)
{
	for (AActor* OwnershipPathActor : OwnershipHierarchyPath)
	{
		TArray<const AActor*>& ExplicitlyLockedActorsWithThisActorOnPath = LockedOwnershipPathActorToExplicitlyLockedActors.FindOrAdd(OwnershipPathActor);
		ExplicitlyLockedActorsWithThisActorOnPath.AddUnique(ExplicitlyLockedActor);

		if (!OwnershipPathActor->OnDestroyed.IsAlreadyBound(this, &UReferenceCountedLockingPolicy::OnOwnershipPathActorDeleted))
		{
			OwnershipPathActor->OnDestroyed.AddDynamic(this, &UReferenceCountedLockingPolicy::OnOwnershipPathActorDeleted);
		}
	}
}
