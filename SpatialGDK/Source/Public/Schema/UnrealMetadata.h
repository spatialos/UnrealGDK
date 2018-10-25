// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "GameFramework/Actor.h"
#include "Schema/Component.h"
#include "SpatialConstants.h"
#include "Platform.h"
#include "UObjectHash.h"
#include "Utils/SchemaUtils.h"

#include <improbable/c_schema.h>
#include <improbable/c_worker.h>
#include "SpatialTypebindingManager.h"

using SubobjectToOffsetMap = TMap<UObject*, uint32>;

namespace improbable
{

struct UnrealMetadata : Component
{
	static const Worker_ComponentId ComponentId = SpatialConstants::UNREAL_METADATA_COMPONENT_ID;

	UnrealMetadata() = default;

	UnrealMetadata(const FString& InStaticPath, const FString& InOwnerWorkerId)
		: StaticPath(InStaticPath), OwnerWorkerAttribute(InOwnerWorkerId) {}

	UnrealMetadata(const Worker_ComponentData& Data)
	{
		Schema_Object* ComponentObject = Schema_GetComponentDataFields(Data.schema_type);

		StaticPath = GetStringFromSchema(ComponentObject, 1);
		OwnerWorkerAttribute = GetStringFromSchema(ComponentObject, 2);
	}

	Worker_ComponentData CreateUnrealMetadataData()
	{
		Worker_ComponentData Data = {};
		Data.component_id = ComponentId;
		Data.schema_type = Schema_CreateComponentData(ComponentId);
		Schema_Object* ComponentObject = Schema_GetComponentDataFields(Data.schema_type);

		AddStringToSchema(ComponentObject, 1, StaticPath);
		AddStringToSchema(ComponentObject, 2, OwnerWorkerAttribute);

		return Data;
	}

	FString StaticPath;
	FString OwnerWorkerAttribute;
};

FORCEINLINE SubobjectToOffsetMap CreateOffsetMapFromActor(AActor* Actor, FClassInfo* Info)
{
	SubobjectToOffsetMap SubobjectNameToOffset;

	for (auto& SubobjectInfoPair : Info->SubobjectInfo)
	{
		UObject* Subobject = Actor->GetDefaultSubobjectByName(FName(*SubobjectInfoPair.Value->SubobjectName));
		uint32 Offset = SubobjectInfoPair.Key;

		check(Subobject);

		SubobjectNameToOffset.Add(Subobject, Offset);
	}

	return SubobjectNameToOffset;
}

}
