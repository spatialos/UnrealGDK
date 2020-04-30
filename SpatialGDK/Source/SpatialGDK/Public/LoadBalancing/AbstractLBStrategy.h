// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "SpatialCommonTypes.h"
#include "SpatialConstants.h"

#include "CoreMinimal.h"
#include "Schema/Interest.h"
#include "UObject/NoExportTypes.h"

#include "AbstractLBStrategy.generated.h"

/**
 * This class can be used to define a load balancing strategy.
 * At runtime, all unreal workers will:
 * 1. Instantiate an instance of the strategy class specified in TODO: where are we adding this?
 * 2. Call the Init method, passing the current USpatialNetDriver.
 * 3. (Translator authoritative worker only): Initialize Worker to VirtualWorkerId mapping with
 *      VirtualWorkerIds from GetVirtualWorkerIds() and begin assigning workers.
 *    (Other Workers): SetLocalVirtualWorkerId when assigned a VirtualWorkerId.
 * 4. For each Actor being replicated:
 *   a) Check if authority should be relinquished by calling ShouldHaveAuthority
 *   b) If true: Update AuthorityDelegation mapping.
 */
UCLASS(abstract)
class SPATIALGDK_API UAbstractLBStrategy : public UObject
{
	GENERATED_BODY()

public:
	UAbstractLBStrategy();

	virtual void Init() {}

	bool IsReady() const { return LocalVirtualWorkerId != SpatialConstants::INVALID_VIRTUAL_WORKER_ID; }

	VirtualWorkerId GetLocalVirtualWorkerId() const { return LocalVirtualWorkerId; };
	virtual void SetLocalVirtualWorkerId(VirtualWorkerId LocalVirtualWorkerId);

	// Deprecated: will be removed ASAP.
	virtual TSet<VirtualWorkerId> GetVirtualWorkerIds() const PURE_VIRTUAL(UAbstractLBStrategy::GetVirtualWorkerIds, return {};)

	virtual bool ShouldHaveAuthority(const AActor& Actor) const { return false; }
	virtual VirtualWorkerId WhoShouldHaveAuthority(const AActor& Actor) const PURE_VIRTUAL(UAbstractLBStrategy::WhoShouldHaveAuthority, return SpatialConstants::INVALID_VIRTUAL_WORKER_ID;)

	/**
	* Get the query constraints required by this worker based on the load balancing strategy used.
	*/
	virtual SpatialGDK::QueryConstraint GetWorkerInterestQueryConstraint(const VirtualWorkerId VirtualWorker) const PURE_VIRTUAL(UAbstractLBStrategy::GetWorkerInterestQueryConstraint, return {};)

	/** True if this load balancing strategy requires handover data to be transmitted. */
	virtual bool RequiresHandoverData() const PURE_VIRTUAL(UAbstractLBStrategy::RequiresHandover, return false;)

	/**
	* Get a logical worker entity position for this strategy. For example, the centre of a grid square in a grid-based strategy. Optional- otherwise returns the origin.
	*/
	virtual FVector GetWorkerEntityPosition() const { return FVector::ZeroVector; }

	/**
	 * GetMinimumRequiredWorkers and SetVirtualWorkerIds are used to assign ranges of virtual worker IDs which will be managed by this strategy.
	 * LastVirtualWorkerId - FirstVirtualWorkerId + 1  is guaranteed to be >= GetMinimumRequiredWorkers.
	 */
	virtual uint32 GetMinimumRequiredWorkers() const PURE_VIRTUAL(UAbstractLBStrategy::GetMinimumRequiredWorkers, return 0;)
	virtual void SetVirtualWorkerIds(const VirtualWorkerId& FirstVirtualWorkerId, const VirtualWorkerId& LastVirtualWorkerId) PURE_VIRTUAL(UAbstractLBStrategy::SetVirtualWorkerIds, return;)

protected:

	VirtualWorkerId LocalVirtualWorkerId;
};
