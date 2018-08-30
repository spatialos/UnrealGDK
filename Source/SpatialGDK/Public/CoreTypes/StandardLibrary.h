#pragma once

#include "CoreTypes//Component.h"
#include "Utils/SchemaUtils.h"
#include "Platform.h"

#include <improbable/c_worker.h>
#include <improbable/c_schema.h>

#include <map>
#include <string>
#include <vector>

struct Coordinates
{
	double X;
	double Y;
	double Z;

	inline static Coordinates FromFVector(const FVector& Location)
	{
		Coordinates Coords;
		Coords.X = Location.X;
		Coords.Y = Location.Y;
		Coords.Z = Location.Z;

		return Coords;
	}

	inline static FVector ToFVector(const Coordinates& Coords)
	{
		FVector Location;
		Location.X = Coords.X;
		Location.Y = Coords.Y;
		Location.Z = Coords.Z;

		return Location;
	}
};

const Worker_ComponentId POSITION_COMPONENT_ID = 54;

struct Position : Component
{
	static const Worker_ComponentId ComponentId = POSITION_COMPONENT_ID;

	Position() = default;

	Position(const Coordinates& InCoords)
		: Coords(InCoords) {}

	Position(const Worker_ComponentData& Data)
	{
		Schema_Object* ComponentObject = Schema_GetComponentDataFields(Data.schema_type);

		Schema_Object* CoordsObject = Schema_GetObject(ComponentObject, 1);

		Coords.X = Schema_GetDouble(CoordsObject, 1);
		Coords.Y = Schema_GetDouble(CoordsObject, 2);
		Coords.Z = Schema_GetDouble(CoordsObject, 3);
	}

	Worker_ComponentData CreatePositionData()
	{
		Worker_ComponentData Data = {};
		Data.component_id = POSITION_COMPONENT_ID;
		Data.schema_type = Schema_CreateComponentData(POSITION_COMPONENT_ID);
		Schema_Object* ComponentObject = Schema_GetComponentDataFields(Data.schema_type);

		Schema_Object* CoordsObject = Schema_AddObject(ComponentObject, 1);

		Schema_AddDouble(CoordsObject, 1, Coords.X);
		Schema_AddDouble(CoordsObject, 2, Coords.Y);
		Schema_AddDouble(CoordsObject, 3, Coords.Z);

		return Data;
	}

	static Worker_ComponentUpdate CreatePositionUpdate(const Coordinates& Coords)
	{
		Worker_ComponentUpdate ComponentUpdate = {};
		ComponentUpdate.component_id = POSITION_COMPONENT_ID;
		ComponentUpdate.schema_type = Schema_CreateComponentUpdate(POSITION_COMPONENT_ID);
		Schema_Object* ComponentObject = Schema_GetComponentUpdateFields(ComponentUpdate.schema_type);

		Schema_Object* CoordsObject = Schema_AddObject(ComponentObject, 1);

		Schema_AddDouble(CoordsObject, 1, Coords.X);
		Schema_AddDouble(CoordsObject, 2, Coords.Y);
		Schema_AddDouble(CoordsObject, 3, Coords.Z);

		return ComponentUpdate;
	}

	Coordinates Coords;
};


const Worker_ComponentId ENTITY_ACL_COMPONENT_ID = 50;

struct EntityAcl : Component
{
	static const Worker_ComponentId ComponentId = ENTITY_ACL_COMPONENT_ID;

	EntityAcl() = default;

	EntityAcl(const WorkerRequirementSet& InReadAcl, const std::map<Worker_ComponentId, WorkerRequirementSet>& InComponentWriteAcl)
		: ReadAcl(InReadAcl), ComponentWriteAcl(InComponentWriteAcl) {}

	EntityAcl(const Worker_ComponentData& Data)
	{
		Schema_Object* ComponentObject = Schema_GetComponentDataFields(Data.schema_type);

		ReadAcl = Schema_GetWorkerRequirementSet(ComponentObject, 1);

		uint32 KVPairCount = Schema_GetObjectCount(ComponentObject, 2);
		for (uint32 i = 0; i < KVPairCount; i++)
		{
			Schema_Object* KVPairObject = Schema_IndexObject(ComponentObject, 2, i);
			std::uint32_t Key = Schema_GetUint32(KVPairObject, SCHEMA_MAP_KEY_FIELD_ID);
			WorkerRequirementSet Value = Schema_GetWorkerRequirementSet(KVPairObject, SCHEMA_MAP_VALUE_FIELD_ID);

			ComponentWriteAcl.emplace(Key, Value);
		}
	}

	Worker_ComponentData CreateEntityAclData()
	{
		Worker_ComponentData Data = {};
		Data.component_id = ENTITY_ACL_COMPONENT_ID;
		Data.schema_type = Schema_CreateComponentData(ENTITY_ACL_COMPONENT_ID);
		Schema_Object* ComponentObject = Schema_GetComponentDataFields(Data.schema_type);

		Schema_AddWorkerRequirementSet(ComponentObject, 1, ReadAcl);

		for (const auto& KVPair : ComponentWriteAcl)
		{
			Schema_Object* KVPairObject = Schema_AddObject(ComponentObject, 2);
			Schema_AddUint32(KVPairObject, SCHEMA_MAP_KEY_FIELD_ID, KVPair.first);
			Schema_AddWorkerRequirementSet(KVPairObject, SCHEMA_MAP_VALUE_FIELD_ID, KVPair.second);
		}

		return Data;
	}

	WorkerRequirementSet ReadAcl;
	std::map<Worker_ComponentId, WorkerRequirementSet> ComponentWriteAcl;
};

const Worker_ComponentId METADATA_COMPONENT_ID = 53;

struct Metadata : Component
{
	static const Worker_ComponentId ComponentId = METADATA_COMPONENT_ID;

	Metadata() = default;

	Metadata(const std::string& InEntityType)
		: EntityType(InEntityType) {}

	Metadata(const Worker_ComponentData& Data)
	{
		Schema_Object* ComponentObject = Schema_GetComponentDataFields(Data.schema_type);

		EntityType = Schema_GetString(ComponentObject, 1);
	}

	Worker_ComponentData CreateMetadataData()
	{
		Worker_ComponentData Data = {};
		Data.component_id = METADATA_COMPONENT_ID;
		Data.schema_type = Schema_CreateComponentData(METADATA_COMPONENT_ID);
		Schema_Object* ComponentObject = Schema_GetComponentDataFields(Data.schema_type);

		Schema_AddString(ComponentObject, 1, EntityType);

		return Data;
	}

	std::string EntityType;
};

const Worker_ComponentId PERSISTENCE_COMPONENT_ID = 55;

struct Persistence : Component
{
	static const Worker_ComponentId ComponentId = PERSISTENCE_COMPONENT_ID;

	Persistence() = default;
	Persistence(const Worker_ComponentData& Data)
	{
	}

	FORCEINLINE Worker_ComponentData CreatePersistenceData()
	{
		Worker_ComponentData Data = {};
		Data.component_id = PERSISTENCE_COMPONENT_ID;
		Data.schema_type = Schema_CreateComponentData(PERSISTENCE_COMPONENT_ID);

		return Data;
	}
};
