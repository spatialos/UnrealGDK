#include "Utils/EntityPool.h"

#include "Interop/SpatialReceiver.h"

DEFINE_LOG_CATEGORY(LogEntityPool);

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
	UE_LOG(LogEntityPool, Log, TEXT("Sending bulk entity ID Reservation Request"));

	// Set up reserve IDs delegate
	ReserveEntityIDsDelegate CacheEntityIDsDelegate;
	CacheEntityIDsDelegate.BindLambda([EntitiesToSpawn, this](Worker_ReserveEntityIdsResponseOp& Op)
	{
		UE_LOG(LogEntityPool, Log, TEXT("Reserved %i entities, caching in pool"), Op.number_of_entity_ids);

		// Ensure we have the same number of reserved IDs as we have entities to spawn
		check(EntitiesToSpawn == Op.number_of_entity_ids);

		for (uint32_t i = 0; i < Op.number_of_entity_ids; i++)
		{
			// Get an entity to spawn and a reserved EntityID
			Worker_EntityId ReservedEntityID = Op.first_entity_id + i;
			ReservedIDs.Insert(ReservedEntityID, 0);
		}

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
	Worker_EntityId NextId = ReservedIDs.Pop();
	UE_LOG(LogEntityPool, Log, TEXT("Popped ID, %i IDs remaining"), ReservedIDs.Num());

	if (ReservedIDs.Num() < REFRESH_THRESHOLD && !bIsAwaitingResponse)
	{
		UE_LOG(LogEntityPool, Log, TEXT("Pool under threshold, reserving more entity IDs"));
		ReserveEntityIDs(REFRESH_COUNT);
	}

	return NextId;
}
