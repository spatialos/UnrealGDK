// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "Utils/SpatialLoadBalancingHandler.h"

#include "EngineClasses/SpatialActorChannel.h"
#include "EngineClasses/SpatialPackageMapClient.h"
#include "Interop/SpatialSender.h"
#include "LoadBalancing/AbstractLBStrategy.h"
#include "LoadBalancing/OwnershipLockingPolicy.h"
#include "Schema/AuthorityIntent.h"
#include "Schema/MigrationDiagnostic.h"
#include "Schema/SpatialDebugging.h"

DEFINE_LOG_CATEGORY(LogSpatialLoadBalancingHandler);

using namespace SpatialGDK;

FSpatialLoadBalancingHandler::FSpatialLoadBalancingHandler(USpatialNetDriver* InNetDriver)
	: NetDriver(InNetDriver)
{
}

FSpatialLoadBalancingHandler::EvaluateActorResult FSpatialLoadBalancingHandler::EvaluateSingleActor(AActor* Actor, AActor*& OutNetOwner,
																									VirtualWorkerId& OutWorkerId)
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
		AActor* NetOwner = GetReplicatedHierarchyRoot(Actor);
		const bool bNetOwnerHasAuth = NetOwner->HasAuthority();

		// Load balance if we are not supposed to be on this worker, or if we are separated from our owner.
		if ((!NetDriver->LoadBalanceStrategy->ShouldHaveAuthority(*NetOwner) || !bNetOwnerHasAuth)
			&& !NetDriver->LockingPolicy->IsLocked(Actor))
		{
			uint64 HierarchyAuthorityReceivedTimestamp = GetLatestAuthorityChangeFromHierarchy(NetOwner);

			const float TimeSinceReceivingAuthInSeconds =
				double(FPlatformTime::Cycles64() - HierarchyAuthorityReceivedTimestamp) * FPlatformTime::GetSecondsPerCycle64();
			const float MigrationBackoffTimeInSeconds = 1.0f;

			if (TimeSinceReceivingAuthInSeconds < MigrationBackoffTimeInSeconds)
			{
				UE_LOG(LogSpatialLoadBalancingHandler, Verbose, TEXT("Tried to change auth too early for actor %s"), *Actor->GetName());
			}
			else
			{
				VirtualWorkerId NewAuthVirtualWorkerId = SpatialConstants::INVALID_VIRTUAL_WORKER_ID;
				if (bNetOwnerHasAuth)
				{
					NewAuthVirtualWorkerId = NetDriver->LoadBalanceStrategy->WhoShouldHaveAuthority(*NetOwner);
				}
				else
				{
					// If we are separated from our owner, it could be prevented from migrating (if it has interest over the current actor),
					// so the load balancing strategy could give us a worker different from where it should be.
					// Instead, we read its currently assigned worker, which will eventually make us land where our owner is.
					Worker_EntityId OwnerId = NetDriver->PackageMap->GetEntityIdFromObject(NetOwner);
					if (AuthorityIntent* OwnerAuthIntent = NetDriver->StaticComponentView->GetComponentData<AuthorityIntent>(OwnerId))
					{
						NewAuthVirtualWorkerId = OwnerAuthIntent->VirtualWorkerId;
					}
					else
					{
						UE_LOG(LogSpatialLoadBalancingHandler, Error, TEXT("Actor %s (%llu) cannot join its owner %s (%llu)"),
							   *Actor->GetName(), EntityId, *NetOwner->GetName(), OwnerId);
					}
				}

				if (NewAuthVirtualWorkerId == SpatialConstants::INVALID_VIRTUAL_WORKER_ID)
				{
					UE_LOG(LogSpatialLoadBalancingHandler, Error,
						   TEXT("Load Balancing Strategy returned invalid virtual worker for actor %s"), *Actor->GetName());
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
	if (SpatialDebugging* DebuggingInfo = NetDriver->StaticComponentView->GetComponentData<SpatialDebugging>(EntityId))
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

void FSpatialLoadBalancingHandler::LogMigrationFailure(EActorMigrationResult ActorMigrationResult, AActor* Actor)
{
	FString FailureReason;

	// Waiting before creating logs to suppress the logs for newly created actors
	if (Actor->GetGameTimeSinceCreation() > 1)
	{
		switch (ActorMigrationResult)
		{
		case EActorMigrationResult::NotAuthoritative:
			FailureReason = TEXT("does not have authority");
			break;
		case EActorMigrationResult::NotReady:
			FailureReason = TEXT("is not ready");
			break;
		case EActorMigrationResult::PendingKill:
			FailureReason = TEXT("is pending kill");
			break;
		case EActorMigrationResult::NotInitialized:
			FailureReason = TEXT("is not initialized");
			break;
		case EActorMigrationResult::Streaming:
			FailureReason = TEXT("is streaming in or out");
			break;
		case EActorMigrationResult::NetDormant:
			FailureReason = TEXT("is startup actor and initially net dormant");
			break;
		case EActorMigrationResult::NoSpatialClassFlags:
			FailureReason = TEXT("does not have spatial class flags");
			break;
		case EActorMigrationResult::DormantOnConnection:
			FailureReason = TEXT("is dormant on connection");
			break;
		default:
			break;
		}
	}

	// If a failure reason is returned log warning
	if (!FailureReason.IsEmpty())
	{
		Worker_EntityId ActorEntityId = NetDriver->PackageMap->GetEntityIdFromObject(Actor);

		// Check if we have recently logged this actor / reason and if so suppress the log
		if (!NetDriver->IsLogged(ActorEntityId, ActorMigrationResult))
		{
			if (ActorMigrationResult == EActorMigrationResult::NotAuthoritative)
			{
				// Request further diagnostics from authoritative server of blocking actor
				Worker_CommandRequest MigrationDiagnosticCommandRequest = MigrationDiagnostic::CreateMigrationDiagnosticRequest();
				NetDriver->Connection->SendCommandRequest(ActorEntityId, &MigrationDiagnosticCommandRequest, RETRY_MAX_TIMES, {});
			}
			else
			{
				AActor* HierarchyRoot = GetReplicatedHierarchyRoot(Actor);
				UE_LOG(LogSpatialLoadBalancingHandler, Warning,
					   TEXT("Prevented Actor %s 's hierarchy from migrating because Actor %s (%llu) %s"), *HierarchyRoot->GetName(),
					   *Actor->GetName(), ActorEntityId, *FailureReason);
			}
		}
	}
}
