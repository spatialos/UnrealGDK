// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "SpatialGDKEditorSnapshotGenerator.h"

#include "Engine/LevelScriptActor.h"
#include "EngineClasses/SpatialWorldSettings.h"
#include "Interop/SpatialClassInfoManager.h"
#include "Schema/Interest.h"
#include "Schema/SnapshotVersionComponent.h"
#include "Schema/SpawnData.h"
#include "Schema/StandardLibrary.h"
#include "Schema/UnrealMetadata.h"
#include "SpatialConstants.h"
#include "SpatialGDKServicesConstants.h"
#include "SpatialGDKSettings.h"
#include "Utils/ComponentFactory.h"
#include "Utils/EntityFactory.h"
#include "Utils/RepDataUtils.h"
#include "Utils/RepLayoutUtils.h"
#include "Utils/SchemaUtils.h"
#include "Utils/SnapshotGenerationTemplate.h"

#include "EngineUtils.h"
#include "HAL/PlatformFile.h"
#include "HAL/PlatformFilemanager.h"
#include "Runtime/Engine/Classes/Engine/World.h"
#include "Runtime/Engine/Classes/Engine/WorldComposition.h"
#include "UObject/UObjectIterator.h"

#include <WorkerSDK/improbable/c_schema.h>
#include <WorkerSDK/improbable/c_worker.h>

using namespace SpatialGDK;

DEFINE_LOG_CATEGORY(LogSpatialGDKSnapshot);

TArray<Worker_ComponentData> UnpackedComponentData;

void SetEntityData(Worker_Entity& Entity, const TArray<FWorkerComponentData>& Components)
{
	Entity.component_count = Components.Num();

#if TRACE_LIB_ACTIVE
	// We have to unpack these as Worker_ComponentData is not the same as FWorkerComponentData
	UnpackedComponentData.Empty();
	UnpackedComponentData.SetNum(Components.Num());
	for (int i = 0, Num = Components.Num(); i < Num; i++)
	{
		UnpackedComponentData[i] = Components[i];
	}
	Entity.components = UnpackedComponentData.GetData();
#else
	Entity.components = Components.GetData();
#endif
}

bool CreateSpawnerEntity(Worker_SnapshotOutputStream* OutputStream)
{
	Worker_Entity SpawnerEntity;
	SpawnerEntity.entity_id = SpatialConstants::INITIAL_SPAWNER_ENTITY_ID;

	Worker_ComponentData PlayerSpawnerData = {};
	PlayerSpawnerData.component_id = SpatialConstants::PLAYER_SPAWNER_COMPONENT_ID;
	PlayerSpawnerData.schema_type = Schema_CreateComponentData();

	Interest SelfInterest;
	Query AuthoritySelfQuery = {};
	AuthoritySelfQuery.ResultComponentIds = { SpatialConstants::GDK_KNOWN_ENTITY_TAG_COMPONENT_ID };
	AuthoritySelfQuery.Constraint.bSelfConstraint = true;
	SelfInterest.ComponentInterestMap.Add(SpatialConstants::GDK_KNOWN_ENTITY_AUTH_COMPONENT_SET_ID);
	SelfInterest.ComponentInterestMap[SpatialConstants::GDK_KNOWN_ENTITY_AUTH_COMPONENT_SET_ID].Queries.Add(AuthoritySelfQuery);

	TArray<FWorkerComponentData> Components;

	AuthorityDelegationMap DelegationMap;
	DelegationMap.Add(SpatialConstants::GDK_KNOWN_ENTITY_AUTH_COMPONENT_SET_ID, SpatialConstants::INITIAL_SNAPSHOT_PARTITION_ENTITY_ID);

	Components.Add(Position(DeploymentOrigin).CreateComponentData());
	Components.Add(Metadata(TEXT("SpatialSpawner")).CreateComponentData());
	Components.Add(Persistence().CreateComponentData());
	Components.Add(PlayerSpawnerData);
	Components.Add(SelfInterest.CreateComponentData());

	// GDK known entities completeness tags.
	Components.Add(ComponentFactory::CreateEmptyComponentData(SpatialConstants::GDK_KNOWN_ENTITY_TAG_COMPONENT_ID));
	Components.Add(AuthorityDelegation(DelegationMap).CreateComponentData());

	SetEntityData(SpawnerEntity, Components);

	Worker_SnapshotOutputStream_WriteEntity(OutputStream, &SpawnerEntity);
	return Worker_SnapshotOutputStream_GetState(OutputStream).stream_state == WORKER_STREAM_STATE_GOOD;
}

Worker_ComponentData CreateDeploymentData()
{
	Worker_ComponentData DeploymentData{};
	DeploymentData.component_id = SpatialConstants::DEPLOYMENT_MAP_COMPONENT_ID;
	DeploymentData.schema_type = Schema_CreateComponentData();
	Schema_Object* DeploymentDataObject = Schema_GetComponentDataFields(DeploymentData.schema_type);

	AddStringToSchema(DeploymentDataObject, SpatialConstants::DEPLOYMENT_MAP_MAP_URL_ID, "");
	Schema_AddBool(DeploymentDataObject, SpatialConstants::DEPLOYMENT_MAP_ACCEPTING_PLAYERS_ID, false);
	Schema_AddInt32(DeploymentDataObject, SpatialConstants::DEPLOYMENT_MAP_SESSION_ID, 0);
	Schema_AddUint32(DeploymentDataObject, SpatialConstants::DEPLOYMENT_MAP_SCHEMA_HASH, 0);

	return DeploymentData;
}

Worker_ComponentData CreateGSMShutdownData()
{
	Worker_ComponentData GSMShutdownData{};
	GSMShutdownData.component_id = SpatialConstants::GSM_SHUTDOWN_COMPONENT_ID;
	GSMShutdownData.schema_type = Schema_CreateComponentData();
	return GSMShutdownData;
}

Worker_ComponentData CreateStartupActorManagerData()
{
	Worker_ComponentData StartupActorManagerData{};
	StartupActorManagerData.component_id = SpatialConstants::STARTUP_ACTOR_MANAGER_COMPONENT_ID;
	StartupActorManagerData.schema_type = Schema_CreateComponentData();
	Schema_Object* StartupActorManagerObject = Schema_GetComponentDataFields(StartupActorManagerData.schema_type);

	Schema_AddBool(StartupActorManagerObject, SpatialConstants::STARTUP_ACTOR_MANAGER_CAN_BEGIN_PLAY_ID, false);

	return StartupActorManagerData;
}

bool CreateGlobalStateManager(Worker_SnapshotOutputStream* OutputStream)
{
	Worker_Entity GSM;
	GSM.entity_id = SpatialConstants::INITIAL_GLOBAL_STATE_MANAGER_ENTITY_ID;

	Interest SelfInterest;
	Query AuthoritySelfQuery = {};
	AuthoritySelfQuery.ResultComponentIds = { SpatialConstants::GDK_KNOWN_ENTITY_TAG_COMPONENT_ID };
	AuthoritySelfQuery.Constraint.bSelfConstraint = true;
	SelfInterest.ComponentInterestMap.Add(SpatialConstants::GDK_KNOWN_ENTITY_AUTH_COMPONENT_SET_ID);
	SelfInterest.ComponentInterestMap[SpatialConstants::GDK_KNOWN_ENTITY_AUTH_COMPONENT_SET_ID].Queries.Add(AuthoritySelfQuery);

	TArray<FWorkerComponentData> Components;

	AuthorityDelegationMap DelegationMap;
	DelegationMap.Add(SpatialConstants::GDK_KNOWN_ENTITY_AUTH_COMPONENT_SET_ID, SpatialConstants::INITIAL_SNAPSHOT_PARTITION_ENTITY_ID);

	Components.Add(Position(DeploymentOrigin).CreateComponentData());
	Components.Add(Metadata(TEXT("GlobalStateManager")).CreateComponentData());
	Components.Add(Persistence().CreateComponentData());
	Components.Add(CreateDeploymentData());
	Components.Add(CreateGSMShutdownData());
	Components.Add(CreateStartupActorManagerData());
	Components.Add(SelfInterest.CreateComponentData());
	Components.Add(SnapshotVersion(SpatialConstants::SPATIAL_SNAPSHOT_VERSION).CreateComponentData());

	// GDK known entities completeness tags.
	Components.Add(ComponentFactory::CreateEmptyComponentData(SpatialConstants::GDK_KNOWN_ENTITY_TAG_COMPONENT_ID));

	Components.Add(AuthorityDelegation(DelegationMap).CreateComponentData());

	SetEntityData(GSM, Components);

	Worker_SnapshotOutputStream_WriteEntity(OutputStream, &GSM);
	return Worker_SnapshotOutputStream_GetState(OutputStream).stream_state == WORKER_STREAM_STATE_GOOD;
}

Worker_ComponentData CreateVirtualWorkerTranslatorData()
{
	Worker_ComponentData VirtualWorkerTranslatorData{};
	VirtualWorkerTranslatorData.component_id = SpatialConstants::VIRTUAL_WORKER_TRANSLATION_COMPONENT_ID;
	VirtualWorkerTranslatorData.schema_type = Schema_CreateComponentData();
	return VirtualWorkerTranslatorData;
}

bool CreateVirtualWorkerTranslator(Worker_SnapshotOutputStream* OutputStream)
{
	Worker_Entity VirtualWorkerTranslator;
	VirtualWorkerTranslator.entity_id = SpatialConstants::INITIAL_VIRTUAL_WORKER_TRANSLATOR_ENTITY_ID;

	Interest SelfInterest;
	Query AuthoritySelfQuery = {};
	AuthoritySelfQuery.ResultComponentIds = { SpatialConstants::GDK_KNOWN_ENTITY_TAG_COMPONENT_ID };
	AuthoritySelfQuery.Constraint.bSelfConstraint = true;
	SelfInterest.ComponentInterestMap.Add(SpatialConstants::GDK_KNOWN_ENTITY_AUTH_COMPONENT_SET_ID);
	SelfInterest.ComponentInterestMap[SpatialConstants::GDK_KNOWN_ENTITY_AUTH_COMPONENT_SET_ID].Queries.Add(AuthoritySelfQuery);

	TArray<FWorkerComponentData> Components;

	AuthorityDelegationMap DelegationMap;
	DelegationMap.Add(SpatialConstants::GDK_KNOWN_ENTITY_AUTH_COMPONENT_SET_ID, SpatialConstants::INITIAL_SNAPSHOT_PARTITION_ENTITY_ID);

	Components.Add(Position(DeploymentOrigin).CreateComponentData());
	Components.Add(Metadata(TEXT("VirtualWorkerTranslator")).CreateComponentData());
	Components.Add(Persistence().CreateComponentData());
	Components.Add(CreateVirtualWorkerTranslatorData());
	Components.Add(SelfInterest.CreateComponentData());

	// GDK known entities completeness tags.
	Components.Add(ComponentFactory::CreateEmptyComponentData(SpatialConstants::GDK_KNOWN_ENTITY_TAG_COMPONENT_ID));

	Components.Add(AuthorityDelegation(DelegationMap).CreateComponentData());

	SetEntityData(VirtualWorkerTranslator, Components);

	Worker_SnapshotOutputStream_WriteEntity(OutputStream, &VirtualWorkerTranslator);
	return Worker_SnapshotOutputStream_GetState(OutputStream).stream_state == WORKER_STREAM_STATE_GOOD;
}

bool CreateSnapshotPartitionEntity(Worker_SnapshotOutputStream* OutputStream)
{
	Worker_Entity SnapshotPartitionEntity;
	SnapshotPartitionEntity.entity_id = SpatialConstants::INITIAL_SNAPSHOT_PARTITION_ENTITY_ID;

	TArray<FWorkerComponentData> Components;

	AuthorityDelegationMap DelegationMap;
	DelegationMap.Add(SpatialConstants::GDK_KNOWN_ENTITY_AUTH_COMPONENT_SET_ID, SpatialConstants::INITIAL_SNAPSHOT_PARTITION_ENTITY_ID);

	Components.Add(Position(DeploymentOrigin).CreateComponentData());
	Components.Add(Metadata(TEXT("SnapshotPartitionEntity")).CreateComponentData());
	Components.Add(Persistence().CreateComponentData());
	Components.Add(AuthorityDelegation(DelegationMap).CreateComponentData());

	SetEntityData(SnapshotPartitionEntity, Components);

	Worker_SnapshotOutputStream_WriteEntity(OutputStream, &SnapshotPartitionEntity);
	return Worker_SnapshotOutputStream_GetState(OutputStream).stream_state == WORKER_STREAM_STATE_GOOD;
}

bool MergeSnapshots(const TArray<FString>& SnapshotPaths, const FString& OutputSnapshotPath)
{
	Worker_ComponentVtable DefaultVtable{};
	Worker_SnapshotParameters Parameters{};
	Parameters.default_component_vtable = &DefaultVtable;

	Worker_SnapshotOutputStream* OutputStream = Worker_SnapshotOutputStream_Create(TCHAR_TO_UTF8(*OutputSnapshotPath), &Parameters);
	if (Worker_SnapshotOutputStream_GetState(OutputStream).stream_state != WORKER_STREAM_STATE_GOOD)
	{
		UE_LOG(LogSpatialGDKSnapshot, Error, TEXT("Failed beginning write into world composition output merged snapshot %s. Error: %s"),
			   *OutputSnapshotPath, UTF8_TO_TCHAR(Worker_SnapshotOutputStream_GetState(OutputStream).error_message));
		return false;
	}
	Schema_EntityId NextAvailableEntityId = 1;

	for (const FString& Path : SnapshotPaths)
	{
		UE_LOG(LogSpatialGDKSnapshot, Display, TEXT("DEBUGLOG: Starting read for merging a snapshot from %s to %s."), *Path,
			   *OutputSnapshotPath);
		Worker_SnapshotInputStream* InputStream = Worker_SnapshotInputStream_Create(TCHAR_TO_UTF8(*Path), &Parameters);
		if (Worker_SnapshotInputStream_GetState(InputStream).stream_state != WORKER_STREAM_STATE_GOOD)
		{
			UE_LOG(LogSpatialGDKSnapshot, Error, TEXT("Failed beginning read from world composition input snapshot %s. Error: %s"), *Path,
				   UTF8_TO_TCHAR(Worker_SnapshotInputStream_GetState(InputStream).error_message));
			return false;
		}

		while (Worker_SnapshotInputStream_HasNext(InputStream))
		{
			const worker::c::Worker_Entity* Entity = Worker_SnapshotInputStream_ReadEntity(InputStream);
			if (Worker_SnapshotInputStream_GetState(InputStream).stream_state != WORKER_STREAM_STATE_GOOD)
			{
				UE_LOG(LogSpatialGDKSnapshot, Error, TEXT("Failed reading entity from world composition input snapshot %s. Error: %s"),
					   *Path, UTF8_TO_TCHAR(Worker_SnapshotInputStream_GetState(InputStream).error_message));
				return false;
			}
			if (Entity == nullptr)
			{
				// signifies error, the caller will have to call Worker_SnapshotOutputStream_GetState
				return false;
			}

			TUniquePtr<Worker_Entity> CopiedEntity;
			TArray<Worker_ComponentData> ComponentDatas;

			if (Entity->entity_id >= NextAvailableEntityId)
			{
				NextAvailableEntityId = Entity->entity_id + 1;
			}
			else
			{
				// These are GDK entities, like the spawner or GSM and there should only be one of them, keep the one from the first merged
				// snapshot, ignore the rest
				if (Entity->entity_id < SpatialConstants::FIRST_AVAILABLE_ENTITY_ID)
				{
					continue;
				}

				CopiedEntity = MakeUnique<Worker_Entity>();
				CopiedEntity->entity_id = NextAvailableEntityId++;
				CopiedEntity->component_count = Entity->component_count;
				for (unsigned int i = 0; i < Entity->component_count; i++)
				{
					Worker_ComponentData ComponentData{};
					ComponentData.component_id = Entity->components[i].component_id;
					ComponentData.schema_type = Schema_CopyComponentData(Entity->components[i].schema_type);
					ComponentDatas.Add(ComponentData);
				}
				CopiedEntity->components = ComponentDatas.GetData();
				Entity = CopiedEntity.Get();
			}

			Worker_SnapshotOutputStream_WriteEntity(OutputStream, Entity);
			if (Worker_SnapshotOutputStream_GetState(OutputStream).stream_state != WORKER_STREAM_STATE_GOOD)
			{
				UE_LOG(LogSpatialGDKSnapshot, Error,
					   TEXT("Failed writing entity into world composition output merged snapshot %s. Error: %s"), *OutputSnapshotPath,
					   UTF8_TO_TCHAR(Worker_SnapshotOutputStream_GetState(OutputStream).error_message));
				return false;
			}
		}

		Worker_SnapshotInputStream_Destroy(InputStream);
	}

	Worker_SnapshotOutputStream_Destroy(OutputStream);
	return true;
}

bool CreateSkeletonEntities(UWorld* World, Worker_SnapshotOutputStream* OutputStream, Worker_EntityId& NextAvailableEntityID)
{
	// We only check the persistent level, as it seems similar to what e.g. FEditorFileUtils::SaveCurrentLevel does
	for (AActor* Actor : World->PersistentLevel->Actors)
	{
		// TODO: Apparently you can get null actors in the above array... not exactly sure why
		if (Actor == nullptr)
		{
			continue;
		}

		if (Actor->IsEditorOnly())
		{
			continue;
		}

		Worker_Entity ActorEntity;
		ActorEntity.entity_id = NextAvailableEntityID++;

		TArray<FWorkerComponentData> Components = EntityFactory::CreateSkeletonEntityComponents(Actor);
		// If the actor cannot be saved in the snapshot (does not have persistence component), do not save it
		if (Components.FindByPredicate([](const FWorkerComponentData& Data) {
				return Data.component_id == SpatialConstants::PERSISTENCE_COMPONENT_ID;
			})
			== nullptr)
		{
			// TODO: We ingore non-persistent entities here, but we probably don't want to, and just add temporary persistence.
			// This would be closer to parity with the previous system
			continue;
		}

		SetEntityData(ActorEntity, Components);

		Worker_SnapshotOutputStream_WriteEntity(OutputStream, &ActorEntity);
		bool bSuccess = Worker_SnapshotOutputStream_GetState(OutputStream).stream_state == WORKER_STREAM_STATE_GOOD;
		if (!bSuccess)
		{
			return false;
		}
	}

	return true;
}

bool ValidateAndCreateSnapshotGenerationPath(FString& SavePath)
{
	FString DirectoryPath = FPaths::GetPath(SavePath);
	if (!FPaths::CollapseRelativeDirectories(DirectoryPath))
	{
		UE_LOG(LogSpatialGDKSnapshot, Error, TEXT("Invalid path: %s - snapshot not generated"), *DirectoryPath);
		return false;
	}

	if (!FPaths::DirectoryExists(DirectoryPath))
	{
		UE_LOG(LogSpatialGDKSnapshot, Display, TEXT("Snapshot directory does not exist - creating directory: %s"), *DirectoryPath);
		if (!FPlatformFileManager::Get().GetPlatformFile().CreateDirectoryTree(*DirectoryPath))
		{
			UE_LOG(LogSpatialGDKSnapshot, Error, TEXT("Unable to create directory: %s - snapshot not generated"), *DirectoryPath);
			return false;
		}
	}
	return true;
}

bool RunUserSnapshotGenerationOverrides(Worker_SnapshotOutputStream* OutputStream, Worker_EntityId& NextAvailableEntityID)
{
	for (TObjectIterator<UClass> SnapshotGenerationClass; SnapshotGenerationClass; ++SnapshotGenerationClass)
	{
		if (SnapshotGenerationClass->IsChildOf(USnapshotGenerationTemplate::StaticClass())
			&& *SnapshotGenerationClass != USnapshotGenerationTemplate::StaticClass())
		{
			UE_LOG(LogSpatialGDKSnapshot, Log, TEXT("Found user snapshot generation class: %s"), *SnapshotGenerationClass->GetName());
			USnapshotGenerationTemplate* SnapshotGenerationObj =
				NewObject<USnapshotGenerationTemplate>(GetTransientPackage(), *SnapshotGenerationClass);
			if (!SnapshotGenerationObj->WriteToSnapshotOutput(OutputStream, NextAvailableEntityID))
			{
				UE_LOG(LogSpatialGDKSnapshot, Error, TEXT("Failure returned in user snapshot generation override method from class: %s"),
					   *SnapshotGenerationClass->GetName());
				return false;
			}
		}
	}
	return true;
}

bool FillSnapshot(Worker_SnapshotOutputStream* OutputStream, UWorld* World)
{
	if (!CreateSpawnerEntity(OutputStream))
	{
		UE_LOG(LogSpatialGDKSnapshot, Error, TEXT("Error generating Spawner in snapshot: %s"),
			   UTF8_TO_TCHAR(Worker_SnapshotOutputStream_GetState(OutputStream).error_message));
		return false;
	}

	if (!CreateGlobalStateManager(OutputStream))
	{
		UE_LOG(LogSpatialGDKSnapshot, Error, TEXT("Error generating GlobalStateManager in snapshot: %s"),
			   UTF8_TO_TCHAR(Worker_SnapshotOutputStream_GetState(OutputStream).error_message));
		return false;
	}

	if (!CreateVirtualWorkerTranslator(OutputStream))
	{
		UE_LOG(LogSpatialGDKSnapshot, Error, TEXT("Error generating VirtualWorkerTranslator in snapshot: %s"),
			   UTF8_TO_TCHAR(Worker_SnapshotOutputStream_GetState(OutputStream).error_message));
		return false;
	}

	if (!CreateSnapshotPartitionEntity(OutputStream))
	{
		UE_LOG(LogSpatialGDKSnapshot, Error, TEXT("Error generating SnapshotPartitionEntity in snapshot: %s"),
			   UTF8_TO_TCHAR(Worker_SnapshotOutputStream_GetState(OutputStream).error_message));
		return false;
	}

	Worker_EntityId NextAvailableEntityID = SpatialConstants::FIRST_AVAILABLE_ENTITY_ID;
	if (GetDefault<USpatialGDKSettings>()->bEnableActorsInSnapshots)
	{
		if (!CreateSkeletonEntities(World, OutputStream, NextAvailableEntityID))
		{
			UE_LOG(LogSpatialGDKSnapshot, Error, TEXT("Error generating skeleton entities in snapshot: %s"),
				   UTF8_TO_TCHAR(Worker_SnapshotOutputStream_GetState(OutputStream).error_message));
			return false;
		}
	}

	if (!RunUserSnapshotGenerationOverrides(OutputStream, NextAvailableEntityID))
	{
		UE_LOG(LogSpatialGDKSnapshot, Error, TEXT("Error running user defined snapshot generation overrides in snapshot: %s"),
			   UTF8_TO_TCHAR(Worker_SnapshotOutputStream_GetState(OutputStream).error_message));
		return false;
	}

	return true;
}

bool SpatialGDKGenerateSnapshot(UWorld* World, FString SnapshotPath)
{
	if (!ValidateAndCreateSnapshotGenerationPath(SnapshotPath))
	{
		return false;
	}

	UE_LOG(LogSpatialGDKSnapshot, Display, TEXT("Saving snapshot to: %s"), *SnapshotPath);

	Worker_ComponentVtable DefaultVtable{};
	Worker_SnapshotParameters Parameters{};
	Parameters.default_component_vtable = &DefaultVtable;

	bool bSuccess = true;
	Worker_SnapshotOutputStream* OutputStream = Worker_SnapshotOutputStream_Create(TCHAR_TO_UTF8(*SnapshotPath), &Parameters);
	if (const char* SchemaError = Worker_SnapshotOutputStream_GetState(OutputStream).error_message)
	{
		UE_LOG(LogSpatialGDKSnapshot, Error, TEXT("Error creating SnapshotOutputStream: %s"), UTF8_TO_TCHAR(SchemaError));
		bSuccess = false;
	}
	else
	{
		bSuccess = FillSnapshot(OutputStream, World);
	}

	Worker_SnapshotOutputStream_Destroy(OutputStream);

	if (GetDefault<USpatialGDKSettings>()->bEnableActorsInSnapshots)
	{
		// However, we now check if this level we saved was saved as part of a world using world composition
		// This is slightly crude, as we could edit the level without opening the enclosing world
		// TODO: Manual merge from somewhere?
		if (World->GetWorldSettings() != nullptr && World->GetWorldSettings()->bEnableWorldComposition
			&& World->WorldComposition != nullptr)
		{
			double StartTime = FPlatformTime::Seconds();
			TArray<FString> SnapshotPaths;
			// Add the root world snapshot
			SnapshotPaths.Add(SnapshotPath);
			// Add the sublevel snapshots
			for (const auto& Tile : World->WorldComposition->GetTilesList())
			{
				// UE_LOG(LogSpatialGDKSnapshot, Display, TEXT("Found tile package name: %s."), *Tile.PackageName.ToString());
				// TODO: Kind of copied from SpatialGDKEditorModule
				// The things in the editor module should move here and then this path combine should be made into a function
				FString SnapshotFile = FPaths::Combine(SpatialGDKServicesConstants::SpatialOSSnapshotFolderPath,
													   Tile.PackageName.ToString() + TEXT(".snapshot"));
				SnapshotPaths.Add(SnapshotFile);
			}
			FString MergedSnapshotFile = SnapshotPath + ".merged"; // meh
			bool bSuccessMerging = MergeSnapshots(SnapshotPaths, MergedSnapshotFile);
			if (!bSuccessMerging)
			{
				bSuccess = false;
				UE_LOG(LogSpatialGDKSnapshot, Error, TEXT("Failed merging world composition snapshots."));
			}
			if (ASpatialWorldSettings* WorldSettings = Cast<ASpatialWorldSettings>(World->GetWorldSettings()))
			{
				IFileHandle* File = FPlatformFileManager::Get().GetPlatformFile().OpenRead(*MergedSnapshotFile);
				WorldSettings->RawPersistentLevelSnapshotData.SetNum(File->Size());
				File->Read(WorldSettings->RawPersistentLevelSnapshotData.GetData(), File->Size());
				// TODO: Is there a nicer way to close the handle? Delete also sounds sketchy
				File->~IFileHandle();
			}
			else
			{
				UE_LOG(LogSpatialGDKSnapshot, Error,
					   TEXT("Could not save the merged snapshot into the world settings. Make sure you are using SpatialWorldSettings."));
			}
			UE_LOG(LogSpatialGDKSnapshot, Log, TEXT("Generating the merged snapshot for the saved world took %6.2fms."),
				   1000.0f * float(FPlatformTime::Seconds() - StartTime));
		}
	}

	return bSuccess;
}

bool SpatialGDKGenerateSnapshotOnLevelSave(UWorld* World)
{
	bool bSuccess = true;
	double StartTime = FPlatformTime::Seconds();
	FString SnapshotFile =
		FPaths::Combine(SpatialGDKServicesConstants::SpatialOSSnapshotFolderPath, World->GetPackage()->GetName() + TEXT(".snapshot"));
	bSuccess &= SpatialGDKGenerateSnapshot(World, SnapshotFile);
	if (!bSuccess)
	{
		UE_LOG(LogSpatialGDKSnapshot, Error, TEXT("Could not generate snapshot on level save."));
		return false;
	}
	if (ASpatialWorldSettings* WorldSettings = Cast<ASpatialWorldSettings>(World->GetWorldSettings()))
	{
		IFileHandle* File = FPlatformFileManager::Get().GetPlatformFile().OpenRead(*SnapshotFile);
		WorldSettings->RawPersistentLevelSnapshotData.SetNum(File->Size());
		bSuccess &= File->Read(WorldSettings->RawPersistentLevelSnapshotData.GetData(), File->Size());
		// TODO: Is there a nicer way to close the handle? Delete also sounds sketchy
		File->~IFileHandle();
	}
	else
	{
		UE_LOG(LogSpatialGDKSnapshot, Error,
			   TEXT("Could not save snapshot into the world settings. Make sure you are using SpatialWorldSettings."));
		return false;
	}
	UE_LOG(LogSpatialGDKSnapshot, Log, TEXT("Generating the snapshot for the saved world took %6.2fms."),
		   1000.0f * float(FPlatformTime::Seconds() - StartTime));
	return true;
}
