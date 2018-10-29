// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "Interop/SpatialTypebindingManager.h"

#include "AssetRegistryModule.h"
#include "Engine/Blueprint.h"
#include "Engine/BlueprintGeneratedClass.h"
#include "Engine/SCS_Node.h"
#include "GameFramework/Actor.h"
#include "Misc/MessageDialog.h"
#include "SpatialNetDriver.h" 
#include "UObject/Class.h"
#include "UObjectIterator.h"
#include "UObject/UObjectIterator.h"

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

	FindSupportedClasses();
	CreateTypebindings();
}

void USpatialTypebindingManager::FindSupportedClasses()
{
	SchemaDatabase->ClassToSchema.GetKeys(SupportedClasses);
}

void USpatialTypebindingManager::CreateTypebindings()
{
	for (UClass* Class : SupportedClasses)
	{
		FClassInfo Info;

		for (TFieldIterator<UFunction> RemoteFunction(Class); RemoteFunction; ++RemoteFunction)
		{
			if (RemoteFunction->FunctionFlags & FUNC_NetClient ||
				RemoteFunction->FunctionFlags & FUNC_NetServer ||
				RemoteFunction->FunctionFlags & FUNC_NetCrossServer ||
				RemoteFunction->FunctionFlags & FUNC_NetMulticast)
			{
				ESchemaComponentType RPCType;
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

				RPCArray.Add(*RemoteFunction);
				Info.RPCInfoMap.Add(*RemoteFunction, RPCInfo);
			}
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
		}

		ForAllSchemaComponentTypes([&](ESchemaComponentType Type) {
			Worker_ComponentId ComponentId = SchemaDatabase->ClassToSchema[Class].SchemaComponents[Type];
			if (ComponentId != 0)
			{
				Info.SchemaComponents[Type] = ComponentId;
				ComponentToClassMap.Add(ComponentId, Class);
				ComponentToOffsetMap.Add(ComponentId, 0);
				ComponentToCategoryMap.Add(ComponentId, (ESchemaComponentType)Type);
			}
		});

		Info.Class = Class;

		ClassInfoMap.Emplace(Class, Info);
	}

	for (UClass* Class : SupportedClasses)
	{
		for (auto& SubobjectDataPair : SchemaDatabase->ClassToSchema[Class].SubobjectData)
		{
			int32 Offset = SubobjectDataPair.Key;
			FSubobjectSchemaData SubobjectSchemaData = SubobjectDataPair.Value;

			FClassInfo* ActorInfo = FindClassInfoByClass(Class);
			FClassInfo* SubobjectInfo = FindClassInfoByClass(SubobjectSchemaData.Class);
			if (SubobjectInfo == nullptr)
			{
				continue;
			}

			SubobjectInfo->SubobjectName = SubobjectSchemaData.Name;

			ForAllSchemaComponentTypes([&](ESchemaComponentType Type)
			{
				Worker_ComponentId ComponentId = SubobjectSchemaData.SchemaComponents[Type];
				if (ComponentId != 0)
				{
					SubobjectInfo->SchemaComponents[Type] = ComponentId;
					ComponentToClassMap.Add(ComponentId, SubobjectSchemaData.Class);
					ComponentToOffsetMap.Add(ComponentId, Offset);
					ComponentToCategoryMap.Add(ComponentId, (ESchemaComponentType)Type);
				}
			});

			ActorInfo->SubobjectInfo.Add(Offset, MakeShared<FClassInfo>(*SubobjectInfo));
		}
	}
}

FClassInfo* USpatialTypebindingManager::FindClassInfoByClass(UClass* Class)
{
	if (FClassInfo* Info = ClassInfoMap.Find(Class))
	{
		return Info;
	}

	return nullptr;
}

FClassInfo* USpatialTypebindingManager::FindClassInfoByClassAndOffset(UClass* Class, uint32 Offset)
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

FClassInfo* USpatialTypebindingManager::FindClassInfoByComponentId(Worker_ComponentId ComponentId)
{
	UClass* Class = FindClassByComponentId(ComponentId);
	return Class != nullptr ? FindClassInfoByClass(Class) : nullptr;
}

FClassInfo* USpatialTypebindingManager::FindClassInfoByObject(UObject* Object)
{
	FClassInfo* Info = nullptr;
	if (AActor* Actor = Cast<AActor>(Object))
	{
		Info = FindClassInfoByClass(Actor->GetClass());
	}
	else
	{
		checkSlow(Cast<AActor>(Object->GetOuter()));

		FUnrealObjectRef ObjectRef = NetDriver->PackageMap->GetUnrealObjectRefFromObject(Object);

		if (ObjectRef != SpatialConstants::NULL_OBJECT_REF)
		{
			Info = FindClassInfoByClassAndOffset(Object->GetOuter()->GetClass(), ObjectRef.Offset);
		}
	}

	return Info;
}

UClass* USpatialTypebindingManager::FindClassByComponentId(Worker_ComponentId ComponentId)
{
	UClass** Class = ComponentToClassMap.Find(ComponentId);
	return Class != nullptr ? *Class : nullptr;
}

bool USpatialTypebindingManager::IsSupportedClass(UClass* Class)
{
	return SupportedClasses.Contains(Class);
}

bool USpatialTypebindingManager::FindOffsetByComponentId(Worker_ComponentId ComponentId, uint32& OutOffset)
{
	if (uint32* Offset = ComponentToOffsetMap.Find(ComponentId))
	{
		OutOffset = *Offset;
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

	return ESchemaComponentType::SCHEMA_Invalid;
}
