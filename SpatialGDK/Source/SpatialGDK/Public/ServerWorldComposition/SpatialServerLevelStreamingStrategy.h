#pragma once

#include "Engine/AbstractServerLevelStreamingStrategy.h"
#include "SpatialCommonTypes.h"
#include "SpatialServerLevelStreamingStrategy.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogSpatialServerLevelStreamingStrategy, Log, All);

class UAbstractLBStrategy;

UCLASS(abstract, NotBlueprintable)
class SPATIALGDK_API USpatialServerLevelStreamingStrategy : public UAbstractServerLevelStreamingStrategy
{
	GENERATED_BODY()

public:
	USpatialServerLevelStreamingStrategy();

	virtual void InitialiseStrategy(const FTilesList& Tiles, const FIntVector& OriginLocation);

	virtual FVisibilityResult GetVisibilityResultForTile(int32 TileIdx) override;
	virtual TSet<FName> GetLoadedLevelNames(VirtualWorkerId Vid) const;

protected:
	// Determine whether the Tile should be loaded, and at what LOD.
	virtual FVisibilityResult GenerateVisibilityResultForTile(const FWorldCompositionTile& Tile, const FIntVector& OriginLocation,
															  VirtualWorkerId Vid) const
		PURE_VIRTUAL(UAbstractServerLevelStreamingStrategy::GenerateVisibilityResultForTile, return { false };);

	virtual void MarkLevelLoaded(const FWorldCompositionTile& Tile, VirtualWorkerId Vid);

	TMap<VirtualWorkerId, TArray<FVisibilityResult>> TileVisibilityResults;
	TMap<VirtualWorkerId, TSet<FName>> LoadedLevelNames;
};
