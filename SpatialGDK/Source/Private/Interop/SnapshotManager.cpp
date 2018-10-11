// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "SnapshotManager.h"

#include "EngineClasses/SpatialNetDriver.h"
#include "EngineClasses/SpatialActorChannel.h"
#include "EngineClasses/SpatialNetConnection.h"
#include "EngineClasses/SpatialPackageMapClient.h"
#include "EngineClasses/SpatialGameInstance.h"
#include "Interop/Connection/SpatialWorkerConnection.h"
#include "Interop/GlobalStateManager.h"
#include "Interop/SpatialReceiver.h"
#include "Schema/UnrealMetadata.h"
#include "SpatialConstants.h"
#include "Utils/EntityRegistry.h"
#include "Runtime/Engine/Public/TimerManager.h"

DEFINE_LOG_CATEGORY(LogSnapshotManager);

using namespace improbable;

void USnapshotManager::Init(USpatialNetDriver* InNetDriver, UGlobalStateManager* InGlobalStateManager)
{
	NetDriver = InNetDriver;
	Receiver = InNetDriver->Receiver;
	GlobalStateManager = InGlobalStateManager;
}

void USnapshotManager::WorldWipe(const USpatialNetDriver::ServerTravelDelegate& Delegate)
{
	Worker_EntityIdConstraint GSMConstraintEID;
	GSMConstraintEID.entity_id = GlobalStateManager->GlobalStateManagerEntityId;

	Worker_Constraint GSMConstraint;
	GSMConstraint.constraint_type = WORKER_CONSTRAINT_TYPE_ENTITY_ID;
	GSMConstraint.entity_id_constraint = GSMConstraintEID;

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
	WorldQueryDelegate.BindLambda([this, RequestID, Delegate](Worker_EntityQueryResponseOp& Op)
	{
		if (Op.status_code != WORKER_STATUS_CODE_SUCCESS)
		{
			UE_LOG(LogGlobalStateManager, Error, TEXT("World query failed: %s"), UTF8_TO_TCHAR(Op.message));
		}

		if (Op.result_count == 0)
		{
			UE_LOG(LogGlobalStateManager, Error, TEXT("No entities in world query"));
		}

		if (Op.result_count >= 1)
		{
			UE_LOG(LogGlobalStateManager, Error, TEXT("Found some entities in the world query: %u"), Op.result_count);
			DeleteEntities(Op);
		}

		Delegate.ExecuteIfBound();
	});

	Receiver->AddEntityQueryDelegate(RequestID, WorldQueryDelegate);
}

void USnapshotManager::DeleteEntities(const Worker_EntityQueryResponseOp& Op)
{
	UE_LOG(LogGlobalStateManager, Error, TEXT("Deleting entities!"));

	for (uint32_t i = 0; i < Op.result_count; i++)
	{
		// Lets pray
		UE_LOG(LogGlobalStateManager, Error, TEXT("- Sending delete request for: %i"), Op.results[i].entity_id);
		NetDriver->Connection->SendDeleteEntityRequest(Op.results[i].entity_id);
	}

	// Also kill the GSM
	NetDriver->Connection->SendDeleteEntityRequest(GlobalStateManager->GlobalStateManagerEntityId);
}

void DeepCopy(Schema_Object* source, Schema_Object* target) {
	auto length = Schema_GetWriteBufferLength(source);
	auto buffer = Schema_AllocateBuffer(target, length);
	Schema_WriteToBuffer(source, buffer);
	Schema_Clear(target);
	Schema_MergeFromBuffer(target, buffer, length);
}

Schema_ComponentData* Schema_DeepCopyComponentData(Schema_ComponentData* source) {
	auto* copy = Schema_CreateComponentData(Schema_GetComponentDataComponentId(source));
	DeepCopy(Schema_GetComponentDataFields(source), Schema_GetComponentDataFields(copy));
	return copy;
}

void USnapshotManager::LoadSnapshot(FString SnapshotName)
{
	Worker_ComponentVtable DefaultVtable{};
	Worker_SnapshotParameters Parameters{};
	Parameters.default_component_vtable = &DefaultVtable;

	FString SnapshotsDirectory = FPaths::ProjectContentDir().Append("Spatial/Snapshots/");
	FString SnapshotPath = SnapshotsDirectory + SnapshotName + ".snapshot";;

	Worker_SnapshotInputStream* Snapshot = Worker_SnapshotInputStream_Create(TCHAR_TO_UTF8(*SnapshotPath), &Parameters);

	FString Error = Worker_SnapshotInputStream_GetError(Snapshot);
	if (!Error.IsEmpty())
	{
		UE_LOG(LogGlobalStateManager, Error, TEXT(" Fucking error reading snapshot mate: %s"), *Error);
		return;
	}

	TArray<TArray<Worker_ComponentData>> EntitiesToSpawn;

	// Get all of the entities from the snapshot.
	while (Snapshot && Worker_SnapshotInputStream_HasNext(Snapshot) > 0)
	{
		Error = Worker_SnapshotInputStream_GetError(Snapshot);
		if (!Error.IsEmpty())
		{
			UE_LOG(LogGlobalStateManager, Error, TEXT("- Error when reading the HasNext.........."));
			return;
		}

		const Worker_Entity* EntityToSpawn = Worker_SnapshotInputStream_ReadEntity(Snapshot);

		Error = Worker_SnapshotInputStream_GetError(Snapshot);
		if (Error.IsEmpty())
		{
			TArray<Worker_ComponentData> EntityComponents;
			for (uint32_t i = 0; i < EntityToSpawn->component_count; ++i) {
				// Entity component data must be deep copied so that it can be used for CreateEntityRequest.
				auto* CopySchemaData = Schema_DeepCopyComponentData(EntityToSpawn->components[i].schema_type);
				auto EntityComponentData = Worker_ComponentData{};
				EntityComponentData.component_id = Schema_GetComponentDataComponentId(CopySchemaData);
				EntityComponentData.schema_type = CopySchemaData;
				EntityComponents.Add(EntityComponentData);
			}

			// Josh - New flow. Bulk reserve the EntityIDs and then send.
			EntitiesToSpawn.Add(EntityComponents);
		}
		else
		{
			UE_LOG(LogGlobalStateManager, Error, TEXT("Fucking error reading entity mate: %s"), *Error);
			// Abort
			Worker_SnapshotInputStream_Destroy(Snapshot);
			return;
		}
	}

	// End it
	Worker_SnapshotInputStream_Destroy(Snapshot);

	// TODO: This needs retry logic in case of failures.
	// Set up reserve IDs delegate
	ReserveEntityIDsDelegate SpawnEntitiesDelegate;
	SpawnEntitiesDelegate.BindLambda([EntitiesToSpawn, this](Worker_ReserveEntityIdsResponseOp& Op) { // Need to get the reserved IDs in here.
		UE_LOG(LogGlobalStateManager, Error, TEXT("Creating entities in snapshot, num of entities: %i"), Op.number_of_entity_ids);

		// Ensure we have the same number of reserved IDs as we have entities to spawn
		check(EntitiesToSpawn.Num() == Op.number_of_entity_ids);

		for (uint32_t i = 0; i < Op.number_of_entity_ids; i++)
		{
			// Get an entity to spawn and a reserved EntityID
			auto& EntityToSpawn = EntitiesToSpawn[i];
			Worker_EntityId EntityID = Op.first_entity_id + i;

			// Check if this is the GSM
			for (auto& ComponentData : EntityToSpawn)
			{
				if (ComponentData.component_id == SpatialConstants::GLOBAL_STATE_MANAGER_COMPONENT_ID)
				{
					// Save the ID in this class
					GlobalStateManager->GlobalStateManagerEntityId = EntityID;
					UE_LOG(LogGlobalStateManager, Error, TEXT("GLOBAL STATE MANAGER WILL BE: %i"), EntityID);
				}
			}

			UE_LOG(LogGlobalStateManager, Warning, TEXT("- Sending entity create request for: %i"), EntityID);
			NetDriver->Connection->SendCreateEntityRequest(EntityToSpawn.Num(), EntityToSpawn.GetData(), &EntityID /**Reserved entity ID**/);
		}

		GlobalStateManager->ToggleAcceptingPlayers(true);
	});

	// Reserve the Entity IDs
	Worker_RequestId ReserveRequestID = NetDriver->Connection->SendReserveEntityIdsRequest(EntitiesToSpawn.Num());

	// Add the spawn delegate
	Receiver->AddReserveEntityIdsDelegate(ReserveRequestID, SpawnEntitiesDelegate);
}
