// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "LoadBalancing/AbstractLBStrategy.h"

#include "CoreMinimal.h"
#include "Math/Box2D.h"
#include "Math/Vector2D.h"

#include "GridBasedLBStrategy.generated.h"

class SpatialVirtualWorkerTranslator;
class UAbstractSpatialMultiWorkerSettings;

DECLARE_LOG_CATEGORY_EXTERN(LogGridBasedLBStrategy, Log, All)

/**
 * A load balancing strategy that divides the world into a grid.
 * Divides the load between Rows * Cols number of workers, each handling a
 * square of the world (WorldWidth / Cols)cm by (WorldHeight / Rows)cm
 *
 * Given a Point, for each Cell:
 * Point is inside Cell iff Min(Cell) <= Point < Max(Cell)
 *
 * Intended Usage: Create a data-only blueprint subclass and change
 * the Cols, Rows, WorldWidth, WorldHeight.
 */
UCLASS(Blueprintable, HideDropdown)
class SPATIALGDK_API UGridBasedLBStrategy : public UAbstractLBStrategy
{
	GENERATED_BODY()

public:
	UGridBasedLBStrategy();

	using LBStrategyRegions = TArray<TPair<VirtualWorkerId, FBox2D>>;

	/* UAbstractLBStrategy Interface */
	virtual void Init() override;

	virtual void SetLocalVirtualWorkerId(VirtualWorkerId InLocalVirtualWorkerId) override;
	virtual TSet<VirtualWorkerId> GetVirtualWorkerIds() const override;

	virtual bool ShouldHaveAuthority(const AActor& Actor) const override;
	virtual VirtualWorkerId WhoShouldHaveAuthority(const AActor& Actor) const override;

	virtual SpatialGDK::QueryConstraint GetWorkerInterestQueryConstraint() const override;

	virtual bool RequiresHandoverData() const override { return Rows * Cols > 1; }

	virtual FVector GetWorkerEntityPosition() const override;

	virtual uint32 GetMinimumRequiredWorkers() const override;
	virtual void SetVirtualWorkerIds(const VirtualWorkerId& FirstVirtualWorkerId, const VirtualWorkerId& LastVirtualWorkerId) override;
	/* End UAbstractLBStrategy Interface */

	LBStrategyRegions GetLBStrategyRegions() const;

protected:
	UPROPERTY(EditDefaultsOnly, meta = (ClampMin = "1"), Category = "Grid Based Load Balancing")
	uint32 Rows;

	UPROPERTY(EditDefaultsOnly, meta = (ClampMin = "1"), Category = "Grid Based Load Balancing")
	uint32 Cols;

	UPROPERTY(EditDefaultsOnly, meta = (ClampMin = "1"), Category = "Grid Based Load Balancing")
	float WorldWidth;

	UPROPERTY(EditDefaultsOnly, meta = (ClampMin = "1"), Category = "Grid Based Load Balancing")
	float WorldHeight;

	UPROPERTY(EditDefaultsOnly, meta = (ClampMin = "0"), Category = "Grid Based Load Balancing")
	float InterestBorder;

private:
	TArray<VirtualWorkerId> VirtualWorkerIds;

	TArray<FBox2D> WorkerCells;
	uint32 LocalCellId;
	bool bIsStrategyUsedOnLocalWorker;

	static bool IsInside(const FBox2D& Box, const FVector2D& Location);
};

UCLASS(Blueprintable)
class SPATIALGDK_API USingleWorkerStrategy : public UGridBasedLBStrategy
{
	GENERATED_BODY()

public:
	USingleWorkerStrategy()
	{
		Rows = 1;
		Cols = 1;
		WorldWidth = 1000000.f;
		WorldHeight = 1000000.f;
		InterestBorder = 0.f;
	}
};
