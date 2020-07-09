// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "Utils/SpatialLoadBalancingHandler.h"

#include "EngineClasses/SpatialActorChannel.h"
#include "EngineClasses/SpatialPackageMapClient.h"
#include "LoadBalancing/AbstractLBStrategy.h"
#include "LoadBalancing/OwnershipLockingPolicy.h"
#include "Interop/SpatialSender.h"
#include "Schema/SpatialDebugging.h"

FSpatialLoadBalancingHandler::FSpatialLoadBalancingHandler(USpatialNetDriver* InNetDriver)
	: NetDriver(InNetDriver)
{

}

FSpatialLoadBalancingHandler::ProcessActorResult FSpatialLoadBalancingHandler::ProcessSingleActor(AActor* Actor, AActor*& OutNetOwner, VirtualWorkerId& OutWorkerId)
{
	const Worker_EntityId EntityId = NetDriver->PackageMap->GetEntityIdFromObject(Actor);
	if (EntityId == SpatialConstants::INVALID_ENTITY_ID)
	{
		return None;
	}

	if (!Actor->HasAuthority())
	{
		return None;
	}

	UpdateActorSpatialDebugging(Actor, EntityId);

	// If this object is in the list of actors to migrate, we have already processed its hierarchy.
	// Remove it from the additional actors to process, and continue.
	if (ActorsToMigrate.Contains(Actor))
	{
		return RemoveAdditional;
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
					OutNetOwner = NetOwner;
					OutWorkerId = NewAuthVirtualWorkerId;
					return Migrate;
				}
			}
		}
	}

	return None;
}

void FSpatialLoadBalancingHandler::ProcessMigrations()
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

void FSpatialLoadBalancingHandler::UpdateActorSpatialDebugging(AActor* Actor, Worker_EntityId EntityId) const
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

void FSpatialLoadBalancingHandler::GetLatestAuthorityChangeFromHierarchy(const AActor* HierarchyActor, uint64& OutTimestamp) const
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
