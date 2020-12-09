// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "EngineClasses/SpatialNetDriver.h"
#include "EngineClasses/SpatialPackageMapClient.h"
#include "Utils/SpatialActorUtils.h"

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

	const TMap<AActor*, VirtualWorkerId>& GetActorsToMigrate() const { return ActorsToMigrate; }

	// Sends the migration instructions and update actor authority.
	void ProcessMigrations();

	enum class EvaluateActorResult
	{
		None,			 // Actor not concerned by load balancing
		Migrate,		 // Actor should migrate
		RemoveAdditional // Actor is already marked as migrating.
	};

	EvaluateActorResult EvaluateSingleActor(AActor* Actor, AActor*& OutNetOwner, VirtualWorkerId& OutWorkerId);

protected:
	void UpdateSpatialDebugInfo(AActor* Actor, Worker_EntityId EntityId) const;

	uint64 GetLatestAuthorityChangeFromHierarchy(const AActor* HierarchyActor) const;

	template <typename ReplicationContext>
	bool CollectActorsToMigrate(ReplicationContext& iCtx, AActor* Actor, bool bNetOwnerHasAuth)
	{
		if (Actor->GetIsReplicated())
		{
			EActorMigrationResult ActorMigration = iCtx.IsActorReadyForMigration(Actor);
			if (ActorMigration != EActorMigrationResult::Success)
			{
				// Prevents an Actor hierarchy from migrating if one of its actor is not ready.
				// Child Actors are always allowed to join the owner.
				// This is a band aid to prevent Actors from being left behind,
				// although it has the risk of creating an infinite lock if the child is unable to become ready.
				if (bNetOwnerHasAuth)
				{
					LogMigrationFailure(ActorMigration, Actor);
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

	void LogMigrationFailure(EActorMigrationResult ActorMigrationResult, AActor* Actor);

	USpatialNetDriver* NetDriver;

	TMap<AActor*, VirtualWorkerId> ActorsToMigrate;
	TSet<AActor*> TempActorsToMigrate;
};
