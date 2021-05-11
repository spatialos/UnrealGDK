// NWX_BEGIN - https://improbableio.atlassian.net/browse/NWX-18915 - [IMPROVEMENT] ClosestNInterestComponent
// NWX_MORE - https://improbableio.atlassian.net/browse/NWX-19238 - [IMPROVEMENT] Move UpdateRequired logic to SpatialActorChannel
// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "EngineClasses/Components/ClosestNInterestComponent.h"

#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "GameFramework/PlayerController.h"
#include "Interop/SpatialClassInfoManager.h"
#include "Interop/SpatialInterestConstraints.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"
#include "Utils/SpatialStatics.h"

DEFINE_LOG_CATEGORY(LogClosestNInterestComponent);

bool UClosestNInterestComponent::bSoftTargetActorLimitExceededWarningShown = false;
bool UClosestNInterestComponent::bSoftTargetEntityLimitExceededWarningShown = false;

void UClosestNInterestComponent::BeginPlay()
{
	Super::BeginPlay();

	const AActor* OwningActor = GetOwner();

	if (OwningActor == nullptr)
	{
		UE_LOG(LogClosestNInterestComponent, Warning, TEXT("UClosestNInterestComponent::BeginPlay() has an invalid OwningActor!"));
		return;
	}

	if (USpatialStatics::IsSpatialNetworkingEnabled() && OwningActor->HasAuthority())
	{
		checkf(!TargetActorClass->IsChildOf<APlayerController>(), TEXT("PlayerControllers are not supported as the TargetActorClass."));

		MaxInterestRangeSqr = FMath::Pow(MaxInterestRange, 2.0f);
		SoftMaxEntityCount = bIncludeHierarchy ? TargetActorMaxCount * SoftMaxAttachedActorsPerTargetActorCount : TargetActorMaxCount;

		TargetActors.Reserve(SoftMaxTargetActorsOnServerCount);
		TargetActorsSorted.Reserve(SoftMaxTargetActorsOnServerCount);
		TargetEntityIds.Reserve(SoftMaxEntityCount);
		PreviousTargetEntityIds.Reserve(SoftMaxEntityCount);

		ConstraintList.AddZeroed_GetRef();

		UpdateQuery();

		if (UnrealServerUpdateFrequency > 0)
		{
			// Schedule query to run at randomly offset start time for amortization
			const float UpdateInterval = 1.0f / UnrealServerUpdateFrequency;

			const UWorld* World = GetWorld();
			check(World != nullptr);

			World->GetTimerManager().SetTimer(UpdateQueryTimerHandle, this, &UClosestNInterestComponent::UpdateQuery, UpdateInterval, true, FMath::FRand() * UpdateInterval);
		}
	}
}

void UClosestNInterestComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	const UWorld* World = GetWorld();
	if (World != nullptr && UpdateQueryTimerHandle.IsValid())
	{
		World->GetTimerManager().ClearTimer(UpdateQueryTimerHandle);
	}
}

void UClosestNInterestComponent::UpdateQuery()
{
	TRACE_CPUPROFILER_EVENT_SCOPE(UClosestNInterestComponent::UpdateQuery);
	const AActor* OwningActor = GetOwner();
	check(OwningActor != nullptr && OwningActor->HasAuthority());

	FindTargetActors();
	FindTargetEntityIds();
	ConstructQuery();

	if (TargetEntityIds != PreviousTargetEntityIds)
	{
		NotifyChannelUpdateRequired();
	}

	PreviousTargetEntityIds.Reset();
	PreviousTargetEntityIds.Append(TargetEntityIds);
}

void UClosestNInterestComponent::FindTargetActors()
{
	TRACE_CPUPROFILER_EVENT_SCOPE(UClosestNInterestComponent::FindTargetActors);
	// If our owner is a PlayerController, we instead use it's Pawn for the distance calculations,
	// since PlayerControllers aren't necessarily configured to move with their pawn (see bAttachToPawn).
	AActor* LocationProvidingOwnerProxy = GetOwner();
	if (const APlayerController* OwningActorAsPC = Cast<APlayerController>(LocationProvidingOwnerProxy))
	{
		LocationProvidingOwnerProxy = OwningActorAsPC->GetPawn();
	}

	if (LocationProvidingOwnerProxy == nullptr)
	{
		// This can happen transiently if the Component is bound to a PlayerController whose controlled pawn is not yet available.
		return;
	}

	TargetActors.Reset();
	{
		if (MaxInterestRange > 0 && TargetActorMaxCount > 0)
		{
			TRACE_CPUPROFILER_EVENT_SCOPE(UClosestNInterestComponent::FindTargetActors_GetActors);
			// TODO: Eliminate this expensive query - we should be able to do this once/frame on the server: https://improbableio.atlassian.net/browse/NWX-18924
			UGameplayStatics::GetAllActorsOfClass(GetWorld(), TargetActorClass, TargetActors);
		}
	}

	const int32 TargetActorCount = TargetActors.Num();

	if (TargetActorCount > SoftMaxTargetActorsOnServerCount && !bSoftTargetActorLimitExceededWarningShown)
	{
		UE_LOG(LogClosestNInterestComponent, Warning,
			TEXT("UClosestNInterestComponent::FindTargetActors() found more target actors (%d) than expected max (%d) and will perform dynamic allocations, consider increasing SoftMaxTargetActorsOnServerCount."),
			TargetActorCount, SoftMaxTargetActorsOnServerCount);
		bSoftTargetActorLimitExceededWarningShown = true;
	}

	int32 InRangeActorCount = 0;
	{
		TRACE_CPUPROFILER_EVENT_SCOPE(UClosestNInterestComponent::FindTargetActors_FindActorsInRange);
		TargetActorsSorted.Reset(TargetActorCount);
		TargetActorsSorted.AddUninitialized(TargetActorCount);

		const FVector OwnerLocation = LocationProvidingOwnerProxy->GetActorLocation();

		for (const auto Actor : TargetActors)
		{
			if (!Actor->IsPendingKill() && Actor != LocationProvidingOwnerProxy)
			{
				const FVector ActorLocation = Actor->GetActorLocation();
				const float DistanceBetweenActorsSqr = FVector::DistSquared(OwnerLocation, ActorLocation);

				if (DistanceBetweenActorsSqr < MaxInterestRangeSqr)
				{
					FActorSortData& Data = TargetActorsSorted[InRangeActorCount++];
					Data.Actor = Actor;
					Data.DistanceSqr = DistanceBetweenActorsSqr;
				}
			}
		}
	}

	//{
	//	TRACE_CPUPROFILER_EVENT_SCOPE(UClosestNInterestComponent::FindTargetActors_PurgeActors);
	//	const int32 ActorsToPurgeCount = TargetActorCount - InRangeActorCount;

	//	if (ActorsToPurgeCount > 0)
	//	{
	//		const bool bAllowShrinking = false;
	//		TargetActorsSorted.RemoveAt(InRangeActorCount, ActorsToPurgeCount, bAllowShrinking);
	//	}
	//}

	{
		TRACE_CPUPROFILER_EVENT_SCOPE(UClosestNInterestComponent::FindTargetActors_SortActors);
		TargetActorsSorted.Sort([](const FActorSortData& LHS, const FActorSortData& RHS) { return LHS.DistanceSqr < RHS.DistanceSqr; });
	}

	{
		TRACE_CPUPROFILER_EVENT_SCOPE(UClosestNInterestComponent::FindTargetActors_PurgeActors);
		const int32 ActorsSortedCount = TargetActorsSorted.Num();
		const int32 ActorsToKeepCount = FMath::Min<int32>(ActorsSortedCount, TargetActorMaxCount);
		const int32 ActorsToPurgeCount = FMath::Max<int32>(ActorsSortedCount - TargetActorMaxCount, 0);

		if (ActorsToPurgeCount > 0)
		{
			const bool bAllowShrinking = false;
			TargetActorsSorted.RemoveAt(ActorsToKeepCount, ActorsToPurgeCount, bAllowShrinking);
		}
	}
}

void UClosestNInterestComponent::FindTargetEntityIds()
{
	TRACE_CPUPROFILER_EVENT_SCOPE(UClosestNInterestComponent::FindTargetEntityIds);
	TargetEntityIds.Reset();

	for (const auto& Data : TargetActorsSorted)
	{
		if (Data.Actor == nullptr)
			continue;

		if (bIncludeHierarchy)
		{
			USpatialStatics::GetEntityIdsInHierarchy(Data.Actor.Get(), TargetEntityIds);
		}
		else
		{
			TargetEntityIds.Add(USpatialStatics::GetActorEntityId(Data.Actor.Get()));
		}
	}

	TargetEntityIds.Sort();
}

void UClosestNInterestComponent::ConstructQuery()
{
	TRACE_CPUPROFILER_EVENT_SCOPE(UClosestNInterestComponent::ConstructDynamicQuery);
	const int32 TargetEntityIdCount = TargetEntityIds.Num();
	
	if (TargetEntityIdCount > SoftMaxEntityCount && !bSoftTargetEntityLimitExceededWarningShown)
	{
		UE_LOG(LogClosestNInterestComponent, Warning,
			TEXT("UClosestNInterestComponent::ConstructQuery() found more entities (%d) than estimated max (%d) and will perform dynamic allocations, consider increasing SoftMaxAttachedActorsPerTargetActorCount."),
			TargetEntityIdCount, SoftMaxEntityCount);
		bSoftTargetEntityLimitExceededWarningShown = true;
	}

	SpatialGDK::QueryConstraint& RootConstraint = ConstraintList[0];

	RootConstraint.OrConstraint.Reset(TargetEntityIdCount);
	RootConstraint.OrConstraint.AddZeroed(TargetEntityIdCount);

	for (int i = 0; i < TargetEntityIdCount; ++i)
	{
		SpatialGDK::QueryConstraint& Constraint = RootConstraint.OrConstraint[i];
		Constraint.EntityIdConstraint = TargetEntityIds[i];
	}
}

void UClosestNInterestComponent::PopulateFrequencyToConstraintsMap(const USpatialClassInfoManager& ClassInfoManager,
	SpatialGDK::FrequencyToConstraintsMap& OutFrequencyToQueryConstraints) const
{
	TArray<SpatialGDK::QueryConstraint>& Constraints = OutFrequencyToQueryConstraints.FindOrAdd(RuntimeUpdateFrequency);
	Constraints.Append(ConstraintList);
}

void UClosestNInterestComponent::UpdateInterestRange_Implementation(float Value)
{
	if (!USpatialStatics::IsSpatialNetworkingEnabled())
		return;

	const AActor* OwningActor = GetOwner();
	if (OwningActor == nullptr || !OwningActor->HasAuthority())
		return;

	MaxInterestRange = Value;
	MaxInterestRangeSqr = FMath::Pow(MaxInterestRange, 2.0f);
	UpdateQuery();
}

