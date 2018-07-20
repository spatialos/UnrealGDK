// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "SpatialGDKEditorGenerateSnapshot.h"
#include "EntityBuilder.h"
#include "SpatialConstants.h"
#include "SpatialOSCommon.h"

#include "CoreMinimal.h"
#include "EngineUtils.h"
#include "GameFramework/Actor.h"
#include "Misc/FileHelper.h"
#include "Paths.h"

#include <improbable/standard_library.h>
#include <improbable/unreal/gdk/level_data.h>
#include <improbable/unreal/gdk/spawner.h>
#include <improbable/unreal/gdk/global_state_manager.h>
#include <improbable/worker.h>

DEFINE_LOG_CATEGORY(LogSpatialGDKSnapshot);

using namespace improbable;

const WorkerAttributeSet UnrealWorkerAttributeSet{worker::List<std::string>{"UnrealWorker"}};
const WorkerAttributeSet UnrealClientAttributeSet{worker::List<std::string>{"UnrealClient"}};

const WorkerRequirementSet UnrealWorkerWritePermission{{UnrealWorkerAttributeSet}};
const WorkerRequirementSet UnrealClientWritePermission{{UnrealClientAttributeSet}};
const WorkerRequirementSet AnyWorkerReadPermission{{UnrealClientAttributeSet, UnrealWorkerAttributeSet}};

const Coordinates Origin{0, 0, 0};

namespace
{
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

worker::Map<std::string, worker::EntityId> CreateSingletonToIdMap()
{
	worker::Map<std::string, worker::EntityId> SingletonToId;

	const FString FileName = "DefaultEditorSpatialGDK.ini";
	const FString ConfigFilePath = FPaths::SourceConfigDir().Append(FileName);

	// Load the SpatialGDK config file
	GConfig->LoadFile(ConfigFilePath);
	FConfigFile* ConfigFile = GConfig->Find(ConfigFilePath, false);
	if (!ConfigFile)
	{
		UE_LOG(LogSpatialGDKSnapshot, Error, TEXT("Could not open .ini file: \"%s\""), *ConfigFilePath);
		return SingletonToId;
	}

	const FString SectionName = "SnapshotGenerator.SingletonActorClasses";
	FConfigSection* SingletonActorClassesSection = ConfigFile->Find(SectionName);
	if (SingletonActorClassesSection == nullptr)
	{
		UE_LOG(LogSpatialGDKSnapshot, Error, TEXT("Could not find section '%s' in '%s'."), *SectionName, *ConfigFilePath);
		return SingletonToId;
	}

	TArray<FName> SingletonActorClasses;
	SingletonActorClassesSection->GetKeys(SingletonActorClasses);

	for (FName ClassName : SingletonActorClasses)
	{
		// Id is initially 0 since no Singleton entities have been created.
		SingletonToId.emplace(std::string(TCHAR_TO_UTF8(*(ClassName.ToString()))), 0);
	}

	return SingletonToId;
}

worker::Entity CreateGlobalStateManagerEntity()
{
	worker::Map<std::string, worker::EntityId> SingletonToId = CreateSingletonToIdMap();

	return improbable::unreal::FEntityBuilder::Begin()
		.AddPositionComponent(Position::Data{Origin}, UnrealWorkerWritePermission)
		.AddMetadataComponent(Metadata::Data("GlobalStateManager"))
		.SetPersistence(true)
		.SetReadAcl(AnyWorkerReadPermission)
		.AddComponent<unreal::GlobalStateManager>(unreal::GlobalStateManager::Data{SingletonToId}, UnrealWorkerWritePermission)
		.AddComponent<improbable::unreal::UnrealMetadata>(improbable::unreal::UnrealMetadata::Data{}, UnrealWorkerWritePermission)
		.Build();
}

} // ::

bool SpatialGDKGenerateSnapshot(FString SavePath, UWorld* World)
{
	UE_LOG(LogSpatialGDKSnapshot, Display, TEXT("Save path %s"), *SavePath);
	if (FPaths::CollapseRelativeDirectories(SavePath))
	{
		std::unordered_map<worker::EntityId, worker::Entity> SnapshotEntities;

		// Create spawner.
		SnapshotEntities.emplace(SpatialConstants::SPAWNER_ENTITY_ID, CreateSpawnerEntity());

		// Create Global State Manager
		SnapshotEntities.emplace(SpatialConstants::GLOBAL_STATE_MANAGER, CreateGlobalStateManagerEntity());

		// Create level entities.
		for (auto EntityPair : CreateLevelEntities(World))
		{
			SnapshotEntities.emplace(std::move(EntityPair));
		}

		const FString FullPath = FPaths::Combine(*SavePath, TEXT("default.snapshot"));

		// Save snapshot.
		worker::Option<std::string> Result = worker::SaveSnapshot(improbable::unreal::Components{}, TCHAR_TO_UTF8(*FullPath), SnapshotEntities);
		if (!Result.empty())
		{
			std::string ErrorString = Result.value_or("");
			UE_LOG(LogSpatialGDKSnapshot, Display, TEXT("Error generating snapshot: %s"), UTF8_TO_TCHAR(ErrorString.c_str()));
			return false;
		}
		else
		{
			UE_LOG(LogSpatialGDKSnapshot, Display, TEXT("Snapshot exported to the path %s"), *FullPath);
			return true;
		}
	}
	else
	{
		UE_LOG(LogSpatialGDKSnapshot, Display, TEXT("Path was invalid - snapshot not generated"));
		return false;
	}
}
