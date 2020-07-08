// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "EngineClasses/SpatialNetDriver.h"
#include "EngineClasses/SpatialActorChannel.h"
#include "EngineClasses/SpatialPackageMapClient.h"
#include "Interop/SpatialSender.h"
#include "LoadBalancing/AbstractLBStrategy.h"
#include "LoadBalancing/OwnershipLockingPolicy.h"
#include "Schema/SpatialDebugging.h"
#include "Utils/SpatialActorUtils.h"

// Template class to handle load balancing for a collection of Actors.
// Since different systems (NetDriver, ReplicationGraph) have different types for Actor collection
// and different ways to declare dependencies between Actors, a CRTP pattern is used to have the
// implementation class provide access to these information.
// The methods to provide are :
//  - GetActorsBeingReplicated() -> returns a range-for compatible iterator over AActor being replicated this frame
//  - AddActorToReplicate(AActor* Actor) -> inform that an additional Actor should be replicated this frame (a dependent Actor)
//  - RemoveAdditionalActor(AActor* Actor) -> inform that an Actor we signaled as needing to replicate was encountered in the list of replicated Actor, and does not need additional handling
//  - GetDependentActors(AActor* Actor) -> returns a range-for compatible iterator over AActor that depends on the given Actor (and should be migrated together)
template <typename Implementation>
class TSpatialLoadBalancingHandler
{
public:

	TSpatialLoadBalancingHandler(USpatialNetDriver* InNetDriver)
		: NetDriver(InNetDriver)
	{

	}

	// Iterates over the list of actors to replicate, to check if they should migrate to another worker
	// and collects additional actors to replicate if needed.
	void HandleLoadBalancing()
	{
		check(NetDriver->LoadBalanceStrategy != nullptr);
		check(NetDriver->LockingPolicy != nullptr);

		for (AActor* Actor : static_cast<Implementation*>(this)->GetActorsBeingReplicated())
		{
			const Worker_EntityId EntityId = NetDriver->PackageMap->GetEntityIdFromObject(Actor);
			if (EntityId == SpatialConstants::INVALID_ENTITY_ID)
			{
				continue;
			}

			if (!Actor->HasAuthority())
			{
				continue;
			}

			UpdateActorSpatialDebugging(Actor, EntityId);

			// If this object is in the list of actors to migrate, we have already processed its hierarchy.
			// Remove it from the additional actors to process, and continue.
			if (ActorsToMigrate.Contains(Actor))
			{
				static_cast<Implementation*>(this)->RemoveAdditionalActor(Actor);
				continue;
			}

			if (NetDriver->StaticComponentView->HasAuthority(EntityId, SpatialConstants::AUTHORITY_INTENT_COMPONENT_ID))
			{
				if (!NetDriver->LoadBalanceStrategy->ShouldHaveAuthority(*Actor) && !NetDriver->LockingPolicy->IsLocked(Actor))
				{
					AActor* NetOwner = SpatialGDK::GetHierarchyRoot(Actor);

					uint64 HierarchyAuthorityReceivedTimestamp = 0;

					GetLatestAuthorityChangeFromHierarchy(NetOwner, HierarchyAuthorityReceivedTimestamp);

					const float TimeSinceReceivingAuthInSeconds = double(FPlatformTime::Cycles64() - HierarchyAuthorityReceivedTimestamp) * FPlatformTime::GetSecondsPerCycle64();
					const float MigrationBackoffTimeInSeconds = 1.0f;

					if (TimeSinceReceivingAuthInSeconds < MigrationBackoffTimeInSeconds)
					{
						UE_LOG(LogSpatialOSNetDriver, Verbose, TEXT("Tried to change auth too early for actor %s"), *Actor->GetName());
					}
					else
					{
						const VirtualWorkerId NewAuthVirtualWorkerId = NetDriver->LoadBalanceStrategy->WhoShouldHaveAuthority(*Actor);
						if (NewAuthVirtualWorkerId == SpatialConstants::INVALID_VIRTUAL_WORKER_ID)
						{
							UE_LOG(LogSpatialOSNetDriver, Error, TEXT("Load Balancing Strategy returned invalid virtual worker for actor %s"), *Actor->GetName());
						}
						else
						{
							CollectActorsToMigrate(NetOwner, Actor, NewAuthVirtualWorkerId);
						}
					}
				}
			}
		}
	}

	const TMap<AActor*, VirtualWorkerId>& GetActorsToMigrate() const
	{
		return ActorsToMigrate;
	}

	// Sends the migration instructions and update actor authority.
	void ProcessMigrations()
	{
		for (const auto& MigrationInfo : ActorsToMigrate)
		{
			AActor* Actor = MigrationInfo.Key;

			NetDriver->Sender->SendAuthorityIntentUpdate(*Actor, MigrationInfo.Value);

			// If we're setting a different authority intent, preemptively changed to ROLE_SimulatedProxy
			Actor->Role = ROLE_SimulatedProxy;
			Actor->RemoteRole = ROLE_Authority;

			Actor->OnAuthorityLost();
		}
		ActorsToMigrate.Empty();
	}

protected:
	USpatialNetDriver* NetDriver;

	TMap<AActor*, VirtualWorkerId> ActorsToMigrate;

	void UpdateActorSpatialDebugging(AActor* Actor, Worker_EntityId EntityId) const
	{
		if (SpatialGDK::SpatialDebugging* DebuggingInfo = NetDriver->StaticComponentView->GetComponentData<SpatialGDK::SpatialDebugging>(EntityId))
		{
			const bool bIsLocked = NetDriver->LockingPolicy->IsLocked(Actor);
			if (DebuggingInfo->IsLocked != bIsLocked)
			{
				DebuggingInfo->IsLocked = bIsLocked;
				FWorkerComponentUpdate DebuggingUpdate = DebuggingInfo->CreateSpatialDebuggingUpdate();
				NetDriver->Connection->SendComponentUpdate(EntityId, &DebuggingUpdate);
			}
		}
	}

	void GetLatestAuthorityChangeFromHierarchy(const AActor* HierarchyActor, uint64& OutTimestamp) const
	{
		if (HierarchyActor->GetIsReplicated())
		{
			if (USpatialActorChannel* Channel = NetDriver->GetOrCreateSpatialActorChannel(const_cast<AActor*>(HierarchyActor)))
			{
				if (Channel->GetAuthorityReceivedTimestamp() > OutTimestamp)
				{
					OutTimestamp = Channel->GetAuthorityReceivedTimestamp();
				}
			}
		}

		for (const AActor* Child : HierarchyActor->Children)
		{
			GetLatestAuthorityChangeFromHierarchy(Child, OutTimestamp);
		}
	}

	void CollectActorsToMigrate(AActor* Actor, const AActor* OriginalActorBeingConsidered, VirtualWorkerId Destination)
	{
		if (Actor->GetIsReplicated() && Actor->HasAuthority())
		{
			if(Actor != OriginalActorBeingConsidered)
			{
				static_cast<Implementation*>(this)->AddActorToReplicate(Actor);
			}
			ActorsToMigrate.Add(Actor, Destination);
		}

		for (AActor* Child : static_cast<Implementation*>(this)->GetDependentActors(Actor))
		{
			CollectActorsToMigrate(Child, OriginalActorBeingConsidered, Destination);
		}
	}
};
