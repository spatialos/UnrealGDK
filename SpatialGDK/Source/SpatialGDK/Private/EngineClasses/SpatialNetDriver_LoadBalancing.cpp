// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "EngineClasses/SpatialNetDriver.h"

#include "Engine/NetworkObjectList.h"
//
#include "EngineClasses/SpatialActorChannel.h"
#include "EngineClasses/SpatialPackageMapClient.h"
#include "Interop/SpatialSender.h"
#include "LoadBalancing/AbstractLBStrategy.h"
#include "LoadBalancing/OwnershipLockingPolicy.h"
#include "Schema/SpatialDebugging.h"
#include "Utils/SpatialActorUtils.h"

void USpatialNetDriver::GetLatestAuthorityChangeFromHierarchy(const AActor* HierarchyActor, uint64& OutTimestamp)
{
	if (HierarchyActor->GetIsReplicated())
	{
		if (USpatialActorChannel* Channel = GetOrCreateSpatialActorChannel(const_cast<AActor*>(HierarchyActor)))
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

void USpatialNetDriver::CollectActorsToMigrate(AActor* HierarchyActor, const AActor* OriginalActorBeingConsidered, VirtualWorkerId Destination, TArray<FMigrationInfo>& OutMigrationInfo, TSet<FNetworkObjectInfo*>& OutAdditionalConsider)
{
	if (HierarchyActor->GetIsReplicated()
	&& HierarchyActor != OriginalActorBeingConsidered
	&& HierarchyActor->HasAuthority())
	{
		if(FNetworkObjectInfo* Info = FindNetworkObjectInfo(HierarchyActor))
		{
			OutAdditionalConsider.Add(Info);
			FMigrationInfo MigInfo = {HierarchyActor, Destination};
			OutMigrationInfo.Add(MigInfo);
		}
	}

	for (AActor* Child : HierarchyActor->Children)
	{
		CollectActorsToMigrate(Child, OriginalActorBeingConsidered, Destination, OutMigrationInfo, OutAdditionalConsider);
	}
}

void USpatialNetDriver::ServerReplicateActors_HandleLoadBalancing(TArray<FNetworkObjectInfo*>& ConsiderList, TArray<FMigrationInfo>& OutMigrationInfo)
{
	TSet<FNetworkObjectInfo*> AdditionalObjectsToConsider;

	for(FNetworkObjectInfo* Object : ConsiderList)
	{
		AActor* Actor = Object->Actor;

		Worker_EntityId EntityId = PackageMap->GetEntityIdFromObject(Actor);
		if (EntityId == SpatialConstants::INVALID_ENTITY_ID)
		{
			continue;
		}

		if (!Actor->HasAuthority())
		{
			continue;
		}

		if (SpatialGDK::SpatialDebugging* DebuggingInfo = StaticComponentView->GetComponentData<SpatialGDK::SpatialDebugging>(EntityId))
		{
			const bool bIsLocked = LockingPolicy->IsLocked(Actor);
			if (DebuggingInfo->IsLocked != bIsLocked)
			{
				DebuggingInfo->IsLocked = bIsLocked;
				FWorkerComponentUpdate DebuggingUpdate = DebuggingInfo->CreateSpatialDebuggingUpdate();
				Connection->SendComponentUpdate(EntityId, &DebuggingUpdate);
			}
		}

		// If this object is already in the addtional objects list, we have already processed its hierarchy.
		// Remove it from the set, and continue.
		if (AdditionalObjectsToConsider.Contains(Object))
		{
			AdditionalObjectsToConsider.Remove(Object);
			continue;
		}
		
		// TODO: the 'bWroteSomethingImportant' check causes problems for actors that need to transition in groups (ex. Character, PlayerController, PlayerState),
		// so disabling it for now.  Figure out a way to deal with this to recover the perf lost by calling ShouldChangeAuthority() frequently. [UNR-2387]
		if (LoadBalanceStrategy != nullptr &&
			StaticComponentView->HasAuthority(EntityId, SpatialConstants::AUTHORITY_INTENT_COMPONENT_ID))
		{
			if (!LoadBalanceStrategy->ShouldHaveAuthority(*Actor) && !LockingPolicy->IsLocked(Actor))
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
					const VirtualWorkerId NewAuthVirtualWorkerId = LoadBalanceStrategy->WhoShouldHaveAuthority(*Actor);
					if (NewAuthVirtualWorkerId == SpatialConstants::INVALID_VIRTUAL_WORKER_ID)
					{
						UE_LOG(LogSpatialOSNetDriver, Error, TEXT("Load Balancing Strategy returned invalid virtual worker for actor %s"), *Actor->GetName());
					}
					else
					{
						FMigrationInfo MigInfo = { Actor, NewAuthVirtualWorkerId };
						OutMigrationInfo.Add(MigInfo);
						CollectActorsToMigrate(NetOwner, Actor, NewAuthVirtualWorkerId, OutMigrationInfo, AdditionalObjectsToConsider);
					}
				}
			}
		}
	}

	// Processing actors that we pull from actor hierarchies, in order to migrate them together.
	// This loop is extracted from UNetDriver::ServerReplicateActors_BuildConsiderList
	for(auto ActorInfoIter = AdditionalObjectsToConsider.CreateIterator(); ActorInfoIter; ++ActorInfoIter)
	{
		FNetworkObjectInfo* ActorInfo = *ActorInfoIter;
		AActor* Actor = ActorInfo->Actor;

		if (Actor->IsPendingKillPending())
		{
			continue;
		}

		// Verify the actor is actually initialized (it might have been intentionally spawn deferred until a later frame)
		if (!Actor->IsActorInitialized())
		{
			continue;
		}

		// Don't send actors that may still be streaming in or out
		ULevel* Level = Actor->GetLevel();
		if (Level->HasVisibilityChangeRequestPending() || Level->bIsAssociatingLevel)
		{
			continue;
		}

		if (Actor->NetDormancy == DORM_Initial && Actor->IsNetStartupActor())
		{
			continue;
		}

		ActorInfo->bPendingNetUpdate = false;

		// Call PreReplication on all actors that will be considered
		Actor->CallPreReplication(this);

		// Add it to the consider list.
		ConsiderList.Add(ActorInfo);
	}
}

void USpatialNetDriver::ServerReplicateActors_ProcessMigration(const TArray<FMigrationInfo>& MigrationInfo)
{
	for(const FMigrationInfo& Info : MigrationInfo )
	{
		Sender->SendAuthorityIntentUpdate(*Info.ActorToMigrate, Info.Destination);

		// If we're setting a different authority intent, preemptively changed to ROLE_SimulatedProxy
		Info.ActorToMigrate->Role = ROLE_SimulatedProxy;
		Info.ActorToMigrate->RemoteRole = ROLE_Authority;

		Info.ActorToMigrate->OnAuthorityLost();
	}
}
