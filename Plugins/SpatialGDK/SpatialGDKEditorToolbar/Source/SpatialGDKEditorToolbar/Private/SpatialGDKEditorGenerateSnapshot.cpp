// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "SpatialGDKEditorGenerateSnapshot.h"
#include "EntityBuilder.h"
#include "SpatialConstants.h"
#include "SpatialGDKCommon.h"

#include "EngineUtils.h"
#include "GameFramework/Actor.h"

#include <improbable/standard_library.h>
#include <improbable/unreal/level_data.h>
#include <improbable/unreal/spawner.h>
#include <improbable/worker.h>

DEFINE_LOG_CATEGORY(LogSpatialGDKSnapshot);

using namespace improbable;

const WorkerAttributeSet UnrealWorkerAttributeSet{
	worker::List<std::string>{"UnrealWorker"}};
const WorkerAttributeSet UnrealClientAttributeSet{
	worker::List<std::string>{"UnrealClient"}};

const WorkerRequirementSet UnrealWorkerWritePermission{
	{UnrealWorkerAttributeSet}};
const WorkerRequirementSet UnrealClientWritePermission{
	{UnrealClientAttributeSet}};
const WorkerRequirementSet AnyWorkerReadPermission{
	{UnrealClientAttributeSet, UnrealWorkerAttributeSet}};

namespace
{
worker::Entity CreateSpawnerEntity()
{
	const Coordinates InitialPosition{0, 0, 0};

	return improbable::unreal::FEntityBuilder::Begin()
		.AddPositionComponent(Position::Data{InitialPosition},
			UnrealWorkerWritePermission)
		.AddMetadataComponent(Metadata::Data("Spawner"))
		.SetPersistence(true)
		.SetReadAcl(AnyWorkerReadPermission)
		.AddComponent<unreal::PlayerSpawner>(unreal::PlayerSpawner::Data{},
			UnrealWorkerWritePermission)
		.Build();
}

worker::Map<worker::EntityId, worker::Entity>
CreateLevelEntities(UWorld* World)
{
	worker::Map<worker::EntityId, worker::Entity> LevelEntities;

	// Build list of static objects in the world.
	worker::Map<uint32_t, std::string> StaticActorMap;
	worker::Map<uint32_t, std::string> PersistentDynamicActorMap;
	uint32_t StaticObjectId = 2;
	for (TActorIterator<AActor> Itr(World); Itr; ++Itr)
	{
		AActor* Actor = *Itr;
		if (!Actor->IsSupportedForNetworking() || Actor->IsEditorOnly() ||
			Actor->GetIsReplicated())
		{
			continue;
		}

		FString PathName = Actor->GetPathName(World);
		StaticActorMap.emplace(StaticObjectId,
			std::string(TCHAR_TO_UTF8(*PathName)));
		worker::EntityId EntityId = 0;
		UE_LOG(LogSpatialGDKSnapshot, Log, TEXT("Found static object in persistent level, adding to level data "
												"entity. Path: %s, "
												"Object ref: (entity ID: %lld, offset: %u)."),
			*PathName,
			EntityId,
			StaticObjectId);
		StaticObjectId++;
	}

	// Set up level data entity.
	const Coordinates InitialPosition{0, 0, 0};
	LevelEntities.emplace(
		SpatialConstants::LEVEL_DATA_ENTITY_ID,
		improbable::unreal::FEntityBuilder::Begin()
			.AddPositionComponent(Position::Data{InitialPosition},
				UnrealWorkerWritePermission)
			.AddMetadataComponent(Metadata::Data("LevelData"))
			.SetPersistence(true)
			.SetReadAcl(AnyWorkerReadPermission)
			.AddComponent<unreal::UnrealLevel>(
				unreal::UnrealLevel::Data{StaticActorMap},
				UnrealWorkerWritePermission)
			.Build());

	// Set up grid of "placeholder" entities to allow workers to be authoritative
	// over _something_.
	int PlaceholderCount = SpatialConstants::PLACEHOLDER_ENTITY_ID_LAST -
		SpatialConstants::PLACEHOLDER_ENTITY_ID_FIRST + 1;
	int PlaceholderCountAxis = sqrt(PlaceholderCount);
	checkf(PlaceholderCountAxis * PlaceholderCountAxis == PlaceholderCount,
		TEXT("The number of placeholders must be a square number."));
	checkf(PlaceholderCountAxis % 2 == 0,
		TEXT("The number of placeholders on each axis must be even."));
	const float CHUNK_SIZE = 5.0f;  // in SpatialOS coordinates.
	int PlaceholderEntityIdCounter =
		SpatialConstants::PLACEHOLDER_ENTITY_ID_FIRST;
	for (int x = -PlaceholderCountAxis / 2; x < PlaceholderCountAxis / 2; x++)
	{
		for (int y = -PlaceholderCountAxis / 2; y < PlaceholderCountAxis / 2; y++)
		{
			const Coordinates PlaceholderPosition{x * CHUNK_SIZE + CHUNK_SIZE * 0.5f,
				0,
				y * CHUNK_SIZE + CHUNK_SIZE * 0.5f};
			LevelEntities.emplace(
				PlaceholderEntityIdCounter,
				improbable::unreal::FEntityBuilder::Begin()
					.AddPositionComponent(Position::Data{PlaceholderPosition},
						UnrealWorkerWritePermission)
					.AddMetadataComponent(Metadata::Data("Placeholder"))
					.SetPersistence(true)
					.SetReadAcl(AnyWorkerReadPermission)
					.AddComponent<unreal::UnrealLevelPlaceholder>(
						unreal::UnrealLevelPlaceholder::Data{},
						UnrealWorkerWritePermission)
					.Build());
			PlaceholderEntityIdCounter++;
		}
	}
	// Sanity check.
	check(PlaceholderEntityIdCounter ==
		SpatialConstants::PLACEHOLDER_ENTITY_ID_LAST + 1);
	return LevelEntities;
}
}  // ::

void SpatialGDKGenerateSnapshot(const FString& SavePath, UWorld* World)
{
	const FString FullPath = FPaths::Combine(*SavePath, TEXT("default.snapshot"));

	worker::SnapshotOutputStream OutputStream{improbable::unreal::Components{},
		TCHAR_TO_UTF8(*FullPath)};

	// Create spawner.
	OutputStream.WriteEntity(SpatialConstants::SPAWNER_ENTITY_ID,
		CreateSpawnerEntity());

	// Create level entities.
	worker::Option<std::string> Result;
	for (auto EntityPair : CreateLevelEntities(World))
	{
		Result = OutputStream.WriteEntity(EntityPair.first, EntityPair.second);

		if (!Result.empty())
		{
			std::string ErrorString = Result.value_or("");
			UE_LOG(LogSpatialGDKSnapshot, Error, TEXT("Error generating snapshot: %s"), UTF8_TO_TCHAR(ErrorString.c_str()));
			return;
		}
	}

	UE_LOG(LogSpatialGDKSnapshot, Display, TEXT("Snapshot exported to the path %s"), *FullPath);
}
