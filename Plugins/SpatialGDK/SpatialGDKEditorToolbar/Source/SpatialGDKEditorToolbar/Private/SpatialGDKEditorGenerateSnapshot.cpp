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

using StringToEntityIdMap = TMap<FString, Worker_EntityId>;

const WorkerAttributeSet UnrealWorkerAttributeSet{ TArray<FString>{TEXT("UnrealWorker")} };
const WorkerAttributeSet UnrealClientAttributeSet{ TArray<FString>{TEXT("UnrealClient")} };

const WorkerRequirementSet UnrealWorkerPermission{ {UnrealWorkerAttributeSet} };
const WorkerRequirementSet UnrealClientPermission{ {UnrealClientAttributeSet} };
const WorkerRequirementSet AnyWorkerPermission{ {UnrealClientAttributeSet, UnrealWorkerAttributeSet} };

const Coordinates Origin{ 0, 0, 0 };

void CreateSpawnerEntity(Worker_SnapshotOutputStream* OutputStream)
{
	Worker_Entity SpawnerEntity;
	SpawnerEntity.entity_id = SpatialConstants::SPAWNER_ENTITY_ID;
	SpawnerEntity.component_count = 6;

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
	SpawnerEntity.components = Components.GetData();

	Worker_SnapshotOutputStream_WriteEntity(OutputStream, &SpawnerEntity);
}

Worker_ComponentData CreateGlobalStateManagerData()
{
	StringToEntityMap SingletonNameToEntityId;
	StringToEntityMap StablyNamedPathToEntityId;

	for (TObjectIterator<UClass> It; It; ++It)
	{
		// Find all singleton classes
		if (It->HasAnySpatialClassFlags(SPATIALCLASS_Singleton) == false)
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

void CreateGlobalStateManager(Worker_SnapshotOutputStream* OutputStream)
{
	Worker_Entity GSM;
	GSM.entity_id = SpatialConstants::GLOBAL_STATE_MANAGER;
	GSM.component_count = 6;

	Worker_ComponentData GSMData = {};
	GSMData.component_id = SpatialConstants::GLOBAL_STATE_MANAGER_COMPONENT_ID;
	GSMData.schema_type = Schema_CreateComponentData(SpatialConstants::GLOBAL_STATE_MANAGER_COMPONENT_ID);

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
	GSM.components = Components.GetData();

	Worker_SnapshotOutputStream_WriteEntity(OutputStream, &GSM);
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
	Worker_SnapshotOutputStream* OutputStream = Worker_SnapshotOutputStream_Create(TCHAR_TO_ANSI(*SavePath), &Parameters);

	CreateSpawnerEntity(OutputStream);
	CreateGlobalStateManager(OutputStream);

	Worker_SnapshotOutputStream_Destroy(OutputStream);
	//const USpatialGDKEditorToolbarSettings* Settings = GetDefault<USpatialGDKEditorToolbarSettings>();
	//FString SavePath = FPaths::Combine(Settings->GetSpatialOSSnapshotPath(), Settings->GetSpatialOSSnapshotFile());
	//if (!ValidateAndCreateSnapshotGenerationPath(SavePath))
	//{
	//	return false;
	//}

	//UE_LOG(LogSpatialGDKSnapshot, Display, TEXT("Saving snapshot to: %s"), *SavePath);
	//worker::SnapshotOutputStream OutputStream = worker::SnapshotOutputStream(improbable::unreal::Components{}, TCHAR_TO_UTF8(*SavePath));

	//// Create spawner entity.
	//worker::Option<std::string> Result = OutputStream.WriteEntity(SpatialConstants::SPAWNER_ENTITY_ID, CreateSpawnerEntity());

	//if (!Result.empty())
	//{
	//	UE_LOG(LogSpatialGDKSnapshot, Error, TEXT("Error generating snapshot: %s"), UTF8_TO_TCHAR(Result.value_or("").c_str()));
	//	return false;
	//}

	//// Create Global State Manager
	//PathNameToEntityIdMap SingletonNameToEntityId;
	//if(!CreateSingletonToIdMap(SingletonNameToEntityId))
	//{
	//	UE_LOG(LogSpatialGDKSnapshot, Error, TEXT("Error generating snapshot: Couldn't create Singleton Name to EntityId map"));
	//	return false;
	//}

	//Result = OutputStream.WriteEntity(SpatialConstants::GLOBAL_STATE_MANAGER, CreateGlobalStateManagerEntity(SingletonNameToEntityId));
	//if (!Result.empty())
	//{
	//	UE_LOG(LogSpatialGDKSnapshot, Error, TEXT("Error generating snapshot: %s"), UTF8_TO_TCHAR(Result.value_or("").c_str()));
	//	return false;
	//}

	//Result = OutputStream.WriteEntity(SpatialConstants::SPECIAL_SPAWNER, CreateSpecialSpawner());

	//if (!Result.empty())
	//{
	//	UE_LOG(LogSpatialGDKSnapshot, Error, TEXT("Error generating snapshot: %s"), UTF8_TO_TCHAR(Result.value_or("").c_str()));
	//	return false;
	//}

	//// Create level entities.
	//for (const auto& EntityPair : CreateLevelEntities(World))
	//{
	//	Result = OutputStream.WriteEntity(EntityPair.first, EntityPair.second);
	//	if (!Result.empty())
	//	{
	//		UE_LOG(LogSpatialGDKSnapshot, Error, TEXT("Error generating snapshot: %s"), UTF8_TO_TCHAR(Result.value_or("").c_str()));
	//		return false;
	//	}
	//}

	//UE_LOG(LogSpatialGDKSnapshot, Display, TEXT("Snapshot exported to the path: %s"), *SavePath);
	return true;
}
