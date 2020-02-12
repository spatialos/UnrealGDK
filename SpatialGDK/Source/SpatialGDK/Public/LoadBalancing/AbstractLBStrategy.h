// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "SpatialCommonTypes.h"
#include "SpatialConstants.h"

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"

#include "AbstractLBStrategy.generated.h"

namespace SpatialGDK
{
	struct Query;
}
class USpatialClassInfoManager;
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
 *   a) Check if authority should be relinquished by calling ShouldHaveAuthority
 *   b) If true: Send authority change request to Translator/Enforcer passing in new
 *        VirtualWorkerId returned by WhoShouldHaveAuthority
 */
UCLASS(abstract)
class SPATIALGDK_API UAbstractLBStrategy : public UObject
{
	GENERATED_BODY()

public:
	UAbstractLBStrategy();

	virtual void Init(const USpatialNetDriver* InNetDriver) {}

	bool IsReady() const { return LocalVirtualWorkerId != SpatialConstants::INVALID_VIRTUAL_WORKER_ID; }

	void SetLocalVirtualWorkerId(VirtualWorkerId LocalVirtualWorkerId);

	virtual TSet<VirtualWorkerId> GetVirtualWorkerIds() const PURE_VIRTUAL(UAbstractLBStrategy::GetVirtualWorkerIds, return {};)

	virtual bool ShouldHaveAuthority(const AActor& Actor) const { return false; }
	virtual VirtualWorkerId WhoShouldHaveAuthority(const AActor& Actor) const PURE_VIRTUAL(UAbstractLBStrategy::WhoShouldHaveAuthority, return SpatialConstants::INVALID_VIRTUAL_WORKER_ID;)

	/**
		* Add any interest queries required by this worker based on the load balancing strategy used.
		*/
	virtual void CreateWorkerInterestQueries(TArray<SpatialGDK::Query>& OutQueries) const PURE_VIRTUAL(UAbstractLBStrategy::CreateWorkerInterestQueries, )

protected:

	VirtualWorkerId LocalVirtualWorkerId;
};
