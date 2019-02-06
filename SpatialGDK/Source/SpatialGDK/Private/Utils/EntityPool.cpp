#include "Utils/EntityPool.h"

#include "Interop/SpatialReceiver.h"

DEFINE_LOG_CATEGORY(LogEntityPool);

using namespace improbable;

void UEntityPool::Init(USpatialNetDriver* InNetDriver)
{
	NetDriver = InNetDriver;
	Receiver = InNetDriver->Receiver;
	ReserveEntityIDs(1000);
}

void UEntityPool::ReserveEntityIDs(int32 EntitiesToSpawn)
{
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
			ReservedIDs.Add(ReservedEntityID);
		}
	});

	// Reserve the Entity IDs
	Worker_RequestId ReserveRequestID = NetDriver->Connection->SendReserveEntityIdsRequest(EntitiesToSpawn);

	// Add the spawn delegate
	Receiver->AddReserveEntityIdsDelegate(ReserveRequestID, CacheEntityIDsDelegate);
}

// UEntityPool::Pop()

//
//Worker_EntityId UEntityPool::GetEntityID()
//{
//	
//}
