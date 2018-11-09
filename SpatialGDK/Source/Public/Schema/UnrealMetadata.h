// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "GameFramework/Actor.h"
#include "Interop/SpatialTypebindingManager.h"
#include "Schema/Component.h"
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

	UnrealMetadata(const FString& InStaticPath, const FString& InOwnerWorkerId, const FString& InClassPath)
		: StaticPath(InStaticPath), OwnerWorkerAttribute(InOwnerWorkerId), ClassPath(InClassPath) {}

	UnrealMetadata(const Worker_ComponentData& Data)
	{
		Schema_Object* ComponentObject = Schema_GetComponentDataFields(Data.schema_type);

		StaticPath = GetStringFromSchema(ComponentObject, 1);
		OwnerWorkerAttribute = GetStringFromSchema(ComponentObject, 2);
		ClassPath = GetStringFromSchema(ComponentObject, 3);
	}

	Worker_ComponentData CreateUnrealMetadataData()
	{
		Worker_ComponentData Data = {};
		Data.component_id = ComponentId;
		Data.schema_type = Schema_CreateComponentData(ComponentId);
		Schema_Object* ComponentObject = Schema_GetComponentDataFields(Data.schema_type);

		AddStringToSchema(ComponentObject, 1, StaticPath);
		AddStringToSchema(ComponentObject, 2, OwnerWorkerAttribute);
		AddStringToSchema(ComponentObject, 3, ClassPath);

		return Data;
	}

	FORCEINLINE UClass* GetNativeEntityClass()
	{
		if (UClass* Class = FindObject<UClass>(ANY_PACKAGE, *ClassPath))
		{
			if (Class->IsChildOf<AActor>())
			{
				return Class;
			}
		}

		return nullptr;
	}

	FString StaticPath;
	FString OwnerWorkerAttribute;
	FString ClassPath;
};

FORCEINLINE SubobjectToOffsetMap CreateOffsetMapFromActor(AActor* Actor, FClassInfo* Info)
{
	SubobjectToOffsetMap SubobjectNameToOffset;

	for (auto& SubobjectInfoPair : Info->SubobjectInfo)
	{
		UObject* Subobject = Actor->GetDefaultSubobjectByName(SubobjectInfoPair.Value->SubobjectName);
		uint32 Offset = SubobjectInfoPair.Key;

		if (Subobject && Subobject->IsPendingKill() == false)
		{
			SubobjectNameToOffset.Add(Subobject, Offset);
		}
	}

	return SubobjectNameToOffset;
}

}
