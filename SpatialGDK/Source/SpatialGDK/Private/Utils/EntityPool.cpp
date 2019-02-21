#include "Utils/EntityPool.h"

#include "Interop/SpatialReceiver.h"

DEFINE_LOG_CATEGORY(LogSpatialEntityPool);

using namespace improbable;

const uint32 INITIAL_RESERVATION_COUNT = 1000;
const uint32 REFRESH_THRESHOLD = 100;
const uint32 REFRESH_COUNT = 500;

void UEntityPool::Init(USpatialNetDriver* InNetDriver)
{
	NetDriver = InNetDriver;
	Receiver = InNetDriver->Receiver;

	bIsReady = false;
	bIsAwaitingResponse = false;

	if (NetDriver->IsServer())
	{
		ReserveEntityIDs(INITIAL_RESERVATION_COUNT);
	}
}

void UEntityPool::ReserveEntityIDs(int32 EntitiesToSpawn)
{
	UE_LOG(LogSpatialEntityPool, Log, TEXT("Sending bulk entity ID Reservation Request"));

	// Set up reserve IDs delegate
	ReserveEntityIDsDelegate CacheEntityIDsDelegate;
	CacheEntityIDsDelegate.BindLambda([EntitiesToSpawn, this](Worker_ReserveEntityIdsResponseOp& Op)
	{
		UE_LOG(LogSpatialEntityPool, Log, TEXT("Reserved %i entities, caching in pool"), Op.number_of_entity_ids);

		// Ensure we have the same number of reserved IDs as we have entities to spawn
		check(EntitiesToSpawn == Op.number_of_entity_ids);

		EntityRange NewEntityRange;
		NewEntityRange.CurrentEntityId = Op.first_entity_id;
		NewEntityRange.LastEntityId = Op.first_entity_id + (Op.number_of_entity_ids - 1);

		ReservedIDs.Add(NewEntityRange);

		bIsReady = true;
		bIsAwaitingResponse = false;
	});

	// Reserve the Entity IDs
	Worker_RequestId ReserveRequestID = NetDriver->Connection->SendReserveEntityIdsRequest(EntitiesToSpawn);
	bIsAwaitingResponse = true;

	// Add the spawn delegate
	Receiver->AddReserveEntityIdsDelegate(ReserveRequestID, CacheEntityIDsDelegate);
}

bool UEntityPool::IsReady()
{
	return bIsReady;
}

Worker_EntityId UEntityPool::Pop()
{
	if (ReservedIDs.Num() == 0)
	{
		// TODO: Improve error message
		UE_LOG(LogSpatialEntityPool, Error, TEXT("Tried to pop an entity from the pool when there were no entity IDs. Try altering your Entity Pool configuration"));
		return SpatialConstants::INVALID_ENTITY_ID;
	}


	EntityRange* CurrentEntityRange = &ReservedIDs[0];
	Worker_EntityId NextId = CurrentEntityRange->CurrentEntityId;

	uint32_t TotalRemainingEntityIds = 0;
	for (EntityRange Range : ReservedIDs)
	{
		TotalRemainingEntityIds += Range.LastEntityId - Range.CurrentEntityId;
	}

	CurrentEntityRange->CurrentEntityId++;

	UE_LOG(LogSpatialEntityPool, Log, TEXT("Popped ID, %i IDs remaining"), TotalRemainingEntityIds);

	if (TotalRemainingEntityIds < REFRESH_THRESHOLD && !bIsAwaitingResponse)
	{
		UE_LOG(LogSpatialEntityPool, Log, TEXT("Pool under threshold, reserving more entity IDs"));
		ReserveEntityIDs(REFRESH_COUNT);
	}

	if (CurrentEntityRange->CurrentEntityId > CurrentEntityRange->LastEntityId)
	{
		ReservedIDs.RemoveAt(0);
	}

	return NextId;
}
