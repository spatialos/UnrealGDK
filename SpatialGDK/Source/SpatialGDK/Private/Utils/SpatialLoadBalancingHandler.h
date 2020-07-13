// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "EngineClasses/SpatialNetDriver.h"
#include "Utils/SpatialActorUtils.h"

// Utility class to handle load balancing for a collection of Actors.
// Since different systems (NetDriver, ReplicationGraph) have different types for Actor collection
// and different ways to declare dependencies between Actors, a traits pattern is used to have a
// context object provide access to these information.
// The methods to provide on the context object are :
//  - GetActorsBeingReplicated() -> returns a range-for compatible iterator over AActor being replicated this frame
//  - AddActorToReplicate(AActor* Actor) -> inform that an additional Actor should be replicated this frame (a dependent Actor)
//  - RemoveAdditionalActor(AActor* Actor) -> inform that an Actor we signaled as needing to replicate was encountered in the list of replicated Actor, and does not need additional handling
//  - GetDependentActors(AActor* Actor) -> returns a range-for compatible iterator over AActor that depends on the given Actor (and should be migrated together)
//  - IsActorReadyForMigration(AActor* Actor) -> called on dependent actors, to check if they are ready to be replicated and migrated
//
// Example iteration context : 
//
//struct FSpatialNetDriverLoadBalancingContext
//{
//	FSpatialNetDriverLoadBalancingContext(TArray<AActor*>& InReplicatedActors)
//		: ReplicatedActors(InReplicatedActors)
//	{
//
//	}
//
//	TArray<AActor*>&  GetActorsBeingReplicated()
//	{
//		return ReplicatedActors;
//	}
//
//	void RemoveAdditionalActor(AActor* Actor)
//	{
//		AdditionalActorsToReplicate.Remove(Actor);
//	}
//
//	void AddActorToReplicate(AActor* Actor)
//	{
//		AdditionalActorsToReplicate.Add(Actor);
//	}
//
//	TArray<AActor*>& GetDependentActors(AActor* Actor)
//	{
//		return Actor->Children;
//	}
//
//	void UpdateWithAdditionalActors()
//	{
//		for (AActor* AdditionalActor : AdditionalActorsToReplicate)
//		{
//			ReplicatedActors.Add(AdditionalActor);
//		}
//	}
//
//	bool IsActorReadyForMigration(AActor* Actor)
//	{
//		return Actor->HasAuthority();
//	}
//
//protected:
//
//	TSet<AActor*> AdditionalActorsToReplicate;
//
//	TArray<AActor*>& ReplicatedActors;
//};

DECLARE_LOG_CATEGORY_EXTERN(LogSpatialLoadBalancingHandler, Log, All);

class FSpatialLoadBalancingHandler
{
public:

	FSpatialLoadBalancingHandler(USpatialNetDriver* InNetDriver);

	// Iterates over the list of actors to replicate, to check if they should migrate to another worker
	// and collects additional actors to replicate if needed.
	template <typename ReplicationContext>
	void EvaluateActorsToMigrate(ReplicationContext& iCtx)
	{
		check(NetDriver->LoadBalanceStrategy != nullptr);
		check(NetDriver->LockingPolicy != nullptr);

		for (AActor* Actor : iCtx.GetActorsBeingReplicated())
		{
			AActor* NetOwner;
			VirtualWorkerId NewAuthWorkerId;
			EvaluateActorResult Result = EvaluateSingleActor(Actor, NetOwner, NewAuthWorkerId);
			switch (Result)
			{
			case EvaluateActorResult::Migrate:
				if (CollectActorsToMigrate(iCtx, NetOwner, NetOwner->HasAuthority()))
				{
					for (AActor* ActorToMigrate : TempActorsToMigrate)
					{
						if (ActorToMigrate != Actor)
						{
							iCtx.AddActorToReplicate(ActorToMigrate);
						}
						ActorsToMigrate.Add(ActorToMigrate, NewAuthWorkerId);
					}
				}
				TempActorsToMigrate.Empty();
				break;
			case EvaluateActorResult::RemoveAdditional:
				iCtx.RemoveAdditionalActor(Actor);
				break;
			}
		}
	}

	const TMap<AActor*, VirtualWorkerId>& GetActorsToMigrate() const
	{
		return ActorsToMigrate;
	}

	// Sends the migration instructions and update actor authority.
	void ProcessMigrations();

protected:
	
	void UpdateSpatialDebugInfo(AActor* Actor, Worker_EntityId EntityId) const;

	uint64 GetLatestAuthorityChangeFromHierarchy(const AActor* HierarchyActor) const;

	enum class EvaluateActorResult
	{
		None,	// Actor not concerned by load balancing
		Migrate,	// Actor should migrate
		RemoveAdditional	// Actor is already marked as migrating.
	};

	EvaluateActorResult EvaluateSingleActor(AActor* Actor, AActor*& OutNetOwner, VirtualWorkerId& OutWorkerId);

	template <typename ReplicationContext>
	bool CollectActorsToMigrate(ReplicationContext& iCtx, AActor* Actor, bool bNetOwnerHasAuth)
	{
		if(Actor->GetIsReplicated())
		{
			if (!iCtx.IsActorReadyForMigration(Actor))
			{
				// Prevents an Actor hierarchy from migrating if one of its actor is not ready.
				// Child Actors are always allowed to join the owner.
				// This is a band aid to prevent Actors from being left behind,
				// although it has the risk of creating an infinite lock if the child is unable to become ready.
				if(bNetOwnerHasAuth)
				{
					AActor* HierarchyRoot = SpatialGDK::GetHierarchyRoot(Actor);
					UE_LOG(LogSpatialLoadBalancingHandler, Warning,
						TEXT("Prevented Actor %s 's hierarchy from migrating because Actor %s is not ready."),
						*HierarchyRoot->GetName(),
						*Actor->GetName());

					return false;
				}
			}
			else
			{
				TempActorsToMigrate.Add(Actor);
			}
		}

		for (AActor* Child : iCtx.GetDependentActors(Actor))
		{
			if (!CollectActorsToMigrate(iCtx, Child, bNetOwnerHasAuth))
			{
				return false;
			}
		}

		return true;
	}

	USpatialNetDriver* NetDriver;

	TMap<AActor*, VirtualWorkerId> ActorsToMigrate;
	TSet<AActor*> TempActorsToMigrate;
};
