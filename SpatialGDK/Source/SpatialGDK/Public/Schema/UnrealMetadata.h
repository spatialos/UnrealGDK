// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "GameFramework/Actor.h"
#include "Interop/SpatialClassInfoManager.h"
#include "Schema/Component.h"
#include "Schema/UnrealObjectRef.h"
#include "SpatialConstants.h"
#include "UObject/Package.h"
#include "UObject/UObjectHash.h"
#include "Utils/SchemaUtils.h"

#include <WorkerSDK/improbable/c_schema.h>
#include <WorkerSDK/improbable/c_worker.h>

using SubobjectToOffsetMap = TMap<UObject*, uint32>;

namespace improbable
{

struct UnrealMetadata : Component
{
	static const Worker_ComponentId ComponentId = SpatialConstants::UNREAL_METADATA_COMPONENT_ID;

	UnrealMetadata() = default;

	UnrealMetadata(const TSchemaOption<FUnrealObjectRef>& InStablyNamedRef, const FString& InOwnerWorkerAttribute, const FString& InClassPath)
		: StablyNamedRef(InStablyNamedRef), OwnerWorkerAttribute(InOwnerWorkerAttribute), ClassPath(InClassPath) {}

	UnrealMetadata(const Worker_ComponentData& Data)
	{
		Schema_Object* ComponentObject = Schema_GetComponentDataFields(Data.schema_type);

		if (Schema_GetObjectCount(ComponentObject, 1) == 1)
		{
			StablyNamedRef = GetObjectRefFromSchema(ComponentObject, 1);
		}
		OwnerWorkerAttribute = GetStringFromSchema(ComponentObject, 2);
		ClassPath = GetStringFromSchema(ComponentObject, 3);
	}

	Worker_ComponentData CreateUnrealMetadataData()
	{
		Worker_ComponentData Data = {};
		Data.component_id = ComponentId;
		Data.schema_type = Schema_CreateComponentData(ComponentId);
		Schema_Object* ComponentObject = Schema_GetComponentDataFields(Data.schema_type);

		if (StablyNamedRef.IsSet())
		{
			AddObjectRefToSchema(ComponentObject, 1, StablyNamedRef.GetValue());
		}
		AddStringToSchema(ComponentObject, 2, OwnerWorkerAttribute);
		AddStringToSchema(ComponentObject, 3, ClassPath);

		return Data;
	}

	FORCEINLINE UClass* GetNativeEntityClass()
	{
		if (UClass* Class = LoadObject<UClass>(nullptr, *ClassPath))
		{
			if (Class->IsChildOf<AActor>())
			{
				return Class;
			}
		}

		return nullptr;
	}

	TSchemaOption<FUnrealObjectRef> StablyNamedRef;
	FString OwnerWorkerAttribute;
	FString ClassPath;
};

FORCEINLINE SubobjectToOffsetMap CreateOffsetMapFromActor(AActor* Actor, const FClassInfo& Info)
{
	SubobjectToOffsetMap SubobjectNameToOffset;

	for (auto& SubobjectInfoPair : Info.SubobjectInfo)
	{
		UObject* Subobject = Actor->GetDefaultSubobjectByName(SubobjectInfoPair.Value->SubobjectName);
		uint32 Offset = SubobjectInfoPair.Key;

		if (Subobject != nullptr && Subobject->IsPendingKill() == false && Subobject->IsSupportedForNetworking())
		{
			SubobjectNameToOffset.Add(Subobject, Offset);
		}
	}

	return SubobjectNameToOffset;
}

}
