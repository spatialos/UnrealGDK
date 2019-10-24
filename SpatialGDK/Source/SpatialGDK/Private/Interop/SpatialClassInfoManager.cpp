// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "Interop/SpatialClassInfoManager.h"

#include "AssetRegistryModule.h"
#include "Engine/BlueprintGeneratedClass.h"
#include "Engine/Engine.h"
#include "GameFramework/Actor.h"
#include "Misc/MessageDialog.h"
#include "Runtime/Launch/Resources/Version.h"
#include "UObject/Class.h"
#include "UObject/UObjectIterator.h"

#if WITH_EDITOR
#include "Kismet/KismetSystemLibrary.h"
#endif

#include "EngineClasses/SpatialNetDriver.h"
#include "EngineClasses/SpatialPackageMapClient.h"
#include "Utils/ActorGroupManager.h"
#include "Utils/RepLayoutUtils.h"

DEFINE_LOG_CATEGORY(LogSpatialClassInfoManager);

bool USpatialClassInfoManager::TryInit(USpatialNetDriver* InNetDriver, UActorGroupManager* InActorGroupManager)
{
	NetDriver = InNetDriver;
	ActorGroupManager = InActorGroupManager;

	FSoftObjectPath SchemaDatabasePath = FSoftObjectPath(TEXT("/Game/Spatial/SchemaDatabase.SchemaDatabase"));
	SchemaDatabase = Cast<USchemaDatabase>(SchemaDatabasePath.TryLoad());

	if (SchemaDatabase == nullptr)
	{
		UE_LOG(LogSpatialClassInfoManager, Error, TEXT("SchemaDatabase not found! Please generate schema or turn off SpatialOS networking."));
		QuitGame();
		return false;
	}

	return true;
}

FORCEINLINE UClass* ResolveClass(FString& ClassPath)
{
	FSoftClassPath SoftClassPath(ClassPath);
	UClass* Class = SoftClassPath.ResolveClass();
	if (Class == nullptr)
	{
		UE_LOG(LogSpatialClassInfoManager, Warning, TEXT("Failed to find class at path %s! Attempting to load it."), *ClassPath);
		Class = SoftClassPath.TryLoadClass<UObject>();
	}
	return Class;
}

ESchemaComponentType GetRPCType(UFunction* RemoteFunction)
{
	if (RemoteFunction->HasAnyFunctionFlags(FUNC_NetMulticast))
	{
		return SCHEMA_NetMulticastRPC;
	}
	else if (RemoteFunction->HasAnyFunctionFlags(FUNC_NetCrossServer))
	{
		return SCHEMA_CrossServerRPC;
	}
	else if (RemoteFunction->HasAnyFunctionFlags(FUNC_NetReliable))
	{
		if (RemoteFunction->HasAnyFunctionFlags(FUNC_NetClient))
		{
			return SCHEMA_ClientReliableRPC;
		}
		else if (RemoteFunction->HasAnyFunctionFlags(FUNC_NetServer))
		{
			return SCHEMA_ServerReliableRPC;
		}
	}
	else
	{
		if (RemoteFunction->HasAnyFunctionFlags(FUNC_NetClient))
		{
			return SCHEMA_ClientUnreliableRPC;
		}
		else if (RemoteFunction->HasAnyFunctionFlags(FUNC_NetServer))
		{
			return SCHEMA_ServerUnreliableRPC;
		}
	}

	return SCHEMA_Invalid;
}

void USpatialClassInfoManager::CreateClassInfoForClass(UClass* Class)
{
	// Remove PIE prefix on class if it exists to properly look up the class.
	FString ClassPath = Class->GetPathName();
	GEngine->NetworkRemapPath(NetDriver, ClassPath, false);

	TSharedRef<FClassInfo> Info = ClassInfoMap.Add(Class, MakeShared<FClassInfo>());
	Info->Class = Class;

	// Note: we have to add Class to ClassInfoMap before quitting, as it is expected to be in there by GetOrCreateClassInfoByClass. Therefore the quitting logic cannot be moved higher up.
	if (!IsSupportedClass(ClassPath))
	{
		UE_LOG(LogSpatialClassInfoManager, Error, TEXT("Could not find class %s in schema database. Double-check whether replication is enabled for this class, the class is explicitly referenced from the starting scene and schema has been generated."), *ClassPath);
		UE_LOG(LogSpatialClassInfoManager, Error, TEXT("Disconnecting due to no generated schema for %s."), *ClassPath);
		QuitGame();
		return;
	}

	TArray<UFunction*> RelevantClassFunctions = SpatialGDK::GetClassRPCFunctions(Class);

	for (UFunction* RemoteFunction : RelevantClassFunctions)
	{
		ESchemaComponentType RPCType = GetRPCType(RemoteFunction);
		checkf(RPCType != SCHEMA_Invalid, TEXT("Could not determine RPCType for RemoteFunction: %s"), *GetPathNameSafe(RemoteFunction));

		FRPCInfo RPCInfo;
		RPCInfo.Type = RPCType;

		// Index is guaranteed to be the same on Clients & Servers since we process remote functions in the same order.
		RPCInfo.Index = Info->RPCs.Num();

		Info->RPCs.Add(RemoteFunction);
		Info->RPCInfoMap.Add(RemoteFunction, RPCInfo);
	}

	const bool bEnableHandover = GetDefault<USpatialGDKSettings>()->bEnableHandover;

	for (TFieldIterator<UProperty> PropertyIt(Class); PropertyIt; ++PropertyIt)
	{
		UProperty* Property = *PropertyIt;

		if (bEnableHandover && (Property->PropertyFlags & CPF_Handover))
		{
			for (int32 ArrayIdx = 0; ArrayIdx < PropertyIt->ArrayDim; ++ArrayIdx)
			{
				FHandoverPropertyInfo HandoverInfo;
				HandoverInfo.Handle = Info->HandoverProperties.Num() + 1; // 1-based index
				HandoverInfo.Offset = Property->GetOffset_ForGC() + Property->ElementSize * ArrayIdx;
				HandoverInfo.ArrayIdx = ArrayIdx;
				HandoverInfo.Property = Property;

				Info->HandoverProperties.Add(HandoverInfo);
			}
		}

		if (Property->PropertyFlags & CPF_AlwaysInterested)
		{
			for (int32 ArrayIdx = 0; ArrayIdx < PropertyIt->ArrayDim; ++ArrayIdx)
			{
				FInterestPropertyInfo InterestInfo;
				InterestInfo.Offset = Property->GetOffset_ForGC() + Property->ElementSize * ArrayIdx;
				InterestInfo.Property = Property;

				Info->InterestProperties.Add(InterestInfo);
			}
		}
	}

	if (Class->IsChildOf<AActor>())
	{
		FinishConstructingActorClassInfo(ClassPath, Info);
	}
	else
	{
		FinishConstructingSubobjectClassInfo(ClassPath, Info);
	}
}

void USpatialClassInfoManager::FinishConstructingActorClassInfo(const FString& ClassPath, TSharedRef<FClassInfo>& Info)
{
	ForAllSchemaComponentTypes([&](ESchemaComponentType Type)
	{
		Worker_ComponentId ComponentId = SchemaDatabase->ActorClassPathToSchema[ClassPath].SchemaComponents[Type];

		if (!GetDefault<USpatialGDKSettings>()->bEnableHandover && Type == SCHEMA_Handover)
		{
			return;
		}

		if (ComponentId != SpatialConstants::INVALID_COMPONENT_ID)
		{
			Info->SchemaComponents[Type] = ComponentId;
			ComponentToClassInfoMap.Add(ComponentId, Info);
			ComponentToOffsetMap.Add(ComponentId, 0);
			ComponentToCategoryMap.Add(ComponentId, (ESchemaComponentType)Type);
		}
	});

	for (auto& SubobjectClassDataPair : SchemaDatabase->ActorClassPathToSchema[ClassPath].SubobjectData)
	{
		int32 Offset = SubobjectClassDataPair.Key;
		FActorSpecificSubobjectSchemaData SubobjectSchemaData = SubobjectClassDataPair.Value;

		UClass* SubobjectClass = ResolveClass(SubobjectSchemaData.ClassPath);
		if (SubobjectClass == nullptr)
		{
			UE_LOG(LogSpatialClassInfoManager, Error, TEXT("Failed to resolve the class for subobject %s (class path: %s) on actor class %s! This subobject will not be able to replicate in Spatial!"), *SubobjectSchemaData.Name.ToString(), *SubobjectSchemaData.ClassPath, *ClassPath);
			continue;
		}

		const FClassInfo& SubobjectInfo = GetOrCreateClassInfoByClass(SubobjectClass);

		// Make a copy of the already made FClassInfo for this specific subobject
		TSharedRef<FClassInfo> ActorSubobjectInfo = MakeShared<FClassInfo>(SubobjectInfo);
		ActorSubobjectInfo->SubobjectName = SubobjectSchemaData.Name;

		ForAllSchemaComponentTypes([&](ESchemaComponentType Type)
		{
			if (!GetDefault<USpatialGDKSettings>()->bEnableHandover && Type == SCHEMA_Handover)
			{
				return;
			}

			Worker_ComponentId ComponentId = SubobjectSchemaData.SchemaComponents[Type];
			if (ComponentId != 0)
			{
				ActorSubobjectInfo->SchemaComponents[Type] = ComponentId;
				ComponentToClassInfoMap.Add(ComponentId, ActorSubobjectInfo);
				ComponentToOffsetMap.Add(ComponentId, Offset);
				ComponentToCategoryMap.Add(ComponentId, ESchemaComponentType(Type));
			}
		});

		Info->SubobjectInfo.Add(Offset, ActorSubobjectInfo);
	}

	if (UClass* ActorClass = Info->Class.Get())
	{
		if (ActorClass->IsChildOf<AActor>())
		{
			Info->ActorGroup = ActorGroupManager->GetActorGroupForClass(TSubclassOf<AActor>(ActorClass));
			Info->WorkerType = ActorGroupManager->GetWorkerTypeForClass(TSubclassOf<AActor>(ActorClass));

			UE_LOG(LogSpatialClassInfoManager, VeryVerbose, TEXT("[%s] is in ActorGroup [%s], on WorkerType [%s]"),
				*ActorClass->GetPathName(), *Info->ActorGroup.ToString(), *Info->WorkerType.ToString())
		}
	}
}

void USpatialClassInfoManager::FinishConstructingSubobjectClassInfo(const FString& ClassPath, TSharedRef<FClassInfo>& Info)
{
	for (const auto& DynamicSubobjectData : SchemaDatabase->SubobjectClassPathToSchema[ClassPath].DynamicSubobjectComponents)
	{
		// Make a copy of the already made FClassInfo for this dynamic subobject
		TSharedRef<FClassInfo> SpecificDynamicSubobjectInfo = MakeShared<FClassInfo>(Info.Get());

		int32 Offset = DynamicSubobjectData.SchemaComponents[SCHEMA_Data];
		check(Offset != SpatialConstants::INVALID_COMPONENT_ID);

		ForAllSchemaComponentTypes([&](ESchemaComponentType Type)
		{
			Worker_ComponentId ComponentId = DynamicSubobjectData.SchemaComponents[Type];

			if (ComponentId != SpatialConstants::INVALID_COMPONENT_ID)
			{
				SpecificDynamicSubobjectInfo->SchemaComponents[Type] = ComponentId;
				ComponentToClassInfoMap.Add(ComponentId, SpecificDynamicSubobjectInfo);
				ComponentToOffsetMap.Add(ComponentId, Offset);
				ComponentToCategoryMap.Add(ComponentId, ESchemaComponentType(Type));
			}
		});

		Info->DynamicSubobjectInfo.Add(SpecificDynamicSubobjectInfo);
	}
}

void USpatialClassInfoManager::TryCreateClassInfoForComponentId(Worker_ComponentId ComponentId)
{
	if (FString* ClassPath = SchemaDatabase->ComponentIdToClassPath.Find(ComponentId))
	{
		if (UClass* Class = LoadObject<UClass>(nullptr, **ClassPath))
		{
			CreateClassInfoForClass(Class);
		}
	}
}

bool USpatialClassInfoManager::IsSupportedClass(const FString& PathName) const
{
	return SchemaDatabase->ActorClassPathToSchema.Contains(PathName) || SchemaDatabase->SubobjectClassPathToSchema.Contains(PathName);
}

const FClassInfo& USpatialClassInfoManager::GetOrCreateClassInfoByClass(UClass* Class)
{
	if (!ClassInfoMap.Contains(Class))
	{
		CreateClassInfoForClass(Class);
	}
	
	return ClassInfoMap[Class].Get();
}

const FClassInfo& USpatialClassInfoManager::GetOrCreateClassInfoByObject(UObject* Object)
{
	if (AActor* Actor = Cast<AActor>(Object))
	{
		return GetOrCreateClassInfoByClass(Actor->GetClass());
	}
	else
	{
		check(Object->GetTypedOuter<AActor>() != nullptr);

		FUnrealObjectRef ObjectRef = NetDriver->PackageMap->GetUnrealObjectRefFromObject(Object);

		check(ObjectRef.IsValid());

		return ComponentToClassInfoMap[ObjectRef.Offset].Get();
	}
}

const FClassInfo& USpatialClassInfoManager::GetClassInfoByComponentId(Worker_ComponentId ComponentId)
{
	if (!ComponentToClassInfoMap.Contains(ComponentId))
	{
		TryCreateClassInfoForComponentId(ComponentId);
	}

	TSharedRef<FClassInfo> Info = ComponentToClassInfoMap.FindChecked(ComponentId);
	return Info.Get();
}

UClass* USpatialClassInfoManager::GetClassByComponentId(Worker_ComponentId ComponentId)
{
	TSharedRef<FClassInfo> Info = ComponentToClassInfoMap.FindChecked(ComponentId);
	if (UClass* Class = Info->Class.Get())
	{
		return Class;
	}
	else
	{
		UE_LOG(LogSpatialClassInfoManager, Warning, TEXT("Class corresponding to component %d has been unloaded! Will try to reload based on the component id."), ComponentId);

		// The weak pointer to the class stored in the FClassInfo will be the same as the one used as the key in ClassInfoMap, so we can use it to clean up the old entry.
		ClassInfoMap.Remove(Info->Class);

		// The old references in the other maps (ComponentToClassInfoMap etc) will be replaced by reloading the info (as a part of LoadClassForComponent).
	}

	return nullptr;
}

uint32 USpatialClassInfoManager::GetComponentIdForClass(const UClass& Class) const
{
	const FString ClassPath = Class.GetPathName();
	if (const FActorSchemaData* ActorSchemaData = SchemaDatabase->ActorClassPathToSchema.Find(Class.GetPathName()))
	{
		return ActorSchemaData->SchemaComponents[SCHEMA_Data];
	}
	return SpatialConstants::INVALID_COMPONENT_ID;
}

TArray<Worker_ComponentId> USpatialClassInfoManager::GetComponentIdsForClassHierarchy(const UClass& BaseClass, const bool bIncludeDerivedTypes /* = true */) const
{
	TArray<Worker_ComponentId> OutComponentIds;

	check(SchemaDatabase);
	if (bIncludeDerivedTypes)
	{
		for (TObjectIterator<UClass> It; It; ++It)
		{
			const UClass* Class = *It;
			check(Class);
			if (Class->IsChildOf(&BaseClass))
			{
				const Worker_ComponentId ComponentId = GetComponentIdForClass(*Class);
				if (ComponentId != SpatialConstants::INVALID_COMPONENT_ID)
				{
					OutComponentIds.Add(ComponentId);
				}
			}
		}
	}
	else
	{
		const uint32 ComponentId = GetComponentIdForClass(BaseClass);
		if (ComponentId != SpatialConstants::INVALID_COMPONENT_ID)
		{
			OutComponentIds.Add(ComponentId);
		}

	}

	return OutComponentIds;
}


bool USpatialClassInfoManager::GetOffsetByComponentId(Worker_ComponentId ComponentId, uint32& OutOffset)
{
	if (!ComponentToOffsetMap.Contains(ComponentId))
	{
		TryCreateClassInfoForComponentId(ComponentId);
	}

	if (uint32* Offset = ComponentToOffsetMap.Find(ComponentId))
	{
		OutOffset = *Offset;
		return true;
	}

	return false;
}

ESchemaComponentType USpatialClassInfoManager::GetCategoryByComponentId(Worker_ComponentId ComponentId)
{
	if (!ComponentToCategoryMap.Contains(ComponentId))
	{
		TryCreateClassInfoForComponentId(ComponentId);
	}

	if (ESchemaComponentType* Category = ComponentToCategoryMap.Find(ComponentId))
	{
		return *Category;
	}

	return ESchemaComponentType::SCHEMA_Invalid;
}

const FRPCInfo& USpatialClassInfoManager::GetRPCInfo(UObject* Object, UFunction* Function)
{
	check(Object != nullptr && Function != nullptr);

	const FClassInfo& Info = GetOrCreateClassInfoByObject(Object);
	const FRPCInfo* RPCInfoPtr = Info.RPCInfoMap.Find(Function);

	// We potentially have a parent function and need to find the child function.
	// This exists as it's possible in blueprints to explicitly call the parent function.
	if (RPCInfoPtr == nullptr)
	{
		for (auto It = Info.RPCInfoMap.CreateConstIterator(); It; ++It)
		{
			if (It.Key()->GetName() == Function->GetName())
			{
				// Matching child function found. Use this for the remote function call.
				RPCInfoPtr = &It.Value();
				break;
			}
		}
	}
	check(RPCInfoPtr != nullptr);
	return *RPCInfoPtr;
}

uint32 USpatialClassInfoManager::GetComponentIdFromLevelPath(const FString& LevelPath)
{
	FString CleanLevelPath = UWorld::RemovePIEPrefix(LevelPath);
	if (const uint32* ComponentId = SchemaDatabase->LevelPathToComponentId.Find(CleanLevelPath))
	{
		return *ComponentId;
	}
	return SpatialConstants::INVALID_COMPONENT_ID;
}

bool USpatialClassInfoManager::IsSublevelComponent(Worker_ComponentId ComponentId)
{
	return SchemaDatabase->LevelComponentIds.Contains(ComponentId);
}

void USpatialClassInfoManager::QuitGame()
{
#if WITH_EDITOR
	// There is no C++ method to quit the current game, so using the Blueprint's QuitGame() that is calling ConsoleCommand("quit")
	// Note: don't use RequestExit() in Editor since it would terminate the Engine loop
#if ENGINE_MINOR_VERSION <= 20
	UKismetSystemLibrary::QuitGame(NetDriver->GetWorld(), nullptr, EQuitPreference::Quit);
#else
	UKismetSystemLibrary::QuitGame(NetDriver->GetWorld(), nullptr, EQuitPreference::Quit, false);
#endif

#else
	FGenericPlatformMisc::RequestExit(false);
#endif
}
