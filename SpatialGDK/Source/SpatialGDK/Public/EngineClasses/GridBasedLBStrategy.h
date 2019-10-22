// Fill out your copyright notice in the Description page of Project Settings.

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

	static UGridBasedLBStrategy* Create(uint32 Rows, uint32 Cols, float WorldWidth, float WorldHeight);

protected:
	UPROPERTY(EditDefaultsOnly, meta = (ClampMin="1"))
	uint32 Rows;

	UPROPERTY(EditDefaultsOnly, meta = (ClampMin = "1"))
	uint32 Cols;

	UPROPERTY(EditDefaultsOnly, meta = (ClampMin = "1"))
	float WorldWidth;

	UPROPERTY(EditDefaultsOnly, meta = (ClampMin = "1"))
	float WorldHeight;

	UPROPERTY(EditDefaultsOnly, meta = (ClampMin = "0"))
	float EdgeFuzziness;

private:

	TArray<uint32> VirtualWorkerIds;

	TArray<FBox2D> WorkerCells;
};
