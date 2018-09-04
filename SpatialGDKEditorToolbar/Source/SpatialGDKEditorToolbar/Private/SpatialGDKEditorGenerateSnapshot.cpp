// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "SpatialGDKEditorGenerateSnapshot.h"

#include "SpatialGDKEditorToolbarSettings.h"
#include "SpatialConstants.h"
#include "Utils/SchemaUtils.h"
#include "Schema/StandardLibrary.h"
#include "Schema/UnrealMetadata.h"

#include "Runtime/Core/Public/HAL/PlatformFilemanager.h"
#include "UObjectIterator.h"

#include <improbable/c_worker.h>
#include <improbable/c_schema.h>

DEFINE_LOG_CATEGORY(LogSpatialGDKSnapshot);

const WorkerAttributeSet UnrealWorkerAttributeSet{ TArray<FString>{TEXT("UnrealWorker")} };
const WorkerAttributeSet UnrealClientAttributeSet{ TArray<FString>{TEXT("UnrealClient")} };

const WorkerRequirementSet UnrealWorkerPermission{ {UnrealWorkerAttributeSet} };
const WorkerRequirementSet UnrealClientPermission{ {UnrealClientAttributeSet} };
const WorkerRequirementSet AnyWorkerPermission{ {UnrealClientAttributeSet, UnrealWorkerAttributeSet} };

const Coordinates Origin{ 0, 0, 0 };

bool CreateSpawnerEntity(Worker_SnapshotOutputStream* OutputStream)
{
	Worker_Entity SpawnerEntity;
	SpawnerEntity.entity_id = SpatialConstants::SPAWNER_ENTITY_ID;

	Worker_ComponentData PlayerSpawnerData = {};
	PlayerSpawnerData.component_id = SpatialConstants::PLAYER_SPAWNER_COMPONENT_ID;
	PlayerSpawnerData.schema_type = Schema_CreateComponentData(SpatialConstants::PLAYER_SPAWNER_COMPONENT_ID);

	TArray<Worker_ComponentData> Components;

	WriteAclMap ComponentWriteAcl;
	ComponentWriteAcl.Add(POSITION_COMPONENT_ID, UnrealWorkerPermission);
	ComponentWriteAcl.Add(METADATA_COMPONENT_ID, UnrealWorkerPermission);
	ComponentWriteAcl.Add(PERSISTENCE_COMPONENT_ID, UnrealWorkerPermission);
	ComponentWriteAcl.Add(UNREAL_METADATA_COMPONENT_ID, UnrealWorkerPermission);
	ComponentWriteAcl.Add(ENTITY_ACL_COMPONENT_ID, UnrealWorkerPermission);
	ComponentWriteAcl.Add(SpatialConstants::PLAYER_SPAWNER_COMPONENT_ID, UnrealWorkerPermission);

	Components.Add(Position(Origin).CreatePositionData());
	Components.Add(Metadata(TEXT("SpatialSpawner")).CreateMetadataData());
	Components.Add(Persistence().CreatePersistenceData());
	Components.Add(UnrealMetadata().CreateUnrealMetadataData());
	Components.Add(PlayerSpawnerData);
	Components.Add(EntityAcl(AnyWorkerPermission, ComponentWriteAcl).CreateEntityAclData());

	SpawnerEntity.component_count = Components.Num();
	SpawnerEntity.components = Components.GetData();

	return Worker_SnapshotOutputStream_WriteEntity(OutputStream, &SpawnerEntity) != 0;
}

Worker_ComponentData CreateGlobalStateManagerData()
{
	StringToEntityMap SingletonNameToEntityId;
	StringToEntityMap StablyNamedPathToEntityId;

	for (TObjectIterator<UClass> It; It; ++It)
	{
		// Find all singleton classes
		if (!It->HasAnySpatialClassFlags(SPATIALCLASS_Singleton))
		{
			continue;
		}

		// Ensure we don't process skeleton or reinitialized classes
		if (It->GetName().StartsWith(TEXT("SKEL_"), ESearchCase::CaseSensitive) || It->GetName().StartsWith(TEXT("REINST_"), ESearchCase::CaseSensitive))
		{
			continue;
		}

		// Id is initially 0 to indicate that this Singleton entity has not been created yet.
		// When the worker authoritative over the GSM sees 0, it knows it is safe to create it.
		SingletonNameToEntityId.Add(*It->GetPathName(), 0);
	}

	Worker_ComponentData Data;
	Data.component_id = SpatialConstants::GLOBAL_STATE_MANAGER_COMPONENT_ID;
	Data.schema_type = Schema_CreateComponentData(SpatialConstants::GLOBAL_STATE_MANAGER_COMPONENT_ID);
	Schema_Object* ComponentObject = Schema_GetComponentDataFields(Data.schema_type);

	Schema_AddStringToEntityMap(ComponentObject, 1, SingletonNameToEntityId);
	Schema_AddStringToEntityMap(ComponentObject, 2, StablyNamedPathToEntityId);

	return Data;
}

bool CreateGlobalStateManager(Worker_SnapshotOutputStream* OutputStream)
{
	Worker_Entity GSM;
	GSM.entity_id = SpatialConstants::GLOBAL_STATE_MANAGER;

	TArray<Worker_ComponentData> Components;

	WriteAclMap ComponentWriteAcl;
	ComponentWriteAcl.Add(POSITION_COMPONENT_ID, UnrealWorkerPermission);
	ComponentWriteAcl.Add(METADATA_COMPONENT_ID, UnrealWorkerPermission);
	ComponentWriteAcl.Add(PERSISTENCE_COMPONENT_ID, UnrealWorkerPermission);
	ComponentWriteAcl.Add(UNREAL_METADATA_COMPONENT_ID, UnrealWorkerPermission);
	ComponentWriteAcl.Add(ENTITY_ACL_COMPONENT_ID, UnrealWorkerPermission);
	ComponentWriteAcl.Add(SpatialConstants::GLOBAL_STATE_MANAGER_COMPONENT_ID, UnrealWorkerPermission);

	Components.Add(Position(Origin).CreatePositionData());
	Components.Add(Metadata(TEXT("GlobalStateManager")).CreateMetadataData());
	Components.Add(Persistence().CreatePersistenceData());
	Components.Add(UnrealMetadata().CreateUnrealMetadataData());
	Components.Add(CreateGlobalStateManagerData());
	Components.Add(EntityAcl(UnrealWorkerPermission, ComponentWriteAcl).CreateEntityAclData());

	GSM.component_count = Components.Num();
	GSM.components = Components.GetData();

	return Worker_SnapshotOutputStream_WriteEntity(OutputStream, &GSM) != 0;
}

bool CreatePlaceholders(Worker_SnapshotOutputStream* OutputStream)
{
	// Set up grid of "placeholder" entities to allow workers to be authoritative over _something_.
	int PlaceholderCount = SpatialConstants::PLACEHOLDER_ENTITY_ID_LAST - SpatialConstants::PLACEHOLDER_ENTITY_ID_FIRST + 1;
	int PlaceholderCountAxis = (int)sqrt(PlaceholderCount);
	checkf(PlaceholderCountAxis * PlaceholderCountAxis == PlaceholderCount, TEXT("The number of placeholders must be a square number."));
	checkf(PlaceholderCountAxis % 2 == 0, TEXT("The number of placeholders on each axis must be even."));
	const float CHUNK_SIZE = 5.0f; // in SpatialOS coordinates.
	int PlaceholderEntityIdCounter = SpatialConstants::PLACEHOLDER_ENTITY_ID_FIRST;
	for (int x = -PlaceholderCountAxis / 2; x < PlaceholderCountAxis / 2; x++)
	{
		for (int y = -PlaceholderCountAxis / 2; y < PlaceholderCountAxis / 2; y++)
		{
			const Coordinates PlaceholderPosition{ x * CHUNK_SIZE + CHUNK_SIZE * 0.5f, 0, y * CHUNK_SIZE + CHUNK_SIZE * 0.5f };

			Worker_Entity Placeholder;
			Placeholder.entity_id = PlaceholderEntityIdCounter;

			TArray<Worker_ComponentData> Components;

			WriteAclMap ComponentWriteAcl;
			ComponentWriteAcl.Add(POSITION_COMPONENT_ID, UnrealWorkerPermission);
			ComponentWriteAcl.Add(METADATA_COMPONENT_ID, UnrealWorkerPermission);
			ComponentWriteAcl.Add(PERSISTENCE_COMPONENT_ID, UnrealWorkerPermission);
			ComponentWriteAcl.Add(UNREAL_METADATA_COMPONENT_ID, UnrealWorkerPermission);
			ComponentWriteAcl.Add(ENTITY_ACL_COMPONENT_ID, UnrealWorkerPermission);

			Components.Add(Position(PlaceholderPosition).CreatePositionData());
			Components.Add(Metadata(TEXT("Placeholder")).CreateMetadataData());
			Components.Add(Persistence().CreatePersistenceData());
			Components.Add(UnrealMetadata().CreateUnrealMetadataData());
			Components.Add(EntityAcl(UnrealWorkerPermission, ComponentWriteAcl).CreateEntityAclData());

			Placeholder.component_count = Components.Num();
			Placeholder.components = Components.GetData();

			if (Worker_SnapshotOutputStream_WriteEntity(OutputStream, &Placeholder) == 0)
			{
				return false;
			}

			PlaceholderEntityIdCounter++;
		}
	}
	// Sanity check.
	check(PlaceholderEntityIdCounter == SpatialConstants::PLACEHOLDER_ENTITY_ID_LAST + 1);

	return true;
}

bool ValidateAndCreateSnapshotGenerationPath(FString& SavePath)
{
	FString DirectoryPath = FPaths::GetPath(SavePath);
	if (!FPaths::CollapseRelativeDirectories(DirectoryPath))
	{
		UE_LOG(LogSpatialGDKSnapshot, Error, TEXT("Invalid path: %s - snapshot not generated"), *DirectoryPath);
		return false;
	}

	if (!FPaths::DirectoryExists(DirectoryPath))
	{
		UE_LOG(LogSpatialGDKSnapshot, Display, TEXT("Snapshot directory does not exist - creating directory: %s"), *DirectoryPath);
		if (!FPlatformFileManager::Get().GetPlatformFile().CreateDirectoryTree(*DirectoryPath))
		{
			UE_LOG(LogSpatialGDKSnapshot, Error, TEXT("Unable to create directory: %s - snapshot not generated"), *DirectoryPath);
			return false;
		}
	}
	return true;
}

bool SpatialGDKGenerateSnapshot(UWorld* World)
{
	const USpatialGDKEditorToolbarSettings* Settings = GetDefault<USpatialGDKEditorToolbarSettings>();
	FString SavePath = FPaths::Combine(Settings->GetSpatialOSSnapshotPath(), Settings->GetSpatialOSSnapshotFile());
	if (!ValidateAndCreateSnapshotGenerationPath(SavePath))
	{
		return false;
	}

	UE_LOG(LogSpatialGDKSnapshot, Display, TEXT("Saving snapshot to: %s"), *SavePath);

	Worker_ComponentVtable DefaultVtable{};
	Worker_SnapshotParameters Parameters{};
	Parameters.default_component_vtable = &DefaultVtable;
	Worker_SnapshotOutputStream* OutputStream = Worker_SnapshotOutputStream_Create(TCHAR_TO_UTF8(*SavePath), &Parameters);

	if (!CreateSpawnerEntity(OutputStream))
	{
		UE_LOG(LogSpatialGDKSnapshot, Error, TEXT("Error generating snapshot: %s"), UTF8_TO_TCHAR(Worker_SnapshotOutputStream_GetError(OutputStream)));
		return false;
	}

	if (!CreateGlobalStateManager(OutputStream))
	{
		UE_LOG(LogSpatialGDKSnapshot, Error, TEXT("Error generating snapshot: %s"), UTF8_TO_TCHAR(Worker_SnapshotOutputStream_GetError(OutputStream)));
		return false;
	}

	if (!CreatePlaceholders(OutputStream))
	{
		UE_LOG(LogSpatialGDKSnapshot, Error, TEXT("Error generating snapshot: %s"), UTF8_TO_TCHAR(Worker_SnapshotOutputStream_GetError(OutputStream)));
		return false;
	}

	Worker_SnapshotOutputStream_Destroy(OutputStream);

	return true;
}
