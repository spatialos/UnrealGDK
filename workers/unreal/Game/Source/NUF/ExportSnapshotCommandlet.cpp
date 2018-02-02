#include "ExportSnapshotCommandlet.h"

#include "EntityBuilder.h"
#include "ExportSnapshotCommandlet.h"
#include "SpatialConstants.h"
#include "SpatialOSCommon.h"
#include "SpatialOSConversionFunctionLibrary.h"
#include "improbable/collections.h"
#include "improbable/standard_library.h"
#include <improbable/spawner/spawner.h>
#include <improbable/package_map/package_map.h>
#include <improbable/worker.h>
#include <unreal/generated/UnrealCharacter.h>
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
		.AddComponent<spawner::PlayerSpawner>(spawner::PlayerSpawner::Data{}, unrealWorkerWritePermission)
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
	ObjectMap.emplace(2578920789, "PersistentLevel.Brush_0");
	ObjectMap.emplace(2392680313, "PersistentLevel.LightmassImportanceVolume_0");
	ObjectMap.emplace(2692449513, "PersistentLevel.DefaultPhysicsVolume_0");
	ObjectMap.emplace(4084350780, "PersistentLevel.PostProcessVolume_0");
	ObjectMap.emplace(2266511128, "PersistentLevel.1M_Cube_Chamfer_2");
	ObjectMap.emplace(3176504740, "PersistentLevel.Bump_StaticMesh");
	ObjectMap.emplace(3778779084, "PersistentLevel.Floor");
	ObjectMap.emplace(2755552271, "PersistentLevel.LeftArm_StaticMesh");
	ObjectMap.emplace(3397426473, "PersistentLevel.Linear_Stair_StaticMesh");
	ObjectMap.emplace(3250948416, "PersistentLevel.Ramp_StaticMesh");
	ObjectMap.emplace(4044673420, "PersistentLevel.RightArm_StaticMesh");
	ObjectMap.emplace(3741580260, "PersistentLevel.Wall6");
	ObjectMap.emplace(3174833489, "PersistentLevel.Wall7_3");
	ObjectMap.emplace(3741575910, "PersistentLevel.Wall8");
	ObjectMap.emplace(3741576345, "PersistentLevel.Wall9");
	ObjectMap.emplace(2407730340, "PersistentLevel.DocumentationActor1");
	ObjectMap.emplace(3157154445, "PersistentLevel.AtmosphericFog_1");
	ObjectMap.emplace(2630414082, "PersistentLevel.SkyLight_0");
	ObjectMap.emplace(3648063513, "PersistentLevel.WorldInfo_0");
	ObjectMap.emplace(2903090501, "PersistentLevel.ThirdPersonExampleMap_C_2");
	ObjectMap.emplace(4208844388, "PersistentLevel.LightSource_0");
	ObjectMap.emplace(3818225277, "PersistentLevel.AbstractNavData-Default");
	ObjectMap.emplace(3116900111, "PersistentLevel.NetworkPlayerStart");
	ObjectMap.emplace(2618737432, "PersistentLevel.SphereReflectionCapture");
	ObjectMap.emplace(2534501830, "PersistentLevel.TextRenderActor_1");
	ObjectMap.emplace(2408470588, "PersistentLevel.PackageMapperUtil_48");
	ObjectMap.emplace(4041226225, "PersistentLevel.SkySphereBlueprint");


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
