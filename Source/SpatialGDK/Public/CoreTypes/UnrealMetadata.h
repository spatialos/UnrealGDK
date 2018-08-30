#pragma once

#include "CoreTypes/Component.h"
#include "Utils/SchemaUtils.h"
#include "Platform.h"

#include <improbable/c_worker.h>
#include <improbable/c_schema.h>

#include <map>

const Worker_ComponentId UNREAL_METADATA_COMPONENT_ID = 100004;

struct UnrealMetadata : Component
{
	static const Worker_ComponentId ComponentId = UNREAL_METADATA_COMPONENT_ID;

	UnrealMetadata() = default;

	UnrealMetadata(const std::string& InStaticPath, const std::string& InOwnerWorkerId, const std::map<std::string, std::uint32_t>& InSubobjectNameToOffset)
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
			std::string Key = Schema_GetString(KVPairObject, SCHEMA_MAP_KEY_FIELD_ID);
			std::uint32_t Value = Schema_GetUint32(KVPairObject, SCHEMA_MAP_VALUE_FIELD_ID);

			SubobjectNameToOffset.emplace(Key, Value);
		}
	}

	Worker_ComponentData CreateUnrealMetadataData()
	{
		Worker_ComponentData Data = {};
		Data.component_id = UNREAL_METADATA_COMPONENT_ID;
		Data.schema_type = Schema_CreateComponentData(UNREAL_METADATA_COMPONENT_ID);
		Schema_Object* ComponentObject = Schema_GetComponentDataFields(Data.schema_type);

		if (!StaticPath.empty())
		{
			Schema_AddString(ComponentObject, 1, StaticPath);
		}
		if (!OwnerWorkerId.empty())
		{
			Schema_AddString(ComponentObject, 2, OwnerWorkerId);
		}

		for (const auto& KVPair : SubobjectNameToOffset)
		{
			Schema_Object* KVPairObject = Schema_AddObject(ComponentObject, 3);
			Schema_AddString(KVPairObject, SCHEMA_MAP_KEY_FIELD_ID, KVPair.first);
			Schema_AddUint32(KVPairObject, SCHEMA_MAP_VALUE_FIELD_ID, KVPair.second);
		}

		return Data;
	}

	std::string StaticPath;
	std::string OwnerWorkerId;
	std::map<std::string, std::uint32_t> SubobjectNameToOffset;
};

