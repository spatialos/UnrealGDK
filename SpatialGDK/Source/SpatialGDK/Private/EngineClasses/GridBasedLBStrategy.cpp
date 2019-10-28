// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "GridBasedLBStrategy.h"
#include "Utils/SpatialActorUtils.h"

UGridBasedLBStrategy::UGridBasedLBStrategy()
	: Super()
	, Rows(1)
	, Cols(1)
	, WorldWidth(10000.f)
	, WorldHeight(10000.f)
{
}

void UGridBasedLBStrategy::Init(const class USpatialNetDriver* InNetDriver)
{
	Super::Init(InNetDriver);

	for (uint32 i = 1; i <= Rows * Cols; i++)
	{
		VirtualWorkerIds.Add(i);
	}

	// TODO(my) Copied this logic wholesale from prototype
	const float WorldWidthMin = -(WorldWidth / 2.f);
	const float WorldHeightMin = -(WorldHeight / 2.f);

	const float ColumnWidth = WorldWidth / Cols;
	const float RowHeight = WorldHeight / Rows;

	for (uint32 Col = 0; Col < Cols; ++Col)
	{
		for (uint32 Row = 0; Row < Rows; ++Row)
		{
			FVector2D Min(WorldWidthMin + (Col * ColumnWidth), WorldHeightMin + (Row * RowHeight));
			FVector2D Max(Min.X + ColumnWidth, Min.Y + RowHeight);
			FBox2D Cell(Min, Max);
			WorkerCells.Add(MoveTemp(Cell));
		}
	}
}

TArray<uint32> UGridBasedLBStrategy::GetVirtualWorkerIds() const
{
	return VirtualWorkerIds;
}

bool UGridBasedLBStrategy::ShouldRelinquishAuthority(const AActor& Actor) const
{
	if (!IsReady())
	{
		return false;
	}

	const FVector2D Actor2DLocation = FVector2D(SpatialGDK::GetActorSpatialPosition(&Actor));


	return !IsInside(WorkerCells[LocalVirtualWorkerId - 1], Actor2DLocation);
}

uint32 UGridBasedLBStrategy::WhoShouldHaveAuthority(const AActor& Actor) const
{
	if (!IsReady())
	{
		return 0;
	}

	const FVector2D Actor2DLocation = FVector2D(SpatialGDK::GetActorSpatialPosition(&Actor));

	for (int i = 0; i < WorkerCells.Num(); i++)
	{
		if (IsInside(WorkerCells[i], Actor2DLocation))
		{
			return VirtualWorkerIds[i];
		}
	}

	return 0;
}

bool UGridBasedLBStrategy::IsInside(const FBox2D& Box, const FVector2D& Location)
{
	return Location.X >= Box.Min.X && Location.Y >= Box.Min.Y
		&& Location.X < Box.Max.X && Location.Y < Box.Max.Y;
}
