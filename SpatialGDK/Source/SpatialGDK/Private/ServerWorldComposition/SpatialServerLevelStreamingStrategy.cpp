#include "ServerWorldComposition/SpatialServerLevelStreamingStrategy.h"
#include "Engine/WorldComposition.h"
#include "EngineClasses/SpatialNetDriver.h"
#include "LoadBalancing/AbstractLBStrategy.h"
#include "SpatialConstants.h"

DEFINE_LOG_CATEGORY(LogSpatialServerLevelStreamingStrategy)

USpatialServerLevelStreamingStrategy::USpatialServerLevelStreamingStrategy()
	: Super()
	, TileVisibilityResults{}
{
}

void USpatialServerLevelStreamingStrategy::InitialiseStrategy(const FTilesList& Tiles, const FIntVector& OriginLocation)
{
	const USpatialNetDriver* SpatialNetDriver = Cast<USpatialNetDriver>(GetWorld()->GetNetDriver());
	const UAbstractLBStrategy* LoadBalanceStrategy = SpatialNetDriver->LoadBalanceStrategy;
	TileVisibilityResults.Empty();

	for (VirtualWorkerId WorkerId = SpatialConstants::INVALID_VIRTUAL_WORKER_ID + 1;
		 WorkerId <= LoadBalanceStrategy->GetMinimumRequiredWorkers(); WorkerId++)
	{
		const int32 NumTiles = Tiles.Num();

		TArray<FVisibilityResult> Results;

		Results.Empty();
		Results.SetNum(NumTiles);

		for (int32 TileIdx = 0; TileIdx < NumTiles; TileIdx++)
		{
			const FWorldCompositionTile& Tile = Tiles[TileIdx];
			FVisibilityResult VisibilityResult = GenerateVisibilityResultForTile(Tile, OriginLocation, WorkerId);
			Results[TileIdx] = VisibilityResult;
			if (VisibilityResult.bShouldBeLoaded)
			{
				MarkLevelLoaded(Tile, WorkerId);
			}
		}

		TileVisibilityResults.Emplace(WorkerId, Results);
	}
}

void USpatialServerLevelStreamingStrategy::MarkLevelLoaded(const FWorldCompositionTile& Tile, VirtualWorkerId Vid)
{
	if (!LoadedLevelNames.Contains(Vid))
	{
		LoadedLevelNames.Emplace(Vid, TSet<FName>());
	}

	LoadedLevelNames[Vid].Add(Tile.PackageName);
	UE_LOG(LogSpatialServerLevelStreamingStrategy, Log, TEXT("Virtual worker %d should load tile: %s"), Vid, *Tile.PackageName.ToString());
}

TSet<FName> USpatialServerLevelStreamingStrategy::GetLoadedLevelNames(VirtualWorkerId Vid) const
{
	if (LoadedLevelNames.Contains(Vid))
	{
		return LoadedLevelNames[Vid];
	}
	return {};
}

FVisibilityResult USpatialServerLevelStreamingStrategy::GetVisibilityResultForTile(int32 TileIdx)
{
	const USpatialNetDriver* SpatialNetDriver = Cast<USpatialNetDriver>(GetWorld()->GetNetDriver());
	const UAbstractLBStrategy* LoadBalanceStrategy = SpatialNetDriver->LoadBalanceStrategy;

	if (!LoadBalanceStrategy->IsReady())
	{
		UE_LOG(LogSpatialServerLevelStreamingStrategy, Verbose, TEXT("GetVisibilityResultForTile: Load balance strategy is not ready."));
		return { false };
	}

	VirtualWorkerId Vid = LoadBalanceStrategy->GetLocalVirtualWorkerId();
	checkf(TileVisibilityResults.Contains(Vid), TEXT("Getting visibility for tile for virtual worker id that hasn't been registered."));

	const TArray<FVisibilityResult>& Result = TileVisibilityResults[Vid];
	checkf(TileIdx < Result.Num(), TEXT("Getting visibility for tile id that hasn't been registered."));

	return Result[TileIdx];
}
