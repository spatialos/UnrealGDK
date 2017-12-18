#include "ExportSnapshotCommandlet.h"

#include "EntityBuilder.h"
#include "ExportSnapshotCommandlet.h"
#include "SpatialConstants.h"
#include "SpatialOSCommon.h"
#include "SpatialOSConversionFunctionLibrary.h"
#include "improbable/collections.h"
#include "improbable/standard_library.h"
#include <improbable/spawner/spawner.h>
#include <improbable/player/player.h>
#include <improbable/package_map/package_map.h>
#include <improbable/worker.h>
#include <generated/UnrealCharacter.h>
#include <array>

using namespace improbable;


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
	snapshotEntities.emplace(std::make_pair(SpatialConstants::SPAWNER_ENTITY_ID, CreateSpawnerEntity()));
	snapshotEntities.emplace(std::make_pair(SpatialConstants::PACKAGE_MAP_ENTITY_ID, CreatePackageMapEntity()));
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
	WorkerRequirementSet unrealClientWritePermission{ { unrealClientAttributeSet } };
	WorkerRequirementSet anyWorkerReadPermission{
		{ unrealClientAttributeSet, unrealWorkerAttributeSet } };

	auto snapshotEntity =
		improbable::unreal::FEntityBuilder::Begin()
		.AddPositionComponent(Position::Data{ initialPosition }, unrealWorkerWritePermission)
		.AddMetadataComponent(Metadata::Data("Spawner"))
		.SetPersistence(true)
		.SetReadAcl(anyWorkerReadPermission)
		.AddComponent<spawner::SpawnerClient>(spawner::SpawnerClient::Data{}, unrealClientWritePermission)
		.AddComponent<spawner::SpawnerServer>(spawner::SpawnerServer::Data{false}, unrealWorkerWritePermission)
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
	ObjectMap.emplace(2578920789, "/Game/ThirdPersonCPP/Maps/ThirdPersonExampleMap.ThirdPersonExampleMap:PersistentLevel.Brush_0");
	ObjectMap.emplace(2392680313, "/Game/ThirdPersonCPP/Maps/ThirdPersonExampleMap.ThirdPersonExampleMap:PersistentLevel.LightmassImportanceVolume_0");
	ObjectMap.emplace(2692449513, "/Game/ThirdPersonCPP/Maps/ThirdPersonExampleMap.ThirdPersonExampleMap:PersistentLevel.DefaultPhysicsVolume_0");
	ObjectMap.emplace(4084350780, "/Game/ThirdPersonCPP/Maps/ThirdPersonExampleMap.ThirdPersonExampleMap:PersistentLevel.PostProcessVolume_0");
	ObjectMap.emplace(2266511128, "/Game/ThirdPersonCPP/Maps/ThirdPersonExampleMap.ThirdPersonExampleMap:PersistentLevel.1M_Cube_Chamfer_2");
	ObjectMap.emplace(3176504740, "/Game/ThirdPersonCPP/Maps/ThirdPersonExampleMap.ThirdPersonExampleMap:PersistentLevel.Bump_StaticMesh");
	ObjectMap.emplace(3778779084, "/Game/ThirdPersonCPP/Maps/ThirdPersonExampleMap.ThirdPersonExampleMap:PersistentLevel.Floor");
	ObjectMap.emplace(2755552271, "/Game/ThirdPersonCPP/Maps/ThirdPersonExampleMap.ThirdPersonExampleMap:PersistentLevel.LeftArm_StaticMesh");
	ObjectMap.emplace(3397426473, "/Game/ThirdPersonCPP/Maps/ThirdPersonExampleMap.ThirdPersonExampleMap:PersistentLevel.Linear_Stair_StaticMesh");
	ObjectMap.emplace(3250948416, "/Game/ThirdPersonCPP/Maps/ThirdPersonExampleMap.ThirdPersonExampleMap:PersistentLevel.Ramp_StaticMesh");
	ObjectMap.emplace(4044673420, "/Game/ThirdPersonCPP/Maps/ThirdPersonExampleMap.ThirdPersonExampleMap:PersistentLevel.RightArm_StaticMesh");
	ObjectMap.emplace(3741580260, "/Game/ThirdPersonCPP/Maps/ThirdPersonExampleMap.ThirdPersonExampleMap:PersistentLevel.Wall6");
	ObjectMap.emplace(3174833489, "/Game/ThirdPersonCPP/Maps/ThirdPersonExampleMap.ThirdPersonExampleMap:PersistentLevel.Wall7_3");
	ObjectMap.emplace(3741575910, "/Game/ThirdPersonCPP/Maps/ThirdPersonExampleMap.ThirdPersonExampleMap:PersistentLevel.Wall8");
	ObjectMap.emplace(3741576345, "/Game/ThirdPersonCPP/Maps/ThirdPersonExampleMap.ThirdPersonExampleMap:PersistentLevel.Wall9");
	ObjectMap.emplace(2407730340, "/Game/ThirdPersonCPP/Maps/ThirdPersonExampleMap.ThirdPersonExampleMap:PersistentLevel.DocumentationActor1");
	ObjectMap.emplace(3157154445, "/Game/ThirdPersonCPP/Maps/ThirdPersonExampleMap.ThirdPersonExampleMap:PersistentLevel.AtmosphericFog_1");
	ObjectMap.emplace(2630414082, "/Game/ThirdPersonCPP/Maps/ThirdPersonExampleMap.ThirdPersonExampleMap:PersistentLevel.SkyLight_0");
	ObjectMap.emplace(3648063513, "/Game/ThirdPersonCPP/Maps/ThirdPersonExampleMap.ThirdPersonExampleMap:PersistentLevel.WorldInfo_0");
	ObjectMap.emplace(2903090501, "/Game/ThirdPersonCPP/Maps/ThirdPersonExampleMap.ThirdPersonExampleMap:PersistentLevel.ThirdPersonExampleMap_C_2");
	ObjectMap.emplace(4208844388, "/Game/ThirdPersonCPP/Maps/ThirdPersonExampleMap.ThirdPersonExampleMap:PersistentLevel.LightSource_0");
	ObjectMap.emplace(3818225277, "/Game/ThirdPersonCPP/Maps/ThirdPersonExampleMap.ThirdPersonExampleMap:PersistentLevel.AbstractNavData-Default");
	ObjectMap.emplace(3116900111, "/Game/ThirdPersonCPP/Maps/ThirdPersonExampleMap.ThirdPersonExampleMap:PersistentLevel.NetworkPlayerStart");
	ObjectMap.emplace(2618737432, "/Game/ThirdPersonCPP/Maps/ThirdPersonExampleMap.ThirdPersonExampleMap:PersistentLevel.SphereReflectionCapture");
	ObjectMap.emplace(2534501830, "/Game/ThirdPersonCPP/Maps/ThirdPersonExampleMap.ThirdPersonExampleMap:PersistentLevel.TextRenderActor_1");
	ObjectMap.emplace(2408470588, "/Game/ThirdPersonCPP/Maps/ThirdPersonExampleMap.ThirdPersonExampleMap:PersistentLevel.PackageMapperUtil_48");
	ObjectMap.emplace(4041226225, "/Game/ThirdPersonCPP/Maps/ThirdPersonExampleMap.ThirdPersonExampleMap:PersistentLevel.SkySphereBlueprint");


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
		.AddComponent<improbable::unreal::UnrealCharacterReplicatedData>(improbable::unreal::UnrealCharacterReplicatedData::Data{}, unrealWorkerWritePermission)
		.AddComponent<improbable::unreal::UnrealCharacterCompleteData>(improbable::unreal::UnrealCharacterCompleteData::Data{}, unrealWorkerWritePermission)
		.Build();

	return snapshotEntity;
}
