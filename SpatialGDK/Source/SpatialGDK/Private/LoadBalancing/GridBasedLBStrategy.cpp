// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "LoadBalancing/GridBasedLBStrategy.h"

#include "EngineClasses/SpatialNetDriver.h"
#include "Utils/SpatialActorUtils.h"

#include "Templates/Tuple.h"

DEFINE_LOG_CATEGORY(LogGridBasedLBStrategy);

UGridBasedLBStrategy::UGridBasedLBStrategy()
	: Super()
	, Rows(1)
	, Cols(1)
	, WorldWidth(1000000.f)
	, WorldHeight(1000000.f)
	, InterestBorder(0.f)
	, LocalCellId(0)
	, bIsStrategyUsedOnLocalWorker(false)
{
}

void UGridBasedLBStrategy::Init()
{
	Super::Init();

	UE_LOG(LogGridBasedLBStrategy, Log, TEXT("GridBasedLBStrategy initialized with Rows = %d and Cols = %d."), Rows, Cols);

	const float WorldWidthMin = -(WorldWidth / 2.f);
	const float WorldHeightMin = -(WorldHeight / 2.f);

	const float ColumnWidth = WorldWidth / Cols;
	const float RowHeight = WorldHeight / Rows;

	// We would like the inspector's representation of the load balancing strategy to match our intuition.
	// +x is forward, so rows are perpendicular to the x-axis and columns are perpendicular to the y-axis.
	float XMin = WorldHeightMin;
	float YMin = WorldWidthMin;
	float XMax, YMax;

	for (uint32 Col = 0; Col < Cols; ++Col)
	{
		YMax = YMin + ColumnWidth;

		for (uint32 Row = 0; Row < Rows; ++Row)
		{
			XMax = XMin + RowHeight;

			FVector2D Min(XMin, YMin);
			FVector2D Max(XMax, YMax);
			FBox2D Cell(Min, Max);
			WorkerCells.Add(Cell);

			XMin = XMax;
		}

		XMin = WorldHeightMin;
		YMin = YMax;
	}
}

void UGridBasedLBStrategy::SetLocalVirtualWorkerId(VirtualWorkerId InLocalVirtualWorkerId)
{
	if (!VirtualWorkerIds.Contains(InLocalVirtualWorkerId))
	{
		// This worker is simulating a layer which is not part of the grid.
		LocalCellId = WorkerCells.Num();
		bIsStrategyUsedOnLocalWorker = false;
	}
	else
	{
		LocalCellId = VirtualWorkerIds.IndexOfByKey(InLocalVirtualWorkerId);
		bIsStrategyUsedOnLocalWorker = true;
	}
	LocalVirtualWorkerId = InLocalVirtualWorkerId;
}

TSet<VirtualWorkerId> UGridBasedLBStrategy::GetVirtualWorkerIds() const
{
	return TSet<VirtualWorkerId>(VirtualWorkerIds);
}

bool UGridBasedLBStrategy::ShouldHaveAuthority(const AActor& Actor) const
{
	if (!IsReady())
	{
		UE_LOG(LogGridBasedLBStrategy, Warning, TEXT("GridBasedLBStrategy not ready to relinquish authority for Actor %s."),
			   *AActor::GetDebugName(&Actor));
		return false;
	}

	if (!bIsStrategyUsedOnLocalWorker)
	{
		return false;
	}

	const FVector2D Actor2DLocation = FVector2D(SpatialGDK::GetActorSpatialPosition(&Actor));
	return IsInside(WorkerCells[LocalCellId], Actor2DLocation);
}

VirtualWorkerId UGridBasedLBStrategy::WhoShouldHaveAuthority(const AActor& Actor) const
{
	if (!IsReady())
	{
		UE_LOG(LogGridBasedLBStrategy, Warning, TEXT("GridBasedLBStrategy not ready to decide on authority for Actor %s."),
			   *AActor::GetDebugName(&Actor));
		return SpatialConstants::INVALID_VIRTUAL_WORKER_ID;
	}

	const FVector2D Actor2DLocation = FVector2D(SpatialGDK::GetActorSpatialPosition(&Actor));

	check(VirtualWorkerIds.Num() == WorkerCells.Num());
	for (int i = 0; i < WorkerCells.Num(); i++)
	{
		if (IsInside(WorkerCells[i], Actor2DLocation))
		{
			UE_LOG(LogGridBasedLBStrategy, Log, TEXT("Actor: %s, grid %d, worker %d for position %s"), *AActor::GetDebugName(&Actor), i,
				   VirtualWorkerIds[i], *Actor2DLocation.ToString());
			return VirtualWorkerIds[i];
		}
	}

	UE_LOG(LogGridBasedLBStrategy, Error, TEXT("GridBasedLBStrategy couldn't determine virtual worker for Actor %s at position %s"),
		   *AActor::GetDebugName(&Actor), *Actor2DLocation.ToString());
	return SpatialConstants::INVALID_VIRTUAL_WORKER_ID;
}

SpatialGDK::QueryConstraint UGridBasedLBStrategy::GetWorkerInterestQueryConstraint() const
{
	// For a grid-based strategy, the interest area is the cell that the worker is authoritative over plus some border region.
	check(IsReady());
	check(bIsStrategyUsedOnLocalWorker);

	const FBox2D Interest2D = WorkerCells[LocalCellId].ExpandBy(InterestBorder);

	const FVector2D Center2D = Interest2D.GetCenter();
	const FVector Center3D{ Center2D.X, Center2D.Y, 0.0f };

	const FVector2D EdgeLengths2D = Interest2D.GetSize();
	check(EdgeLengths2D.X > 0.0f && EdgeLengths2D.Y > 0.0f);
	const FVector EdgeLengths3D{ EdgeLengths2D.X, EdgeLengths2D.Y, FLT_MAX };

	SpatialGDK::QueryConstraint Constraint;
	Constraint.BoxConstraint =
		SpatialGDK::BoxConstraint{ SpatialGDK::Coordinates::FromFVector(Center3D), SpatialGDK::EdgeLength::FromFVector(EdgeLengths3D) };
	return Constraint;
}

FVector UGridBasedLBStrategy::GetWorkerEntityPosition() const
{
	check(IsReady());
	check(bIsStrategyUsedOnLocalWorker);
	const FVector2D Centre = WorkerCells[LocalCellId].GetCenter();
	return FVector{ Centre.X, Centre.Y, 0.f };
}

uint32 UGridBasedLBStrategy::GetMinimumRequiredWorkers() const
{
	return Rows * Cols;
}

void UGridBasedLBStrategy::SetVirtualWorkerIds(const VirtualWorkerId& FirstVirtualWorkerId, const VirtualWorkerId& LastVirtualWorkerId)
{
	UE_LOG(LogGridBasedLBStrategy, Log, TEXT("Setting VirtualWorkerIds %d to %d"), FirstVirtualWorkerId, LastVirtualWorkerId);
	for (VirtualWorkerId CurrentVirtualWorkerId = FirstVirtualWorkerId; CurrentVirtualWorkerId <= LastVirtualWorkerId;
		 CurrentVirtualWorkerId++)
	{
		VirtualWorkerIds.Add(CurrentVirtualWorkerId);
	}
}

bool UGridBasedLBStrategy::IsInside(const FBox2D& Box, const FVector2D& Location)
{
	return Location.X >= Box.Min.X && Location.Y >= Box.Min.Y && Location.X < Box.Max.X && Location.Y < Box.Max.Y;
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
