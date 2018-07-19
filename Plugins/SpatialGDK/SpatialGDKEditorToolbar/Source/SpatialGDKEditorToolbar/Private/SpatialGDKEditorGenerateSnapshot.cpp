// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "SpatialGDKEditorGenerateSnapshot.h"
#include "EntityBuilder.h"
#include "SpatialConstants.h"
#include "SpatialOSCommon.h"
#include "Runtime/Core/Public/HAL/PlatformFilemanager.h"

#include <improbable/standard_library.h>
#include <improbable/unreal/gdk/level_data.h>
#include <improbable/unreal/gdk/spawner.h>
#include <improbable/worker.h>

DEFINE_LOG_CATEGORY(LogSpatialGDKSnapshot);

using namespace improbable;

const WorkerAttributeSet UnrealWorkerAttributeSet{worker::List<std::string>{"UnrealWorker"}};
const WorkerAttributeSet UnrealClientAttributeSet{worker::List<std::string>{"UnrealClient"}};

const WorkerRequirementSet UnrealWorkerWritePermission{{UnrealWorkerAttributeSet}};
const WorkerRequirementSet UnrealClientWritePermission{{UnrealClientAttributeSet}};
const WorkerRequirementSet AnyWorkerReadPermission{{UnrealClientAttributeSet, UnrealWorkerAttributeSet}};

namespace
{
worker::Entity CreateSpawnerEntity()
{
	const Coordinates InitialPosition{0, 0, 0};
	improbable::WorkerAttributeSet WorkerAttribute{{worker::List<std::string>{"UnrealWorker"}}};
	improbable::WorkerRequirementSet WorkersOnly{{WorkerAttribute}};
	improbable::unreal::UnrealMetadata::Data UnrealMetadata;

	return improbable::unreal::FEntityBuilder::Begin()
		.AddPositionComponent(Position::Data{InitialPosition}, UnrealWorkerWritePermission)
		.AddMetadataComponent(Metadata::Data("SpatialSpawner"))
		.SetPersistence(true)
		.SetReadAcl(AnyWorkerReadPermission)
		.AddComponent<unreal::PlayerSpawner>(unreal::PlayerSpawner::Data{}, UnrealWorkerWritePermission)
		.AddComponent<improbable::unreal::UnrealMetadata>(UnrealMetadata, WorkersOnly)
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
} // ::

bool SpatialGDKGenerateSnapshot(FString SavePath, UWorld* World)
{
	UE_LOG(LogSpatialGDKSnapshot, Display, TEXT("Save path %s"), *SavePath);

	if (!FPaths::CollapseRelativeDirectories(SavePath))
	{
		UE_LOG(LogSpatialGDKSnapshot, Display, TEXT("Path was invalid - snapshot not generated"));
		return false;
	}

	if (!FPaths::DirectoryExists(SavePath))
	{
		UE_LOG(LogSpatialGDKSnapshot, Display, TEXT("Path non existent - creating path for snapshot"));
		if (!FPlatformFileManager::Get().GetPlatformFile().CreateDirectoryTree(*SavePath))
		{
			UE_LOG(LogSpatialGDKSnapshot, Display, TEXT("Unable to create path - exiting snapshot generation"));
			return false;
		}
	}

	const FString FullPath = FPaths::Combine(*SavePath, TEXT("default.snapshot"));
	worker::SnapshotOutputStream OutputStream = worker::SnapshotOutputStream(improbable::unreal::Components{}, TCHAR_TO_UTF8(*FullPath));

	// Create spawner entity.
	worker::Option<std::string> Result = OutputStream.WriteEntity(SpatialConstants::SPAWNER_ENTITY_ID, CreateSpawnerEntity());

	if (!Result.empty())
	{
		UE_LOG(LogSpatialGDKSnapshot, Display, TEXT("Error generating snapshot: %s"), UTF8_TO_TCHAR(Result.value_or("").c_str()));
		return false;
	}

	// Create level entities.
	for (const auto& EntityPair : CreateLevelEntities(World))
	{
		Result = OutputStream.WriteEntity(EntityPair.first, EntityPair.second);
		if (!Result.empty())
		{
			UE_LOG(LogSpatialGDKSnapshot, Display, TEXT("Error generating snapshot: %s"), UTF8_TO_TCHAR(Result.value_or("").c_str()));
			return false;
		}
	}

	UE_LOG(LogSpatialGDKSnapshot, Display, TEXT("Snapshot exported to the path %s"), *FullPath);
	return true;
}
