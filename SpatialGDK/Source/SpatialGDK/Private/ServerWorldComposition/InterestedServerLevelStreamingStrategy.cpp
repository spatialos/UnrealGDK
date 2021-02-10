#include "ServerWorldComposition/InterestedServerLevelStreamingStrategy.h"

#include "Engine/WorldComposition.h"
#include "EngineClasses/SpatialNetDriver.h"
#include "LoadBalancing/AbstractLBStrategy.h"

DEFINE_LOG_CATEGORY(LogInterestedServerLevelStreamingStrategy);

FVisibilityResult UInterestedServerLevelStreamingStrategy::GenerateVisibilityResultForTile(const FWorldCompositionTile& Tile,
																						   const FIntVector& OriginLocation,
																						   VirtualWorkerId Vid) const
{
	if (IsTileVisibleFromInterest(Tile, OriginLocation, Vid))
	{
		return FVisibilityResult{ true };
	}

	return FVisibilityResult{ false };
}

bool UInterestedServerLevelStreamingStrategy::IsTileVisibleFromInterest(const FWorldCompositionTile& Tile, const FIntVector& OriginLocation,
																		VirtualWorkerId Vid) const
{
	const USpatialNetDriver* SpatialNetDriver = Cast<USpatialNetDriver>(GetWorld()->GetNetDriver());
	const UAbstractLBStrategy* LoadBalanceStrategy = SpatialNetDriver->LoadBalanceStrategy;

	if (!LoadBalanceStrategy)
	{
		UE_LOG(LogInterestedServerLevelStreamingStrategy, Error,
			   TEXT("No load balancing strategy set for InterestedServerLevelStreamingStrategy. All tiles will be loaded."));
		return true;
	}

	const SpatialGDK::QueryConstraint WorkerInterestQuery = LoadBalanceStrategy->GetWorkerInterestQueryConstraint(Vid);
	const auto BoxConstraint = WorkerInterestQuery.BoxConstraint;

	if (!BoxConstraint.IsSet())
	{
		UE_LOG(LogInterestedServerLevelStreamingStrategy, Error,
			   TEXT("Load balancing strategy does not use single box constraint for interest. Only load balancing strategies with single "
					"box constraints are currently supported. All tiles will be loaded."));
		return true;
	}

	SpatialGDK::Coordinates SpatialCenter = BoxConstraint->Center;
	SpatialGDK::EdgeLength SpatialEdgeLength = BoxConstraint->EdgeLength;

	FVector Center3D = SpatialGDK::Coordinates::ToFVector(SpatialCenter);
	FVector EdgeLength3D = SpatialGDK::EdgeLength::ToFVector(SpatialEdgeLength);

	FVector2D Center2D = FVector2D(Center3D.X, Center3D.Y);
	FVector2D EdgeLength2D = FVector2D(EdgeLength3D.X, EdgeLength3D.Y);

	FBox2D InterestBounds(Center2D - EdgeLength2D / 2.0f, Center2D + EdgeLength2D / 2.0f);

	// Calculate bounds of level in world coordinates
	FIntPoint WorldOriginLocation2D = FIntPoint(OriginLocation.X, OriginLocation.Y);
	FIntPoint LevelPosition2D = FIntPoint(Tile.Info.AbsolutePosition.X, Tile.Info.AbsolutePosition.Y);
	FIntPoint LevelOffset2D = LevelPosition2D - WorldOriginLocation2D;
	FBox TileBounds3D = Tile.Info.Bounds.ShiftBy(FVector(LevelOffset2D));
	FBox2D TileBounds2D(FVector2D(TileBounds3D.Min.X, TileBounds3D.Min.Y), FVector2D(TileBounds3D.Max.X, TileBounds3D.Max.Y));

	return TileBounds2D.Intersect(InterestBounds);
}
