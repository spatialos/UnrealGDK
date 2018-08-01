// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "SpatialGDKEditorGenerateSnapshot.h"
#include "EntityBuilder.h"
#include "SpatialConstants.h"
#include "SpatialOSCommon.h"
#include "SpatialGDKEditorUtils.h"
#include "SpatialGDKEditorToolbarSettings.h"
#include "Runtime/Core/Public/HAL/PlatformFilemanager.h"
#include <improbable/standard_library.h>
#include <improbable/unreal/gdk/level_data.h>
#include <improbable/unreal/gdk/spawner.h>
#include <improbable/unreal/gdk/global_state_manager.h>
#include <improbable/worker.h>

DEFINE_LOG_CATEGORY(LogSpatialGDKSnapshot);

using namespace improbable;

namespace
{

using NameToEntityIdMap = worker::Map<std::string, worker::EntityId>;

const WorkerAttributeSet UnrealWorkerAttributeSet{worker::List<std::string>{"UnrealWorker"}};
const WorkerAttributeSet UnrealClientAttributeSet{worker::List<std::string>{"UnrealClient"}};

const WorkerRequirementSet UnrealWorkerWritePermission{{UnrealWorkerAttributeSet}};
const WorkerRequirementSet UnrealClientWritePermission{{UnrealClientAttributeSet}};
const WorkerRequirementSet AnyWorkerReadPermission{{UnrealClientAttributeSet, UnrealWorkerAttributeSet}};

const Coordinates Origin{0, 0, 0};

worker::Entity CreateSpawnerEntity()
{
	improbable::unreal::UnrealMetadata::Data UnrealMetadata;

	return improbable::unreal::FEntityBuilder::Begin()
		.AddPositionComponent(Position::Data{Origin}, UnrealWorkerWritePermission)
		.AddMetadataComponent(Metadata::Data("SpatialSpawner"))
		.SetPersistence(true)
		.SetReadAcl(AnyWorkerReadPermission)
		.AddComponent<unreal::PlayerSpawner>(unreal::PlayerSpawner::Data{}, UnrealWorkerWritePermission)
		.AddComponent<improbable::unreal::UnrealMetadata>(UnrealMetadata, UnrealWorkerWritePermission)
		.Build();
}

worker::Map<worker::EntityId, worker::Entity> CreateLevelEntities(UWorld* World)
{
	worker::Map<worker::EntityId, worker::Entity> LevelEntities;

	// Set up grid of "placeholder" entities to allow workers to be authoritative over _something_.
	int PlaceholderCount = SpatialConstants::PLACEHOLDER_ENTITY_ID_LAST - SpatialConstants::PLACEHOLDER_ENTITY_ID_FIRST + 1;
	int PlaceholderCountAxis = sqrt(PlaceholderCount);
	checkf(PlaceholderCountAxis * PlaceholderCountAxis == PlaceholderCount, TEXT("The number of placeholders must be a square number."));
	checkf(PlaceholderCountAxis % 2 == 0, TEXT("The number of placeholders on each axis must be even."));
	const float CHUNK_SIZE = 5.0f; // in SpatialOS coordinates.
	int PlaceholderEntityIdCounter = SpatialConstants::PLACEHOLDER_ENTITY_ID_FIRST;
	for (int x = -PlaceholderCountAxis / 2; x < PlaceholderCountAxis / 2; x++)
	{
		for (int y = -PlaceholderCountAxis / 2; y < PlaceholderCountAxis / 2; y++)
		{
			const Coordinates PlaceholderPosition{x * CHUNK_SIZE + CHUNK_SIZE * 0.5f, 0, y * CHUNK_SIZE + CHUNK_SIZE * 0.5f};
			LevelEntities.emplace(PlaceholderEntityIdCounter, improbable::unreal::FEntityBuilder::Begin()
				.AddPositionComponent(Position::Data{PlaceholderPosition}, UnrealWorkerWritePermission)
				.AddMetadataComponent(Metadata::Data("Placeholder"))
				.SetPersistence(true)
				.SetReadAcl(AnyWorkerReadPermission)
				.AddComponent<unreal::UnrealLevelPlaceholder>(unreal::UnrealLevelPlaceholder::Data{}, UnrealWorkerWritePermission)
				.Build());
			PlaceholderEntityIdCounter++;
		}
	}
	// Sanity check.
	check(PlaceholderEntityIdCounter == SpatialConstants::PLACEHOLDER_ENTITY_ID_LAST + 1);
	return LevelEntities;
}

bool CreateSingletonToIdMap(NameToEntityIdMap& SingletonNameToEntityId)
{
	const FString FileName = "DefaultEditorSpatialGDK.ini";
	const FString ConfigFilePath = FPaths::SourceConfigDir().Append(FileName);

	// Load the SpatialGDK config file
	const FConfigFile* ConfigFile = LoadConfigFile(ConfigFilePath);
	if (!ConfigFile)
	{
		return false;
	}

	const FString SectionName = "SnapshotGenerator.SingletonActorClasses";
	const FConfigSection* SingletonActorClassesSection = GetConfigSection(ConfigFilePath, SectionName);
	if (SingletonActorClassesSection == nullptr)
	{
		return false;
	}

	TArray<FName> SingletonActorClasses;
	SingletonActorClassesSection->GetKeys(SingletonActorClasses);

	for (FName ClassName : SingletonActorClasses)
	{
		// Id is initially 0 to indicate that this Singleton entity has not been created yet.
		// When the worker authoritative over the GSM sees 0, it knows it is safe to create it.
		SingletonNameToEntityId.emplace(std::string(TCHAR_TO_UTF8(*(ClassName.ToString()))), 0);
	}

	return true;
}

worker::Entity CreateGlobalStateManagerEntity(const NameToEntityIdMap& SingletonNameToEntityId)
{
	return improbable::unreal::FEntityBuilder::Begin()
		.AddPositionComponent(Position::Data{Origin}, UnrealWorkerWritePermission)
		.AddMetadataComponent(Metadata::Data("GlobalStateManager"))
		.SetPersistence(true)
		.SetReadAcl(AnyWorkerReadPermission)
		.AddComponent<unreal::GlobalStateManager>(unreal::GlobalStateManager::Data{SingletonNameToEntityId}, UnrealWorkerWritePermission)
		.AddComponent<improbable::unreal::UnrealMetadata>(improbable::unreal::UnrealMetadata::Data{}, UnrealWorkerWritePermission)
		.Build();
}

} // ::

worker::Option<FString> SetupSnapshotGenerationPath(FString SavePath)
{
	const USpatialGDKEditorToolbarSettings* Settings = GetDefault<USpatialGDKEditorToolbarSettings>();
	FString SnapshotFileName = TEXT("default.snapshot");

	if (Settings != nullptr)
	{
		if (!Settings->SpatialOSSnapshotPath.Path.IsEmpty())
		{
			SavePath = Settings->SpatialOSSnapshotPath.Path;
		}

		if (!Settings->SpatialOSSnapshotFile.IsEmpty())
		{
			SnapshotFileName = Settings->SpatialOSSnapshotFile;
		}
	}

	if (!FPaths::CollapseRelativeDirectories(SavePath))
	{
		UE_LOG(LogSpatialGDKSnapshot, Error, TEXT("Invalid path: %s - snapshot not generated"), *SavePath);
		return {};
	}

	if (!FPaths::DirectoryExists(SavePath))
	{
		UE_LOG(LogSpatialGDKSnapshot, Display, TEXT("Snapshot directory does not exist - creating directory: %s"), *SavePath);
		if (!FPlatformFileManager::Get().GetPlatformFile().CreateDirectoryTree(*SavePath))
		{
			UE_LOG(LogSpatialGDKSnapshot, Error, TEXT("Unable to create directory: %s - snapshot not generated"), *SavePath);
			return {};
		}
	}

	FString FullPath = FPaths::Combine(*SavePath, SnapshotFileName);
	return { FullPath };
}

bool SpatialGDKGenerateSnapshot(FString SavePath, UWorld* World)
{
	FString FullSavePath = SetupSnapshotGenerationPath(SavePath).value_or("");

	if (FullSavePath.IsEmpty())
	{
		return false;
	}

	UE_LOG(LogSpatialGDKSnapshot, Display, TEXT("Saving snapshot to: %s"), *FullSavePath);
	worker::SnapshotOutputStream OutputStream = worker::SnapshotOutputStream(improbable::unreal::Components{}, TCHAR_TO_UTF8(*FullSavePath));

	// Create spawner entity.
	worker::Option<std::string> Result = OutputStream.WriteEntity(SpatialConstants::SPAWNER_ENTITY_ID, CreateSpawnerEntity());

	if (!Result.empty())
	{
		UE_LOG(LogSpatialGDKSnapshot, Error, TEXT("Error generating snapshot: %s"), UTF8_TO_TCHAR(Result.value_or("").c_str()));
		return false;
	}

	// Create Global State Manager
	NameToEntityIdMap SingletonNameToEntityId;
	if(!CreateSingletonToIdMap(SingletonNameToEntityId))
	{
		UE_LOG(LogSpatialGDKSnapshot, Error, TEXT("Error generating snapshot: Couldn't create Singleton Name to EntityId map"));
		return false;
	}

	Result = OutputStream.WriteEntity(SpatialConstants::GLOBAL_STATE_MANAGER, CreateGlobalStateManagerEntity(SingletonNameToEntityId));
	if (!Result.empty())
	{
		UE_LOG(LogSpatialGDKSnapshot, Error, TEXT("Error generating snapshot: %s"), UTF8_TO_TCHAR(Result.value_or("").c_str()));
		return false;
	}

	// Create level entities.
	for (const auto& EntityPair : CreateLevelEntities(World))
	{
		Result = OutputStream.WriteEntity(EntityPair.first, EntityPair.second);
		if (!Result.empty())
		{
			UE_LOG(LogSpatialGDKSnapshot, Error, TEXT("Error generating snapshot: %s"), UTF8_TO_TCHAR(Result.value_or("").c_str()));
			return false;
		}
	}

	UE_LOG(LogSpatialGDKSnapshot, Display, TEXT("Snapshot exported to the path: %s"), *FullSavePath);
	return true;
}
