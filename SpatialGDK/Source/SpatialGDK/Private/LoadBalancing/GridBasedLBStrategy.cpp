// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "LoadBalancing/GridBasedLBStrategy.h"

#include "EngineClasses/SpatialNetDriver.h"
#include "Schema/Interest.h"
#include "Utils/SpatialActorUtils.h"

#include "Templates/Tuple.h"

DEFINE_LOG_CATEGORY(LogGridBasedLBStrategy);

UGridBasedLBStrategy::UGridBasedLBStrategy()
	: Super()
	, Rows(1)
	, Cols(1)
	, WorldWidth(10000.f)
	, WorldHeight(10000.f)
	, InterestBorder(0.f)
{
}

void UGridBasedLBStrategy::Init(const USpatialNetDriver* InNetDriver)
{
	Super::Init(InNetDriver);

	UE_LOG(LogGridBasedLBStrategy, Log, TEXT("GridBasedLBStrategy initialized with Rows = %d and Cols = %d."), Rows, Cols);

	for (uint32 i = 1; i <= Rows * Cols; i++)
	{
		VirtualWorkerIds.Add(i);
	}

	const float WorldWidthMin = -(WorldWidth / 2.f);
	const float WorldHeightMin = -(WorldHeight / 2.f);

	const float ColumnWidth = WorldWidth / Cols;
	const float RowHeight = WorldHeight / Rows;

	float XMin = WorldWidthMin;
	float YMin = WorldHeightMin;
	float XMax, YMax;

	for (uint32 Col = 0; Col < Cols; ++Col)
	{
		XMax = XMin + ColumnWidth;

		for (uint32 Row = 0; Row < Rows; ++Row)
		{
			YMax = YMin + RowHeight;

			FVector2D Min(XMin, YMin);
			FVector2D Max(XMax, YMax);
			FBox2D Cell(Min, Max);
			WorkerCells.Add(Cell);

			YMin = YMax;
		}

		YMin = WorldHeightMin;
		XMin = XMax;
	}
}

TSet<VirtualWorkerId> UGridBasedLBStrategy::GetVirtualWorkerIds() const
{
	return TSet<VirtualWorkerId>(VirtualWorkerIds);
}

bool UGridBasedLBStrategy::ShouldRelinquishAuthority(const AActor& Actor) const
{
	if (!IsReady())
	{
		UE_LOG(LogGridBasedLBStrategy, Warning, TEXT("GridBasedLBStrategy not ready to relinquish authority for Actor %s."), *AActor::GetDebugName(&Actor));
		return false;
	}

	const FVector2D Actor2DLocation = FVector2D(SpatialGDK::GetActorSpatialPosition(&Actor));

	return !IsInside(WorkerCells[LocalVirtualWorkerId - 1], Actor2DLocation);
}

VirtualWorkerId UGridBasedLBStrategy::WhoShouldHaveAuthority(const AActor& Actor) const
{
	if (!IsReady())
	{
		UE_LOG(LogGridBasedLBStrategy, Warning, TEXT("GridBasedLBStrategy not ready to decide on authority for Actor %s."), *AActor::GetDebugName(&Actor));
		return SpatialConstants::INVALID_VIRTUAL_WORKER_ID;
	}

	const FVector2D Actor2DLocation = FVector2D(SpatialGDK::GetActorSpatialPosition(&Actor));

	for (int i = 0; i < WorkerCells.Num(); i++)
	{
		if (IsInside(WorkerCells[i], Actor2DLocation))
		{
			return VirtualWorkerIds[i];
		}
	}

	return SpatialConstants::INVALID_VIRTUAL_WORKER_ID;
}

void UGridBasedLBStrategy::CreateWorkerInterestQueries(TArray<SpatialGDK::Query>& OutQueries) const
{
	// For a grid-based strategy, the interest area is the cell that the worker is authoritative over plus some border region.
	// If there is only a single server, then there is no need to create interest since the worker has authority over the entire world (in principle, at least).
	check(IsReady());
	check(InterestBorder >= 0);
	if (Rows * Cols > 0)
	{
		const FBox2D Interest2D = WorkerCells[LocalVirtualWorkerId - 1].ExpandBy(InterestBorder);
		const FVector Min = FVector{ Interest2D.Min.X, Interest2D.Min.Y, FLT_MIN };
		const FVector Max = FVector{ Interest2D.Max.X, Interest2D.Max.Y, FLT_MAX };
		const FBox Interest3D = FBox{ Min, Max };
		SpatialGDK::QueryConstraint Constraint;
		Constraint.BoxConstraint = SpatialGDK::BoxConstraint{ SpatialGDK::Coordinates::FromFVector(Interest3D.GetCenter()), SpatialGDK::EdgeLength::FromFVector(2 * Interest3D.GetExtent()) };
		SpatialGDK::Query Query;
		Query.Constraint = Constraint;
		Query.FullSnapshotResult = true;
		OutQueries.Add(Query);
	}
}

bool UGridBasedLBStrategy::IsInside(const FBox2D& Box, const FVector2D& Location)
{
	return Location.X >= Box.Min.X && Location.Y >= Box.Min.Y
		&& Location.X < Box.Max.X && Location.Y < Box.Max.Y;
}

UGridBasedLBStrategy::LBStrategyRegions UGridBasedLBStrategy::GetLBStrategyRegions() const
{
	LBStrategyRegions VirtualWorkerToCell;
	VirtualWorkerToCell.SetNum(WorkerCells.Num());

	for (int i = 0; i < WorkerCells.Num(); i++)
	{
		VirtualWorkerToCell[i] = MakeTuple(VirtualWorkerIds[i], WorkerCells[i]);
	}
	return VirtualWorkerToCell;
}
