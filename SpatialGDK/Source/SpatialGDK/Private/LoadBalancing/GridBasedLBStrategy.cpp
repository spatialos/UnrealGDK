// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "LoadBalancing/GridBasedLBStrategy.h"

#include "EngineClasses/SpatialNetDriver.h"
#include "Utils/SpatialActorUtils.h"
#include "Schema/Interest.h"
#include "Schema/AuthorityIntent.h"
#include "EngineClasses/SpatialPackageMapClient.h"
#include "Interop/SpatialStaticComponentView.h"

DEFINE_LOG_CATEGORY(LogGridBasedLBStrategy);

UGridBasedLBStrategy::UGridBasedLBStrategy()
	: Super()
	, Rows(1)
	, Cols(1)
	, WorldWidth(10000.f)
	, WorldHeight(10000.f)
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

	USpatialNetDriver* NetDriver = Cast<USpatialNetDriver>(Actor.GetNetDriver());
	check(NetDriver);

	const Worker_EntityId EntityId = NetDriver->PackageMap->GetEntityIdFromObject(&Actor);
	check(EntityId != SpatialConstants::INVALID_ENTITY_ID);
	check(NetDriver->StaticComponentView->GetAuthority(EntityId, SpatialConstants::AUTHORITY_INTENT_COMPONENT_ID));

	SpatialGDK::AuthorityIntent* AuthorityIntentComponent = NetDriver->StaticComponentView->GetComponentData<SpatialGDK::AuthorityIntent>(EntityId);
	if (AuthorityIntentComponent->VirtualWorkerId == SpatialConstants::INVALID_VIRTUAL_WORKER_ID) return true;

	FVector2D Actor2DLocation = FVector2D(SpatialGDK::GetActorSpatialPosition(&Actor));
	// hack to prevent things going out of bounds
	Actor2DLocation = FVector2D(FMath::Clamp(FMath::RoundToInt(Actor2DLocation.X / 10.f) * 10.f, -WorldWidth / 2, WorldWidth / 2), FMath::Clamp(FMath::RoundToInt(Actor2DLocation.Y / 10.f) * 10.f, -WorldHeight / 2, WorldHeight / 2));
	return !IsInside(WorkerCells[LocalVirtualWorkerId - 1], Actor2DLocation);
}

VirtualWorkerId UGridBasedLBStrategy::WhoShouldHaveAuthority(const AActor& Actor) const
{
	if (!IsReady())
	{
		UE_LOG(LogGridBasedLBStrategy, Warning, TEXT("GridBasedLBStrategy not ready to decide on authority for Actor %s."), *AActor::GetDebugName(&Actor));
		return SpatialConstants::INVALID_VIRTUAL_WORKER_ID;
	}

	FVector2D Actor2DLocation = FVector2D(SpatialGDK::GetActorSpatialPosition(&Actor));
	// hack to prevent things going out of bounds
	Actor2DLocation = FVector2D(FMath::Clamp(FMath::RoundToInt(Actor2DLocation.X / 10.f) * 10.f, -WorldWidth / 2, WorldWidth / 2), FMath::Clamp(FMath::RoundToInt(Actor2DLocation.Y / 10.f) * 10.f, -WorldHeight / 2, WorldHeight / 2));

	for (int i = 0; i < WorkerCells.Num(); i++)
	{
		if (IsInside(WorkerCells[i], Actor2DLocation))
		{
			return VirtualWorkerIds[i];
		}
	}

	// This should be unreachable because of the hack above
	UE_LOG(LogGridBasedLBStrategy, Warning, TEXT("Actor %s outside of specified Grid [%f,%f]"), *AActor::GetDebugName(&Actor), Actor2DLocation.X, Actor2DLocation.Y);
	return SpatialConstants::INVALID_VIRTUAL_WORKER_ID;
}


FVector UGridBasedLBStrategy::GetLocalVirtualWorkerActorPosition() const
{
	if (LocalVirtualWorkerId == SpatialConstants::INVALID_VIRTUAL_WORKER_ID || (int32) LocalVirtualWorkerId > WorkerCells.Num())
	{
		UE_LOG(LogGridBasedLBStrategy, Warning, TEXT("Tried to get actor position for invalid virtual worker ID %d"), LocalVirtualWorkerId);
		return FVector();
	}
	return FVector(WorkerCells[LocalVirtualWorkerId - 1].GetCenter(), 0);
}


void UGridBasedLBStrategy::UpdateLocalWorkerInterest(SpatialGDK::Interest* ServerWorkerInterest) const
{
	if (LocalVirtualWorkerId == SpatialConstants::INVALID_VIRTUAL_WORKER_ID || (int32)LocalVirtualWorkerId > WorkerCells.Num())
	{
		UE_LOG(LogGridBasedLBStrategy, Warning, TEXT("Invalid local virtual worker ID %d when adjusting query"), LocalVirtualWorkerId);
		return;
	}
	SpatialGDK::ComponentInterest& ComponentInterest = ServerWorkerInterest->ComponentInterestMap[SpatialConstants::POSITION_COMPONENT_ID];
	if (ComponentInterest.Queries.Num() == 1)
	{
		const FBox2D& Box = WorkerCells[LocalVirtualWorkerId - 1];
		const FVector2D BoxCenter = Box.GetCenter();
		SpatialGDK::Query& query = ComponentInterest.Queries[0];
		SpatialGDK::QueryConstraint QueryConstraint;
		FVector2D Size = Box.GetSize();
		SpatialGDK::EdgeLength Length;
		Length.X = (Size.X * 0.01) + 50;
		Length.Z = (Size.Y * 0.01) + 50;
		Length.Y = 10000;
		SpatialGDK::BoxConstraint BoxConstraint;
		BoxConstraint.EdgeLength = Length;
		BoxConstraint.Center = SpatialGDK::Coordinates{ BoxCenter.Y * 0.01, 0, BoxCenter.X * 0.01 };
		QueryConstraint.BoxConstraint = BoxConstraint;
		query.Constraint.OrConstraint.Add(QueryConstraint);
	}
	else
	{
		UE_LOG(LogGridBasedLBStrategy, Warning, TEXT("Failed to adjust QBI query for server worker entity, query format changed."));
	}
}

bool UGridBasedLBStrategy::IsInside(const FBox2D& Box, const FVector2D& Location)
{
	return Location.X >= Box.Min.X && Location.Y >= Box.Min.Y
		&& Location.X < Box.Max.X && Location.Y < Box.Max.Y;
}
