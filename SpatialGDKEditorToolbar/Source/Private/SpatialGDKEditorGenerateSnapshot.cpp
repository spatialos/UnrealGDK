// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "SpatialGDKEditorGenerateSnapshot.h"

#include "Engine/LevelScriptActor.h"
#include "Schema/Rotation.h"
#include "Schema/StandardLibrary.h"
#include "Schema/UnrealMetadata.h"
#include "SpatialActorChannel.h"
#include "SpatialConstants.h"
#include "SpatialGDKEditorToolbarSettings.h"
#include "SpatialNetConnection.h"
#include "SpatialNetDriver.h"
#include "SpatialTypebindingManager.h"
#include "Utils/ComponentFactory.h"
#include "Utils/EntityRegistry.h"
#include "Utils/RepDataUtils.h"
#include "Utils/RepLayoutUtils.h"
#include "Utils/SchemaUtils.h"

#include "EngineUtils.h"
#include "Runtime/Core/Public/HAL/PlatformFilemanager.h"
#include "UObjectIterator.h"

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
	ComponentWriteAcl.Add(SpatialConstants::UNREAL_METADATA_COMPONENT_ID, UnrealServerPermission);
	ComponentWriteAcl.Add(SpatialConstants::ENTITY_ACL_COMPONENT_ID, UnrealServerPermission);
	ComponentWriteAcl.Add(SpatialConstants::PLAYER_SPAWNER_COMPONENT_ID, UnrealServerPermission);

	Components.Add(improbable::Position(Origin).CreatePositionData());
	Components.Add(improbable::Metadata(TEXT("SpatialSpawner")).CreateMetadataData());
	Components.Add(improbable::Persistence().CreatePersistenceData());
	Components.Add(improbable::UnrealMetadata().CreateUnrealMetadataData());
	Components.Add(PlayerSpawnerData);
	Components.Add(improbable::EntityAcl(AnyWorkerPermission, ComponentWriteAcl).CreateEntityAclData());

	SpawnerEntity.component_count = Components.Num();
	SpawnerEntity.components = Components.GetData();

	return Worker_SnapshotOutputStream_WriteEntity(OutputStream, &SpawnerEntity) != 0;
}

Worker_ComponentData CreateGlobalStateManagerData()
{
	StringToEntityMap SingletonNameToEntityId;
	StringToEntityMap StablyNamedPathToEntityId;

	for (TObjectIterator<UClass> It; It; ++It)
	{
		// Find all singleton classes
		if (!It->HasAnySpatialClassFlags(SPATIALCLASS_Singleton))
		{
			continue;
		}

		// Ensure we don't process skeleton or reinitialized classes
		if (It->GetName().StartsWith(TEXT("SKEL_"), ESearchCase::CaseSensitive) || It->GetName().StartsWith(TEXT("REINST_"), ESearchCase::CaseSensitive))
		{
			continue;
		}

		// Id is initially 0 to indicate that this Singleton entity has not been created yet.
		// When the worker authoritative over the GSM sees 0, it knows it is safe to create it.
		SingletonNameToEntityId.Add(*It->GetPathName(), 0);
	}

	Worker_ComponentData Data;
	Data.component_id = SpatialConstants::GLOBAL_STATE_MANAGER_COMPONENT_ID;
	Data.schema_type = Schema_CreateComponentData(SpatialConstants::GLOBAL_STATE_MANAGER_COMPONENT_ID);
	Schema_Object* ComponentObject = Schema_GetComponentDataFields(Data.schema_type);

	AddStringToEntityMapToSchema(ComponentObject, 1, SingletonNameToEntityId);
	AddStringToEntityMapToSchema(ComponentObject, 2, StablyNamedPathToEntityId);

	return Data;
}

Worker_ComponentData CreateDeploymentData()
{
	// Construct the Deployment component data object.
	Worker_ComponentData DeploymentData;
	DeploymentData.component_id = SpatialConstants::GLOBAL_STATE_MANAGER_DEPLOYMENT_COMPONENT_ID;
	DeploymentData.schema_type = Schema_CreateComponentData(SpatialConstants::GLOBAL_STATE_MANAGER_DEPLOYMENT_COMPONENT_ID);
	Schema_Object* DeploymentDataObject = Schema_GetComponentDataFields(DeploymentData.schema_type);

	// Add the MapURL schema field.
	Schema_Object* MapURLObject = Schema_AddObject(DeploymentDataObject, SpatialConstants::GLOBAL_STATE_MANAGER_MAP_URL_ID);
	AddStringToSchema(MapURLObject, 1, TEXT("default")); // TODO: Fill this with the map name of the map the snapshot is being generated for.

	// Add the accepting players schema field.
	Schema_AddBool(DeploymentDataObject, SpatialConstants::GLOBAL_STATE_MANAGER_ACCEPTING_PLAYERS_ID, false);

	return DeploymentData;
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
	ComponentWriteAcl.Add(SpatialConstants::UNREAL_METADATA_COMPONENT_ID, UnrealServerPermission);
	ComponentWriteAcl.Add(SpatialConstants::ENTITY_ACL_COMPONENT_ID, UnrealServerPermission);
	ComponentWriteAcl.Add(SpatialConstants::GLOBAL_STATE_MANAGER_COMPONENT_ID, UnrealServerPermission);
	ComponentWriteAcl.Add(SpatialConstants::GLOBAL_STATE_MANAGER_DEPLOYMENT_COMPONENT_ID, UnrealServerPermission);

	Components.Add(improbable::Position(Origin).CreatePositionData());
	Components.Add(improbable::Metadata(TEXT("GlobalStateManager")).CreateMetadataData());
	Components.Add(improbable::Persistence().CreatePersistenceData());
	Components.Add(improbable::UnrealMetadata().CreateUnrealMetadataData());
	Components.Add(CreateGlobalStateManagerData());
	Components.Add(CreateDeploymentData());
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
			ComponentWriteAcl.Add(SpatialConstants::UNREAL_METADATA_COMPONENT_ID, UnrealServerPermission);
			ComponentWriteAcl.Add(SpatialConstants::ENTITY_ACL_COMPONENT_ID, UnrealServerPermission);

			Components.Add(improbable::Position(PlaceholderPosition).CreatePositionData());
			Components.Add(improbable::Metadata(TEXT("Placeholder")).CreateMetadataData());
			Components.Add(improbable::Persistence().CreatePersistenceData());
			Components.Add(improbable::UnrealMetadata().CreateUnrealMetadataData());
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

// Set up classes needed for Startup Actor creation
void SetupStartupActorCreation(USpatialNetDriver*& NetDriver, USpatialNetConnection*& NetConnection, USpatialPackageMapClient*& PackageMap, USpatialTypebindingManager*& TypebindingManager, UEntityRegistry*& EntityRegistry, UWorld* World)
{
	NetDriver = NewObject<USpatialNetDriver>();
	NetDriver->ChannelClasses[CHTYPE_Actor] = USpatialActorChannel::StaticClass();
	NetDriver->TypebindingManager = TypebindingManager;
	NetDriver->GuidCache = MakeShareable(new FSpatialNetGUIDCache(NetDriver));
	NetDriver->World = World;

	TypebindingManager = NewObject<USpatialTypebindingManager>();
	TypebindingManager->Init(NetDriver);

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

void CleanupNetDriverAndConnection(USpatialNetDriver* NetDriver, USpatialNetConnection* NetConnection)
{
	// On clean up of the NetDriver due to garbage collection, either the ServerConnection or ClientConnections need to be not nullptr.
	// However if the ServerConnection is set on creation, using the FObjectReplicator to create the initial state of the actor,
	// the editor will crash. Therefore we set the ServerConnection after we are done using the NetDriver.
	NetDriver->ServerConnection = NetConnection;
}

TArray<Worker_ComponentData> CreateStartupActorData(USpatialActorChannel* Channel, AActor* Actor, USpatialTypebindingManager* TypebindingManager, USpatialNetDriver* NetDriver)
{
	FClassInfo* Info = TypebindingManager->FindClassInfoByClass(Actor->GetClass());
	check(Info);

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
	for (int32 RPCType = SCHEMA_FirstRPC; RPCType <= SCHEMA_LastRPC; RPCType++)
	{
		if (Info->SchemaComponents[RPCType] != 0)
		{
			ComponentData.Add(ComponentFactory::CreateEmptyComponentData(Info->SchemaComponents[RPCType]));
		}
	}

	// Visit each supported subobject and create component data for initial state of each subobject
	for (auto& SubobjectInfoPair : Info->SubobjectInfo)
	{
		uint32 Offset = SubobjectInfoPair.Key;
		FClassInfo* SubobjectInfo = SubobjectInfoPair.Value.Get();

		if (UObject* Subobject = NetDriver->PackageMap->GetObjectFromUnrealObjectRef(FUnrealObjectRef(Channel->GetEntityId(), Offset)))
		{
			FRepChangeState SubobjectRepChanges = Channel->CreateInitialRepChangeState(Subobject);
			FHandoverChangeState SubobjectHandoverChanges = Channel->CreateInitialHandoverChangeState(SubobjectInfo);

			// Create component data for initial state of subobject
			ComponentData.Append(DataFactory.CreateComponentDatas(Subobject, SubobjectInfo, SubobjectRepChanges, SubobjectHandoverChanges));

			// Add subobject RPCs to entity
			for (int32 RPCType = SCHEMA_FirstRPC; RPCType <= SCHEMA_LastRPC; RPCType++)
			{
				if (SubobjectInfo->SchemaComponents[RPCType] != 0)
				{
					ComponentData.Add(ComponentFactory::CreateEmptyComponentData(SubobjectInfo->SchemaComponents[RPCType]));
				}
			}
		}
	}

	return ComponentData;
}

bool CreateStartupActor(Worker_SnapshotOutputStream* OutputStream, AActor* Actor, Worker_EntityId EntityId, USpatialNetConnection* NetConnection, USpatialTypebindingManager* TypebindingManager)
{
	Worker_Entity Entity;
	Entity.entity_id = EntityId;

	UClass* ActorClass = Actor->GetClass();

	FClassInfo* ActorInfo = TypebindingManager->FindClassInfoByClass(ActorClass);
	check(ActorInfo);

	WriteAclMap ComponentWriteAcl;

	ComponentWriteAcl.Add(SpatialConstants::POSITION_COMPONENT_ID, UnrealServerPermission);
	ComponentWriteAcl.Add(SpatialConstants::ROTATION_COMPONENT_ID, UnrealServerPermission);
	ComponentWriteAcl.Add(SpatialConstants::ENTITY_ACL_COMPONENT_ID, UnrealServerPermission);

	ForAllSchemaComponentTypes([&](ESchemaComponentType Type) {
		Worker_ComponentId ComponentId = ActorInfo->SchemaComponents[Type];

		if (ComponentId == SpatialConstants::INVALID_COMPONENT_ID)
		{
			return;
		}

		if (Type == SCHEMA_ClientRPC)
		{
			// No write attribute for RPC_Client since a Startup Actor will have no owner on level start
			return;
		}

		ComponentWriteAcl.Add(ComponentId, UnrealServerPermission);
	});


	for (auto& SubobjectInfoPair : ActorInfo->SubobjectInfo)
	{
		FClassInfo& SubobjectInfo = *SubobjectInfoPair.Value;

		ForAllSchemaComponentTypes([&](ESchemaComponentType Type)
		{
			Worker_ComponentId ComponentId = SubobjectInfo.SchemaComponents[Type];
			if (ComponentId == SpatialConstants::INVALID_COMPONENT_ID)
			{
				return;
			}

			if (Type == SCHEMA_ClientRPC)
			{
				// No write attribute for RPC_Client since a Startup Actor will have no owner on level start
				return;
			}

			ComponentWriteAcl.Add(ComponentId, UnrealServerPermission);
		});
	}

	USpatialActorChannel* Channel = Cast<USpatialActorChannel>(NetConnection->CreateChannel(CHTYPE_Actor, 1));
	Channel->SetEntityId(EntityId);

	FString StaticPath = Actor->GetPathName(nullptr);

	TArray<Worker_ComponentData> Components;
	Components.Add(improbable::Position(improbable::Coordinates::FromFVector(Channel->GetActorSpatialPosition(Actor))).CreatePositionData());
	Components.Add(improbable::Metadata(ActorClass->GetName()).CreateMetadataData());
	Components.Add(improbable::EntityAcl(AnyWorkerPermission, ComponentWriteAcl).CreateEntityAclData());
	Components.Add(improbable::Persistence().CreatePersistenceData());
	Components.Add(improbable::Rotation(Actor->GetActorRotation()).CreateRotationData());
	Components.Add(improbable::UnrealMetadata(StaticPath, {}, ActorClass->GetPathName()).CreateUnrealMetadataData());

	Components.Append(CreateStartupActorData(Channel, Actor, TypebindingManager, Cast<USpatialNetDriver>(NetConnection->Driver)));

	Entity.component_count = Components.Num();
	Entity.components = Components.GetData();

	return Worker_SnapshotOutputStream_WriteEntity(OutputStream, &Entity) != 0;
}

bool ProcessSupportedActors(UWorld* World, USpatialTypebindingManager* TypebindingManager, TFunction<bool(AActor*, Worker_EntityId)> Process)
{
	Worker_EntityId CurrentEntityId = SpatialConstants::PLACEHOLDER_ENTITY_ID_LAST + 1;

	for (TActorIterator<AActor> It(World); It; ++It)
	{
		AActor* Actor = *It;
		UClass* ActorClass = Actor->GetClass();

		// If Actor is critical to the level, skip
		if (ActorClass->IsChildOf<AWorldSettings>() || ActorClass->IsChildOf<ALevelScriptActor>())
		{
			continue;
		}

		if (Actor->IsEditorOnly() || !TypebindingManager->IsSupportedClass(ActorClass) || !Actor->GetIsReplicated())
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

bool CreateStartupActors(Worker_SnapshotOutputStream* OutputStream, UWorld* World)
{
	USpatialNetDriver* NetDriver = nullptr;
	USpatialNetConnection* NetConnection = nullptr;
	USpatialPackageMapClient* PackageMap = nullptr;
	USpatialTypebindingManager* TypebindingManager = nullptr;
	UEntityRegistry* EntityRegistry = nullptr;

	SetupStartupActorCreation(NetDriver, NetConnection, PackageMap, TypebindingManager, EntityRegistry, World);

	bool bSuccess = true;

	// Need to add all actors in the world to the package map so they have assigned UnrealObjRefs for the ComponentFactory to use
	bSuccess &= ProcessSupportedActors(World, TypebindingManager, [&PackageMap, &EntityRegistry, &TypebindingManager](AActor* Actor, Worker_EntityId EntityId)
	{
		EntityRegistry->AddToRegistry(EntityId, Actor);
		FClassInfo* Info = TypebindingManager->FindClassInfoByClass(Actor->GetClass());
		PackageMap->ResolveEntityActor(Actor, EntityId, improbable::CreateOffsetMapFromActor(Actor, Info));
		return true;
	});

	bSuccess &= ProcessSupportedActors(World, TypebindingManager, [&NetConnection, &OutputStream, &TypebindingManager](AActor* Actor, Worker_EntityId EntityId)
	{
		return CreateStartupActor(OutputStream, Actor, EntityId, NetConnection, TypebindingManager);
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

bool SpatialGDKGenerateSnapshot(UWorld* World)
{
	const USpatialGDKEditorToolbarSettings* Settings = GetDefault<USpatialGDKEditorToolbarSettings>();
	FString SavePath = FPaths::Combine(Settings->GetSpatialOSSnapshotPath(), Settings->GetSpatialOSSnapshotFile());
	if (!ValidateAndCreateSnapshotGenerationPath(SavePath))
	{
		return false;
	}

	UE_LOG(LogSpatialGDKSnapshot, Display, TEXT("Saving snapshot to: %s"), *SavePath);

	Worker_ComponentVtable DefaultVtable{};
	Worker_SnapshotParameters Parameters{};
	Parameters.default_component_vtable = &DefaultVtable;
	Worker_SnapshotOutputStream* OutputStream = Worker_SnapshotOutputStream_Create(TCHAR_TO_UTF8(*SavePath), &Parameters);

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

	if (!CreatePlaceholders(OutputStream))
	{
		UE_LOG(LogSpatialGDKSnapshot, Error, TEXT("Error generating Placeholders in snapshot: %s"), UTF8_TO_TCHAR(Worker_SnapshotOutputStream_GetError(OutputStream)));
		return false;
	}

	if (!CreateStartupActors(OutputStream, World))
	{
		UE_LOG(LogSpatialGDKSnapshot, Error, TEXT("Error generating Startup Actors in snapshot: %s"), UTF8_TO_TCHAR(Worker_SnapshotOutputStream_GetError(OutputStream)));
		return false;
	}

	Worker_SnapshotOutputStream_Destroy(OutputStream);

	return true;
}
