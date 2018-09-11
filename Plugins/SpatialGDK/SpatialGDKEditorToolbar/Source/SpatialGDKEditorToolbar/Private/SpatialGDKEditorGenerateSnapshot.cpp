// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "SpatialGDKEditorGenerateSnapshot.h"

#include "SpatialGDKEditorToolbarSettings.h"
#include "SpatialConstants.h"
#include "Utils/SchemaUtils.h"
#include "Utils/RepDataUtils.h"
#include "Utils/RepLayoutUtils.h"
#include "Utils/ComponentFactory.h"
#include "Schema/StandardLibrary.h"
#include "Schema/UnrealMetadata.h"
#include "Schema/SnapshotEntity.h"
#include "SpatialTypebindingManager.h"

#include "Runtime/Core/Public/HAL/PlatformFilemanager.h"
#include "UObjectIterator.h"
#include "EngineUtils.h"

#include <improbable/c_worker.h>
#include <improbable/c_schema.h>

DEFINE_LOG_CATEGORY(LogSpatialGDKSnapshot);

const WorkerAttributeSet UnrealWorkerAttributeSet{ TArray<FString>{TEXT("UnrealWorker")} };
const WorkerAttributeSet UnrealClientAttributeSet{ TArray<FString>{TEXT("UnrealClient")} };

const WorkerRequirementSet UnrealWorkerPermission{ {UnrealWorkerAttributeSet} };
const WorkerRequirementSet UnrealClientPermission{ {UnrealClientAttributeSet} };
const WorkerRequirementSet AnyWorkerPermission{ {UnrealClientAttributeSet, UnrealWorkerAttributeSet} };

const Coordinates Origin{ 0, 0, 0 };

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

	Components.Add(Position(Origin).CreatePositionData());
	Components.Add(Metadata(TEXT("SpatialSpawner")).CreateMetadataData());
	Components.Add(Persistence().CreatePersistenceData());
	Components.Add(UnrealMetadata().CreateUnrealMetadataData());
	Components.Add(PlayerSpawnerData);
	Components.Add(EntityAcl(AnyWorkerPermission, ComponentWriteAcl).CreateEntityAclData());

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

	Schema_AddStringToEntityMap(ComponentObject, 1, SingletonNameToEntityId);
	Schema_AddStringToEntityMap(ComponentObject, 2, StablyNamedPathToEntityId);

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

	Components.Add(Position(Origin).CreatePositionData());
	Components.Add(Metadata(TEXT("GlobalStateManager")).CreateMetadataData());
	Components.Add(Persistence().CreatePersistenceData());
	Components.Add(UnrealMetadata().CreateUnrealMetadataData());
	Components.Add(CreateGlobalStateManagerData());
	Components.Add(EntityAcl(UnrealWorkerPermission, ComponentWriteAcl).CreateEntityAclData());

	GSM.component_count = Components.Num();
	GSM.components = Components.GetData();

	return Worker_SnapshotOutputStream_WriteEntity(OutputStream, &GSM) != 0;
}

bool CreateStartupActors(Worker_SnapshotOutputStream* OutputStream, UWorld* World)
{
	Worker_EntityId CurrentEntityId = SpatialConstants::GLOBAL_STATE_MANAGER + 1;

	USpatialTypebindingManager* TypebindingManager = NewObject<USpatialTypebindingManager>();
	TypebindingManager->Init();
	WorkerAttributeSet OwningClientAttribute = { TEXT("workerId:") };  //No owning client since we are creating a snapshot
	WorkerRequirementSet OwningClientOnly = { OwningClientAttribute };

	for (TActorIterator<AActor> Iterator(World); Iterator; ++Iterator)
	{
		AActor* Actor = *Iterator;
		if (Actor->IsEditorOnly())
		{
			continue;
		}
		check(!Actor->IsA(APlayerController::StaticClass()));
		UClass* ActorClass = Actor->GetClass();

		if (ActorClass->HasAnySpatialClassFlags(SPATIALCLASS_GenerateTypeBindings))
		{
			Worker_Entity ActorEntity;
			ActorEntity.entity_id = CurrentEntityId++;

			FClassInfo* Info = TypebindingManager->FindClassInfoByClass(Actor->GetClass());
			check(Info);

			WriteAclMap ComponentWriteAcl;
			ComponentWriteAcl.Add(POSITION_COMPONENT_ID, UnrealWorkerPermission);
			ComponentWriteAcl.Add(Info->SingleClientComponent, UnrealWorkerPermission);
			ComponentWriteAcl.Add(Info->MultiClientComponent, UnrealWorkerPermission);
			ComponentWriteAcl.Add(Info->HandoverComponent, UnrealWorkerPermission);
			ComponentWriteAcl.Add(Info->RPCComponents[RPC_Client], OwningClientOnly);
			ComponentWriteAcl.Add(Info->RPCComponents[RPC_Server], UnrealWorkerPermission);
			ComponentWriteAcl.Add(Info->RPCComponents[RPC_CrossServer], UnrealWorkerPermission);
			ComponentWriteAcl.Add(Info->RPCComponents[RPC_NetMulticast], UnrealWorkerPermission);

			for (UClass* SubobjectClass : Info->SubobjectClasses)
			{
				FClassInfo* ClassInfo = TypebindingManager->FindClassInfoByClass(SubobjectClass);
				check(ClassInfo);

				ComponentWriteAcl.Add(ClassInfo->SingleClientComponent, UnrealWorkerPermission);
				ComponentWriteAcl.Add(ClassInfo->MultiClientComponent, UnrealWorkerPermission);
				// Not adding handover since component's handover properties will be handled by the actor anyway
				ComponentWriteAcl.Add(ClassInfo->RPCComponents[RPC_Client], OwningClientOnly);
				ComponentWriteAcl.Add(ClassInfo->RPCComponents[RPC_Server], UnrealWorkerPermission);
				ComponentWriteAcl.Add(ClassInfo->RPCComponents[RPC_CrossServer], UnrealWorkerPermission);
				ComponentWriteAcl.Add(ClassInfo->RPCComponents[RPC_NetMulticast], UnrealWorkerPermission);
			}

			FString StaticPath = Actor->GetPathName(nullptr);

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

			TArray<Worker_ComponentData> Components;
			Components.Add(Position(Coordinates::FromFVector(Actor->GetActorLocation())).CreatePositionData());			// YOLO - Check if actor location is the right location
			Components.Add(Metadata(FSoftClassPath(ActorClass).ToString()).CreateMetadataData());
			Components.Add(EntityAcl(AnyWorkerPermission, ComponentWriteAcl).CreateEntityAclData());
			Components.Add(Persistence().CreatePersistenceData());
			Components.Add(UnrealMetadata(StaticPath, {}, SubobjectNameToOffset).CreateUnrealMetadataData());			// Owning client is empty string, since it does not make sense in snapshot generation (should only affect player controllers anyway?)
			Components.Add(SnapshotEntity().CreateSnapshotEntityData());												// Mark the entity as a snapshot entity

			Components.Add(ComponentFactory::CreateEmptyComponentData(Info->SingleClientComponent));
			Components.Add(ComponentFactory::CreateEmptyComponentData(Info->MultiClientComponent));
			Components.Add(ComponentFactory::CreateEmptyComponentData(Info->HandoverComponent));

			for (int RPCType = 0; RPCType < RPC_Count; RPCType++)
			{
				Components.Add(ComponentFactory::CreateEmptyComponentData(Info->RPCComponents[RPCType]));
			}

			for (UClass* SubobjectClass : Info->SubobjectClasses)
			{
				FClassInfo* ComponentInfo = TypebindingManager->FindClassInfoByClass(SubobjectClass);
				check(ComponentInfo);

				TArray<UObject*> DefaultSubobjects;
				Actor->GetDefaultSubobjects(DefaultSubobjects);
				UObject** FoundSubobject = DefaultSubobjects.FindByPredicate([SubobjectClass](const UObject* Obj)
				{
					return (Obj->GetClass() == SubobjectClass);
				});
				check(FoundSubobject);
				UObject* Subobject = *FoundSubobject;

				Components.Add(ComponentFactory::CreateEmptyComponentData(ComponentInfo->SingleClientComponent));
				Components.Add(ComponentFactory::CreateEmptyComponentData(ComponentInfo->MultiClientComponent));

				// Not adding handover since component's handover properties will be handled by the actor anyway

				for (int RPCType = 0; RPCType < RPC_Count; RPCType++)
				{
					Components.Add(ComponentFactory::CreateEmptyComponentData(ComponentInfo->RPCComponents[RPCType]));
				}
			}

			ActorEntity.component_count = Components.Num();
			ActorEntity.components = Components.GetData();

			if (Worker_SnapshotOutputStream_WriteEntity(OutputStream, &ActorEntity) == 0)
			{
				return false;
			}
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
		UE_LOG(LogSpatialGDKSnapshot, Error, TEXT("Error generating snapshot: %s"), UTF8_TO_TCHAR(Worker_SnapshotOutputStream_GetError(OutputStream)));
		return false;
	}

	if (!CreateGlobalStateManager(OutputStream))
	{
		UE_LOG(LogSpatialGDKSnapshot, Error, TEXT("Error generating snapshot: %s"), UTF8_TO_TCHAR(Worker_SnapshotOutputStream_GetError(OutputStream)));
		return false;
	}

	if (!CreateStartupActors(OutputStream, World))
	{
		UE_LOG(LogSpatialGDKSnapshot, Error, TEXT("Error generating snapshot: %s"), UTF8_TO_TCHAR(Worker_SnapshotOutputStream_GetError(OutputStream)));
		return false;
	}

	Worker_SnapshotOutputStream_Destroy(OutputStream);

	return true;
}
