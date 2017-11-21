#include "ExportSnapshotCommandlet.h"

#include "EntityBuilder.h"
#include "ExportSnapshotCommandlet.h"
#include "SpatialOSCommon.h"
#include "SpatialOSConversionFunctionLibrary.h"
#include "improbable/collections.h"
#include "improbable/standard_library.h"
#include <improbable/spawner/spawner.h>
#include <improbable/player/player.h>
#include <improbable/package_map/package_map.h>
#include <improbable/worker.h>
#include <generated/UnrealNative.h>
#include <array>

using namespace improbable;

const int g_SpawnerEntityId = 1;
const int g_playerEntityId = 2;
const int g_PackageMapEntityId = 3;

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
	snapshotEntities.emplace(std::make_pair(g_PackageMapEntityId, CreatePackageMapEntity()));
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

worker::Entity UExportSnapshotCommandlet::CreatePackageMapEntity() const
{
	const Coordinates initialPosition{ 0, 0, 0 };

	WorkerAttributeSet unrealWorkerAttributeSet{ worker::List<std::string>{"UnrealWorker"} };
	WorkerAttributeSet unrealClientAttributeSet{ worker::List<std::string>{"UnrealClient"} };

	WorkerRequirementSet unrealWorkerWritePermission{ { unrealWorkerAttributeSet } };
	WorkerRequirementSet anyWorkerReadPermission{
		{ unrealClientAttributeSet, unrealWorkerAttributeSet } };

	worker::Map<uint32_t, std::string> ObjectMap;
	ObjectMap.emplace(34539994, "/Game/ThirdPersonCPP/Maps/ThirdPersonExampleMap.ThirdPersonExampleMap:PersistentLevel.Brush_0");
	ObjectMap.emplace(390156, "/Game/ThirdPersonCPP/Maps/ThirdPersonExampleMap.ThirdPersonExampleMap:PersistentLevel.LightmassImportanceVolume_0");
	ObjectMap.emplace(429497295, "/Game/ThirdPersonCPP/Maps/ThirdPersonExampleMap.ThirdPersonExampleMap:PersistentLevel.DefaultPhysicsVolume_0");
	ObjectMap.emplace(986715, "/Game/ThirdPersonCPP/Maps/ThirdPersonExampleMap.ThirdPersonExampleMap:PersistentLevel.PostProcessVolume_0");
	ObjectMap.emplace(2354876, "/Game/ThirdPersonCPP/Maps/ThirdPersonExampleMap.ThirdPersonExampleMap:PersistentLevel.PackageMapperUtil_48");
	ObjectMap.emplace(78604, "/Game/ThirdPersonCPP/Maps/ThirdPersonExampleMap.ThirdPersonExampleMap:PersistentLevel.Bump_StaticMesh");
	ObjectMap.emplace(25616786, "/Game/ThirdPersonCPP/Maps/ThirdPersonExampleMap.ThirdPersonExampleMap:PersistentLevel.Linear_Stair_StaticMesh");
	ObjectMap.emplace(234329, "/Game/ThirdPersonCPP/Maps/ThirdPersonExampleMap.ThirdPersonExampleMap:PersistentLevel.Wall8");
	ObjectMap.emplace(23505582, "/Game/ThirdPersonCPP/Maps/ThirdPersonExampleMap.ThirdPersonExampleMap:PersistentLevel.RightArm_StaticMesh");
	ObjectMap.emplace(93893, "/Game/ThirdPersonCPP/Maps/ThirdPersonExampleMap.ThirdPersonExampleMap:PersistentLevel.Wall9");
	ObjectMap.emplace(90261578, "/Game/ThirdPersonCPP/Maps/ThirdPersonExampleMap.ThirdPersonExampleMap:PersistentLevel.SkyLight_0");
	ObjectMap.emplace(645003, "/Game/ThirdPersonCPP/Maps/ThirdPersonExampleMap.ThirdPersonExampleMap:PersistentLevel.ThirdPersonExampleMap_C_0");
	ObjectMap.emplace(230948463, "/Game/ThirdPersonCPP/Maps/ThirdPersonExampleMap.ThirdPersonExampleMap:PersistentLevel.LightSource_0");
	ObjectMap.emplace(5237, "/Game/ThirdPersonCPP/Maps/ThirdPersonExampleMap.ThirdPersonExampleMap:PersistentLevel.AbstractNavData-Default");
	ObjectMap.emplace(599990, "/Game/ThirdPersonCPP/Maps/ThirdPersonExampleMap.ThirdPersonExampleMap:PersistentLevel.SkySphereBlueprint");


	auto snapshotEntity =
		improbable::unreal::FEntityBuilder::Begin()
		.AddPositionComponent(Position::Data{ initialPosition }, unrealWorkerWritePermission)
		.AddMetadataComponent(Metadata::Data("PackageMap"))
		.SetPersistence(true)
		.SetReadAcl(anyWorkerReadPermission)
		.AddComponent<package_map::PackageMap>(package_map::PackageMap::Data{ObjectMap}, unrealWorkerWritePermission)
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
