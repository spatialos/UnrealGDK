#include "Utils/EntityPool.h"
#include "TimerManager.h"

#include "Interop/SpatialReceiver.h"

DEFINE_LOG_CATEGORY(LogSpatialEntityPool);

using namespace improbable;

const uint32 INITIAL_RESERVATION_COUNT = 1000;
const uint32 REFRESH_THRESHOLD = 100;
const uint32 REFRESH_COUNT = 500;

void UEntityPool::Init(USpatialNetDriver* InNetDriver, FTimerManager* InTimerManager)
{
	NetDriver = InNetDriver;
	Receiver = InNetDriver->Receiver;
	TimerManager = InTimerManager;

	if (NetDriver->IsServer())
	{
		ReserveEntityIDs(INITIAL_RESERVATION_COUNT);
	}
}

void UEntityPool::ReserveEntityIDs(int32 EntitiesToReserve)
{
	UE_LOG(LogSpatialEntityPool, Log, TEXT("Sending bulk entity ID Reservation Request"));

	checkfSlow(!bIsAwaitingResponse, TEXT("Trying to reserve Entity IDs while another reserve request is in flight"));

	// Set up reserve IDs delegate
	ReserveEntityIDsDelegate CacheEntityIDsDelegate;
	CacheEntityIDsDelegate.BindLambda([EntitiesToReserve, this](Worker_ReserveEntityIdsResponseOp& Op)
	{
		// Ensure we have the same number of reserved IDs as we have entities to spawn
		check(EntitiesToReserve == Op.number_of_entity_ids);

		// Clean up any expired Entity ranges
		ReservedEntityIDRanges = ReservedEntityIDRanges.FilterByPredicate([](const EntityRange& Element)
		{
			return !Element.bExpired;
		});

		EntityRange NewEntityRange = {};
		NewEntityRange.CurrentEntityId = Op.first_entity_id;
		NewEntityRange.LastEntityId = Op.first_entity_id + (Op.number_of_entity_ids - 1);
		NewEntityRange.EntityRangeId = NextEntityRangeId++;

		UE_LOG(LogSpatialEntityPool, Log, TEXT("Reserved %d entities, caching in pool, Entity IDs: (%d, %d) Range ID: %d"), Op.number_of_entity_ids, Op.first_entity_id, NewEntityRange.LastEntityId, NewEntityRange.EntityRangeId);

		ReservedEntityIDRanges.Add(NewEntityRange);

		TWeakObjectPtr<UEntityPool> WeakEntityPoolPtr = this;
		FTimerHandle ExpirationTimer;
		TimerManager->SetTimer(ExpirationTimer, [WeakEntityPoolPtr, ExpiringEntityRangeId = NewEntityRange.EntityRangeId]()
		{
			if (!WeakEntityPoolPtr.IsValid())
			{
				return;
			}

			WeakEntityPoolPtr->OnEntityRangeExpired(ExpiringEntityRangeId);
		}, 180.0f, false); // 3 minute timeout for reserved Entity IDs, timeout for Runtime is 5 minutes

		bIsReady = true;
		bIsAwaitingResponse = false;
	});

	// Reserve the Entity IDs
	Worker_RequestId ReserveRequestID = NetDriver->Connection->SendReserveEntityIdsRequest(EntitiesToReserve);
	bIsAwaitingResponse = true;

	// Add the spawn delegate
	Receiver->AddReserveEntityIdsDelegate(ReserveRequestID, CacheEntityIDsDelegate);
}

bool UEntityPool::IsReady()
{
	return bIsReady;
}

void UEntityPool::OnEntityRangeExpired(uint32 ExpiringEntityRangeId)
{
	UE_LOG(LogSpatialEntityPool, Log, TEXT("Entity range expired! Range ID: %d"), ExpiringEntityRangeId);

	int32 FoundEntityRangeIndex = ReservedEntityIDRanges.IndexOfByPredicate([ExpiringEntityRangeId](const EntityRange& Element)
	{
		return Element.EntityRangeId == ExpiringEntityRangeId;
	});

	if (FoundEntityRangeIndex == INDEX_NONE)
	{
		// This entity range has already been cleaned up as a result of running out of Entity IDs.
		return;
	}

	if (FoundEntityRangeIndex < ReservedEntityIDRanges.Num() - 1)
	{
		// This is not the most recent entity range, just clean up without requesting additional IDs.
		ReservedEntityIDRanges.RemoveAt(FoundEntityRangeIndex);
	}
	else
	{
		// Reserve then cleanup
		if (!bIsAwaitingResponse)
		{
			ReserveEntityIDs(REFRESH_COUNT);
		}
		// Mark this entity range as expired, so it gets cleaned up when we receive a new entity range from Spatial.
		ReservedEntityIDRanges[FoundEntityRangeIndex].bExpired = true;
	}
}

Worker_EntityId UEntityPool::Pop()
{
	if (ReservedEntityIDRanges.Num() == 0)
	{
		// TODO: Improve error message
		UE_LOG(LogSpatialEntityPool, Error, TEXT("Tried to pop an entity from the pool when there were no entity IDs. Try altering your Entity Pool configuration"));
		return SpatialConstants::INVALID_ENTITY_ID;
	}

	EntityRange& CurrentEntityRange = ReservedEntityIDRanges[0];
	Worker_EntityId NextId = CurrentEntityRange.CurrentEntityId++;

	uint32_t TotalRemainingEntityIds = 0;
	for (EntityRange Range : ReservedEntityIDRanges)
	{
		TotalRemainingEntityIds += Range.LastEntityId - Range.CurrentEntityId + 1;
	}

	// TODO: make Verbose after testing
	UE_LOG(LogSpatialEntityPool, Log, TEXT("Popped ID, %i IDs remaining"), TotalRemainingEntityIds);

	if (TotalRemainingEntityIds < REFRESH_THRESHOLD && !bIsAwaitingResponse)
	{
		UE_LOG(LogSpatialEntityPool, Log, TEXT("Pool under threshold, reserving more entity IDs"));
		ReserveEntityIDs(REFRESH_COUNT);
	}

	if (CurrentEntityRange.CurrentEntityId > CurrentEntityRange.LastEntityId)
	{
		ReservedEntityIDRanges.RemoveAt(0);
	}

	return NextId;
}
