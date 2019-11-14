// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "LoadBalancing/GridBasedLBStrategy.h"

#include "EngineClasses/SpatialNetDriver.h"
#include "Utils/SpatialActorUtils.h"

UGridBasedLBStrategy::UGridBasedLBStrategy()
	: Super()
	, Rows(1)
	, Cols(1)
	, WorldWidth(10000.f)
	, WorldHeight(10000.f)
{
}

void UGridBasedLBStrategy::Init(const USpatialNetDriver* InNetDriver, const SpatialVirtualWorkerTranslator* SpatialVirtualWorkerTranslator)
{
	Super::Init(InNetDriver, SpatialVirtualWorkerTranslator);

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
		return false;
	}

	const FVector2D Actor2DLocation = FVector2D(SpatialGDK::GetActorSpatialPosition(&Actor));


	return !IsInside(WorkerCells[GetLocalVirtualWorkerId() - 1], Actor2DLocation);
}

VirtualWorkerId UGridBasedLBStrategy::WhoShouldHaveAuthority(const AActor& Actor) const
{
	if (!IsReady())
	{
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

bool UGridBasedLBStrategy::IsInside(const FBox2D& Box, const FVector2D& Location)
{
	return Location.X >= Box.Min.X && Location.Y >= Box.Min.Y
		&& Location.X < Box.Max.X && Location.Y < Box.Max.Y;
}
