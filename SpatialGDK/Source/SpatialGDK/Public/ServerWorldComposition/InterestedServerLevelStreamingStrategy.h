#pragma once

#include "SpatialServerLevelStreamingStrategy.h"
#include "InterestedServerLevelStreamingStrategy.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogInterestedServerLevelStreamingStrategy, Log, All)

UCLASS(Blueprintable)
class SPATIALGDK_API UInterestedServerLevelStreamingStrategy : public USpatialServerLevelStreamingStrategy
{
	GENERATED_BODY()

protected:
	virtual FVisibilityResult GenerateVisibilityResultForTile(const FWorldCompositionTile& Tile, const FIntVector& OriginLocation,
															  VirtualWorkerId Vid) const override;


	virtual FVisibilityResult GetVisibilityResultForLevel(const FVector& LevelPosition, const float LevelExtent, const FIntVector& OriginLocation) const override;

	/**
	Reads interest query from load balancing strategy to determine whether a tile should be visible.
	Currently only supports strategies with a single BoxConstraint.
	 */
	bool IsTileVisibleFromInterest(const FWorldCompositionTile& Tile, const FIntVector& OriginLocation, VirtualWorkerId Vid) const;
	bool IsTileVisibleFromInterest(const int Level, const FVector& LevelPosition, const float LevelExtent, const FIntVector& OriginLocation,
								   VirtualWorkerId Vid) const;
};
