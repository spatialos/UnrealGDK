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

SpatialSnapshotManager::SpatialSnapshotManager()
	: Connection(nullptr)
	, GlobalStateManager(nullptr)
	, Receiver(nullptr)
{}

void SpatialSnapshotManager::Init(USpatialWorkerConnection* InConnection, UGlobalStateManager* InGlobalStateManager, USpatialReceiver* InReceiver)
{
	Connection = InConnection;
	Receiver = InReceiver;
	GlobalStateManager = InGlobalStateManager;
}

// WorldWipe will send out an expensive entity query for every entity in the deployment.
// It does this by sending an entity query for all entities with the Unreal Metadata Component
// Once it has the response to this query, it will send deletion requests for all found entities.
// Should only be triggered by the worker which is authoritative over the GSM.
void SpatialSnapshotManager::WorldWipe(const USpatialNetDriver::PostWorldWipeDelegate& PostWorldWipeDelegate)
{
	UE_LOG(LogSnapshotManager, Log, TEXT("World wipe for deployment has been triggered. All entities with the UnrealMetaData component will be deleted!"));

	Worker_Constraint UnrealMetadataConstraint;
	UnrealMetadataConstraint.constraint_type = WORKER_CONSTRAINT_TYPE_COMPONENT;
	UnrealMetadataConstraint.constraint.component_constraint.component_id = SpatialConstants::UNREAL_METADATA_COMPONENT_ID;

	Worker_EntityQuery WorldQuery{};
	WorldQuery.constraint = UnrealMetadataConstraint;
	WorldQuery.result_type = WORKER_RESULT_TYPE_SNAPSHOT;

	Worker_RequestId RequestID;
	check(Connection.IsValid());
	RequestID = Connection->SendEntityQueryRequest(&WorldQuery);

	EntityQueryDelegate WorldQueryDelegate;
	WorldQueryDelegate.BindLambda([this, PostWorldWipeDelegate](const Worker_EntityQueryResponseOp& Op)
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

			// The world is now ready to finish ServerTravel which means loading in a new map.
			PostWorldWipeDelegate.ExecuteIfBound();
		}
	});

	check(Receiver.IsValid());
	Receiver->AddEntityQueryDelegate(RequestID, WorldQueryDelegate);
}

void SpatialSnapshotManager::DeleteEntities(const Worker_EntityQueryResponseOp& Op)
{
	UE_LOG(LogSnapshotManager, Log, TEXT("Deleting %u entities."), Op.result_count);

	for (uint32_t i = 0; i < Op.result_count; i++)
	{
		UE_LOG(LogSnapshotManager, Verbose, TEXT("Sending delete request for: %i"), Op.results[i].entity_id);
		check(Connection.IsValid());
		Connection->SendDeleteEntityRequest(Op.results[i].entity_id);
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
void SpatialSnapshotManager::LoadSnapshot(const FString& SnapshotName)
{
	FString SnapshotPath = GetSnapshotPath(SnapshotName);

	UE_LOG(LogSnapshotManager, Log, TEXT("Loading snapshot: '%s'"), *SnapshotPath);

	Worker_ComponentVtable DefaultVtable{};
	Worker_SnapshotParameters Parameters{};
	Parameters.default_component_vtable = &DefaultVtable;

	Worker_SnapshotInputStream* Snapshot = Worker_SnapshotInputStream_Create(TCHAR_TO_UTF8(*SnapshotPath), &Parameters);

	FString Error = Worker_SnapshotInputStream_GetState(Snapshot).error_message;
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
		Error = Worker_SnapshotInputStream_GetState(Snapshot).error_message;
		if (!Error.IsEmpty())
		{
			UE_LOG(LogSnapshotManager, Error, TEXT("Error when reading snapshot. Aborting load snapshot: %s"), *Error);
			Worker_SnapshotInputStream_Destroy(Snapshot);
			return;
		}

		const Worker_Entity* EntityToSpawn = Worker_SnapshotInputStream_ReadEntity(Snapshot);

		Error = Worker_SnapshotInputStream_GetState(Snapshot).error_message;
		if (Error.IsEmpty())
		{
			TArray<Worker_ComponentData> EntityComponents;
			for (uint32_t i = 0; i < EntityToSpawn->component_count; ++i)
			{
				// Entity component data must be deep copied so that it can be used for CreateEntityRequest.
				Schema_ComponentData* CopySchemaData = Schema_CopyComponentData(EntityToSpawn->components[i].schema_type);
				Worker_ComponentData EntityComponentData{};
				EntityComponentData.component_id = EntityToSpawn->components[i].component_id;
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
	SpawnEntitiesDelegate.BindLambda([EntitiesToSpawn, this](const Worker_ReserveEntityIdsResponseOp& Op)
	{
		UE_LOG(LogSnapshotManager, Log, TEXT("Creating entities in snapshot, number of entities to spawn: %i"), Op.number_of_entity_ids);

		// Ensure we have the same number of reserved IDs as we have entities to spawn
		check(EntitiesToSpawn.Num() == Op.number_of_entity_ids);

		check(GlobalStateManager.IsValid());

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
			check(Connection.IsValid());
			Connection->SendCreateEntityRequest(MoveTemp(EntityToSpawn), &ReservedEntityID);
		}

		GlobalStateManager->SetAcceptingPlayers(true);
	});

	// Reserve the Entity IDs
	check(Connection.IsValid());
	Worker_RequestId ReserveRequestID = Connection->SendReserveEntityIdsRequest(EntitiesToSpawn.Num());

	// TODO: UNR-654
	// References to entities that are stored within the snapshot need remapping once we know the new entity IDs.

	// Add the spawn delegate
	check(Receiver.IsValid());
	Receiver->AddReserveEntityIdsDelegate(ReserveRequestID, SpawnEntitiesDelegate);
}
