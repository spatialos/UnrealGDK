#include "ExportSnapshotCommandlet.h"

#include "EntityBuilder.h"
#include "ExportSnapshotCommandlet.h"
#include "SpatialConstants.h"
#include "SpatialOSCommon.h"
#include "SpatialOSConversionFunctionLibrary.h"

#include <improbable/worker.h>
#include <improbable/standard_library.h>
#include <improbable/spawner/spawner.h>
#include <unreal/level_data.h>
#include <unreal/generated/UnrealCharacter.h>

using namespace improbable;

const WorkerAttributeSet UnrealWorkerAttributeSet{worker::List<std::string>{"UnrealWorker"}};
const WorkerAttributeSet UnrealClientAttributeSet{worker::List<std::string>{"UnrealClient"}};

const WorkerRequirementSet UnrealWorkerWritePermission{{UnrealWorkerAttributeSet}};
const WorkerRequirementSet UnrealClientWritePermission{{UnrealClientAttributeSet}};
const WorkerRequirementSet AnyWorkerReadPermission{{UnrealClientAttributeSet, UnrealWorkerAttributeSet}};

UExportSnapshotCommandlet::UExportSnapshotCommandlet()
{
}

UExportSnapshotCommandlet::~UExportSnapshotCommandlet()
{
}

int32 UExportSnapshotCommandlet::Main(const FString& Params)
{
	FString CombinedPath = FPaths::Combine(*FPaths::GetPath(FPaths::GetProjectFilePath()), TEXT("../../../snapshots"));
	UE_LOG(LogTemp, Display, TEXT("Combined path %s"), *CombinedPath);
	if (FPaths::CollapseRelativeDirectories(CombinedPath))
	{
		GenerateSnapshot(CombinedPath);
	}
	else
	{
		UE_LOG(LogTemp, Display, TEXT("Path was invalid - snapshot not generated"));
	}

	return 0;
}

void UExportSnapshotCommandlet::GenerateSnapshot(const FString& savePath) const
{
	const FString FullPath = FPaths::Combine(*savePath, TEXT("default.snapshot"));

	std::unordered_map<worker::EntityId, worker::Entity> SnapshotEntities;
	SnapshotEntities.emplace(std::make_pair(SpatialConstants::SPAWNER_ENTITY_ID, CreateSpawnerEntity()));
	SnapshotEntities.emplace(std::make_pair(SpatialConstants::LEVEL_DATA_ENTITY_ID, CreateLevelDataEntity()));
	worker::Option<std::string> Result = worker::SaveSnapshot(improbable::unreal::Components{}, TCHAR_TO_UTF8(*FullPath), SnapshotEntities);
	if (!Result.empty())
	{
		std::string ErrorString = Result.value_or("");
		UE_LOG(LogTemp, Display, TEXT("Error: %s"), UTF8_TO_TCHAR(ErrorString.c_str()));
	}
	else
	{
		UE_LOG(LogTemp, Display, TEXT("Snapshot exported to the path %s"), *FullPath);
	}
}

worker::Entity UExportSnapshotCommandlet::CreateSpawnerEntity() const
{
	const Coordinates InitialPosition{0, 0, 0};

	auto snapshotEntity =
		improbable::unreal::FEntityBuilder::Begin()
		.AddPositionComponent(Position::Data{InitialPosition}, UnrealWorkerWritePermission)
		.AddMetadataComponent(Metadata::Data("Spawner"))
		.SetPersistence(true)
		.SetReadAcl(AnyWorkerReadPermission)
		.AddComponent<spawner::PlayerSpawner>(spawner::PlayerSpawner::Data{}, UnrealWorkerWritePermission)
		.Build();

	return snapshotEntity;
}

worker::Entity UExportSnapshotCommandlet::CreateLevelDataEntity() const
{
	const Coordinates InitialPosition{0, 0, 0};

	worker::Map<uint32_t, std::string> StaticActorMap;
	StaticActorMap.emplace(2578920789, "PersistentLevel.Brush_0");
	StaticActorMap.emplace(2392680313, "PersistentLevel.LightmassImportanceVolume_0");
	StaticActorMap.emplace(2692449513, "PersistentLevel.DefaultPhysicsVolume_0");
	StaticActorMap.emplace(4084350780, "PersistentLevel.PostProcessVolume_0");
	StaticActorMap.emplace(2266511128, "PersistentLevel.1M_Cube_Chamfer_2");
	StaticActorMap.emplace(3176504740, "PersistentLevel.Bump_StaticMesh");
	StaticActorMap.emplace(3778779084, "PersistentLevel.Floor");
	StaticActorMap.emplace(2755552271, "PersistentLevel.LeftArm_StaticMesh");
	StaticActorMap.emplace(3397426473, "PersistentLevel.Linear_Stair_StaticMesh");
	StaticActorMap.emplace(3250948416, "PersistentLevel.Ramp_StaticMesh");
	StaticActorMap.emplace(4044673420, "PersistentLevel.RightArm_StaticMesh");
	StaticActorMap.emplace(3741580260, "PersistentLevel.Wall6");
	StaticActorMap.emplace(3174833489, "PersistentLevel.Wall7_3");
	StaticActorMap.emplace(3741575910, "PersistentLevel.Wall8");
	StaticActorMap.emplace(3741576345, "PersistentLevel.Wall9");
	StaticActorMap.emplace(2407730340, "PersistentLevel.DocumentationActor1");
	StaticActorMap.emplace(3157154445, "PersistentLevel.AtmosphericFog_1");
	StaticActorMap.emplace(2630414082, "PersistentLevel.SkyLight_0");
	StaticActorMap.emplace(3648063513, "PersistentLevel.WorldInfo_0");
	StaticActorMap.emplace(2903090501, "PersistentLevel.ThirdPersonExampleMap_C_2");
	StaticActorMap.emplace(4208844388, "PersistentLevel.LightSource_0");
	StaticActorMap.emplace(3818225277, "PersistentLevel.AbstractNavData-Default");
	StaticActorMap.emplace(3116900111, "PersistentLevel.NetworkPlayerStart");
	StaticActorMap.emplace(2618737432, "PersistentLevel.SphereReflectionCapture");
	StaticActorMap.emplace(2534501830, "PersistentLevel.TextRenderActor_1");
	StaticActorMap.emplace(2408470588, "PersistentLevel.PackageMapperUtil_48");
	StaticActorMap.emplace(4041226225, "PersistentLevel.SkySphereBlueprint");

	auto snapshotEntity =
		improbable::unreal::FEntityBuilder::Begin()
		.AddPositionComponent(Position::Data{InitialPosition}, UnrealWorkerWritePermission)
		.AddMetadataComponent(Metadata::Data("LevelData"))
		.SetPersistence(true)
		.SetReadAcl(AnyWorkerReadPermission)
		.AddComponent<unreal::UnrealLevel>(unreal::UnrealLevel::Data{StaticActorMap}, UnrealWorkerWritePermission)
		.Build();

	return snapshotEntity;
}
