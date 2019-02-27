// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "SpatialGDKEditorSnapshotGenerator.h"

#include "Engine/LevelScriptActor.h"
#include "Schema/Interest.h"
#include "Schema/StandardLibrary.h"
#include "Schema/SpawnData.h"
#include "Schema/UnrealMetadata.h"
#include "EngineClasses/SpatialActorChannel.h"
#include "EngineClasses/SpatialNetConnection.h"
#include "EngineClasses/SpatialNetDriver.h"
#include "Interop/SpatialClassInfoManager.h"
#include "SpatialConstants.h"
#include "SpatialGDKEditorSettings.h"
#include "Utils/ComponentFactory.h"
#include "Utils/EntityRegistry.h"
#include "Utils/RepDataUtils.h"
#include "Utils/RepLayoutUtils.h"
#include "Utils/SchemaUtils.h"

#include "EngineUtils.h"
#include "HAL/PlatformFile.h"
#include "HAL/PlatformFilemanager.h"
#include "UObject/UObjectIterator.h"

#include <WorkerSDK/improbable/c_worker.h>
#include <WorkerSDK/improbable/c_schema.h>

using namespace improbable;

DEFINE_LOG_CATEGORY(LogSpatialGDKSnapshot);

const WorkerAttributeSet UnrealServerAttributeSet{ TArray<FString>{SpatialConstants::ServerWorkerType} };
const WorkerAttributeSet UnrealClientAttributeSet{ TArray<FString>{SpatialConstants::ClientWorkerType} };

const WorkerRequirementSet UnrealServerPermission{ { UnrealServerAttributeSet } };
const WorkerRequirementSet UnrealClientPermission{ {UnrealClientAttributeSet} };
const WorkerRequirementSet AnyWorkerPermission{ {UnrealClientAttributeSet, UnrealServerAttributeSet } };

const improbable::Coordinates Origin{ 0, 0, 0 };

bool CreateSpawnerEntity(Worker_SnapshotOutputStream* OutputStream)
{
	Worker_Entity SpawnerEntity;
	SpawnerEntity.entity_id = SpatialConstants::INITIAL_SPAWNER_ENTITY_ID;

	Worker_ComponentData PlayerSpawnerData = {};
	PlayerSpawnerData.component_id = SpatialConstants::PLAYER_SPAWNER_COMPONENT_ID;
	PlayerSpawnerData.schema_type = Schema_CreateComponentData(SpatialConstants::PLAYER_SPAWNER_COMPONENT_ID);

	TArray<Worker_ComponentData> Components;

	WriteAclMap ComponentWriteAcl;
	ComponentWriteAcl.Add(SpatialConstants::POSITION_COMPONENT_ID, UnrealServerPermission);
	ComponentWriteAcl.Add(SpatialConstants::METADATA_COMPONENT_ID, UnrealServerPermission);
	ComponentWriteAcl.Add(SpatialConstants::PERSISTENCE_COMPONENT_ID, UnrealServerPermission);
	ComponentWriteAcl.Add(SpatialConstants::ENTITY_ACL_COMPONENT_ID, UnrealServerPermission);
	ComponentWriteAcl.Add(SpatialConstants::PLAYER_SPAWNER_COMPONENT_ID, UnrealServerPermission);

	Components.Add(improbable::Position(Origin).CreatePositionData());
	Components.Add(improbable::Metadata(TEXT("SpatialSpawner")).CreateMetadataData());
	Components.Add(improbable::Persistence().CreatePersistenceData());
	Components.Add(improbable::EntityAcl(AnyWorkerPermission, ComponentWriteAcl).CreateEntityAclData());
	Components.Add(PlayerSpawnerData);

	SpawnerEntity.component_count = Components.Num();
	SpawnerEntity.components = Components.GetData();

	return Worker_SnapshotOutputStream_WriteEntity(OutputStream, &SpawnerEntity) != 0;
}

Worker_ComponentData CreateSingletonManagerData()
{
	StringToEntityMap SingletonNameToEntityId;

	Worker_ComponentData Data{};
	Data.component_id = SpatialConstants::SINGLETON_MANAGER_COMPONENT_ID;
	Data.schema_type = Schema_CreateComponentData(SpatialConstants::SINGLETON_MANAGER_COMPONENT_ID);
	Schema_Object* ComponentObject = Schema_GetComponentDataFields(Data.schema_type);

	AddStringToEntityMapToSchema(ComponentObject, 1, SingletonNameToEntityId);

	return Data;
}

Worker_ComponentData CreateDeploymentData()
{
	Worker_ComponentData DeploymentData{};
	DeploymentData.component_id = SpatialConstants::DEPLOYMENT_MAP_COMPONENT_ID;
	DeploymentData.schema_type = Schema_CreateComponentData(SpatialConstants::DEPLOYMENT_MAP_COMPONENT_ID);
	Schema_Object* DeploymentDataObject = Schema_GetComponentDataFields(DeploymentData.schema_type);

	Schema_Object* MapURLObject = Schema_AddObject(DeploymentDataObject, SpatialConstants::DEPLOYMENT_MAP_MAP_URL_ID);
	AddStringToSchema(MapURLObject, 1, TEXT("default")); // TODO: Fill this with the map name of the map the snapshot is being generated for.

	Schema_AddBool(DeploymentDataObject, SpatialConstants::DEPLOYMENT_MAP_ACCEPTING_PLAYERS_ID, false);

	return DeploymentData;
}

Worker_ComponentData CreateGSMShutdownData()
{
	Worker_ComponentData GSMShutdownData;
	GSMShutdownData.component_id = SpatialConstants::GSM_SHUTDOWN_COMPONENT_ID;
	GSMShutdownData.schema_type = Schema_CreateComponentData(SpatialConstants::GSM_SHUTDOWN_COMPONENT_ID);
	return GSMShutdownData;
}

Worker_ComponentData CreateStartupActorManagerData()
{
	Worker_ComponentData StartupActorManagerData{};
	StartupActorManagerData.component_id = SpatialConstants::STARTUP_ACTOR_MANAGER_COMPONENT_ID;
	StartupActorManagerData.schema_type = Schema_CreateComponentData(SpatialConstants::STARTUP_ACTOR_MANAGER_COMPONENT_ID);
	Schema_Object* StartupActorManagerObject = Schema_GetComponentDataFields(StartupActorManagerData.schema_type);

	Schema_AddBool(StartupActorManagerObject, SpatialConstants::STARTUP_ACTOR_MANAGER_CAN_BEGIN_PLAY_ID, false);

	return StartupActorManagerData;
}

bool CreateGlobalStateManager(Worker_SnapshotOutputStream* OutputStream)
{
	Worker_Entity GSM;
	GSM.entity_id = SpatialConstants::INITIAL_GLOBAL_STATE_MANAGER_ENTITY_ID;

	TArray<Worker_ComponentData> Components;

	WriteAclMap ComponentWriteAcl;
	ComponentWriteAcl.Add(SpatialConstants::POSITION_COMPONENT_ID, UnrealServerPermission);
	ComponentWriteAcl.Add(SpatialConstants::METADATA_COMPONENT_ID, UnrealServerPermission);
	ComponentWriteAcl.Add(SpatialConstants::PERSISTENCE_COMPONENT_ID, UnrealServerPermission);
	ComponentWriteAcl.Add(SpatialConstants::ENTITY_ACL_COMPONENT_ID, UnrealServerPermission);
	ComponentWriteAcl.Add(SpatialConstants::SINGLETON_MANAGER_COMPONENT_ID, UnrealServerPermission);
	ComponentWriteAcl.Add(SpatialConstants::DEPLOYMENT_MAP_COMPONENT_ID, UnrealServerPermission);
	ComponentWriteAcl.Add(SpatialConstants::GSM_SHUTDOWN_COMPONENT_ID, UnrealServerPermission);
	ComponentWriteAcl.Add(SpatialConstants::STARTUP_ACTOR_MANAGER_COMPONENT_ID, UnrealServerPermission);

	Components.Add(improbable::Position(Origin).CreatePositionData());
	Components.Add(improbable::Metadata(TEXT("GlobalStateManager")).CreateMetadataData());
	Components.Add(improbable::Persistence().CreatePersistenceData());
	Components.Add(CreateSingletonManagerData());
	Components.Add(CreateDeploymentData());
	Components.Add(CreateGSMShutdownData());
	Components.Add(CreateStartupActorManagerData());

	Components.Add(improbable::EntityAcl(UnrealServerPermission, ComponentWriteAcl).CreateEntityAclData());

	GSM.component_count = Components.Num();
	GSM.components = Components.GetData();

	return Worker_SnapshotOutputStream_WriteEntity(OutputStream, &GSM) != 0;
}

bool CreatePlaceholders(Worker_SnapshotOutputStream* OutputStream)
{
	// Set up grid of "placeholder" entities to allow workers to be authoritative over _something_.
	int PlaceholderCount = SpatialConstants::PLACEHOLDER_ENTITY_ID_LAST - SpatialConstants::PLACEHOLDER_ENTITY_ID_FIRST + 1;
	int PlaceholderCountAxis = static_cast<int>(sqrt(PlaceholderCount));
	checkf(PlaceholderCountAxis * PlaceholderCountAxis == PlaceholderCount, TEXT("The number of placeholders must be a square number."));
	checkf(PlaceholderCountAxis % 2 == 0, TEXT("The number of placeholders on each axis must be even."));
	const float CHUNK_SIZE = 5.0f; // in SpatialOS coordinates.
	int PlaceholderEntityIdCounter = SpatialConstants::PLACEHOLDER_ENTITY_ID_FIRST;
	for (int x = -PlaceholderCountAxis / 2; x < PlaceholderCountAxis / 2; x++)
	{
		for (int y = -PlaceholderCountAxis / 2; y < PlaceholderCountAxis / 2; y++)
		{
			const improbable::Coordinates PlaceholderPosition{ x * CHUNK_SIZE + CHUNK_SIZE * 0.5f, 0, y * CHUNK_SIZE + CHUNK_SIZE * 0.5f };

			Worker_Entity Placeholder;
			Placeholder.entity_id = PlaceholderEntityIdCounter;

			TArray<Worker_ComponentData> Components;

			WriteAclMap ComponentWriteAcl;
			ComponentWriteAcl.Add(SpatialConstants::POSITION_COMPONENT_ID, UnrealServerPermission);
			ComponentWriteAcl.Add(SpatialConstants::METADATA_COMPONENT_ID, UnrealServerPermission);
			ComponentWriteAcl.Add(SpatialConstants::PERSISTENCE_COMPONENT_ID, UnrealServerPermission);
			ComponentWriteAcl.Add(SpatialConstants::ENTITY_ACL_COMPONENT_ID, UnrealServerPermission);

			Components.Add(improbable::Position(PlaceholderPosition).CreatePositionData());
			Components.Add(improbable::Metadata(TEXT("Placeholder")).CreateMetadataData());
			Components.Add(improbable::Persistence().CreatePersistenceData());
			Components.Add(improbable::EntityAcl(UnrealServerPermission, ComponentWriteAcl).CreateEntityAclData());

			Placeholder.component_count = Components.Num();
			Placeholder.components = Components.GetData();

			if (Worker_SnapshotOutputStream_WriteEntity(OutputStream, &Placeholder) == 0)
			{
				return false;
			}

			PlaceholderEntityIdCounter++;
		}
	}
	// Sanity check.
	check(PlaceholderEntityIdCounter == SpatialConstants::PLACEHOLDER_ENTITY_ID_LAST + 1);

	return true;
}

// This function is not in use.
// Set up classes needed for Startup Actor creation
void SetupStartupActorCreation(USpatialNetDriver*& NetDriver, USpatialNetConnection*& NetConnection, USpatialPackageMapClient*& PackageMap, USpatialClassInfoManager*& ClassInfoManager, UEntityRegistry*& EntityRegistry, UWorld* World)
{
	NetDriver = NewObject<USpatialNetDriver>();
	NetDriver->ChannelClasses[CHTYPE_Actor] = USpatialActorChannel::StaticClass();
	NetDriver->GuidCache = MakeShareable(new FSpatialNetGUIDCache(NetDriver));
	NetDriver->World = World;

	ClassInfoManager = NewObject<USpatialClassInfoManager>();
	ClassInfoManager->Init(NetDriver);

	NetDriver->ClassInfoManager = ClassInfoManager;

	EntityRegistry = NewObject<UEntityRegistry>();
	NetDriver->EntityRegistry = EntityRegistry;

	NetConnection = NewObject<USpatialNetConnection>();
	NetConnection->Driver = NetDriver;
	NetConnection->State = USOCK_Closed;

	PackageMap = NewObject<USpatialPackageMapClient>();
	PackageMap->Initialize(NetConnection, NetDriver->GuidCache);

	NetConnection->PackageMap = PackageMap;
	NetDriver->PackageMap = PackageMap;
}

// This function is not in use.
void CleanupNetDriverAndConnection(USpatialNetDriver* NetDriver, USpatialNetConnection* NetConnection)
{
	// On clean up of the NetDriver due to garbage collection, either the ServerConnection or ClientConnections need to be not nullptr.
	// However if the ServerConnection is set on creation, using the FObjectReplicator to create the initial state of the actor,
	// the editor will crash. Therefore we set the ServerConnection after we are done using the NetDriver.
	NetDriver->ServerConnection = NetConnection;
}

// This function is not in use.
TArray<Worker_ComponentData> CreateStartupActorData(USpatialActorChannel* Channel, AActor* Actor, USpatialClassInfoManager* ClassInfoManager, USpatialNetDriver* NetDriver)
{
	const FClassInfo& Info = ClassInfoManager->GetOrCreateClassInfoByClass(Actor->GetClass());

	// This ensures that the Actor has prepared it's replicated fields before replicating. For instance, the simulate physics on a UPrimitiveComponent
	// will be queried and set the Actor's ReplicatedMovement.bRepPhysics field. These fields are then serialized correctly within the snapshot. We are
	// modifying the editor's instance of the actor here, but in theory those fields should be inferred or transient anyway, so it shouldn't be a problem.
	Actor->CallPreReplication(NetDriver);

	FRepChangeState InitialRepChanges = Channel->CreateInitialRepChangeState(Actor);
	FHandoverChangeState InitialHandoverChanges = Channel->CreateInitialHandoverChangeState(Info);

	// Created just to satisfy the ComponentFactory constructor
	FUnresolvedObjectsMap UnresolvedObjectsMap;
	FUnresolvedObjectsMap HandoverUnresolvedObjectsMap;
	ComponentFactory DataFactory(UnresolvedObjectsMap, HandoverUnresolvedObjectsMap, NetDriver);

	// Create component data from initial state of Actor (which is the state the Actor is in before running the level)
	TArray<Worker_ComponentData> ComponentData = DataFactory.CreateComponentDatas(Actor, Info, InitialRepChanges, InitialHandoverChanges);

	// Add Actor RPCs to entity
	ComponentData.Add(ComponentFactory::CreateEmptyComponentData(SpatialConstants::CLIENT_RPC_ENDPOINT_COMPONENT_ID));
	ComponentData.Add(ComponentFactory::CreateEmptyComponentData(SpatialConstants::SERVER_RPC_ENDPOINT_COMPONENT_ID));
	ComponentData.Add(ComponentFactory::CreateEmptyComponentData(SpatialConstants::NETMULTICAST_RPCS_COMPONENT_ID));

	// Visit each supported subobject and create component data for initial state of each subobject
	for (auto& SubobjectInfoPair : Info.SubobjectInfo)
	{
		uint32 Offset = SubobjectInfoPair.Key;
		FClassInfo& SubobjectInfo = SubobjectInfoPair.Value.Get();

		TWeakObjectPtr<UObject> Subobject = NetDriver->PackageMap->GetObjectFromUnrealObjectRef(FUnrealObjectRef(Channel->GetEntityId(), Offset));
		if (Subobject.IsValid())
		{
			FRepChangeState SubobjectRepChanges = Channel->CreateInitialRepChangeState(Subobject);
			FHandoverChangeState SubobjectHandoverChanges = Channel->CreateInitialHandoverChangeState(SubobjectInfo);

			// Create component data for initial state of subobject
			ComponentData.Append(DataFactory.CreateComponentDatas(Subobject.Get(), SubobjectInfo, SubobjectRepChanges, SubobjectHandoverChanges));
		}
	}

	return ComponentData;
}

// This function is not in use.
bool CreateStartupActor(Worker_SnapshotOutputStream* OutputStream, AActor* Actor, Worker_EntityId EntityId, USpatialNetConnection* NetConnection, USpatialClassInfoManager* ClassInfoManager)
{
	Worker_Entity Entity;
	Entity.entity_id = EntityId;

	UClass* ActorClass = Actor->GetClass();

	const FClassInfo& ActorInfo = ClassInfoManager->GetOrCreateClassInfoByClass(ActorClass);

	WriteAclMap ComponentWriteAcl;

	ComponentWriteAcl.Add(SpatialConstants::POSITION_COMPONENT_ID, UnrealServerPermission);
	ComponentWriteAcl.Add(SpatialConstants::INTEREST_COMPONENT_ID, UnrealServerPermission);
	ComponentWriteAcl.Add(SpatialConstants::SPAWN_DATA_COMPONENT_ID, UnrealServerPermission);
	ComponentWriteAcl.Add(SpatialConstants::ENTITY_ACL_COMPONENT_ID, UnrealServerPermission);

	ForAllSchemaComponentTypes([&](ESchemaComponentType Type)
	{
		Worker_ComponentId ComponentId = ActorInfo.SchemaComponents[Type];

		if (ComponentId == SpatialConstants::INVALID_COMPONENT_ID)
		{
			return;
		}

		if (Type == SCHEMA_ClientReliableRPC)
		{
			// No write attribute for RPC_Client since a Startup Actor will have no owner on level start
			return;
		}

		ComponentWriteAcl.Add(ComponentId, UnrealServerPermission);
	});


	for (auto& SubobjectInfoPair : ActorInfo.SubobjectInfo)
	{
		FClassInfo& SubobjectInfo = SubobjectInfoPair.Value.Get();

		// Static subobjects aren't guaranteed to exist on actor instances, check they are present before adding write acls
		UObject* Subobject = Actor->GetDefaultSubobjectByName(SubobjectInfo.SubobjectName);
		if (Subobject == nullptr || Subobject->IsPendingKill())
		{
			continue;
		}

		ForAllSchemaComponentTypes([&](ESchemaComponentType Type)
		{
			Worker_ComponentId ComponentId = SubobjectInfo.SchemaComponents[Type];
			if (ComponentId == SpatialConstants::INVALID_COMPONENT_ID)
			{
				return;
			}

			if (Type == SCHEMA_ClientReliableRPC)
			{
				// No write attribute for RPC_Client since a Startup Actor will have no owner on level start
				return;
			}

			ComponentWriteAcl.Add(ComponentId, UnrealServerPermission);
		});
	}

	USpatialActorChannel* Channel = Cast<USpatialActorChannel>(NetConnection->CreateChannel(CHTYPE_Actor, 1));
	Channel->SetEntityId(EntityId);

	TArray<Worker_ComponentData> Components;
	Components.Add(improbable::Position(improbable::Coordinates::FromFVector(Channel->GetActorSpatialPosition(Actor))).CreatePositionData());
	Components.Add(improbable::Metadata(ActorClass->GetName()).CreateMetadataData());
	Components.Add(improbable::EntityAcl(AnyWorkerPermission, ComponentWriteAcl).CreateEntityAclData());
	Components.Add(improbable::Persistence().CreatePersistenceData());
	Components.Add(improbable::SpawnData(Actor).CreateSpawnDataData());
	Components.Add(improbable::UnrealMetadata({}, {}, ActorClass->GetPathName()).CreateUnrealMetadataData());
	Components.Add(improbable::Interest().CreateInterestData());

	Components.Append(CreateStartupActorData(Channel, Actor, ClassInfoManager, Cast<USpatialNetDriver>(NetConnection->Driver)));

	Entity.component_count = Components.Num();
	Entity.components = Components.GetData();

	return Worker_SnapshotOutputStream_WriteEntity(OutputStream, &Entity) != 0;
}

// This function is not in use.
bool ProcessSupportedActors(const TSet<AActor*>& Actors, USpatialClassInfoManager* ClassInfoManager, TFunction<bool(AActor*, Worker_EntityId)> Process)
{
	Worker_EntityId CurrentEntityId = SpatialConstants::PLACEHOLDER_ENTITY_ID_LAST + 1;

	for (AActor* Actor : Actors)
	{
		UClass* ActorClass = Actor->GetClass();

		// If Actor is critical to the level, skip
		if (ActorClass->IsChildOf<AWorldSettings>())
		{
			continue;
		}

		if (Actor->IsEditorOnly() || Actor->IsPendingKill() || !ClassInfoManager->IsSupportedClass(ActorClass) || !Actor->GetIsReplicated())
		{
			continue;
		}

		if (!Process(Actor, CurrentEntityId))
		{
			return false;
		}

		CurrentEntityId++;
	}

	return true;
}

// This function is not in use.
bool CreateStartupActors(Worker_SnapshotOutputStream* OutputStream, UWorld* World)
{
	USpatialNetDriver* NetDriver = nullptr;
	USpatialNetConnection* NetConnection = nullptr;
	USpatialPackageMapClient* PackageMap = nullptr;
	USpatialClassInfoManager* ClassInfoManager = nullptr;
	UEntityRegistry* EntityRegistry = nullptr;

	SetupStartupActorCreation(NetDriver, NetConnection, PackageMap, ClassInfoManager, EntityRegistry, World);

	// Create set of world actors (World actor iterator returns same actor multiple times in some circumstances)
	TSet<AActor*> WorldActors;
	for (TActorIterator<AActor> It(World); It; ++It)
	{
		WorldActors.Add(*It);
	}

	bool bSuccess = true;

	// Need to add all actors in the world to the package map so they have assigned UnrealObjRefs for the ComponentFactory to use
	bSuccess &= ProcessSupportedActors(WorldActors, ClassInfoManager, [&PackageMap, &EntityRegistry, &ClassInfoManager](AActor* Actor, Worker_EntityId EntityId)
	{
		EntityRegistry->AddToRegistry(EntityId, Actor);
		PackageMap->ResolveEntityActor(Actor, EntityId);
		return true;
	});

	bSuccess &= ProcessSupportedActors(WorldActors, ClassInfoManager, [&NetConnection, &OutputStream, &ClassInfoManager](AActor* Actor, Worker_EntityId EntityId)
	{
		return CreateStartupActor(OutputStream, Actor, EntityId, NetConnection, ClassInfoManager);
	});

	CleanupNetDriverAndConnection(NetDriver, NetConnection);

	return bSuccess;
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

bool FillSnapshot(Worker_SnapshotOutputStream* OutputStream, UWorld* World)
{
	if (!CreateSpawnerEntity(OutputStream))
	{
		UE_LOG(LogSpatialGDKSnapshot, Error, TEXT("Error generating Spawner in snapshot: %s"), UTF8_TO_TCHAR(Worker_SnapshotOutputStream_GetError(OutputStream)));
		return false;
	}

	if (!CreateGlobalStateManager(OutputStream))
	{
		UE_LOG(LogSpatialGDKSnapshot, Error, TEXT("Error generating GlobalStateManager in snapshot: %s"), UTF8_TO_TCHAR(Worker_SnapshotOutputStream_GetError(OutputStream)));
		return false;
	}

	const USpatialGDKEditorSettings* SpatialGDKSettings = GetDefault<USpatialGDKEditorSettings>();
	if (SpatialGDKSettings->bGeneratePlaceholderEntitiesInSnapshot)
	{
		if (!CreatePlaceholders(OutputStream))
		{
			UE_LOG(LogSpatialGDKSnapshot, Error, TEXT("Error generating Placeholders in snapshot: %s"), UTF8_TO_TCHAR(Worker_SnapshotOutputStream_GetError(OutputStream)));
			return false;
		}
	}

	return true;
}

bool SpatialGDKGenerateSnapshot(UWorld* World, FString SnapshotFilename)
{
	const USpatialGDKEditorSettings* Settings = GetDefault<USpatialGDKEditorSettings>();
	FString SavePath = FPaths::Combine(Settings->GetSpatialOSSnapshotFolderPath(), SnapshotFilename);
	if (!ValidateAndCreateSnapshotGenerationPath(SavePath))
	{
		return false;
	}

	UE_LOG(LogSpatialGDKSnapshot, Display, TEXT("Saving snapshot to: %s"), *SavePath);

	Worker_ComponentVtable DefaultVtable{};
	Worker_SnapshotParameters Parameters{};
	Parameters.default_component_vtable = &DefaultVtable;

	bool bSuccess = true;
	Worker_SnapshotOutputStream* OutputStream = Worker_SnapshotOutputStream_Create(TCHAR_TO_UTF8(*SavePath), &Parameters);
	if (const char* SchemaError = Worker_SnapshotOutputStream_GetError(OutputStream))
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
