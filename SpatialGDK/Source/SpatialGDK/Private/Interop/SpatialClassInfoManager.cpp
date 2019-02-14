// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "Interop/SpatialClassInfoManager.h"

#include "AssetRegistryModule.h"
#include "Engine/Blueprint.h"
#include "Engine/BlueprintGeneratedClass.h"
#include "Engine/SCS_Node.h"
#include "GameFramework/Actor.h"
#include "Misc/MessageDialog.h"
#include "UObject/Class.h"
#include "UObject/UObjectIterator.h"

#include "EngineClasses/SpatialNetDriver.h"
#include "EngineClasses/SpatialPackageMapClient.h"
#include "Utils/RepLayoutUtils.h"

DEFINE_LOG_CATEGORY(LogSpatialClassInfoManager);

void USpatialClassInfoManager::Init(USpatialNetDriver* InNetDriver)
{
	NetDriver = InNetDriver;
	
	TSoftObjectPtr<USchemaDatabase> SchemaDatabasePtr(FSoftObjectPath(TEXT("/Game/Spatial/SchemaDatabase.SchemaDatabase")));
	SchemaDatabasePtr.LoadSynchronous();
	SchemaDatabase = SchemaDatabasePtr.Get();

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

void USpatialClassInfoManager::CreateClassInfoForClass(UClass* Class)
{
	checkf(IsSupportedClass(Class), TEXT("Could not find class in schema database: %s"), *Class->GetPathName());

	TSharedRef<FClassInfo> Info = ClassInfoMap.Add(Class, MakeShared<FClassInfo>());
	Info->Class = Class;

	TArray<UFunction*> RelevantClassFunctions = improbable::GetClassRPCFunctions(Class);

	for (UFunction* RemoteFunction : RelevantClassFunctions)
	{
		ESchemaComponentType RPCType = SCHEMA_Invalid;
		if (RemoteFunction->FunctionFlags & FUNC_NetClient)
		{
			RPCType = SCHEMA_ClientRPC;
		}
		else if (RemoteFunction->FunctionFlags & FUNC_NetServer)
		{
			RPCType = SCHEMA_ServerRPC;
		}
		else if (RemoteFunction->FunctionFlags & FUNC_NetCrossServer)
		{
			RPCType = SCHEMA_CrossServerRPC;
		}
		else if (RemoteFunction->FunctionFlags & FUNC_NetMulticast)
		{
			RPCType = SCHEMA_NetMulticastRPC;
		}
		else
		{
			checkNoEntry();
		}

		TArray<UFunction*>& RPCArray = Info->RPCs.FindOrAdd(RPCType);

		FRPCInfo RPCInfo;
		RPCInfo.Type = RPCType;
		RPCInfo.Index = RPCArray.Num();

		RPCArray.Add(RemoteFunction);
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
		Worker_ComponentId ComponentId = SchemaDatabase->ClassPathToSchema[Class->GetPathName()].SchemaComponents[Type];
		if (ComponentId != 0)
		{
			Info->SchemaComponents[Type] = ComponentId;
			ComponentToClassInfoMap.Add(ComponentId, Info);
			ComponentToOffsetMap.Add(ComponentId, 0);
			ComponentToCategoryMap.Add(ComponentId, (ESchemaComponentType)Type);
		}
	});

	for (auto& SubobjectClassDataPair : SchemaDatabase->ClassPathToSchema[Class->GetPathName()].SubobjectData)
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

bool USpatialClassInfoManager::IsSupportedClass(UClass* Class) const
{
	return SchemaDatabase->ClassPathToSchema.Contains(Class->GetPathName());
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
