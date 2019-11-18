// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "SpatialConstants.h"
#include "UObject/NoExportTypes.h"
#include "AbstractLBStrategy.generated.h"

class USpatialNetDriver;

/**
 * This class can be used to define a load balancing strategy.
 * At runtime, all unreal workers will:
 * 1. Instantiate an instance of the strategy class specified in TODO: where are we adding this?
 * 2. Call the Init method, passing the current USpatialNetDriver.
 * 3. (Translator / Enforcer only): Initialize Worker to VirtualWorkerId mapping with
 *      VirtualWorkerIds from GetVirtualWorkerIds() and begin assinging workers.
 *    (Other Workers): SetLocalVirtualWorkerId when assigned a VirtualWorkerId.
 * 4. For each Actor being replicated:
 *   a) Check if authority should be relinquished by calling ShouldRelinquishAuthority
 *   b) If true: Send authority change request to Translator/Enforcer passing in new
 *        VirtualWorkerId returned by WhoShouldHaveAuthority
 */
UCLASS(abstract)
class SPATIALGDK_API UAbstractLBStrategy : public UObject
{
	GENERATED_BODY()

public:
	UAbstractLBStrategy();

	virtual void Init(const USpatialNetDriver* InNetDriver);

	bool IsReady() const { return LocalVirtualWorkerId != SpatialConstants::INVALID_VIRTUAL_WORKER_ID; }

	void SetLocalVirtualWorkerId(VirtualWorkerId LocalVirtualWorkerId);
	VirtualWorkerId GetLocalVirtualWorkerId() const { return LocalVirtualWorkerId; }

	virtual TSet<VirtualWorkerId> GetVirtualWorkerIds() const PURE_VIRTUAL(UAbstractLBStrategy::GetVirtualWorkerIds, return {};)

	virtual bool ShouldRelinquishAuthority(const AActor& Actor) const { return false; }
	virtual VirtualWorkerId WhoShouldHaveAuthority(const AActor& Actor) const PURE_VIRTUAL(UAbstractLBStrategy::WhoShouldHaveAuthority, return SpatialConstants::INVALID_VIRTUAL_WORKER_ID; )

protected:

	VirtualWorkerId LocalVirtualWorkerId;
};
