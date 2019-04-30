// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "Interop/SpatialClassInfoManager.h"

#include "AssetRegistryModule.h"
#include "Engine/BlueprintGeneratedClass.h"
#include "Engine/Engine.h"
#include "GameFramework/Actor.h"
#include "Misc/MessageDialog.h"
#include "UObject/Class.h"
#include "UObject/UObjectIterator.h"
#if WITH_EDITOR
#include "Kismet/KismetSystemLibrary.h"
#endif

#include "EngineClasses/SpatialNetDriver.h"
#include "EngineClasses/SpatialPackageMapClient.h"
#include "Utils/RepLayoutUtils.h"

DEFINE_LOG_CATEGORY(LogSpatialClassInfoManager);

void USpatialClassInfoManager::Init(USpatialNetDriver* InNetDriver)
{
	NetDriver = InNetDriver;

	FSoftObjectPath SchemaDatabasePath = FSoftObjectPath(TEXT("/Game/Spatial/SchemaDatabase.SchemaDatabase"));
	SchemaDatabase = Cast<USchemaDatabase>(SchemaDatabasePath.TryLoad());

	if (SchemaDatabase == nullptr)
	{
		FMessageDialog::Debugf(FText::FromString(TEXT("SchemaDatabase not found! No classes will be supported for SpatialOS replication.")));
		return;
	}
}

FORCEINLINE UClass* ResolveClass(FString& ClassPath)
{
	FSoftClassPath SoftClassPath(ClassPath);
	UClass* Class = SoftClassPath.ResolveClass();
	checkf(Class, TEXT("Failed to load class at path %s"), *ClassPath);
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
#if WITH_EDITOR
		// There is no C++ method to quit the current game, so using the Blueprint's QuitGame() that is calling ConsoleCommand("quit")
		// Note: don't use RequestExit() in Editor since it would terminate the Engine loop
		UKismetSystemLibrary::QuitGame(NetDriver->GetWorld(), nullptr, EQuitPreference::Quit);
#else
		FGenericPlatformMisc::RequestExit(false);
#endif
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

	for (TFieldIterator<UProperty> PropertyIt(Class); PropertyIt; ++PropertyIt)
	{
		UProperty* Property = *PropertyIt;

		if (Property->PropertyFlags & CPF_Handover)
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

	ForAllSchemaComponentTypes([&](ESchemaComponentType Type)
	{
		Worker_ComponentId ComponentId = SchemaDatabase->ClassPathToSchema[ClassPath].SchemaComponents[Type];
		if (ComponentId != 0)
		{
			Info->SchemaComponents[Type] = ComponentId;
			ComponentToClassInfoMap.Add(ComponentId, Info);
			ComponentToOffsetMap.Add(ComponentId, 0);
			ComponentToCategoryMap.Add(ComponentId, (ESchemaComponentType)Type);
		}
	});

	for (auto& SubobjectClassDataPair : SchemaDatabase->ClassPathToSchema[ClassPath].SubobjectData)
	{
		int32 Offset = SubobjectClassDataPair.Key;
		FSubobjectSchemaData SubobjectSchemaData = SubobjectClassDataPair.Value;

		UClass* SubobjectClass = ResolveClass(SubobjectSchemaData.ClassPath);

		const FClassInfo& SubobjectInfo = GetOrCreateClassInfoByClass(SubobjectClass);

		// Make a copy of the already made FClassInfo for this specific subobject
		TSharedRef<FClassInfo> ActorSubobjectInfo = MakeShared<FClassInfo>(SubobjectInfo);
		ActorSubobjectInfo->SubobjectName = SubobjectSchemaData.Name;

		ForAllSchemaComponentTypes([&](ESchemaComponentType Type)
		{
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
}

bool USpatialClassInfoManager::IsSupportedClass(const FString& PathName) const
{
	return SchemaDatabase->ClassPathToSchema.Contains(PathName);
}

const FClassInfo& USpatialClassInfoManager::GetOrCreateClassInfoByClass(UClass* Class)
{
	if (ClassInfoMap.Find(Class) == nullptr)
	{
		CreateClassInfoForClass(Class);
	}
	
	return ClassInfoMap[Class].Get();
}

const FClassInfo& USpatialClassInfoManager::GetOrCreateClassInfoByClassAndOffset(UClass* Class, uint32 Offset)
{
	const FClassInfo& Info = GetOrCreateClassInfoByClass(Class);

	if (Offset == 0)
	{
		return Info;
	}

	TSharedRef<FClassInfo> SubobjectInfo = Info.SubobjectInfo.FindChecked(Offset);
	return SubobjectInfo.Get();
}

const FClassInfo& USpatialClassInfoManager::GetOrCreateClassInfoByObject(UObject* Object)
{
	if (AActor* Actor = Cast<AActor>(Object))
	{
		return GetOrCreateClassInfoByClass(Actor->GetClass());
	}
	else
	{
		check(Cast<AActor>(Object->GetOuter()));

		FUnrealObjectRef ObjectRef = NetDriver->PackageMap->GetUnrealObjectRefFromObject(Object);

		check(ObjectRef.IsValid())

		return GetOrCreateClassInfoByClassAndOffset(Object->GetOuter()->GetClass(), ObjectRef.Offset);
	}
}

const FClassInfo& USpatialClassInfoManager::GetClassInfoByComponentId(Worker_ComponentId ComponentId) const
{
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

bool USpatialClassInfoManager::GetOffsetByComponentId(Worker_ComponentId ComponentId, uint32& OutOffset)
{
	if (uint32* Offset = ComponentToOffsetMap.Find(ComponentId))
	{
		OutOffset = *Offset;
		return true;
	}

	return false;
}

ESchemaComponentType USpatialClassInfoManager::GetCategoryByComponentId(Worker_ComponentId ComponentId)
{
	if (ESchemaComponentType* Category = ComponentToCategoryMap.Find(ComponentId))
	{
		return *Category;
	}

	return ESchemaComponentType::SCHEMA_Invalid;
}

bool USpatialClassInfoManager::IsSublevelComponent(Worker_ComponentId ComponentId)
{
	return SchemaDatabase->LevelComponentIds.Contains(ComponentId);
}
