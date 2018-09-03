#pragma once

#include "Schema/Component.h"
#include "Utils/SchemaUtils.h"
#include "Platform.h"

#include <improbable/c_worker.h>
#include <improbable/c_schema.h>

using SubobjectToOffsetMap = TMap<FString, uint32>;

const Worker_ComponentId UNREAL_METADATA_COMPONENT_ID = 100004;

struct UnrealMetadata : Component
{
	static const Worker_ComponentId ComponentId = UNREAL_METADATA_COMPONENT_ID;

	UnrealMetadata() = default;

	UnrealMetadata(const FString& InStaticPath, const FString& InOwnerWorkerId, const SubobjectToOffsetMap& InSubobjectNameToOffset)
		: StaticPath(InStaticPath), OwnerWorkerId(InOwnerWorkerId), SubobjectNameToOffset(InSubobjectNameToOffset) {}

	UnrealMetadata(const Worker_ComponentData& Data)
	{
		Schema_Object* ComponentObject = Schema_GetComponentDataFields(Data.schema_type);

		if (Schema_GetBytesCount(ComponentObject, 1) > 0)
		{
			StaticPath = Schema_GetString(ComponentObject, 1);
		}

		if (Schema_GetBytesCount(ComponentObject, 2) > 0)
		{
			OwnerWorkerId = Schema_GetString(ComponentObject, 2);
		}

		uint32 KVPairCount = Schema_GetObjectCount(ComponentObject, 3);
		for (uint32 i = 0; i < KVPairCount; i++)
		{
			Schema_Object* KVPairObject = Schema_IndexObject(ComponentObject, 3, i);
			FString Key = Schema_GetString(KVPairObject, SCHEMA_MAP_KEY_FIELD_ID);
			uint32 Value = Schema_GetUint32(KVPairObject, SCHEMA_MAP_VALUE_FIELD_ID);

			SubobjectNameToOffset.Add(Key, Value);
		}
	}

	Worker_ComponentData CreateUnrealMetadataData()
	{
		Worker_ComponentData Data = {};
		Data.component_id = UNREAL_METADATA_COMPONENT_ID;
		Data.schema_type = Schema_CreateComponentData(UNREAL_METADATA_COMPONENT_ID);
		Schema_Object* ComponentObject = Schema_GetComponentDataFields(Data.schema_type);

		if (!StaticPath.IsEmpty())
		{
			Schema_AddString(ComponentObject, 1, StaticPath);
		}
		if (!OwnerWorkerId.IsEmpty())
		{
			Schema_AddString(ComponentObject, 2, OwnerWorkerId);
		}

		for (const auto& KVPair : SubobjectNameToOffset)
		{
			Schema_Object* KVPairObject = Schema_AddObject(ComponentObject, 3);
			Schema_AddString(KVPairObject, SCHEMA_MAP_KEY_FIELD_ID, KVPair.Key);
			Schema_AddUint32(KVPairObject, SCHEMA_MAP_VALUE_FIELD_ID, KVPair.Value);
		}

		return Data;
	}

	FString StaticPath;
	FString OwnerWorkerId;
	SubobjectToOffsetMap SubobjectNameToOffset;
};

