// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "Interop/SpatialTypebindingManager.h"

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

DEFINE_LOG_CATEGORY(LogSpatialTypebindingManager);

void USpatialTypebindingManager::Init(USpatialNetDriver* InNetDriver)
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

void USpatialTypebindingManager::AddTypebindingsForClass(UClass* Class)
{
	checkf(IsSupportedClass(Class), TEXT("Could not find class in schema database: %s"), *Class->GetPathName());

	FClassInfo& Info = ClassInfoMap.Add(Class);
	Info.Class = Class;

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

		TArray<UFunction*>& RPCArray = Info.RPCs.FindOrAdd(RPCType);

		FRPCInfo RPCInfo;
		RPCInfo.Type = RPCType;
		RPCInfo.Index = RPCArray.Num();

		RPCArray.Add(RemoteFunction);
		Info.RPCInfoMap.Add(RemoteFunction, RPCInfo);
	}

	for (TFieldIterator<UProperty> PropertyIt(Class); PropertyIt; ++PropertyIt)
	{
		UProperty* Property = *PropertyIt;

		if (Property->PropertyFlags & CPF_Handover)
		{
			for (int32 ArrayIdx = 0; ArrayIdx < PropertyIt->ArrayDim; ++ArrayIdx)
			{
				FHandoverPropertyInfo HandoverInfo;
				HandoverInfo.Handle = Info.HandoverProperties.Num() + 1; // 1-based index
				HandoverInfo.Offset = Property->GetOffset_ForGC() + Property->ElementSize * ArrayIdx;
				HandoverInfo.ArrayIdx = ArrayIdx;
				HandoverInfo.Property = Property;

				Info.HandoverProperties.Add(HandoverInfo);
			}
		}

		if (Property->PropertyFlags & CPF_AlwaysInterested)
		{
			for (int32 ArrayIdx = 0; ArrayIdx < PropertyIt->ArrayDim; ++ArrayIdx)
			{
				FInterestPropertyInfo InterestInfo;
				InterestInfo.Offset = Property->GetOffset_ForGC() + Property->ElementSize * ArrayIdx;
				InterestInfo.Property = Property;

				Info.InterestProperties.Add(InterestInfo);
			}
		}
	}

	ForAllSchemaComponentTypes([&](ESchemaComponentType Type)
	{
		Worker_ComponentId ComponentId = SchemaDatabase->ClassPathToSchema[Class->GetPathName()].SchemaComponents[Type];
		if (ComponentId != 0)
		{
			Info.SchemaComponents[Type] = ComponentId;
			ComponentToClassInfoMap.Emplace(ComponentId, MakeShared<FClassInfo>(Info));
			ComponentToOffsetMap.Add(ComponentId, 0);
			ComponentToCategoryMap.Add(ComponentId, (ESchemaComponentType)Type);
		}
	});

	for (auto& SubobjectClassDataPair : SchemaDatabase->ClassPathToSchema[Class->GetPathName()].SubobjectData)
	{
		int32 Offset = SubobjectClassDataPair.Key;
		FSubobjectSchemaData SubobjectSchemaData = SubobjectClassDataPair.Value;

		UClass* SubobjectClass = ResolveClass(SubobjectSchemaData.ClassPath);

		FClassInfo* SubobjectInfoPtr = FindClassInfoByClass(SubobjectClass);
		if (SubobjectInfoPtr == nullptr)
		{
			continue;
		}

		// Make a copy of the already made FClassInfo for this specific subobject
		TSharedPtr<FClassInfo> ActorSubobjectInfo = MakeShared<FClassInfo>(*SubobjectInfoPtr);
		ActorSubobjectInfo->SubobjectName = SubobjectSchemaData.Name;

		ForAllSchemaComponentTypes([&](ESchemaComponentType Type)
		{
			Worker_ComponentId ComponentId = SubobjectSchemaData.SchemaComponents[Type];
			if (ComponentId != 0)
			{
				ActorSubobjectInfo->SchemaComponents[Type] = ComponentId;
				ComponentToClassInfoMap.Emplace(ComponentId, ActorSubobjectInfo);
				ComponentToOffsetMap.Add(ComponentId, Offset);
				ComponentToCategoryMap.Add(ComponentId, ESchemaComponentType(Type));
			}
		});

		// This is needed because the recursive call to FindClassInfoByClass for subobjects changes the map
		Info = ClassInfoMap.FindChecked(Class);
		Info.SubobjectInfo.Add(Offset, ActorSubobjectInfo);
	}
}

UClass* USpatialTypebindingManager::LoadClassForComponent(Worker_ComponentId ComponentId)
{
	// Only try to load classes for generated components
	if (ComponentId < SpatialConstants::STARTING_GENERATED_COMPONENT_ID)
	{
		return nullptr;
	}

	for (auto& ClassDataPair : SchemaDatabase->ClassPathToSchema)
	{
		for (int32 Type = SCHEMA_Begin; Type < SCHEMA_Count; Type++)
		{
			const Worker_ComponentId ObjectComponentId = ClassDataPair.Value.SchemaComponents[Type];
			if (ComponentId == ObjectComponentId)
			{
				UClass* Class = ResolveClass(ClassDataPair.Key);
				AddTypebindingsForClass(Class);
				return Class;
			}
		}

		for (auto& SubobjectClassDataPair : ClassDataPair.Value.SubobjectData)
		{
			for (int32 Type = SCHEMA_Begin; Type < SCHEMA_Count; Type++)
			{
				const Worker_ComponentId SubobjectComponentId = SubobjectClassDataPair.Value.SchemaComponents[Type];
				if (ComponentId == SubobjectComponentId)
				{
					UClass* Class = ResolveClass(SubobjectClassDataPair.Value.ClassPath);
					UClass* ActorClass = ResolveClass(ClassDataPair.Key);
					AddTypebindingsForClass(ActorClass);
					return Class;
				}
			};
		}
	}

	UE_LOG(LogSpatialTypebindingManager, Warning, TEXT("Failed to find class for component %u in schema database"), ComponentId);
	return nullptr;
}

bool USpatialTypebindingManager::IsSupportedClass(UClass* Class) const
{
	return SchemaDatabase->ClassPathToSchema.Contains(Class->GetPathName());
}

FClassInfo* USpatialTypebindingManager::FindClassInfoByClass(UClass* Class)
{
	// This could be optimised to a single map lookup in all cases if we Find first, but we keep this pattern for readability
	if (!ClassInfoMap.Contains(Class))
	{
		AddTypebindingsForClass(Class);
	}
	
	return ClassInfoMap.Find(Class);
}

FClassInfo* USpatialTypebindingManager::FindClassInfoByActorClassAndOffset(UClass* Class, uint32 Offset)
{
	if (FClassInfo* Info = FindClassInfoByClass(Class))
	{
		if (Offset == 0)
		{
			return Info;
		}

		if (TSharedPtr<FClassInfo>* SubobjectInfo = Info->SubobjectInfo.Find(Offset))
		{
			return SubobjectInfo->Get();
		}

		return nullptr;
	}

	return nullptr;
}

FClassInfo* USpatialTypebindingManager::FindClassInfoByObject(UObject* Object)
{
	if (AActor* Actor = Cast<AActor>(Object))
	{
		return FindClassInfoByClass(Actor->GetClass());
	}
	else
	{
		checkSlow(Cast<AActor>(Object->GetOuter()));

		FUnrealObjectRef ObjectRef = NetDriver->PackageMap->GetUnrealObjectRefFromObject(Object);

		if (ObjectRef != SpatialConstants::NULL_OBJECT_REF && ObjectRef != SpatialConstants::UNRESOLVED_OBJECT_REF)
		{
			return FindClassInfoByActorClassAndOffset(Object->GetOuter()->GetClass(), ObjectRef.Offset);
		}
	}

	return nullptr;
}

FClassInfo* USpatialTypebindingManager::FindClassInfoByComponentId(Worker_ComponentId ComponentId)
{
	if (TSharedPtr<FClassInfo>* Info = ComponentToClassInfoMap.Find(ComponentId))
	{
		return Info->Get();
	}

	if (LoadClassForComponent(ComponentId) != nullptr)
	{
		return ComponentToClassInfoMap[ComponentId].Get();
	}

	return nullptr;
}

UClass* USpatialTypebindingManager::FindClassByComponentId(Worker_ComponentId ComponentId)
{
	if (TSharedPtr<FClassInfo>* Info = ComponentToClassInfoMap.Find(ComponentId))
	{
		return (*Info)->Class;
	}

	return LoadClassForComponent(ComponentId);
}

bool USpatialTypebindingManager::FindOffsetByComponentId(Worker_ComponentId ComponentId, uint32& OutOffset)
{
	if (uint32* Offset = ComponentToOffsetMap.Find(ComponentId))
	{
		OutOffset = *Offset;
		return true;
	}

	if (FindClassByComponentId(ComponentId) != nullptr)
	{
		OutOffset = ComponentToOffsetMap[ComponentId];
		return true;
	}

	return false;
}

ESchemaComponentType USpatialTypebindingManager::FindCategoryByComponentId(Worker_ComponentId ComponentId)
{
	if (ESchemaComponentType* Category = ComponentToCategoryMap.Find(ComponentId))
	{
		return *Category;
	}

	if (FindClassByComponentId(ComponentId) != nullptr)
	{
		return ComponentToCategoryMap[ComponentId];
	}

	return ESchemaComponentType::SCHEMA_Invalid;
}
