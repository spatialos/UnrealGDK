// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "SpatialGDKEditorSnapshotGenerator.h"

#include "Engine/LevelScriptActor.h"
#include "Interop/SpatialClassInfoManager.h"
#include "Schema/ComponentPresence.h"
#include "Schema/Interest.h"
#include "Schema/SpawnData.h"
#include "Schema/StandardLibrary.h"
#include "Schema/UnrealMetadata.h"
#include "SpatialConstants.h"
#include "SpatialGDKSettings.h"
#include "Utils/EntityFactory.h"
#include "Utils/ComponentFactory.h"
#include "Utils/RepDataUtils.h"
#include "Utils/RepLayoutUtils.h"
#include "Utils/SchemaUtils.h"
#include "Utils/SnapshotGenerationTemplate.h"

#include "EngineUtils.h"
#include "HAL/PlatformFile.h"
#include "HAL/PlatformFilemanager.h"
#include "UObject/UObjectIterator.h"

#include <WorkerSDK/improbable/c_worker.h>
#include <WorkerSDK/improbable/c_schema.h>

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

	TArray<FWorkerComponentData> Components;

	WriteAclMap ComponentWriteAcl;
	ComponentWriteAcl.Add(SpatialConstants::POSITION_COMPONENT_ID, SpatialConstants::UnrealServerPermission);
	ComponentWriteAcl.Add(SpatialConstants::METADATA_COMPONENT_ID, SpatialConstants::UnrealServerPermission);
	ComponentWriteAcl.Add(SpatialConstants::PERSISTENCE_COMPONENT_ID, SpatialConstants::UnrealServerPermission);
	ComponentWriteAcl.Add(SpatialConstants::ENTITY_ACL_COMPONENT_ID, SpatialConstants::UnrealServerPermission);
	ComponentWriteAcl.Add(SpatialConstants::PLAYER_SPAWNER_COMPONENT_ID, SpatialConstants::UnrealServerPermission);
	ComponentWriteAcl.Add(SpatialConstants::COMPONENT_PRESENCE_COMPONENT_ID, SpatialConstants::UnrealServerPermission);

	AuthorityDelegationMap DelegationMap;
	DelegationMap.Add(SpatialConstants::POSITION_COMPONENT_ID, SpatialConstants::INITIAL_SNAPSHOT_PARTITION_ENTITY_ID);
	DelegationMap.Add(SpatialConstants::METADATA_COMPONENT_ID, SpatialConstants::INITIAL_SNAPSHOT_PARTITION_ENTITY_ID);
	DelegationMap.Add(SpatialConstants::PERSISTENCE_COMPONENT_ID, SpatialConstants::INITIAL_SNAPSHOT_PARTITION_ENTITY_ID);
	DelegationMap.Add(SpatialConstants::ENTITY_ACL_COMPONENT_ID, SpatialConstants::INITIAL_SNAPSHOT_PARTITION_ENTITY_ID);
	DelegationMap.Add(SpatialConstants::AUTHORITY_DELEGATION_COMPONENT_ID, SpatialConstants::INITIAL_SNAPSHOT_PARTITION_ENTITY_ID);
	DelegationMap.Add(SpatialConstants::PLAYER_SPAWNER_COMPONENT_ID, SpatialConstants::INITIAL_SNAPSHOT_PARTITION_ENTITY_ID);
	DelegationMap.Add(SpatialConstants::COMPONENT_PRESENCE_COMPONENT_ID, SpatialConstants::INITIAL_SNAPSHOT_PARTITION_ENTITY_ID);

	Components.Add(AuthorityDelegation(DelegationMap).CreateAuthorityDelegationData());
	Components.Add(Position(DeploymentOrigin).CreatePositionData());
	Components.Add(Metadata(TEXT("SpatialSpawner")).CreateMetadataData());
	Components.Add(Persistence().CreatePersistenceData());
	Components.Add(EntityAcl(SpatialConstants::ClientOrServerPermission, ComponentWriteAcl).CreateEntityAclData());
	Components.Add(PlayerSpawnerData);
	Components.Add(ComponentPresence(EntityFactory::GetComponentPresenceList(Components)).CreateComponentPresenceData());

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
	Worker_ComponentData GSMShutdownData;
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

	TArray<FWorkerComponentData> Components;

	WriteAclMap ComponentWriteAcl;
	ComponentWriteAcl.Add(SpatialConstants::POSITION_COMPONENT_ID, SpatialConstants::UnrealServerPermission);
	ComponentWriteAcl.Add(SpatialConstants::METADATA_COMPONENT_ID, SpatialConstants::UnrealServerPermission);
	ComponentWriteAcl.Add(SpatialConstants::PERSISTENCE_COMPONENT_ID, SpatialConstants::UnrealServerPermission);
	ComponentWriteAcl.Add(SpatialConstants::ENTITY_ACL_COMPONENT_ID, SpatialConstants::UnrealServerPermission);
	ComponentWriteAcl.Add(SpatialConstants::DEPLOYMENT_MAP_COMPONENT_ID, SpatialConstants::UnrealServerPermission);
	ComponentWriteAcl.Add(SpatialConstants::GSM_SHUTDOWN_COMPONENT_ID, SpatialConstants::UnrealServerPermission);
	ComponentWriteAcl.Add(SpatialConstants::STARTUP_ACTOR_MANAGER_COMPONENT_ID, SpatialConstants::UnrealServerPermission);
	ComponentWriteAcl.Add(SpatialConstants::COMPONENT_PRESENCE_COMPONENT_ID, SpatialConstants::UnrealServerPermission);

	AuthorityDelegationMap DelegationMap;
	DelegationMap.Add(SpatialConstants::POSITION_COMPONENT_ID, SpatialConstants::INITIAL_SNAPSHOT_PARTITION_ENTITY_ID);
	DelegationMap.Add(SpatialConstants::METADATA_COMPONENT_ID, SpatialConstants::INITIAL_SNAPSHOT_PARTITION_ENTITY_ID);
	DelegationMap.Add(SpatialConstants::PERSISTENCE_COMPONENT_ID, SpatialConstants::INITIAL_SNAPSHOT_PARTITION_ENTITY_ID);
	DelegationMap.Add(SpatialConstants::ENTITY_ACL_COMPONENT_ID, SpatialConstants::INITIAL_SNAPSHOT_PARTITION_ENTITY_ID);
	DelegationMap.Add(SpatialConstants::AUTHORITY_DELEGATION_COMPONENT_ID, SpatialConstants::INITIAL_SNAPSHOT_PARTITION_ENTITY_ID);
	DelegationMap.Add(SpatialConstants::DEPLOYMENT_MAP_COMPONENT_ID, SpatialConstants::INITIAL_SNAPSHOT_PARTITION_ENTITY_ID);
	DelegationMap.Add(SpatialConstants::GSM_SHUTDOWN_COMPONENT_ID, SpatialConstants::INITIAL_SNAPSHOT_PARTITION_ENTITY_ID);
	DelegationMap.Add(SpatialConstants::STARTUP_ACTOR_MANAGER_COMPONENT_ID, SpatialConstants::INITIAL_SNAPSHOT_PARTITION_ENTITY_ID);
	DelegationMap.Add(SpatialConstants::COMPONENT_PRESENCE_COMPONENT_ID, SpatialConstants::INITIAL_SNAPSHOT_PARTITION_ENTITY_ID);

	WorkerRequirementSet ReadACL = { SpatialConstants::UnrealServerAttributeSet };

	Components.Add(Position(DeploymentOrigin).CreatePositionData());
	Components.Add(Metadata(TEXT("GlobalStateManager")).CreateMetadataData());
	Components.Add(Persistence().CreatePersistenceData());
	Components.Add(CreateDeploymentData());
	Components.Add(CreateGSMShutdownData());
	Components.Add(CreateStartupActorManagerData());
	Components.Add(EntityAcl(ReadACL, ComponentWriteAcl).CreateEntityAclData());
	Components.Add(ComponentPresence(EntityFactory::GetComponentPresenceList(Components)).CreateComponentPresenceData());
	Components.Add(AuthorityDelegation(DelegationMap).CreateAuthorityDelegationData());

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

	TArray<FWorkerComponentData> Components;

	WriteAclMap ComponentWriteAcl;
	ComponentWriteAcl.Add(SpatialConstants::POSITION_COMPONENT_ID, SpatialConstants::UnrealServerPermission);
	ComponentWriteAcl.Add(SpatialConstants::METADATA_COMPONENT_ID, SpatialConstants::UnrealServerPermission);
	ComponentWriteAcl.Add(SpatialConstants::PERSISTENCE_COMPONENT_ID, SpatialConstants::UnrealServerPermission);
	ComponentWriteAcl.Add(SpatialConstants::ENTITY_ACL_COMPONENT_ID, SpatialConstants::UnrealServerPermission);
	ComponentWriteAcl.Add(SpatialConstants::VIRTUAL_WORKER_TRANSLATION_COMPONENT_ID, SpatialConstants::UnrealServerPermission);
	ComponentWriteAcl.Add(SpatialConstants::COMPONENT_PRESENCE_COMPONENT_ID, SpatialConstants::UnrealServerPermission);

	AuthorityDelegationMap DelegationMap;
	DelegationMap.Add(SpatialConstants::POSITION_COMPONENT_ID, SpatialConstants::INITIAL_SNAPSHOT_PARTITION_ENTITY_ID);
	DelegationMap.Add(SpatialConstants::METADATA_COMPONENT_ID, SpatialConstants::INITIAL_SNAPSHOT_PARTITION_ENTITY_ID);
	DelegationMap.Add(SpatialConstants::PERSISTENCE_COMPONENT_ID, SpatialConstants::INITIAL_SNAPSHOT_PARTITION_ENTITY_ID);
	DelegationMap.Add(SpatialConstants::ENTITY_ACL_COMPONENT_ID, SpatialConstants::INITIAL_SNAPSHOT_PARTITION_ENTITY_ID);
	DelegationMap.Add(SpatialConstants::AUTHORITY_DELEGATION_COMPONENT_ID, SpatialConstants::INITIAL_SNAPSHOT_PARTITION_ENTITY_ID);
	DelegationMap.Add(SpatialConstants::VIRTUAL_WORKER_TRANSLATION_COMPONENT_ID, SpatialConstants::INITIAL_SNAPSHOT_PARTITION_ENTITY_ID);
	DelegationMap.Add(SpatialConstants::COMPONENT_PRESENCE_COMPONENT_ID, SpatialConstants::INITIAL_SNAPSHOT_PARTITION_ENTITY_ID);

	WorkerRequirementSet ReadACL = { SpatialConstants::UnrealServerAttributeSet };

	Components.Add(Position(DeploymentOrigin).CreatePositionData());
	Components.Add(Metadata(TEXT("VirtualWorkerTranslator")).CreateMetadataData());
	Components.Add(Persistence().CreatePersistenceData());
	Components.Add(CreateVirtualWorkerTranslatorData());
	Components.Add(EntityAcl(ReadACL, ComponentWriteAcl).CreateEntityAclData());
	Components.Add(ComponentPresence(EntityFactory::GetComponentPresenceList(Components)).CreateComponentPresenceData());
	Components.Add(AuthorityDelegation(DelegationMap).CreateAuthorityDelegationData());

	SetEntityData(VirtualWorkerTranslator, Components);

	Worker_SnapshotOutputStream_WriteEntity(OutputStream, &VirtualWorkerTranslator);
	return Worker_SnapshotOutputStream_GetState(OutputStream).stream_state == WORKER_STREAM_STATE_GOOD;
}

bool CreateSnapshotPartitionEntity(Worker_SnapshotOutputStream* OutputStream)
{
	Worker_Entity SnapshotPartitionEntity;
	SnapshotPartitionEntity.entity_id = SpatialConstants::INITIAL_SNAPSHOT_PARTITION_ENTITY_ID;

	TArray<FWorkerComponentData> Components;

	WriteAclMap ComponentWriteAcl;
	ComponentWriteAcl.Add(SpatialConstants::POSITION_COMPONENT_ID, SpatialConstants::UnrealServerPermission);
	ComponentWriteAcl.Add(SpatialConstants::METADATA_COMPONENT_ID, SpatialConstants::UnrealServerPermission);
	ComponentWriteAcl.Add(SpatialConstants::PERSISTENCE_COMPONENT_ID, SpatialConstants::UnrealServerPermission);
	ComponentWriteAcl.Add(SpatialConstants::ENTITY_ACL_COMPONENT_ID, SpatialConstants::UnrealServerPermission);
	ComponentWriteAcl.Add(SpatialConstants::AUTHORITY_DELEGATION_COMPONENT_ID, SpatialConstants::UnrealServerPermission);
	ComponentWriteAcl.Add(SpatialConstants::COMPONENT_PRESENCE_COMPONENT_ID, SpatialConstants::UnrealServerPermission);

	AuthorityDelegationMap DelegationMap;
	DelegationMap.Add(SpatialConstants::POSITION_COMPONENT_ID, SpatialConstants::INITIAL_SNAPSHOT_PARTITION_ENTITY_ID);
	DelegationMap.Add(SpatialConstants::METADATA_COMPONENT_ID, SpatialConstants::INITIAL_SNAPSHOT_PARTITION_ENTITY_ID);
	DelegationMap.Add(SpatialConstants::PERSISTENCE_COMPONENT_ID, SpatialConstants::INITIAL_SNAPSHOT_PARTITION_ENTITY_ID);
	DelegationMap.Add(SpatialConstants::ENTITY_ACL_COMPONENT_ID, SpatialConstants::INITIAL_SNAPSHOT_PARTITION_ENTITY_ID);
	DelegationMap.Add(SpatialConstants::AUTHORITY_DELEGATION_COMPONENT_ID, SpatialConstants::INITIAL_SNAPSHOT_PARTITION_ENTITY_ID);
	DelegationMap.Add(SpatialConstants::COMPONENT_PRESENCE_COMPONENT_ID, SpatialConstants::INITIAL_SNAPSHOT_PARTITION_ENTITY_ID);

	Components.Add(Position(DeploymentOrigin).CreatePositionData());
	Components.Add(Metadata(TEXT("SnapshotPartitionEntity")).CreateMetadataData());
	Components.Add(Persistence().CreatePersistenceData());
	Components.Add(EntityAcl(CreateReadACLForAlwaysRelevantEntities(), ComponentWriteAcl).CreateEntityAclData());
	Components.Add(AuthorityDelegation(DelegationMap).CreateAuthorityDelegationData());
	Components.Add(ComponentPresence(EntityFactory::GetComponentPresenceList(Components)).CreateComponentPresenceData());

	SetEntityData(SnapshotPartitionEntity, Components);

	Worker_SnapshotOutputStream_WriteEntity(OutputStream, &SnapshotPartitionEntity);
	return Worker_SnapshotOutputStream_GetState(OutputStream).stream_state == WORKER_STREAM_STATE_GOOD;
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
		if (SnapshotGenerationClass->IsChildOf(USnapshotGenerationTemplate::StaticClass()) && *SnapshotGenerationClass != USnapshotGenerationTemplate::StaticClass())
		{
			UE_LOG(LogSpatialGDKSnapshot, Log, TEXT("Found user snapshot generation class: %s"), *SnapshotGenerationClass->GetName());
			USnapshotGenerationTemplate *SnapshotGenerationObj = NewObject<USnapshotGenerationTemplate>(GetTransientPackage(), *SnapshotGenerationClass);
			if (!SnapshotGenerationObj->WriteToSnapshotOutput(OutputStream, NextAvailableEntityID))
			{
				UE_LOG(LogSpatialGDKSnapshot, Error, TEXT("Failure returned in user snapshot generation override method from class: %s"), *SnapshotGenerationClass->GetName());
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
		UE_LOG(LogSpatialGDKSnapshot, Error, TEXT("Error generating Spawner in snapshot: %s"), UTF8_TO_TCHAR(Worker_SnapshotOutputStream_GetState(OutputStream).error_message));
		return false;
	}

	if (!CreateGlobalStateManager(OutputStream))
	{
		UE_LOG(LogSpatialGDKSnapshot, Error, TEXT("Error generating GlobalStateManager in snapshot: %s"), UTF8_TO_TCHAR(Worker_SnapshotOutputStream_GetState(OutputStream).error_message));
		return false;
	}

	if (!CreateVirtualWorkerTranslator(OutputStream))
	{
		UE_LOG(LogSpatialGDKSnapshot, Error, TEXT("Error generating VirtualWorkerTranslator in snapshot: %s"), UTF8_TO_TCHAR(Worker_SnapshotOutputStream_GetState(OutputStream).error_message));
		return false;
	}

	if (!CreateSnapshotPartitionEntity(OutputStream))
	{
		UE_LOG(LogSpatialGDKSnapshot, Error, TEXT("Error generating SnapshotPartitionEntity in snapshot: %s"), UTF8_TO_TCHAR(Worker_SnapshotOutputStream_GetState(OutputStream).error_message));
		return false;
	}

	Worker_EntityId NextAvailableEntityID = SpatialConstants::FIRST_AVAILABLE_ENTITY_ID;
	if (!RunUserSnapshotGenerationOverrides(OutputStream, NextAvailableEntityID))
	{
		UE_LOG(LogSpatialGDKSnapshot, Error, TEXT("Error running user defined snapshot generation overrides in snapshot: %s"), UTF8_TO_TCHAR(Worker_SnapshotOutputStream_GetState(OutputStream).error_message));
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

	return bSuccess;
}
