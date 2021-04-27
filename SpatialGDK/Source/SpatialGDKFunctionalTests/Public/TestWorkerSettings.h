// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "LoadBalancing/GridBasedLBStrategy.h"
#include "LoadBalancing/SpatialMultiWorkerSettings.h"

#include "TestWorkerSettings.generated.h"

/**
 * A selection of load balancing strategies for testing zoning features.
 */

/**
 * Full interest grid strategies
 * The strategies in this group have a world-wide interest border, so everything should be in view.
 * UTest1x2FullInterestGridStrategy
 * UTest2x1FullInterestGridStrategy
 * UTest2x2FullInterestGridStrategy
 */
UCLASS()
class SPATIALGDKFUNCTIONALTESTS_API UTest1x2FullInterestGridStrategy : public UGridBasedLBStrategy
{
	GENERATED_BODY()

public:
	UTest1x2FullInterestGridStrategy()
	{
		Rows = 1;
		Cols = 2;
		// Maximum of the world size, so that the entire world is in the view of both the server-workers at all times
		InterestBorder = FMath::Max(WorldWidth, WorldHeight);
	}
};

UCLASS()
class SPATIALGDKFUNCTIONALTESTS_API UTest2x1FullInterestGridStrategy : public UGridBasedLBStrategy
{
	GENERATED_BODY()

public:
	UTest2x1FullInterestGridStrategy()
	{
		Rows = 2;
		Cols = 1;
		// Maximum of the world size, so that the entire world is in the view of both the server-workers at all times
		InterestBorder = FMath::Max(WorldWidth, WorldHeight);
	}
};

UCLASS()
class SPATIALGDKFUNCTIONALTESTS_API UTest2x2FullInterestGridStrategy : public UGridBasedLBStrategy
{
	GENERATED_BODY()

public:
	UTest2x2FullInterestGridStrategy()
	{
		Rows = 2;
		Cols = 2;
		// Maximum of the world size, so that the entire world is in the view of both the server-workers at all times
		InterestBorder = FMath::Max(WorldWidth, WorldHeight);
	}
};

/**
 * Small interest grid strategies
 * The strategies in this group have a 150 unit interest border, a very small border
 * to create a narrow range in which an actor is visible to both server-workers.
 * UTest1x2SmallInterestGridStrategy
 * UTest2x1SmallInterestGridStrategy
 * UTest2x2SmallInterestGridStrategy
 */
UCLASS()
class SPATIALGDKFUNCTIONALTESTS_API UTest1x2SmallInterestGridStrategy : public UTest1x2FullInterestGridStrategy
{
	GENERATED_BODY()

public:
	UTest1x2SmallInterestGridStrategy() { InterestBorder = 1.0f; }
};

UCLASS()
class SPATIALGDKFUNCTIONALTESTS_API UTest2x1SmallInterestGridStrategy : public UTest2x1FullInterestGridStrategy
{
	GENERATED_BODY()

public:
	UTest2x1SmallInterestGridStrategy() { InterestBorder = 150.0f; }
};

UCLASS()
class SPATIALGDKFUNCTIONALTESTS_API UTest2x2SmallInterestGridStrategy : public UTest2x2FullInterestGridStrategy
{
	GENERATED_BODY()

public:
	UTest2x2SmallInterestGridStrategy() { InterestBorder = 150.0f; }
};

/**
 * No interest grid strategies
 * The strategies in this group have no interest border, so the
 * workers' interest will be limited to their authority region.
 * UTest1x2NoInterestGridStrategy
 * UTest2x1NoInterestGridStrategy
 * UTest2x2NoInterestGridStrategy
 */
UCLASS()
class SPATIALGDKFUNCTIONALTESTS_API UTest1x2NoInterestGridStrategy : public UTest1x2FullInterestGridStrategy
{
	GENERATED_BODY()

public:
	UTest1x2NoInterestGridStrategy() { InterestBorder = 0.0f; }
};

UCLASS()
class SPATIALGDKFUNCTIONALTESTS_API UTest2x1NoInterestGridStrategy : public UTest2x1FullInterestGridStrategy
{
	GENERATED_BODY()

public:
	UTest2x1NoInterestGridStrategy() { InterestBorder = 0.0f; }
};

UCLASS()
class SPATIALGDKFUNCTIONALTESTS_API UTest2x2NoInterestGridStrategy : public UTest2x2FullInterestGridStrategy
{
	GENERATED_BODY()

public:
	UTest2x2NoInterestGridStrategy() { InterestBorder = 0.0f; }
};

/**
 * Worker settings that use the above LB strategies.
 */
UCLASS()
class SPATIALGDKFUNCTIONALTESTS_API UTest1x2FullInterestWorkerSettings : public USpatialMultiWorkerSettings
{
	GENERATED_BODY()

public:
	UTest1x2FullInterestWorkerSettings() { WorkerLayers[0].LoadBalanceStrategy = UTest1x2FullInterestGridStrategy::StaticClass(); }
};

UCLASS()
class SPATIALGDKFUNCTIONALTESTS_API UTest2x1FullInterestWorkerSettings : public USpatialMultiWorkerSettings
{
	GENERATED_BODY()

public:
	UTest2x1FullInterestWorkerSettings() { WorkerLayers[0].LoadBalanceStrategy = UTest2x1FullInterestGridStrategy::StaticClass(); }
};

UCLASS()
class SPATIALGDKFUNCTIONALTESTS_API UTest2x2FullInterestWorkerSettings : public USpatialMultiWorkerSettings
{
	GENERATED_BODY()

public:
	UTest2x2FullInterestWorkerSettings() { WorkerLayers[0].LoadBalanceStrategy = UTest2x2FullInterestGridStrategy::StaticClass(); }
};

UCLASS()
class SPATIALGDKFUNCTIONALTESTS_API UTest1x2SmallInterestWorkerSettings : public USpatialMultiWorkerSettings
{
	GENERATED_BODY()

public:
	UTest1x2SmallInterestWorkerSettings() { WorkerLayers[0].LoadBalanceStrategy = UTest1x2SmallInterestGridStrategy::StaticClass(); }
};

UCLASS()
class SPATIALGDKFUNCTIONALTESTS_API UTest2x1SmallInterestWorkerSettings : public USpatialMultiWorkerSettings
{
	GENERATED_BODY()

public:
	UTest2x1SmallInterestWorkerSettings() { WorkerLayers[0].LoadBalanceStrategy = UTest2x1SmallInterestGridStrategy::StaticClass(); }
};

UCLASS()
class SPATIALGDKFUNCTIONALTESTS_API UTest2x2SmallInterestWorkerSettings : public USpatialMultiWorkerSettings
{
	GENERATED_BODY()

public:
	UTest2x2SmallInterestWorkerSettings() { WorkerLayers[0].LoadBalanceStrategy = UTest2x2SmallInterestGridStrategy::StaticClass(); }
};

UCLASS()
class SPATIALGDKFUNCTIONALTESTS_API UTest1x2NoInterestWorkerSettings : public USpatialMultiWorkerSettings
{
	GENERATED_BODY()

public:
	UTest1x2NoInterestWorkerSettings() { WorkerLayers[0].LoadBalanceStrategy = UTest1x2NoInterestGridStrategy::StaticClass(); }
};

UCLASS()
class SPATIALGDKFUNCTIONALTESTS_API UTest2x1NoInterestWorkerSettings : public USpatialMultiWorkerSettings
{
	GENERATED_BODY()

public:
	UTest2x1NoInterestWorkerSettings() { WorkerLayers[0].LoadBalanceStrategy = UTest2x1NoInterestGridStrategy::StaticClass(); }
};

UCLASS()
class SPATIALGDKFUNCTIONALTESTS_API UTest2x2NoInterestWorkerSettings : public USpatialMultiWorkerSettings
{
	GENERATED_BODY()

public:
	UTest2x2NoInterestWorkerSettings() { WorkerLayers[0].LoadBalanceStrategy = UTest2x2NoInterestGridStrategy::StaticClass(); }
};
