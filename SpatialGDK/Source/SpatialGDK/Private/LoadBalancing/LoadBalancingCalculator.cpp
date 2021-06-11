#include "LoadBalancing/LoadBalancingCalculator.h"
#include "LoadBalancing/LBDataStorage.h"
#include "LoadBalancing/PartitionManager.h"

namespace SpatialGDK
{
FGridBalancingCalculator::FGridBalancingCalculator(uint32 GridX, uint32 GridY, float Height, float Width, float InInterestBorder)
	: Rows(GridY)
	, Cols(GridX)
	, WorldWidth(Width)
	, WorldHeight(Height)
	, InterestBorder(InInterestBorder)
{
}

void FGridBalancingCalculator::CollectPartitionsToAdd(const FString& Prefix, FPartitionManager& PartitionMgr,
													  TArray<FPartitionHandle>& OutPartitions)
{
	if (Partitions.Num() == 0)
	{
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
				Cells.Add(Cell);

				const FVector2D Center2D = Cell.GetCenter();
				const FVector Center3D{ Center2D.X, Center2D.Y, 0.0f };

				const FVector2D EdgeLengths2D = Cell.GetSize();
				check(EdgeLengths2D.X > 0.0f && EdgeLengths2D.Y > 0.0f);
				const FVector EdgeLengths3D{ EdgeLengths2D.X + InterestBorder, EdgeLengths2D.Y + InterestBorder, FLT_MAX };

				SpatialGDK::QueryConstraint Constraint;
				Constraint.BoxConstraint = SpatialGDK::BoxConstraint{ SpatialGDK::Coordinates::FromFVector(Center3D),
																	  SpatialGDK::EdgeLength::FromFVector(EdgeLengths3D) };

				FString DisplayName = FString::Printf(TEXT("GridCell %i (%i, %i)"), Cells.Num() - 1, Row, Col);
				if (!Prefix.IsEmpty())
				{
					DisplayName = Prefix + TEXT(", ") + DisplayName;
				}
				FPartitionHandle NewPartition = PartitionMgr.CreatePartition(DisplayName, nullptr, Constraint);
				Partitions.Add(NewPartition);

				OutPartitions.Add(NewPartition);

				XMin = XMax;
			}

			XMin = WorldHeightMin;
			YMin = YMax;
		}
	}
}

void FGridBalancingCalculator::CollectEntitiesToMigrate(FMigrationContext& Ctx)
{
	TMap<Worker_EntityId_Key, FVector> const& PositionData = DataStorage->GetPositions();
	TSet<Worker_EntityId_Key> NotChecked;
	ToRefresh = ToRefresh.Union(DataStorage->GetModifiedEntities());
	for (Worker_EntityId EntityId : ToRefresh)
	{
		if (Ctx.MigratingEntities.Contains(EntityId))
		{
			NotChecked.Add(EntityId);
			continue;
		}

		const FVector& Position = PositionData.FindChecked(EntityId);
		const FVector2D Actor2DLocation(Position);

		int32& CurAssignment = Assignment.FindOrAdd(EntityId, -1);

		if (CurAssignment >= 0 && CurAssignment < Cells.Num())
		{
			if (Cells[CurAssignment].IsInside(Actor2DLocation))
			{
				continue;
			}
		}
		int32 NewAssignment = -1;
		for (int i = 0; i < Cells.Num(); i++)
		{
			if (Cells[i].IsInside(Actor2DLocation))
			{
				NewAssignment = i;
			}
		}

		if (NewAssignment >= 0 && NewAssignment < Cells.Num() && ensure(NewAssignment != CurAssignment))
		{
			CurAssignment = NewAssignment;
			Ctx.EntitiesToMigrate.Add(EntityId, Partitions[NewAssignment]);
		}
	}
	ToRefresh = MoveTemp(NotChecked);
}

FLayerLoadBalancingCalculator::FLayerLoadBalancingCalculator(TArray<FName> InLayerNames,
															 TArray<TUniquePtr<FLoadBalancingCalculator>>&& InLayers)
	: LayerNames(InLayerNames)
	, Layers(MoveTemp(InLayers))
{
}

void FLayerLoadBalancingCalculator::CollectPartitionsToAdd(const FString& Prefix, FPartitionManager& PartitionMgr,
														   TArray<FPartitionHandle>& OutPartitions)
{
	for (int32 i = 0; i < Layers.Num(); ++i)
	{
		FString DisplayName = FString::Printf(TEXT("Layer %i"), i);
		if (!Prefix.IsEmpty())
		{
			DisplayName = Prefix + TEXT(", ") + DisplayName;
		}
		FLoadBalancingCalculator* Calculator = Layers[i].Get();
		Layers[i]->CollectPartitionsToAdd(DisplayName, PartitionMgr, OutPartitions);
	}
}

void FLayerLoadBalancingCalculator::CollectEntitiesToMigrate(FMigrationContext& Ctx)
{
	TSet<Worker_EntityId_Key> FilteredSet;
	for (int32 LayerIdx = 0; LayerIdx < Layers.Num(); ++LayerIdx)
	{
		TUniquePtr<FLoadBalancingCalculator>& Calculator = Layers[LayerIdx];
		FilteredSet.Empty();
		for (auto EntityId : Ctx.ModifiedEntities)
		{
			const int32* Group = GroupStorage->GetGroups().Find(EntityId);
			if (Group && *Group == LayerIdx)
			{
				FilteredSet.Add(EntityId);
			}
		}

		FMigrationContext LayerCtx(Ctx.MigratingEntities, FilteredSet);
		Calculator->CollectEntitiesToMigrate(LayerCtx);
		for (auto MigrationEntry : LayerCtx.EntitiesToMigrate)
		{
			Ctx.EntitiesToMigrate.Add(MigrationEntry);
		}
	}
}
} // namespace SpatialGDK
