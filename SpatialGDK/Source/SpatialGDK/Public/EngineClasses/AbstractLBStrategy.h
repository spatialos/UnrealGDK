// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "AbstractLBStrategy.generated.h"

class USpatialNetDriver;

/**
 * 
 */
UCLASS(abstract)
class SPATIALGDK_API UAbstractLBStrategy : public UObject
{
	GENERATED_BODY()

public:
	UAbstractLBStrategy();

	virtual void Init(const class USpatialNetDriver* InNetDriver);

	bool IsReady() const { return LocalVirtualWorkerId != 0; }

	void SetLocalVirtualWorkerId(uint32 LocalVirtualWorkerId);

	virtual TArray<uint32> GetVirtualWorkerIds() const PURE_VIRTUAL(UAbstractLBStrategy::GetVirtualWorkerIds, return {} ; )

	virtual bool ShouldRelinquishAuthority(const AActor& Actor) const { return false; }
	virtual uint32 WhoShouldHaveAuthority(const AActor& Actor) const PURE_VIRTUAL(UAbstractLBStrategy::WhoShouldHaveAuthority, return 0; )

protected:

	uint32 LocalVirtualWorkerId;

	const class USpatialNetDriver* NetDriver;

};
