// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "Interop/SnapshotManager.h"

#include "EngineClasses/SpatialNetDriver.h"
#include "Interop/Connection/SpatialWorkerConnection.h"
#include "Interop/GlobalStateManager.h"
#include "Interop/SpatialReceiver.h"
#include "SpatialConstants.h"
#include "Utils/SchemaUtils.h"

DEFINE_LOG_CATEGORY(LogSnapshotManager);

using namespace SpatialGDK;

void USnapshotManager::Init(USpatialNetDriver* InNetDriver)
{
	NetDriver = InNetDriver;
	Receiver = InNetDriver->Receiver;
	GlobalStateManager = InNetDriver->GlobalStateManager;
}

// WorldWipe will send out an expensive entity query for every entity in the deployment.
// It does this by having an entity query for all entities that are not the GSM (workaround for not having the ability to make a query for all entities).
// Once it has the response to this query, it will send deletion requests for all found entities and then one for the GSM itself.
// Should only be triggered by the worker which is authoritative over the GSM.
void USnapshotManager::WorldWipe(const USpatialNetDriver::PostWorldWipeDelegate& PostWorldWipeDelegate)
{
	UE_LOG(LogSnapshotManager, Log, TEXT("World wipe for deployment has been triggered. All entities will be deleted!"));

	Worker_Constraint GSMConstraint;
	GSMConstraint.constraint_type = WORKER_CONSTRAINT_TYPE_ENTITY_ID;
	GSMConstraint.entity_id_constraint.entity_id = GlobalStateManager->GlobalStateManagerEntityId;

	Worker_NotConstraint NotGSMConstraint;
	NotGSMConstraint.constraint = &GSMConstraint;

	Worker_Constraint WorldConstraint;
	WorldConstraint.constraint_type = WORKER_CONSTRAINT_TYPE_NOT;
	WorldConstraint.not_constraint = NotGSMConstraint;

	Worker_EntityQuery WorldQuery{};
	WorldQuery.constraint = WorldConstraint;
	WorldQuery.result_type = WORKER_RESULT_TYPE_SNAPSHOT;

	Worker_RequestId RequestID;
	RequestID = NetDriver->Connection->SendEntityQueryRequest(&WorldQuery);

	EntityQueryDelegate WorldQueryDelegate;
	WorldQueryDelegate.BindLambda([this, PostWorldWipeDelegate](Worker_EntityQueryResponseOp& Op)
	{
		if (Op.status_code != WORKER_STATUS_CODE_SUCCESS)
		{
			UE_LOG(LogSnapshotManager, Error, TEXT("SnapshotManager WorldWipe - World entity query failed: %s"), UTF8_TO_TCHAR(Op.message));
		}
		else if (Op.result_count == 0)
		{
			UE_LOG(LogSnapshotManager, Error, TEXT("SnapshotManager WorldWipe - No entities found in world entity query"));
		}
		else
		{
			// Send deletion requests for all entities found in the world entity query.
			DeleteEntities(Op);

			// Also make sure that we kill the GSM.
			NetDriver->Connection->SendDeleteEntityRequest(GlobalStateManager->GlobalStateManagerEntityId);

			// The world is now ready to finish ServerTravel which means loading in a new map.
			PostWorldWipeDelegate.ExecuteIfBound();
		}
	});

	Receiver->AddEntityQueryDelegate(RequestID, WorldQueryDelegate);
}

void USnapshotManager::DeleteEntities(const Worker_EntityQueryResponseOp& Op)
{
	UE_LOG(LogSnapshotManager, Log, TEXT("Deleting %u entities."), Op.result_count);

	for (uint32_t i = 0; i < Op.result_count; i++)
	{
		UE_LOG(LogSnapshotManager, Verbose, TEXT("Sending delete request for: %i"), Op.results[i].entity_id);
		NetDriver->Connection->SendDeleteEntityRequest(Op.results[i].entity_id);
	}
}

// GetSnapshotPath will take a snapshot (with or without the .snapshot extension) name and convert it to a relative path in the Game/Content folder.
FString GetSnapshotPath(const FString& SnapshotName)
{
	FString SnapshotsDirectory = FPaths::ProjectContentDir() + TEXT("Spatial/Snapshots/");

	if (!SnapshotName.EndsWith(TEXT(".snapshot")))
	{
		return SnapshotsDirectory + SnapshotName + TEXT(".snapshot");
	}

	return SnapshotsDirectory + SnapshotName;
}

// LoadSnapshot will take a snapshot name which should be on disk and attempt to read and spawn all of the entities in that snapshot.
// This should only be called from the worker which has authority over the GSM.
void USnapshotManager::LoadSnapshot(const FString& SnapshotName)
{
	FString SnapshotPath = GetSnapshotPath(SnapshotName);

	UE_LOG(LogSnapshotManager, Log, TEXT("Loading snapshot: '%s'"), *SnapshotPath);

	Worker_ComponentVtable DefaultVtable{};
	Worker_SnapshotParameters Parameters{};
	Parameters.default_component_vtable = &DefaultVtable;

	Worker_SnapshotInputStream* Snapshot = Worker_SnapshotInputStream_Create(TCHAR_TO_UTF8(*SnapshotPath), &Parameters);

	FString Error = Worker_SnapshotInputStream_GetError(Snapshot);
	if (!Error.IsEmpty())
	{
		UE_LOG(LogSnapshotManager, Error, TEXT("Error when attempting to read snapshot '%s': %s"), *SnapshotPath, *Error);
		Worker_SnapshotInputStream_Destroy(Snapshot);
		return;
	}

	TArray<TArray<Worker_ComponentData>> EntitiesToSpawn;

	// Get all of the entities from the snapshot.
	while (Worker_SnapshotInputStream_HasNext(Snapshot) > 0)
	{
		Error = Worker_SnapshotInputStream_GetError(Snapshot);
		if (!Error.IsEmpty())
		{
			UE_LOG(LogSnapshotManager, Error, TEXT("Error when reading snapshot. Aborting load snapshot: %s"), *Error);
			Worker_SnapshotInputStream_Destroy(Snapshot);
			return;
		}

		const Worker_Entity* EntityToSpawn = Worker_SnapshotInputStream_ReadEntity(Snapshot);

		Error = Worker_SnapshotInputStream_GetError(Snapshot);
		if (Error.IsEmpty())
		{
			TArray<Worker_ComponentData> EntityComponents;
			for (uint32_t i = 0; i < EntityToSpawn->component_count; ++i)
			{
				// Entity component data must be deep copied so that it can be used for CreateEntityRequest.
				Schema_ComponentData* CopySchemaData = DeepCopyComponentData(EntityToSpawn->components[i].schema_type);
				Worker_ComponentData EntityComponentData{};
				EntityComponentData.component_id = Schema_GetComponentDataComponentId(CopySchemaData);
				EntityComponentData.schema_type = CopySchemaData;
				EntityComponents.Add(EntityComponentData);
			}

			EntitiesToSpawn.Add(EntityComponents);
		}
		else
		{
			UE_LOG(LogSnapshotManager, Error, TEXT("Error when reading snapshot. Aborting load snapshot: %s"), *Error);
			Worker_SnapshotInputStream_Destroy(Snapshot);
			return;
		}
	}

	Worker_SnapshotInputStream_Destroy(Snapshot);

	// Set up reserve IDs delegate
	ReserveEntityIDsDelegate SpawnEntitiesDelegate;
	SpawnEntitiesDelegate.BindLambda([EntitiesToSpawn, this](Worker_ReserveEntityIdsResponseOp& Op)
	{
		UE_LOG(LogSnapshotManager, Log, TEXT("Creating entities in snapshot, number of entities to spawn: %i"), Op.number_of_entity_ids);

		// Ensure we have the same number of reserved IDs as we have entities to spawn
		check(EntitiesToSpawn.Num() == Op.number_of_entity_ids);

		for (uint32_t i = 0; i < Op.number_of_entity_ids; i++)
		{
			// Get an entity to spawn and a reserved EntityID
			TArray<Worker_ComponentData> EntityToSpawn = EntitiesToSpawn[i];
			Worker_EntityId ReservedEntityID = Op.first_entity_id + i;

			// Check if this is the GSM
			for (auto& ComponentData : EntityToSpawn)
			{
				if (ComponentData.component_id == SpatialConstants::SINGLETON_MANAGER_COMPONENT_ID)
				{
					// Save the new GSM Entity ID.
					GlobalStateManager->GlobalStateManagerEntityId = ReservedEntityID;
				}
			}

			UE_LOG(LogSnapshotManager, Log, TEXT("Sending entity create request for: %i"), ReservedEntityID);
			NetDriver->Connection->SendCreateEntityRequest(MoveTemp(EntityToSpawn), &ReservedEntityID);
		}

		GlobalStateManager->SetAcceptingPlayers(true);
	});

	// Reserve the Entity IDs
	Worker_RequestId ReserveRequestID = NetDriver->Connection->SendReserveEntityIdsRequest(EntitiesToSpawn.Num());

	// TODO: UNR-654
	// References to entities that are stored within the snapshot need remapping once we know the new entity IDs.

	// Add the spawn delegate
	Receiver->AddReserveEntityIdsDelegate(ReserveRequestID, SpawnEntitiesDelegate);
}
