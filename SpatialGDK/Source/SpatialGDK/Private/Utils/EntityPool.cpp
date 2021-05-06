// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "Utils/EntityPool.h"

#include "Interop/SpatialReceiver.h"
#include "Interop/SpatialSender.h"
#include "SpatialGDKSettings.h"

DEFINE_LOG_CATEGORY(LogSpatialEntityPool);

using namespace SpatialGDK;

void UEntityPool::Init(USpatialNetDriver* InNetDriver)
{
	NetDriver = InNetDriver;

	ReserveEntityIDs(GetDefault<USpatialGDKSettings>()->EntityPoolInitialReservationCount);
}

void UEntityPool::ReserveEntityIDs(uint32 EntitiesToReserve)
{
	UE_LOG(LogSpatialEntityPool, Verbose, TEXT("Sending bulk entity ID Reservation Request for %d IDs"), EntitiesToReserve);

	checkf(!bIsAwaitingResponse, TEXT("Trying to reserve Entity IDs while another reserve request is in flight"));

	// TODO: UNR-4979 Allow full range of uint32 when SQD-1150 is fixed
	const uint32 TempMaxEntitiesToReserve = static_cast<uint32>(MAX_int32);
	if (EntitiesToReserve > TempMaxEntitiesToReserve)
	{
		UE_LOG(LogSpatialEntityPool, Log, TEXT("Clamping requested 'EntitiesToReserve' to MAX_int32 (from %u to %d)"), EntitiesToReserve,
			   MAX_int32);
		EntitiesToReserve = TempMaxEntitiesToReserve;
	}

	// Set up reserve IDs delegate
	ReserveEntityIDsDelegate CacheEntityIDsDelegate;
	CacheEntityIDsDelegate.BindLambda([EntitiesToReserve, this](const Worker_ReserveEntityIdsResponseOp& Op) {
		bIsAwaitingResponse = false;
		if (Op.status_code != WORKER_STATUS_CODE_SUCCESS)
		{
			// UNR-630 - Temporary hack to avoid failure to reserve entities due to timeout on large maps
			if (Op.status_code == WORKER_STATUS_CODE_TIMEOUT)
			{
				UE_LOG(LogSpatialEntityPool, Warning, TEXT("Failed to reserve entity IDs Reason: %s. Retrying..."),
					   UTF8_TO_TCHAR(Op.message));
				ReserveEntityIDs(EntitiesToReserve);
			}
			else
			{
				UE_LOG(LogSpatialEntityPool, Error, TEXT("Failed to reserve entity IDs Reason: %s."), UTF8_TO_TCHAR(Op.message));
			}

			return;
		}

		// Ensure we received the same number of reserved IDs as we requested
		check(EntitiesToReserve == Op.number_of_entity_ids);

		EntityRange NewEntityRange = {};
		NewEntityRange.CurrentEntityId = Op.first_entity_id;
		NewEntityRange.LastEntityId = Op.first_entity_id + (Op.number_of_entity_ids - 1);

		UE_LOG(LogSpatialEntityPool, Verbose, TEXT("Reserved %d entities, caching in pool, Entity IDs: (%d, %d)"), Op.number_of_entity_ids,
			   Op.first_entity_id, NewEntityRange.LastEntityId);

		ReservedEntityIDRanges.Add(NewEntityRange);

		if (!bIsReady)
		{
			bIsReady = true;
			EntityPoolReadyDelegate.Broadcast();
		}
	});

	// Reserve the Entity IDs
	const Worker_RequestId ReserveRequestID = NetDriver->Connection->SendReserveEntityIdsRequest(EntitiesToReserve, RETRY_UNTIL_COMPLETE);
	bIsAwaitingResponse = true;
	// Add the spawn delegate
	ReserveEntityIdsHandler.AddRequest(ReserveRequestID, CacheEntityIDsDelegate);
}

void UEntityPool::Advance()
{
	ReserveEntityIdsHandler.ProcessOps(NetDriver->Connection->GetCoordinator().GetViewDelta().GetWorkerMessages());
}

Worker_EntityId UEntityPool::GetNextEntityId()
{
	if (ReservedEntityIDRanges.Num() == 0)
	{
		// TODO: Improve error message
		UE_LOG(LogSpatialEntityPool, Warning,
			   TEXT("Tried to pop an entity ID from the pool when there were no entity IDs. Try altering your Entity Pool configuration"));
		return SpatialConstants::INVALID_ENTITY_ID;
	}

	EntityRange& CurrentEntityRange = ReservedEntityIDRanges[0];
	const Worker_EntityId NextId = CurrentEntityRange.CurrentEntityId++;

	uint32_t TotalRemainingEntityIds = 0;
	for (EntityRange Range : ReservedEntityIDRanges)
	{
		TotalRemainingEntityIds += Range.LastEntityId - Range.CurrentEntityId + 1;
	}

	UE_LOG(LogSpatialEntityPool, Verbose, TEXT("Popped ID, %i IDs remaining"), TotalRemainingEntityIds);

	if (TotalRemainingEntityIds < GetDefault<USpatialGDKSettings>()->EntityPoolRefreshThreshold && !bIsAwaitingResponse)
	{
		UE_LOG(LogSpatialEntityPool, Verbose, TEXT("Pool under threshold, reserving more entity IDs"));
		ReserveEntityIDs(GetDefault<USpatialGDKSettings>()->EntityPoolRefreshCount);
	}

	if (CurrentEntityRange.CurrentEntityId > CurrentEntityRange.LastEntityId)
	{
		ReservedEntityIDRanges.RemoveAt(0);
	}

	return NextId;
}

FEntityPoolReadyEvent& UEntityPool::GetEntityPoolReadyDelegate()
{
	return EntityPoolReadyDelegate;
}
