#include "ExportSnapshotCommandlet.h"

#include "EntityBuilder.h"
#include "ExportSnapshotCommandlet.h"
#include "SpatialOSCommon.h"
#include "SpatialOSConversionFunctionLibrary.h"
#include "improbable/collections.h"
#include "improbable/standard_library.h"
#include <improbable/spawner/spawner.h>
#include <improbable/player/player.h>
#include <improbable/worker.h>
#include <generated/UnrealNative.h>
#include <array>

using namespace improbable;

const int g_SpawnerEntityId = 1;
const int g_playerEntityId = 2;

UExportSnapshotCommandlet::UExportSnapshotCommandlet()
{
}

UExportSnapshotCommandlet::~UExportSnapshotCommandlet()
{
}

int32 UExportSnapshotCommandlet::Main(const FString& Params)
{
	FString combinedPath =
		FPaths::Combine(*FPaths::GetPath(FPaths::GetProjectFilePath()), TEXT("../../../snapshots"));
	UE_LOG(LogTemp, Display, TEXT("Combined path %s"), *combinedPath);
	if (FPaths::CollapseRelativeDirectories(combinedPath))
	{
		GenerateSnapshot(combinedPath);
	}
	else
	{
		UE_LOG(LogTemp, Display, TEXT("Path was invalid - snapshot not generated"));
	}

	return 0;
}

void UExportSnapshotCommandlet::GenerateSnapshot(const FString& savePath) const
{
	const FString fullPath = FPaths::Combine(*savePath, TEXT("default.snapshot"));

	std::unordered_map<worker::EntityId, worker::Entity> snapshotEntities;
	//snapshotEntities.emplace(std::make_pair(g_SpawnerEntityId, CreateSpawnerEntity()));
	snapshotEntities.emplace(std::make_pair(g_playerEntityId, CreatePlayerEntity()));
	worker::Option<std::string> Result =
		worker::SaveSnapshot(improbable::unreal::Components{}, TCHAR_TO_UTF8(*fullPath), snapshotEntities);
	if (!Result.empty())
	{
		std::string ErrorString = Result.value_or("");
		UE_LOG(LogTemp, Display, TEXT("Error: %s"), UTF8_TO_TCHAR(ErrorString.c_str()));
	}
	else
	{
		UE_LOG(LogTemp, Display, TEXT("Snapshot exported to the path %s"), *fullPath);
	}
}

worker::Entity UExportSnapshotCommandlet::CreateSpawnerEntity() const
{
	const Coordinates initialPosition{ 0, 0, 0 };

	WorkerAttributeSet unrealWorkerAttributeSet{ worker::List<std::string>{"UnrealWorker"} };
	WorkerAttributeSet unrealClientAttributeSet{ worker::List<std::string>{"UnrealClient"} };

	WorkerRequirementSet unrealWorkerWritePermission{ { unrealWorkerAttributeSet } };
	WorkerRequirementSet anyWorkerReadPermission{
		{ unrealClientAttributeSet, unrealWorkerAttributeSet } };

	auto snapshotEntity =
		improbable::unreal::FEntityBuilder::Begin()
		.AddPositionComponent(Position::Data{ initialPosition }, unrealWorkerWritePermission)
		.AddMetadataComponent(Metadata::Data("Spawner"))
		.SetPersistence(true)
		.SetReadAcl(anyWorkerReadPermission)
		.AddComponent<spawner::Spawner>(spawner::Spawner::Data{}, unrealWorkerWritePermission)
		.Build();

	return snapshotEntity;
}

// This entity is just a placeholder with absolutely no functionality.
worker::Entity UExportSnapshotCommandlet::CreatePlayerEntity() const
{
	FVector UnrealPos = FVector(-1000, -100, 228);
	const Coordinates initialPosition = USpatialOSConversionFunctionLibrary::UnrealCoordinatesToSpatialOsCoordinatesCast(UnrealPos);

	WorkerAttributeSet unrealWorkerAttributeSet{ worker::List<std::string>{"UnrealWorker"} };
	WorkerAttributeSet unrealClientAttributeSet{ worker::List<std::string>{"UnrealClient"} };

	WorkerRequirementSet unrealWorkerWritePermission{ { unrealWorkerAttributeSet } };
	WorkerRequirementSet unrealClientWritePermission{ { unrealClientAttributeSet } };
	WorkerRequirementSet anyWorkerReadPermission{
		{ unrealClientAttributeSet, unrealWorkerAttributeSet } };

	auto snapshotEntity = improbable::unreal::FEntityBuilder::Begin()
		.AddPositionComponent(Position::Data{initialPosition}, unrealWorkerWritePermission)
		.AddMetadataComponent(Metadata::Data{"NUFCharacter_BP"})
		.SetPersistence(true)
		.SetReadAcl(anyWorkerReadPermission)
		.AddComponent<player::PlayerControlClient>(player::PlayerControlClient::Data{}, unrealClientWritePermission)
		.AddComponent<player::PlayerControlServer>(player::PlayerControlServer::Data{}, unrealWorkerWritePermission)
		.AddComponent<improbable::unreal::UnrealACharacterReplicatedData>(improbable::unreal::UnrealACharacterReplicatedData::Data{}, unrealWorkerWritePermission)
		.AddComponent<improbable::unreal::UnrealACharacterCompleteData>(improbable::unreal::UnrealACharacterCompleteData::Data{}, unrealWorkerWritePermission)
		.Build();

	return snapshotEntity;
}
