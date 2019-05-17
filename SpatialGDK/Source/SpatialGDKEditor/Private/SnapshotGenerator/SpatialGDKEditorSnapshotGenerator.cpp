// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "SpatialGDKEditorSnapshotGenerator.h"

#include "Engine/LevelScriptActor.h"
#include "EngineClasses/SpatialActorChannel.h"
#include "EngineClasses/SpatialNetConnection.h"
#include "EngineClasses/SpatialNetDriver.h"
#include "Interop/SpatialClassInfoManager.h"
#include "Schema/Interest.h"
#include "Schema/SpawnData.h"
#include "Schema/StandardLibrary.h"
#include "Schema/UnrealMetadata.h"
#include "SpatialConstants.h"
#include "SpatialGDKEditorSettings.h"
#include "Utils/ComponentFactory.h"
#include "Utils/RepDataUtils.h"
#include "Utils/RepLayoutUtils.h"
#include "Utils/SchemaUtils.h"
#include "Utils/SnapshotGenerationTemplate.h"

#include "EngineUtils.h"
#include "HAL/PlatformFile.h"
#include "HAL/PlatformFilemanager.h"
#include "UObject/UObjectIterator.h"

#include <WorkerSDK/improbable/c_worker.h>
#include <WorkerSDK/improbable/c_schema.h>

using namespace SpatialGDK;

DEFINE_LOG_CATEGORY(LogSpatialGDKSnapshot);

bool CreateSpawnerEntity(Worker_SnapshotOutputStream* OutputStream)
{
	Worker_Entity SpawnerEntity;
	SpawnerEntity.entity_id = SpatialConstants::INITIAL_SPAWNER_ENTITY_ID;

	Worker_ComponentData PlayerSpawnerData = {};
	PlayerSpawnerData.component_id = SpatialConstants::PLAYER_SPAWNER_COMPONENT_ID;
	PlayerSpawnerData.schema_type = Schema_CreateComponentData(SpatialConstants::PLAYER_SPAWNER_COMPONENT_ID);

	TArray<Worker_ComponentData> Components;

	WriteAclMap ComponentWriteAcl;
	ComponentWriteAcl.Add(SpatialConstants::POSITION_COMPONENT_ID, SpatialConstants::UnrealServerPermission);
	ComponentWriteAcl.Add(SpatialConstants::METADATA_COMPONENT_ID, SpatialConstants::UnrealServerPermission);
	ComponentWriteAcl.Add(SpatialConstants::PERSISTENCE_COMPONENT_ID, SpatialConstants::UnrealServerPermission);
	ComponentWriteAcl.Add(SpatialConstants::ENTITY_ACL_COMPONENT_ID, SpatialConstants::UnrealServerPermission);
	ComponentWriteAcl.Add(SpatialConstants::PLAYER_SPAWNER_COMPONENT_ID, SpatialConstants::UnrealServerPermission);

	Components.Add(Position(Origin).CreatePositionData());
	Components.Add(Metadata(TEXT("SpatialSpawner")).CreateMetadataData());
	Components.Add(Persistence().CreatePersistenceData());
	Components.Add(EntityAcl(SpatialConstants::ClientOrServerPermission, ComponentWriteAcl).CreateEntityAclData());
	Components.Add(PlayerSpawnerData);

	SpawnerEntity.component_count = Components.Num();
	SpawnerEntity.components = Components.GetData();

	return Worker_SnapshotOutputStream_WriteEntity(OutputStream, &SpawnerEntity) != 0;
}

Worker_ComponentData CreateSingletonManagerData()
{
	StringToEntityMap SingletonNameToEntityId;

	Worker_ComponentData Data{};
	Data.component_id = SpatialConstants::SINGLETON_MANAGER_COMPONENT_ID;
	Data.schema_type = Schema_CreateComponentData(SpatialConstants::SINGLETON_MANAGER_COMPONENT_ID);
	Schema_Object* ComponentObject = Schema_GetComponentDataFields(Data.schema_type);

	AddStringToEntityMapToSchema(ComponentObject, 1, SingletonNameToEntityId);

	return Data;
}

Worker_ComponentData CreateDeploymentData()
{
	Worker_ComponentData DeploymentData{};
	DeploymentData.component_id = SpatialConstants::DEPLOYMENT_MAP_COMPONENT_ID;
	DeploymentData.schema_type = Schema_CreateComponentData(SpatialConstants::DEPLOYMENT_MAP_COMPONENT_ID);
	Schema_Object* DeploymentDataObject = Schema_GetComponentDataFields(DeploymentData.schema_type);

	Schema_Object* MapURLObject = Schema_AddObject(DeploymentDataObject, SpatialConstants::DEPLOYMENT_MAP_MAP_URL_ID);
	AddStringToSchema(MapURLObject, 1, TEXT("default")); // TODO: Fill this with the map name of the map the snapshot is being generated for.

	Schema_AddBool(DeploymentDataObject, SpatialConstants::DEPLOYMENT_MAP_ACCEPTING_PLAYERS_ID, false);

	return DeploymentData;
}

Worker_ComponentData CreateGSMShutdownData()
{
	Worker_ComponentData GSMShutdownData;
	GSMShutdownData.component_id = SpatialConstants::GSM_SHUTDOWN_COMPONENT_ID;
	GSMShutdownData.schema_type = Schema_CreateComponentData(SpatialConstants::GSM_SHUTDOWN_COMPONENT_ID);
	return GSMShutdownData;
}

Worker_ComponentData CreateStartupActorManagerData()
{
	Worker_ComponentData StartupActorManagerData{};
	StartupActorManagerData.component_id = SpatialConstants::STARTUP_ACTOR_MANAGER_COMPONENT_ID;
	StartupActorManagerData.schema_type = Schema_CreateComponentData(SpatialConstants::STARTUP_ACTOR_MANAGER_COMPONENT_ID);
	Schema_Object* StartupActorManagerObject = Schema_GetComponentDataFields(StartupActorManagerData.schema_type);

	Schema_AddBool(StartupActorManagerObject, SpatialConstants::STARTUP_ACTOR_MANAGER_CAN_BEGIN_PLAY_ID, false);

	return StartupActorManagerData;
}

bool CreateGlobalStateManager(Worker_SnapshotOutputStream* OutputStream)
{
	Worker_Entity GSM;
	GSM.entity_id = SpatialConstants::INITIAL_GLOBAL_STATE_MANAGER_ENTITY_ID;

	TArray<Worker_ComponentData> Components;

	WriteAclMap ComponentWriteAcl;
	ComponentWriteAcl.Add(SpatialConstants::POSITION_COMPONENT_ID, SpatialConstants::UnrealServerPermission);
	ComponentWriteAcl.Add(SpatialConstants::METADATA_COMPONENT_ID, SpatialConstants::UnrealServerPermission);
	ComponentWriteAcl.Add(SpatialConstants::PERSISTENCE_COMPONENT_ID, SpatialConstants::UnrealServerPermission);
	ComponentWriteAcl.Add(SpatialConstants::ENTITY_ACL_COMPONENT_ID, SpatialConstants::UnrealServerPermission);
	ComponentWriteAcl.Add(SpatialConstants::SINGLETON_MANAGER_COMPONENT_ID, SpatialConstants::UnrealServerPermission);
	ComponentWriteAcl.Add(SpatialConstants::DEPLOYMENT_MAP_COMPONENT_ID, SpatialConstants::UnrealServerPermission);
	ComponentWriteAcl.Add(SpatialConstants::GSM_SHUTDOWN_COMPONENT_ID, SpatialConstants::UnrealServerPermission);
	ComponentWriteAcl.Add(SpatialConstants::STARTUP_ACTOR_MANAGER_COMPONENT_ID, SpatialConstants::UnrealServerPermission);

	Components.Add(Position(Origin).CreatePositionData());
	Components.Add(Metadata(TEXT("GlobalStateManager")).CreateMetadataData());
	Components.Add(Persistence().CreatePersistenceData());
	Components.Add(CreateSingletonManagerData());
	Components.Add(CreateDeploymentData());
	Components.Add(CreateGSMShutdownData());
	Components.Add(CreateStartupActorManagerData());
	Components.Add(EntityAcl(SpatialConstants::UnrealServerPermission, ComponentWriteAcl).CreateEntityAclData());

	GSM.component_count = Components.Num();
	GSM.components = Components.GetData();

	return Worker_SnapshotOutputStream_WriteEntity(OutputStream, &GSM) != 0;
}

bool CreatePlaceholders(Worker_SnapshotOutputStream* OutputStream)
{
	// Set up grid of "placeholder" entities to allow workers to be authoritative over _something_.
	int PlaceholderCount = SpatialConstants::PLACEHOLDER_ENTITY_ID_LAST - SpatialConstants::PLACEHOLDER_ENTITY_ID_FIRST + 1;
	int PlaceholderCountAxis = static_cast<int>(sqrt(PlaceholderCount));
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
			ComponentWriteAcl.Add(SpatialConstants::POSITION_COMPONENT_ID, SpatialConstants::UnrealServerPermission);
			ComponentWriteAcl.Add(SpatialConstants::METADATA_COMPONENT_ID, SpatialConstants::UnrealServerPermission);
			ComponentWriteAcl.Add(SpatialConstants::PERSISTENCE_COMPONENT_ID, SpatialConstants::UnrealServerPermission);
			ComponentWriteAcl.Add(SpatialConstants::ENTITY_ACL_COMPONENT_ID, SpatialConstants::UnrealServerPermission);

			Components.Add(Position(PlaceholderPosition).CreatePositionData());
			Components.Add(Metadata(TEXT("Placeholder")).CreateMetadataData());
			Components.Add(Persistence().CreatePersistenceData());
			Components.Add(EntityAcl(SpatialConstants::UnrealServerPermission, ComponentWriteAcl).CreateEntityAclData());

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

bool RunUserSnapshotGenerationOverrides(Worker_SnapshotOutputStream* OutputStream)
{
	Worker_EntityId NextEntityId = SpatialConstants::PLACEHOLDER_ENTITY_ID_LAST + 1;
	for (TObjectIterator<UClass> SnapshotGenerationClass; SnapshotGenerationClass; ++SnapshotGenerationClass)
	{
		if (SnapshotGenerationClass->IsChildOf(USnapshotGenerationTemplate::StaticClass()) && *SnapshotGenerationClass != USnapshotGenerationTemplate::StaticClass())
		{
			UE_LOG(LogSpatialGDKSnapshot, Log, TEXT("Found user snapshot generation class: %s"), *SnapshotGenerationClass->GetName());
			USnapshotGenerationTemplate *SnapshotGenerationObj = NewObject<USnapshotGenerationTemplate>(GetTransientPackage(), *SnapshotGenerationClass);
			if (!SnapshotGenerationObj->WriteToSnapshotOutput(OutputStream, NextEntityId))
			{
				UE_LOG(LogSpatialGDKSnapshot, Error, TEXT("Failure returned in user snapshot generation override method from class: %s"), *SnapshotGenerationClass->GetName());
				return false;
			}
		}
	}
	return true;
}

bool FillSnapshot(Worker_SnapshotOutputStream* OutputStream, UWorld* World)
{
	if (!CreateSpawnerEntity(OutputStream))
	{
		UE_LOG(LogSpatialGDKSnapshot, Error, TEXT("Error generating Spawner in snapshot: %s"), UTF8_TO_TCHAR(Worker_SnapshotOutputStream_GetError(OutputStream)));
		return false;
	}

	if (!CreateGlobalStateManager(OutputStream))
	{
		UE_LOG(LogSpatialGDKSnapshot, Error, TEXT("Error generating GlobalStateManager in snapshot: %s"), UTF8_TO_TCHAR(Worker_SnapshotOutputStream_GetError(OutputStream)));
		return false;
	}

	const USpatialGDKEditorSettings* SpatialGDKSettings = GetDefault<USpatialGDKEditorSettings>();
	if (SpatialGDKSettings->bGeneratePlaceholderEntitiesInSnapshot)
	{
		if (!CreatePlaceholders(OutputStream))
		{
			UE_LOG(LogSpatialGDKSnapshot, Error, TEXT("Error generating Placeholders in snapshot: %s"), UTF8_TO_TCHAR(Worker_SnapshotOutputStream_GetError(OutputStream)));
			return false;
		}
	}

	if (!RunUserSnapshotGenerationOverrides(OutputStream))
	{
		UE_LOG(LogSpatialGDKSnapshot, Error, TEXT("Error running user defined snapshot generation overrides in snapshot: %s"), UTF8_TO_TCHAR(Worker_SnapshotOutputStream_GetError(OutputStream)));
		return false;
	}

	return true;
}

bool SpatialGDKGenerateSnapshot(UWorld* World, FString SnapshotFilename)
{
	const USpatialGDKEditorSettings* Settings = GetDefault<USpatialGDKEditorSettings>();
	FString SavePath = FPaths::Combine(Settings->GetSpatialOSSnapshotFolderPath(), SnapshotFilename);
	if (!ValidateAndCreateSnapshotGenerationPath(SavePath))
	{
		return false;
	}

	UE_LOG(LogSpatialGDKSnapshot, Display, TEXT("Saving snapshot to: %s"), *SavePath);

	Worker_ComponentVtable DefaultVtable{};
	Worker_SnapshotParameters Parameters{};
	Parameters.default_component_vtable = &DefaultVtable;

	bool bSuccess = true;
	Worker_SnapshotOutputStream* OutputStream = Worker_SnapshotOutputStream_Create(TCHAR_TO_UTF8(*SavePath), &Parameters);
	if (const char* SchemaError = Worker_SnapshotOutputStream_GetError(OutputStream))
	{
		UE_LOG(LogSpatialGDKSnapshot, Error, TEXT("Error creating SnapshotOutputStream: %s"), UTF8_TO_TCHAR(SchemaError));
		bSuccess = false;
	}
	else
	{
		bSuccess = FillSnapshot(OutputStream, World);
	}

	Worker_SnapshotOutputStream_Destroy(OutputStream);

	return bSuccess;
}
