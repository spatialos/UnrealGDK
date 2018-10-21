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

using SubobjectToOffsetMap = TMap<FString, uint32>;

namespace improbable
{

struct UnrealMetadata : Component
{
	static const Worker_ComponentId ComponentId = SpatialConstants::UNREAL_METADATA_COMPONENT_ID;

	UnrealMetadata() = default;

	UnrealMetadata(const FString& InStaticPath, const FString& InOwnerWorkerId, const SubobjectToOffsetMap& InSubobjectNameToOffset)
		: StaticPath(InStaticPath), OwnerWorkerAttribute(InOwnerWorkerId), SubobjectNameToOffset(InSubobjectNameToOffset) {}

	UnrealMetadata(const Worker_ComponentData& Data)
	{
		Schema_Object* ComponentObject = Schema_GetComponentDataFields(Data.schema_type);

		StaticPath = GetStringFromSchema(ComponentObject, 1);
		OwnerWorkerAttribute = GetStringFromSchema(ComponentObject, 2);

		uint32 KVPairCount = Schema_GetObjectCount(ComponentObject, 3);
		for (uint32 i = 0; i < KVPairCount; i++)
		{
			Schema_Object* KVPairObject = Schema_IndexObject(ComponentObject, 3, i);
			FString Key = GetStringFromSchema(KVPairObject, SCHEMA_MAP_KEY_FIELD_ID);
			uint32 Value = Schema_GetUint32(KVPairObject, SCHEMA_MAP_VALUE_FIELD_ID);

			SubobjectNameToOffset.Add(Key, Value);
		}
	}

	Worker_ComponentData CreateUnrealMetadataData()
	{
		Worker_ComponentData Data = {};
		Data.component_id = ComponentId;
		Data.schema_type = Schema_CreateComponentData(ComponentId);
		Schema_Object* ComponentObject = Schema_GetComponentDataFields(Data.schema_type);

		AddStringToSchema(ComponentObject, 1, StaticPath);
		AddStringToSchema(ComponentObject, 2, OwnerWorkerAttribute);

		for (const auto& KVPair : SubobjectNameToOffset)
		{
			Schema_Object* KVPairObject = Schema_AddObject(ComponentObject, 3);
			AddStringToSchema(KVPairObject, SCHEMA_MAP_KEY_FIELD_ID, KVPair.Key);
			Schema_AddUint32(KVPairObject, SCHEMA_MAP_VALUE_FIELD_ID, KVPair.Value);
		}

		return Data;
	}

	FString StaticPath;
	FString OwnerWorkerAttribute;
	SubobjectToOffsetMap SubobjectNameToOffset;
};

FORCEINLINE SubobjectToOffsetMap CreateOffsetMapFromActor(AActor* Actor, FClassInfo* Info)
{
	SubobjectToOffsetMap SubobjectNameToOffset;

	for (auto& SubobjectInfoPair : Info->SubobjectInfo)
	{
		uint32 Offset = SubobjectInfoPair.Key;

		UObjectPropertyBase* Property = SubobjectInfoPair.Value->SubobjectProperty;
		check(Property);

		UObject* Subobject = Property->GetObjectPropertyValue_InContainer(Actor);
		check(Subobject);

		SubobjectNameToOffset.Add(Subobject->GetName(), Offset);
	}

	return SubobjectNameToOffset;
}

}
