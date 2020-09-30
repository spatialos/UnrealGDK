// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "SpatialGDKEditorSnapshotGenerator.h"

#include "Engine/LevelScriptActor.h"
#include "EngineClasses/SpatialActorChannel.h"
#include "EngineClasses/SpatialNetConnection.h"
#include "EngineClasses/SpatialPackageMapClient.h"
#include "EngineClasses/SpatialWorldSettings.h"
#include "Interop/SpatialClassInfoManager.h"
#include "Interop/SpatialSender.h"
#include "Schema/ComponentPresence.h"
#include "Schema/Interest.h"
#include "Schema/SpawnData.h"
#include "Schema/StandardLibrary.h"
#include "Schema/UnrealMetadata.h"
#include "SpatialConstants.h"
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

	TArray<FWorkerComponentData> Components;

	WriteAclMap ComponentWriteAcl;
	ComponentWriteAcl.Add(SpatialConstants::POSITION_COMPONENT_ID, SpatialConstants::UnrealServerPermission);
	ComponentWriteAcl.Add(SpatialConstants::METADATA_COMPONENT_ID, SpatialConstants::UnrealServerPermission);
	ComponentWriteAcl.Add(SpatialConstants::PERSISTENCE_COMPONENT_ID, SpatialConstants::UnrealServerPermission);
	ComponentWriteAcl.Add(SpatialConstants::ENTITY_ACL_COMPONENT_ID, SpatialConstants::UnrealServerPermission);
	ComponentWriteAcl.Add(SpatialConstants::PLAYER_SPAWNER_COMPONENT_ID, SpatialConstants::UnrealServerPermission);
	ComponentWriteAcl.Add(SpatialConstants::COMPONENT_PRESENCE_COMPONENT_ID, SpatialConstants::UnrealServerPermission);

	Components.Add(Position(DeploymentOrigin).CreatePositionData());
	Components.Add(Metadata(TEXT("SpatialSpawner")).CreateMetadataData());
	Components.Add(Persistence().CreatePersistenceData());
	Components.Add(EntityAcl(SpatialConstants::ClientOrServerPermission, ComponentWriteAcl).CreateEntityAclData());
	Components.Add(PlayerSpawnerData);

	// GDK known entities completeness tags
	Components.Add(ComponentFactory::CreateEmptyComponentData(SpatialConstants::SERVER_AUTH_GDK_KNOWN_ENTITY_TAG_COMPONENT_ID));
	Components.Add(ComponentFactory::CreateEmptyComponentData(SpatialConstants::SERVER_NON_AUTH_GDK_KNOWN_ENTITY_TAG_COMPONENT_ID));
	Components.Add(ComponentFactory::CreateEmptyComponentData(SpatialConstants::CLIENT_GDK_KNOWN_ENTITY_TAG_COMPONENT_ID));

	// Presence component. Must be calculated after all other components have been added.
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

	WorkerRequirementSet ReadACL = { SpatialConstants::UnrealServerAttributeSet };

	Components.Add(Position(DeploymentOrigin).CreatePositionData());
	Components.Add(Metadata(TEXT("GlobalStateManager")).CreateMetadataData());
	Components.Add(Persistence().CreatePersistenceData());
	Components.Add(CreateDeploymentData());
	Components.Add(CreateGSMShutdownData());
	Components.Add(CreateStartupActorManagerData());
	Components.Add(EntityAcl(ReadACL, ComponentWriteAcl).CreateEntityAclData());

	// GDK known entities completeness tags
	Components.Add(ComponentFactory::CreateEmptyComponentData(SpatialConstants::SERVER_AUTH_GDK_KNOWN_ENTITY_TAG_COMPONENT_ID));
	Components.Add(ComponentFactory::CreateEmptyComponentData(SpatialConstants::SERVER_NON_AUTH_GDK_KNOWN_ENTITY_TAG_COMPONENT_ID));
	Components.Add(ComponentFactory::CreateEmptyComponentData(SpatialConstants::CLIENT_GDK_KNOWN_ENTITY_TAG_COMPONENT_ID));

	// Presence component. Must be calculated after all other components have been added.
	Components.Add(ComponentPresence(EntityFactory::GetComponentPresenceList(Components)).CreateComponentPresenceData());

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

	WorkerRequirementSet ReadACL = { SpatialConstants::UnrealServerAttributeSet };

	Components.Add(Position(DeploymentOrigin).CreatePositionData());
	Components.Add(Metadata(TEXT("VirtualWorkerTranslator")).CreateMetadataData());
	Components.Add(Persistence().CreatePersistenceData());
	Components.Add(CreateVirtualWorkerTranslatorData());
	Components.Add(EntityAcl(ReadACL, ComponentWriteAcl).CreateEntityAclData());

	// GDK known entities completeness tags
	Components.Add(ComponentFactory::CreateEmptyComponentData(SpatialConstants::SERVER_AUTH_GDK_KNOWN_ENTITY_TAG_COMPONENT_ID));
	Components.Add(ComponentFactory::CreateEmptyComponentData(SpatialConstants::SERVER_NON_AUTH_GDK_KNOWN_ENTITY_TAG_COMPONENT_ID));
	Components.Add(ComponentFactory::CreateEmptyComponentData(SpatialConstants::CLIENT_GDK_KNOWN_ENTITY_TAG_COMPONENT_ID));

	// Presence component. Must be calculated after all other components have been added.
	Components.Add(ComponentPresence(EntityFactory::GetComponentPresenceList(Components)).CreateComponentPresenceData());

	SetEntityData(VirtualWorkerTranslator, Components);

	Worker_SnapshotOutputStream_WriteEntity(OutputStream, &VirtualWorkerTranslator);
	return Worker_SnapshotOutputStream_GetState(OutputStream).stream_state == WORKER_STREAM_STATE_GOOD;
}

// Set up classes needed for Startup Actor creation
void SetupStartupActorCreation(USpatialNetDriver*& NetDriver, USpatialNetConnection*& NetConnection,
							   USpatialClassInfoManager*& ClassInfoManager, UWorld* World)
{
	// COPIED FROM SPATIAL NET DRIVER - TODO: Refactor
	// Make absolutely sure that the actor channel that we are using is our Spatial actor channel
	// Copied from what the Engine does with UActorChannel
	NetDriver = NewObject<USpatialNetDriver>();
	FChannelDefinition SpatialChannelDefinition{};
	SpatialChannelDefinition.ChannelName = NAME_Actor;
	SpatialChannelDefinition.ClassName = FName(*USpatialActorChannel::StaticClass()->GetPathName());
	SpatialChannelDefinition.ChannelClass = USpatialActorChannel::StaticClass();
	SpatialChannelDefinition.bServerOpen = true;

	NetDriver->ChannelDefinitions[CHTYPE_Actor] = SpatialChannelDefinition;
	NetDriver->ChannelDefinitionMap[NAME_Actor] = SpatialChannelDefinition;
	NetDriver->World = World;
	ULayeredLBStrategy* LayeredStrategy = NewObject<ULayeredLBStrategy>(NetDriver);
	LayeredStrategy->SetLayers(
		USpatialStatics::GetSpatialMultiWorkerClass(World)->GetDefaultObject<UAbstractSpatialMultiWorkerSettings>()->WorkerLayers);
	NetDriver->LoadBalanceStrategy = LayeredStrategy;
	NetDriver->PackageMap = NewObject<USpatialPackageMapClient>();
	NetDriver->PackageMap->Initialize(NetConnection, MakeShared<FSpatialNetGUIDCache>(NetDriver));
	ClassInfoManager = NewObject<USpatialClassInfoManager>();
	ClassInfoManager->TryInit(NetDriver);
	NetDriver->ClassInfoManager = ClassInfoManager;
	NetDriver->InterestFactory = MakeUnique<SpatialGDK::InterestFactory>(ClassInfoManager, NetDriver->PackageMap);

	NetConnection = NewObject<USpatialNetConnection>();
	NetConnection->Driver = NetDriver;
	NetConnection->State = USOCK_Closed;
}

bool CreateStartupActor(Worker_SnapshotOutputStream* OutputStream, AActor* Actor, Worker_EntityId EntityId,
						USpatialNetConnection* NetConnection, USpatialClassInfoManager* ClassInfoManager)
{
	UClass* ActorClass = Actor->GetClass();
	USpatialNetDriver* SpatialNetDriver = CastChecked<USpatialNetDriver>(NetConnection->Driver);

	uint32 BytesWritten;
	SpatialGDK::EntityFactory EntityFactory(SpatialNetDriver, SpatialNetDriver->PackageMap, ClassInfoManager,
											SpatialNetDriver->GetRPCService());
	USpatialActorChannel* Channel =
		(USpatialActorChannel*)NetConnection->CreateChannelByName(NAME_Actor, EChannelCreateFlags::OpenedLocally);
	Channel->Actor = Actor;
	SpatialGDK::FRPCsOnEntityCreationMap EmptyMap;
	TArray<FWorkerComponentData> ComponentDatas = EntityFactory.CreateEntityComponents(Channel, EmptyMap, BytesWritten);
	if (!ComponentDatas.ContainsByPredicate([](const FWorkerComponentData& Data) {
			return Data.component_id == SpatialConstants::PERSISTENCE_COMPONENT_ID;
		}))
	{
		UE_LOG(LogTemp, Log, TEXT("Actor not persistent trying to be saved in the snapshot, skipping - Actor %s"), *Actor->GetPathName());
		return false;
	}

	// TODO: LevelComponentData
	// ComponentDatas.Add(SpatialNetDriver->Sender->CreateLevelComponentData(Actor));
	ComponentDatas.Add(ComponentPresence(EntityFactory::GetComponentPresenceList(ComponentDatas)).CreateComponentPresenceData());

	Worker_Entity Entity;
	Entity.entity_id = EntityId;

	TArray<Worker_ComponentData> Data;
	Data.Reserve(ComponentDatas.Num());
	for (auto& Component : ComponentDatas)
	{
		Data.Emplace(Worker_ComponentData(Component)); // This will probably not fly with the tracking libraries
	}

	Entity.component_count = Data.Num();
	Entity.components = Data.GetData();

	Worker_SnapshotOutputStream_WriteEntity(OutputStream, &Entity);

	return true;
}

void CleanupNetDriverAndConnection(USpatialNetDriver* NetDriver, USpatialNetConnection* NetConnection)
{
	// On clean up of the NetDriver due to garbage collection, either the ServerConnection or ClientConnections need to be not nullptr.
	// However if the ServerConnection is set on creation, using the FObjectReplicator to create the initial state of the actor,
	// the editor will crash. Therefore we set the ServerConnection after we are done using the NetDriver.
	NetDriver->ServerConnection = NetConnection;
}

bool CreateStartupActors(Worker_SnapshotOutputStream* OutputStream, UWorld* World, Worker_EntityId& CurrentEntityId)
{
	USpatialNetDriver* NetDriver = nullptr;
	USpatialNetConnection* NetConnection = nullptr;
	USpatialClassInfoManager* ClassInfoManager = nullptr;

	SetupStartupActorCreation(NetDriver, NetConnection, ClassInfoManager, World);

	for (TActorIterator<AActor> It(World); It; ++It)
	{
		AActor* Actor = *It;
		UClass* ActorClass = Actor->GetClass();

		// If Actor is critical to the level, skip
		if (ActorClass->IsChildOf<AWorldSettings>() || ActorClass->IsChildOf<ALevelScriptActor>())
		{
			continue;
		}

		if (Actor->IsEditorOnly() || !ClassInfoManager->IsSupportedClass(ActorClass->GetPathName()) || !Actor->GetIsReplicated())
		{
			continue;
		}

		CreateStartupActor(OutputStream, Actor, CurrentEntityId, NetConnection, ClassInfoManager);

		CurrentEntityId++;
	}

	// CleanupNetDriverAndConnection(NetDriver, NetConnection);

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

	Worker_EntityId NextAvailableEntityID = SpatialConstants::FIRST_AVAILABLE_ENTITY_ID;
	if (!CreateStartupActors(OutputStream, World, NextAvailableEntityID))
	{
		UE_LOG(LogSpatialGDKSnapshot, Error, TEXT("Error generating Startup Actors in snapshot: %s"),
			   UTF8_TO_TCHAR(Worker_SnapshotOutputStream_GetState(OutputStream).error_message));
		return false;
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

	return bSuccess;
}
