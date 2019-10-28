// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "EngineClasses/AbstractLBStrategy.h"
#include "GridBasedLBStrategy.generated.h"

/**
 * 
 */
UCLASS(Blueprintable)
class SPATIALGDK_API UGridBasedLBStrategy : public UAbstractLBStrategy
{
	GENERATED_BODY()

public:
	UGridBasedLBStrategy();

/* UAbstractLBStrategy Interface */
	virtual void Init(const class USpatialNetDriver* InNetDriver) override;

	virtual TArray<uint32> GetVirtualWorkerIds() const;

	virtual bool ShouldRelinquishAuthority(const AActor& Actor) const override;
	virtual uint32 WhoShouldHaveAuthority(const AActor& Actor) const override;
/* End UAbstractLBStrategy Interface */

protected:
	UPROPERTY(EditDefaultsOnly, meta = (ClampMin="1"))
	uint32 Rows;

	UPROPERTY(EditDefaultsOnly, meta = (ClampMin = "1"))
	uint32 Cols;

	UPROPERTY(EditDefaultsOnly, meta = (ClampMin = "1"))
	float WorldWidth;

	UPROPERTY(EditDefaultsOnly, meta = (ClampMin = "1"))
	float WorldHeight;

private:

	TArray<uint32> VirtualWorkerIds;

	TArray<FBox2D> WorkerCells;

	static bool IsInside(const FBox2D& Box, const FVector2D& Location);
};
