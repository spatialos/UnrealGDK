// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "improbable/c_schema.h"
#include "improbable/c_worker.h"

#include <map>

FORCEINLINE void Schema_AddString(Schema_Object* Object, Schema_FieldId Id, const std::string& Value)
{
	std::uint32_t StringLength = Value.size();
	std::uint8_t* StringBuffer = Schema_AllocateBuffer(Object, sizeof(char) * StringLength);
	memcpy(StringBuffer, Value.c_str(), sizeof(char) * StringLength);
	Schema_AddBytes(Object, Id, StringBuffer, sizeof(char) * StringLength);
}

FORCEINLINE std::string Schema_IndexString(Schema_Object* Object, Schema_FieldId Id, std::uint32_t Index)
{
	std::uint32_t StringLength = Schema_IndexBytesLength(Object, Id, Index);
	return std::string((const char*)Schema_IndexBytes(Object, Id, Index), StringLength);
}

FORCEINLINE std::string Schema_GetString(Schema_Object* Object, Schema_FieldId Id)
{
	return Schema_IndexString(Object, Id, 0);
}

using WorkerAttributeSet = std::vector<std::string>;
using WorkerRequirementSet = std::vector<WorkerAttributeSet>;

FORCEINLINE void Schema_AddWorkerRequirementSet(Schema_Object* Object, Schema_FieldId Id, const WorkerRequirementSet& Value)
{
	auto RequirementSetObject = Schema_AddObject(Object, Id);
	for (auto& AttributeSet : Value)
	{
		auto AttributeSetObject = Schema_AddObject(RequirementSetObject, 1);

		for (auto& Attribute : AttributeSet)
		{
			Schema_AddString(AttributeSetObject, 1, Attribute);
		}
	}
}

FORCEINLINE WorkerRequirementSet Schema_GetWorkerRequirementSet(Schema_Object* Object, Schema_FieldId Id)
{
	auto RequirementSetObject = Schema_GetObject(Object, Id);

	uint32 AttributeSetCount = Schema_GetObjectCount(RequirementSetObject, 1);
	WorkerRequirementSet RequirementSet;
	RequirementSet.reserve(AttributeSetCount);

	for (uint32 i = 0; i < AttributeSetCount; i++)
	{
		auto AttributeSetObject = Schema_IndexObject(RequirementSetObject, 1, i);

		uint32 AttributeCount = Schema_GetBytesCount(AttributeSetObject, 1);
		WorkerAttributeSet AttributeSet;
		AttributeSet.reserve(AttributeCount);

		for (uint32 j = 0; j < AttributeCount; j++)
		{
			uint32 AttributeLength = Schema_IndexBytesLength(AttributeSetObject, 1, j);
			AttributeSet.emplace_back((const char*)Schema_IndexBytes(AttributeSetObject, 1, j), AttributeLength);
		}

		RequirementSet.push_back(AttributeSet);
	}

	return RequirementSet;
}

struct Position
{
	double X;
	double Y;
	double Z;
};

FORCEINLINE Position LocationToCAPIPosition(const FVector& Location)
{
	return {Location.Y * 0.01, Location.Z * 0.01, Location.X * 0.01};
}

FORCEINLINE FVector CAPIPositionToLocation(const Position& Coords)
{
	return {float(Coords.Z * 100.0), float(Coords.X * 100.0), float(Coords.Y * 100.0)};
}

struct ComponentData
{
	virtual ~ComponentData() {}

	bool bIsDynamic = false;
};

// EntityAcl
const Worker_ComponentId ENTITY_ACL_COMPONENT_ID = 50;

struct EntityAclData : ComponentData
{
	static const Worker_ComponentId ComponentId = ENTITY_ACL_COMPONENT_ID;

	EntityAclData() = default;
	EntityAclData(const WorkerRequirementSet& InReadAcl, const std::map<Worker_ComponentId, WorkerRequirementSet>& InComponentWriteAcl)
		: ReadAcl(InReadAcl), ComponentWriteAcl(InComponentWriteAcl) {}
	EntityAclData(const Worker_ComponentData& Data)
	{
		auto ComponentObject = Schema_GetComponentDataFields(Data.schema_type);

		ReadAcl = Schema_GetWorkerRequirementSet(ComponentObject, 1);

		uint32 KVPairCount = Schema_GetObjectCount(ComponentObject, 2);
		for (uint32 i = 0; i < KVPairCount; i++)
		{
			auto KVPairObject = Schema_IndexObject(ComponentObject, 2, i);
			std::uint32_t Key = Schema_GetUint32(KVPairObject, SCHEMA_MAP_KEY_FIELD_ID);
			WorkerRequirementSet Value = Schema_GetWorkerRequirementSet(KVPairObject, SCHEMA_MAP_VALUE_FIELD_ID);

			ComponentWriteAcl.emplace(Key, Value);
		}
	}

	WorkerRequirementSet ReadAcl;
	std::map<Worker_ComponentId, WorkerRequirementSet> ComponentWriteAcl;
};

FORCEINLINE void CreateEntityAclData(Worker_ComponentData& Data, const EntityAclData& EntityAcl)
{
	Data.component_id = ENTITY_ACL_COMPONENT_ID;
	Data.schema_type = Schema_CreateComponentData(ENTITY_ACL_COMPONENT_ID);
	auto ComponentObject = Schema_GetComponentDataFields(Data.schema_type);

	Schema_AddWorkerRequirementSet(ComponentObject, 1, EntityAcl.ReadAcl);

	for (auto& KVPair : EntityAcl.ComponentWriteAcl)
	{
		auto KVPairObject = Schema_AddObject(ComponentObject, 2);
		Schema_AddUint32(KVPairObject, SCHEMA_MAP_KEY_FIELD_ID, KVPair.first);
		Schema_AddWorkerRequirementSet(KVPairObject, SCHEMA_MAP_VALUE_FIELD_ID, KVPair.second);
	}
}

// Metadata
const Worker_ComponentId METADATA_COMPONENT_ID = 53;

struct MetadataData : ComponentData
{
	static const Worker_ComponentId ComponentId = METADATA_COMPONENT_ID;

	MetadataData() = default;
	MetadataData(const std::string& InEntityType)
		: EntityType(InEntityType) {}
	MetadataData(const Worker_ComponentData& Data)
	{
		auto ComponentObject = Schema_GetComponentDataFields(Data.schema_type);

		EntityType = Schema_GetString(ComponentObject, 1);
	}

	std::string EntityType;
};

FORCEINLINE void CreateMetadataData(Worker_ComponentData& Data, const MetadataData& Metadata)
{
	Data.component_id = METADATA_COMPONENT_ID;
	Data.schema_type = Schema_CreateComponentData(METADATA_COMPONENT_ID);
	auto ComponentObject = Schema_GetComponentDataFields(Data.schema_type);

	Schema_AddString(ComponentObject, 1, Metadata.EntityType);
}

// Position
const Worker_ComponentId POSITION_COMPONENT_ID = 54;

struct PositionData : ComponentData
{
	static const Worker_ComponentId ComponentId = POSITION_COMPONENT_ID;

	PositionData() = default;
	PositionData(const Position& InCoords)
		: Coords(InCoords) {}
	PositionData(const Worker_ComponentData& Data)
	{
		auto ComponentObject = Schema_GetComponentDataFields(Data.schema_type);

		auto CoordsObject = Schema_GetObject(ComponentObject, 1);

		Coords.X = Schema_GetDouble(CoordsObject, 1);
		Coords.Y = Schema_GetDouble(CoordsObject, 2);
		Coords.Z = Schema_GetDouble(CoordsObject, 3);
	}

	Position Coords;
};

FORCEINLINE void CreatePositionData(Worker_ComponentData& Data, const PositionData& Position)
{
	Data.component_id = POSITION_COMPONENT_ID;
	Data.schema_type = Schema_CreateComponentData(POSITION_COMPONENT_ID);
	auto ComponentObject = Schema_GetComponentDataFields(Data.schema_type);

	auto CoordsObject = Schema_AddObject(ComponentObject, 1);

	Schema_AddDouble(CoordsObject, 1, Position.Coords.X);
	Schema_AddDouble(CoordsObject, 2, Position.Coords.Y);
	Schema_AddDouble(CoordsObject, 3, Position.Coords.Z);
}

// Persistence
const Worker_ComponentId PERSISTENCE_COMPONENT_ID = 55;

struct PersistenceData : ComponentData
{
	static const Worker_ComponentId ComponentId = PERSISTENCE_COMPONENT_ID;

	PersistenceData() = default;
	PersistenceData(const Worker_ComponentData& Data)
	{
	}
};

FORCEINLINE void CreatePersistenceData(Worker_ComponentData& Data, const PersistenceData& Persistence)
{
	Data.component_id = PERSISTENCE_COMPONENT_ID;
	Data.schema_type = Schema_CreateComponentData(PERSISTENCE_COMPONENT_ID);
}

// UnrealMetadata
const Worker_ComponentId UNREAL_METADATA_COMPONENT_ID = 100004;

struct UnrealMetadataData : ComponentData
{
	static const Worker_ComponentId ComponentId = UNREAL_METADATA_COMPONENT_ID;

	UnrealMetadataData() = default;
	UnrealMetadataData(const std::string& InStaticPath, const std::string& InOwnerWorkerId, const std::map<std::string, std::uint32_t>& InSubobjectNameToOffset)
		: StaticPath(InStaticPath), OwnerWorkerId(InOwnerWorkerId), SubobjectNameToOffset(InSubobjectNameToOffset) {}
	UnrealMetadataData(const Worker_ComponentData& Data)
	{
		auto ComponentObject = Schema_GetComponentDataFields(Data.schema_type);

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
			auto KVPairObject = Schema_IndexObject(ComponentObject, 3, i);
			std::string Key = Schema_GetString(KVPairObject, SCHEMA_MAP_KEY_FIELD_ID);
			std::uint32_t Value = Schema_GetUint32(KVPairObject, SCHEMA_MAP_VALUE_FIELD_ID);

			SubobjectNameToOffset.emplace(Key, Value);
		}
	}

	std::string StaticPath;
	std::string OwnerWorkerId;
	std::map<std::string, std::uint32_t> SubobjectNameToOffset;
};

FORCEINLINE void CreateUnrealMetadataData(Worker_ComponentData& Data, const UnrealMetadataData& UnrealMetadata)
{
	Data.component_id = UNREAL_METADATA_COMPONENT_ID;
	Data.schema_type = Schema_CreateComponentData(UNREAL_METADATA_COMPONENT_ID);
	auto ComponentObject = Schema_GetComponentDataFields(Data.schema_type);

	if (!UnrealMetadata.StaticPath.empty())
	{
		Schema_AddString(ComponentObject, 1, UnrealMetadata.StaticPath);
	}
	if (!UnrealMetadata.OwnerWorkerId.empty())
	{
		Schema_AddString(ComponentObject, 2, UnrealMetadata.OwnerWorkerId);
	}

	for (auto& KVPair : UnrealMetadata.SubobjectNameToOffset)
	{
		auto KVPairObject = Schema_AddObject(ComponentObject, 3);
		Schema_AddString(KVPairObject, SCHEMA_MAP_KEY_FIELD_ID, KVPair.first);
		Schema_AddUint32(KVPairObject, SCHEMA_MAP_VALUE_FIELD_ID, KVPair.second);
	}
}

// Dynamic (any Unreal Rep component)
struct DynamicData : ComponentData
{
	DynamicData()
	{
		bIsDynamic = true;
	}
	DynamicData(const Worker_ComponentData& InData)
		: Data(Worker_AcquireComponentData(&InData))
	{
		bIsDynamic = true;
	}

	~DynamicData()
	{
		Worker_ReleaseComponentData(Data);
	}

	Worker_ComponentData* Data;
};

void CreateDynamicData(Worker_ComponentData& Data, Worker_ComponentId ComponentId, const struct FPropertyChangeState& InitialChanges, class USpatialPackageMapClient* PackageMap);
