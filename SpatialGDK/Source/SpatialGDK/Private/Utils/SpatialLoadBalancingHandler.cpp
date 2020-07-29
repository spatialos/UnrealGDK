// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "Utils/SpatialLoadBalancingHandler.h"

#include "EngineClasses/SpatialActorChannel.h"
#include "EngineClasses/SpatialPackageMapClient.h"
#include "LoadBalancing/AbstractLBStrategy.h"
#include "LoadBalancing/OwnershipLockingPolicy.h"
#include "Interop/SpatialSender.h"
#include "Schema/AuthorityIntent.h"
#include "Schema/SpatialDebugging.h"

DEFINE_LOG_CATEGORY(LogSpatialLoadBalancingHandler);

FSpatialLoadBalancingHandler::FSpatialLoadBalancingHandler(USpatialNetDriver* InNetDriver)
	: NetDriver(InNetDriver)
{

}

FSpatialLoadBalancingHandler::EvaluateActorResult FSpatialLoadBalancingHandler::EvaluateSingleActor(AActor* Actor, AActor*& OutNetOwner, VirtualWorkerId& OutWorkerId)
{
	const Worker_EntityId EntityId = NetDriver->PackageMap->GetEntityIdFromObject(Actor);
	if (EntityId == SpatialConstants::INVALID_ENTITY_ID)
	{
		return EvaluateActorResult::None;
	}

	if (!Actor->HasAuthority())
	{
		return EvaluateActorResult::None;
	}

	UpdateSpatialDebugInfo(Actor, EntityId);

	// If this object is in the list of actors to migrate, we have already processed its hierarchy.
	// Remove it from the additional actors to process, and continue.
	if (ActorsToMigrate.Contains(Actor))
	{
		return EvaluateActorResult::RemoveAdditional;
	}

	if (NetDriver->StaticComponentView->HasAuthority(EntityId, SpatialConstants::AUTHORITY_INTENT_COMPONENT_ID))
	{
		AActor* NetOwner = SpatialGDK::GetHierarchyRoot(Actor);
		const bool bNetOwnerHasAuth = NetOwner->HasAuthority();
		if ((!bNetOwnerHasAuth || !NetDriver->LoadBalanceStrategy->ShouldHaveAuthority(*Actor)) && !NetDriver->LockingPolicy->IsLocked(Actor))
		{
			uint64 HierarchyAuthorityReceivedTimestamp = GetLatestAuthorityChangeFromHierarchy(NetOwner);

			const float TimeSinceReceivingAuthInSeconds = double(FPlatformTime::Cycles64() - HierarchyAuthorityReceivedTimestamp) * FPlatformTime::GetSecondsPerCycle64();
			const float MigrationBackoffTimeInSeconds = 1.0f;

			if (TimeSinceReceivingAuthInSeconds < MigrationBackoffTimeInSeconds)
			{
				UE_LOG(LogSpatialLoadBalancingHandler, Verbose, TEXT("Tried to change auth too early for actor %s"), *Actor->GetName());
			}
			else
			{
				VirtualWorkerId NewAuthVirtualWorkerId = SpatialConstants::INVALID_VIRTUAL_WORKER_ID;
				if(bNetOwnerHasAuth)
				{
					NewAuthVirtualWorkerId = NetDriver->LoadBalanceStrategy->WhoShouldHaveAuthority(*NetOwner);
				}
				else
				{
					Worker_EntityId OwnerId = NetDriver->PackageMap->GetEntityIdFromObject(NetOwner);
					if (SpatialGDK::AuthorityIntent* OwnerAuthIntent = NetDriver->StaticComponentView->GetComponentData<SpatialGDK::AuthorityIntent>(OwnerId))
					{
						NewAuthVirtualWorkerId = OwnerAuthIntent->VirtualWorkerId;
					}
					else
					{
						UE_LOG(LogSpatialLoadBalancingHandler, Error, TEXT("Actor %s (%llu) cannot join its owner %s (%llu)"), *Actor->GetName(), EntityId, *NetOwner->GetName(), OwnerId);
					}
				}
				if (NewAuthVirtualWorkerId == SpatialConstants::INVALID_VIRTUAL_WORKER_ID)
				{
					UE_LOG(LogSpatialLoadBalancingHandler, Error, TEXT("Load Balancing Strategy returned invalid virtual worker for actor %s"), *Actor->GetName());
				}
				else
				{
					OutNetOwner = NetOwner;
					OutWorkerId = NewAuthVirtualWorkerId;
					return EvaluateActorResult::Migrate;
				}
			}
		}
	}

	return EvaluateActorResult::None;
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

void FSpatialLoadBalancingHandler::UpdateSpatialDebugInfo(AActor* Actor, Worker_EntityId EntityId) const
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

uint64 FSpatialLoadBalancingHandler::GetLatestAuthorityChangeFromHierarchy(const AActor* HierarchyActor) const
{
	uint64 LatestTimestamp = 0;
	for (const AActor* Child : HierarchyActor->Children)
	{
		LatestTimestamp = FMath::Max(LatestTimestamp, GetLatestAuthorityChangeFromHierarchy(Child));
	}

	if (HierarchyActor->GetIsReplicated() && HierarchyActor->HasAuthority())
	{
		if (USpatialActorChannel* Channel = NetDriver->GetOrCreateSpatialActorChannel(const_cast<AActor*>(HierarchyActor)))
		{
			LatestTimestamp = FMath::Max(LatestTimestamp, Channel->GetAuthorityReceivedTimestamp());
		}
	}

	return LatestTimestamp;
}
