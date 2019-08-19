// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "EngineClasses/SpatialLoadBalancingStrategy.h"

#include "EngineClasses/SpatialActorChannel.h"
#include "EngineClasses/SpatialGameInstance.h"
#include "EngineClasses/SpatialNetDriver.h"
#include "EngineClasses/SpatialVirtualWorkerTranslator.h"

DEFINE_LOG_CATEGORY(LogSpatialLoadBalancer);

void USpatialLoadBalancingStrategy::Init(const ASpatialVirtualWorkerTranslator* InTranslator)
{
	Translator = InTranslator;
	OnWorkerAssignmentChangedDelegateHandle = Translator->OnWorkerAssignmentChanged.AddLambda([this](const TArray<FString>& NewAssignments) {
		OnWorkerAssignmentChanged(NewAssignments);
	});
}

USpatialLoadBalancingStrategy::~USpatialLoadBalancingStrategy()
{
	Translator->OnWorkerAssignmentChanged.Remove(OnWorkerAssignmentChangedDelegateHandle);
	OnWorkerAssignmentChangedDelegateHandle.Reset();
	Translator = nullptr;
}

const FString USpatialLoadBalancingStrategy::GetWorkerId() const
{
	const UWorld* World = GetWorld();
	if (World == nullptr)
	{
		return "";
	}
	USpatialGameInstance* GameInstance = Cast<USpatialGameInstance>(World->GetGameInstance());
	if (GameInstance == nullptr)
	{
		return "";
	}
	return GameInstance->GetSpatialWorkerId();
}

void USpatialLoadBalancingStrategy::OnWorkerAssignmentChanged(const TArray<FString> NewAssignments)
{
	int32 OldWorkerIndex = LocalWorkerIndex;
	NewAssignments.Find(GetWorkerId(), LocalWorkerIndex);
	UE_LOG(LogSpatialLoadBalancer, Log, TEXT("New virtual worker mapping received. Local worker index was %d and is now %d"), OldWorkerIndex, LocalWorkerIndex);
	checkf(OldWorkerIndex == INDEX_NONE || LocalWorkerIndex == OldWorkerIndex, TEXT("A worker's mapping to virtual worker changed unexpectedly: %s"), *GetWorkerId());
}


void UGridBasedLoadBalancingStrategy::Init(const ASpatialVirtualWorkerTranslator* InTranslator)
{
	USpatialLoadBalancingStrategy::Init(InTranslator);

	// Assume 200m * 200m world for the moment.
	// TODO: get this from ?? (ideally from the deployment)
	const float WorldWidth = 20000.f;
	const float WorldHeight = 20000.f;

	const float WorldWidthMin = -(WorldWidth / 2.f);
	const float WorldHeightMin = -(WorldHeight / 2.f);

	const float ColumnWidth = WorldWidth / ColumnCount;
	const float RowHeight = WorldHeight / RowCount;

	for (int32 Col = 0; Col < ColumnCount; ++Col)
	{
		for (int32 Row = 0; Row < RowCount; ++Row)
		{
			FVector2D Min(WorldWidthMin + (Col * ColumnWidth), WorldHeightMin + (Row * RowHeight));
			FVector2D Max(Min.X + ColumnWidth, Min.Y + RowHeight);
			FBox2D Cell(Min, Max);

			// Fuzzy cell edges to avoid floating point and boundary errors (2cm overlap).
			// This implies that an actor will be in 2 or more cells when crossing
			// boundaries. This is OK, since we first ask the current authoritative
			// worker if it should relinquish authority, and it will not do that until
			// the actor leaves its cell, including the fuzzy boundary. When we then ask
			// which worker should get authority, a different worker will always be
			// selected (assuming we have complete coverage over the world.)
			Cell = Cell.ExpandBy(1.0f);

			WorkerCells.Add(MoveTemp(Cell));
		}
	}
}

bool UGridBasedLoadBalancingStrategy::IsActorInCell(const AActor& Actor, const FBox2D& Cell) const
{
	// TODO - timgibson - move GetActorSpatialPosition to SpatialActorUtils.cpp
	FVector ActorPosition = USpatialActorChannel::GetActorSpatialPosition(&Actor);
	FVector2D Actor2DPosition(ActorPosition.X, ActorPosition.Y);

	return Cell.IsInside(Actor2DPosition);
}

bool UGridBasedLoadBalancingStrategy::ShouldChangeAuthority(const AActor& Actor) const
{
	int32 CellIndex = GetLocalWorkerIndex();
	if (CellIndex == INDEX_NONE)
	{
		// If we don't know who we are, then we shouldn't have authority.
		return true;
	}
	return (IsActorInCell(Actor, WorkerCells[CellIndex]) == false);
}

FString UGridBasedLoadBalancingStrategy::GetAuthoritativeVirtualWorkerId(const AActor& Actor) const
{
	int32 PossibleIndex = WorkerCells.IndexOfByPredicate([&](const FBox2D& Cell)
	{
		return IsActorInCell(Actor, Cell);
	});
	if (PossibleIndex == INDEX_NONE)
	{
		UE_LOG(LogSpatialLoadBalancer, Warning, TEXT("Failed to find an acceptable authoritative worker for %s"), *GetNameSafe(&Actor));
		return "";
	}
	else
	{
		return Translator->GetVirtualWorkers()[PossibleIndex];
	}
}

