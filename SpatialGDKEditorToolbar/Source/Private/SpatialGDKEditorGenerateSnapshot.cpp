// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "SpatialGDKEditorGenerateSnapshot.h"

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
#include "Utils/RepDataUtils.h"
#include "Utils/RepLayoutUtils.h"
#include "Utils/SchemaUtils.h"

#include "EngineUtils.h"
#include "Runtime/Core/Public/HAL/PlatformFilemanager.h"
#include "UObjectIterator.h"

#include <improbable/c_worker.h>
#include <improbable/c_schema.h>

DEFINE_LOG_CATEGORY(LogSpatialGDKSnapshot);

const WorkerAttributeSet UnrealWorkerAttributeSet{ TArray<FString>{TEXT("UnrealWorker")} };
const WorkerAttributeSet UnrealClientAttributeSet{ TArray<FString>{TEXT("UnrealClient")} };

const WorkerRequirementSet UnrealWorkerPermission{ {UnrealWorkerAttributeSet} };
const WorkerRequirementSet UnrealClientPermission{ {UnrealClientAttributeSet} };
const WorkerRequirementSet AnyWorkerPermission{ {UnrealClientAttributeSet, UnrealWorkerAttributeSet} };

const improbable::Coordinates Origin{ 0, 0, 0 };

bool CreateSpawnerEntity(Worker_SnapshotOutputStream* OutputStream)
{
	Worker_Entity SpawnerEntity;
	SpawnerEntity.entity_id = SpatialConstants::SPAWNER_ENTITY_ID;

	Worker_ComponentData PlayerSpawnerData = {};
	PlayerSpawnerData.component_id = SpatialConstants::PLAYER_SPAWNER_COMPONENT_ID;
	PlayerSpawnerData.schema_type = Schema_CreateComponentData(SpatialConstants::PLAYER_SPAWNER_COMPONENT_ID);

	TArray<Worker_ComponentData> Components;

	WriteAclMap ComponentWriteAcl;
	ComponentWriteAcl.Add(POSITION_COMPONENT_ID, UnrealWorkerPermission);
	ComponentWriteAcl.Add(METADATA_COMPONENT_ID, UnrealWorkerPermission);
	ComponentWriteAcl.Add(PERSISTENCE_COMPONENT_ID, UnrealWorkerPermission);
	ComponentWriteAcl.Add(UNREAL_METADATA_COMPONENT_ID, UnrealWorkerPermission);
	ComponentWriteAcl.Add(ENTITY_ACL_COMPONENT_ID, UnrealWorkerPermission);
	ComponentWriteAcl.Add(SpatialConstants::PLAYER_SPAWNER_COMPONENT_ID, UnrealWorkerPermission);

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

bool CreateGlobalStateManager(Worker_SnapshotOutputStream* OutputStream)
{
	Worker_Entity GSM;
	GSM.entity_id = SpatialConstants::GLOBAL_STATE_MANAGER;

	TArray<Worker_ComponentData> Components;

	WriteAclMap ComponentWriteAcl;
	ComponentWriteAcl.Add(POSITION_COMPONENT_ID, UnrealWorkerPermission);
	ComponentWriteAcl.Add(METADATA_COMPONENT_ID, UnrealWorkerPermission);
	ComponentWriteAcl.Add(PERSISTENCE_COMPONENT_ID, UnrealWorkerPermission);
	ComponentWriteAcl.Add(UNREAL_METADATA_COMPONENT_ID, UnrealWorkerPermission);
	ComponentWriteAcl.Add(ENTITY_ACL_COMPONENT_ID, UnrealWorkerPermission);
	ComponentWriteAcl.Add(SpatialConstants::GLOBAL_STATE_MANAGER_COMPONENT_ID, UnrealWorkerPermission);

	Components.Add(improbable::Position(Origin).CreatePositionData());
	Components.Add(improbable::Metadata(TEXT("GlobalStateManager")).CreateMetadataData());
	Components.Add(improbable::Persistence().CreatePersistenceData());
	Components.Add(improbable::UnrealMetadata().CreateUnrealMetadataData());
	Components.Add(CreateGlobalStateManagerData());
	Components.Add(improbable::EntityAcl(UnrealWorkerPermission, ComponentWriteAcl).CreateEntityAclData());

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
			ComponentWriteAcl.Add(POSITION_COMPONENT_ID, UnrealWorkerPermission);
			ComponentWriteAcl.Add(METADATA_COMPONENT_ID, UnrealWorkerPermission);
			ComponentWriteAcl.Add(PERSISTENCE_COMPONENT_ID, UnrealWorkerPermission);
			ComponentWriteAcl.Add(UNREAL_METADATA_COMPONENT_ID, UnrealWorkerPermission);
			ComponentWriteAcl.Add(ENTITY_ACL_COMPONENT_ID, UnrealWorkerPermission);

			Components.Add(improbable::Position(PlaceholderPosition).CreatePositionData());
			Components.Add(improbable::Metadata(TEXT("Placeholder")).CreateMetadataData());
			Components.Add(improbable::Persistence().CreatePersistenceData());
			Components.Add(improbable::UnrealMetadata().CreateUnrealMetadataData());
			Components.Add(improbable::EntityAcl(UnrealWorkerPermission, ComponentWriteAcl).CreateEntityAclData());

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
void SetupStartupActorCreation(USpatialNetDriver*& NetDriver, USpatialNetConnection*& NetConnection, USpatialTypebindingManager*& TypebindingManager)
{
	TypebindingManager = NewObject<USpatialTypebindingManager>();
	TypebindingManager->Init();

	NetDriver = NewObject<USpatialNetDriver>();
	NetDriver->ChannelClasses[CHTYPE_Actor] = USpatialActorChannel::StaticClass();
	NetDriver->TypebindingManager = TypebindingManager;

	NetConnection = NewObject<USpatialNetConnection>();
	NetConnection->Driver = NetDriver;
	NetConnection->State = USOCK_Closed;
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

	FRepChangeState InitialRepChanges = Channel->CreateInitialRepChangeState(Actor);
	FHandoverChangeState InitialHandoverChanges = Channel->CreateInitialHandoverChangeState(Info);

	// Created just to satisfy the ComponentFactory constructor
	FUnresolvedObjectsMap UnresolvedObjectsMap;
	FUnresolvedObjectsMap HandoverUnresolvedObjectsMap;
	ComponentFactory DataFactory(UnresolvedObjectsMap, HandoverUnresolvedObjectsMap, NetDriver);

	// Create component data from initial state of Actor (which is the state the Actor is in before running the level)
	TArray<Worker_ComponentData> ComponentData = DataFactory.CreateComponentDatas(Actor, InitialRepChanges, InitialHandoverChanges);

	// Add Actor RPCs to entity
	for (int RPCType = 0; RPCType < RPC_Count; RPCType++)
	{
		ComponentData.Add(ComponentFactory::CreateEmptyComponentData(Info->RPCComponents[RPCType]));
	}

	// Visit each supported subobject and create component data for initial state of each subobject
	for (UClass* SubobjectClass : Info->SubobjectClasses)
	{
		FClassInfo* ComponentInfo = TypebindingManager->FindClassInfoByClass(SubobjectClass);
		check(ComponentInfo);

		// Find subobject corresponding to supported class
		TArray<UObject*> DefaultSubobjects;
		Actor->GetDefaultSubobjects(DefaultSubobjects);
		UObject** FoundSubobject = DefaultSubobjects.FindByPredicate([SubobjectClass](const UObject* Obj)
		{
			return (Obj->GetClass() == SubobjectClass);
		});
		check(FoundSubobject);
		UObject* Subobject = *FoundSubobject;

		FRepChangeState SubobjectRepChanges = Channel->CreateInitialRepChangeState(Subobject);
		FHandoverChangeState SubobjectHandoverChanges = Channel->CreateInitialHandoverChangeState(ComponentInfo);

		// Create component data for initial state of subobject
		ComponentData.Append(DataFactory.CreateComponentDatas(Subobject, SubobjectRepChanges, SubobjectHandoverChanges));

		// Add subobject RPCs to entity
		for (int RPCType = 0; RPCType < RPC_Count; RPCType++)
		{
			ComponentData.Add(ComponentFactory::CreateEmptyComponentData(ComponentInfo->RPCComponents[RPCType]));
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
	ComponentWriteAcl.Add(POSITION_COMPONENT_ID, UnrealWorkerPermission);
	ComponentWriteAcl.Add(ROTATION_COMPONENT_ID, UnrealWorkerPermission);
	ComponentWriteAcl.Add(ActorInfo->SingleClientComponent, UnrealWorkerPermission);
	ComponentWriteAcl.Add(ActorInfo->MultiClientComponent, UnrealWorkerPermission);
	ComponentWriteAcl.Add(ActorInfo->HandoverComponent, UnrealWorkerPermission);
	// No write attribute for RPC_Client since a Startup Actor will have no owner on level start
	ComponentWriteAcl.Add(ActorInfo->RPCComponents[RPC_Server], UnrealWorkerPermission);
	ComponentWriteAcl.Add(ActorInfo->RPCComponents[RPC_CrossServer], UnrealWorkerPermission);
	ComponentWriteAcl.Add(ActorInfo->RPCComponents[RPC_NetMulticast], UnrealWorkerPermission);

	for (UClass* SubobjectClass : ActorInfo->SubobjectClasses)
	{
		FClassInfo* SubobjectInfo = TypebindingManager->FindClassInfoByClass(SubobjectClass);
		check(SubobjectInfo);

		ComponentWriteAcl.Add(SubobjectInfo->SingleClientComponent, UnrealWorkerPermission);
		ComponentWriteAcl.Add(SubobjectInfo->MultiClientComponent, UnrealWorkerPermission);
		ComponentWriteAcl.Add(SubobjectInfo->HandoverComponent, UnrealWorkerPermission);
		// No write attribute for RPC_Client since a Startup Actor will have no owner on level start
		ComponentWriteAcl.Add(SubobjectInfo->RPCComponents[RPC_Server], UnrealWorkerPermission);
		ComponentWriteAcl.Add(SubobjectInfo->RPCComponents[RPC_CrossServer], UnrealWorkerPermission);
		ComponentWriteAcl.Add(SubobjectInfo->RPCComponents[RPC_NetMulticast], UnrealWorkerPermission);
	}

	USpatialActorChannel* Channel = Cast<USpatialActorChannel>(NetConnection->CreateChannel(CHTYPE_Actor, 1));

	uint32 CurrentOffset = 1;
	SubobjectToOffsetMap SubobjectNameToOffset;
	ForEachObjectWithOuter(Actor, [&CurrentOffset, &SubobjectNameToOffset](UObject* Object)
	{
		// Objects can only be allocated NetGUIDs if this is true.
		if (Object->IsSupportedForNetworking() && !Object->IsPendingKill() && !Object->IsEditorOnly())
		{
			SubobjectNameToOffset.Add(Object->GetName(), CurrentOffset);
			CurrentOffset++;
		}
	});

	FString StaticPath = Actor->GetPathName(nullptr);

	TArray<Worker_ComponentData> Components;
	Components.Add(improbable::Position(improbable::Coordinates::FromFVector(Channel->GetActorSpatialPosition(Actor))).CreatePositionData());
	Components.Add(improbable::Metadata(FSoftClassPath(ActorClass).ToString()).CreateMetadataData());
	Components.Add(improbable::EntityAcl(AnyWorkerPermission, ComponentWriteAcl).CreateEntityAclData());
	Components.Add(improbable::Persistence().CreatePersistenceData());
	Components.Add(improbable::Rotation(Actor->GetActorRotation()).CreateRotationData());
	Components.Add(improbable::UnrealMetadata(StaticPath, {}, SubobjectNameToOffset).CreateUnrealMetadataData());

	Components.Append(CreateStartupActorData(Channel, Actor, TypebindingManager, Cast<USpatialNetDriver>(NetConnection->Driver)));

	Entity.component_count = Components.Num();
	Entity.components = Components.GetData();

	return Worker_SnapshotOutputStream_WriteEntity(OutputStream, &Entity) != 0;
}

bool CreateStartupActors(Worker_SnapshotOutputStream* OutputStream, UWorld* World)
{
	USpatialNetDriver* NetDriver = nullptr;
	USpatialNetConnection* NetConnection = nullptr;
	USpatialTypebindingManager* TypebindingManager = nullptr;

	SetupStartupActorCreation(NetDriver, NetConnection, TypebindingManager);

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

		CreateStartupActor(OutputStream, Actor, CurrentEntityId, NetConnection, TypebindingManager);

		CurrentEntityId++;
	}

	CleanupNetDriverAndConnection(NetDriver, NetConnection);

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
