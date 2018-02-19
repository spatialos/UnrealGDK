// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "NUFEditorGenerateSnapshot.h"
#include "EntityBuilder.h"
#include "NUF/SpatialConstants.h"
#include "SpatialOSCommon.h"

#include "GameFramework/Actor.h"
#include "EngineUtils.h"

#include <improbable/worker.h>
#include <improbable/standard_library.h>
#include <improbable/unreal/spawner.h>
#include <improbable/unreal/level_data.h>

#include "NUF/Generated/SpatialTypeBinding_Character.h"

using namespace improbable;

const WorkerAttributeSet UnrealWorkerAttributeSet{worker::List<std::string>{"UnrealWorker"}};
const WorkerAttributeSet UnrealClientAttributeSet{worker::List<std::string>{"UnrealClient"}};

const WorkerRequirementSet UnrealWorkerWritePermission{{UnrealWorkerAttributeSet}};
const WorkerRequirementSet UnrealClientWritePermission{{UnrealClientAttributeSet}};
const WorkerRequirementSet AnyWorkerReadPermission{{UnrealClientAttributeSet, UnrealWorkerAttributeSet}};

namespace {
worker::Entity CreateSpawnerEntity()
{
	const Coordinates InitialPosition{0, 0, 0};

	return improbable::unreal::FEntityBuilder::Begin()
		.AddPositionComponent(Position::Data{InitialPosition}, UnrealWorkerWritePermission)
		.AddMetadataComponent(Metadata::Data("Spawner"))
		.SetPersistence(true)
		.SetReadAcl(AnyWorkerReadPermission)
		.AddComponent<unreal::PlayerSpawner>(unreal::PlayerSpawner::Data{}, UnrealWorkerWritePermission)
		.Build();
}

worker::Map<worker::EntityId, worker::Entity> CreateLevelEntities(UWorld* World)
{
	worker::Map<worker::EntityId, worker::Entity> LevelEntities;

	// Build list of static objects in the world.
	worker::Map<uint32_t, std::string> StaticActorMap;
	worker::Map<uint32_t, std::string> PersistentDynamicActorMap;
	uint32_t StaticObjectId = 2;
	for (TActorIterator<AActor> Itr(World); Itr; ++Itr)
	{
		AActor* Actor = *Itr;
		if (!Actor->IsSupportedForNetworking() || Actor->IsEditorOnly() || Actor->GetIsReplicated())
		{
			continue;
		}

		FString PathName = Actor->GetPathName(World);
		StaticActorMap.emplace(StaticObjectId, std::string(TCHAR_TO_UTF8(*PathName)));
		worker::EntityId EntityId = 0;
		UE_LOG(LogTemp, Log, TEXT("Static object in world: %s (entity ID: %llu, offset: %u)."), *PathName, EntityId, StaticObjectId);
		StaticObjectId++;
	}

	// Set up level data entity.
	const Coordinates InitialPosition{0, 0, 0};
	LevelEntities.emplace(SpatialConstants::LEVEL_DATA_ENTITY_ID, improbable::unreal::FEntityBuilder::Begin()
		.AddPositionComponent(Position::Data{InitialPosition}, UnrealWorkerWritePermission)
		.AddMetadataComponent(Metadata::Data("LevelData"))
		.SetPersistence(true)
		.SetReadAcl(AnyWorkerReadPermission)
		.AddComponent<unreal::UnrealLevel>(unreal::UnrealLevel::Data{StaticActorMap}, UnrealWorkerWritePermission)
		.Build());
	return LevelEntities;
}
} // ::

void NUFGenerateSnapshot(const FString& SavePath, UWorld* World)
{
	const FString FullPath = FPaths::Combine(*SavePath, TEXT("default.snapshot"));

	std::unordered_map<worker::EntityId, worker::Entity> SnapshotEntities;
	
	// Create spawner.
	SnapshotEntities.emplace(SpatialConstants::SPAWNER_ENTITY_ID, CreateSpawnerEntity());

	// Create level entities.
	for (auto EntityPair : CreateLevelEntities(World))
	{
		SnapshotEntities.emplace(std::move(EntityPair));
	}

	// Save snapshot.
	worker::Option<std::string> Result = worker::SaveSnapshot(improbable::unreal::Components{}, TCHAR_TO_UTF8(*FullPath), SnapshotEntities);
	if (!Result.empty())
	{
		std::string ErrorString = Result.value_or("");
		UE_LOG(LogTemp, Display, TEXT("Error generating snapshot: %s"), UTF8_TO_TCHAR(ErrorString.c_str()));
	}
	else
	{
		UE_LOG(LogTemp, Display, TEXT("Snapshot exported to the path %s"), *FullPath);
	}
}
