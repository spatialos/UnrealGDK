// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "Platform.h"

#include "Schema/Component.h"
#include "Utils/SchemaUtils.h"

#include <improbable/c_schema.h>
#include <improbable/c_worker.h>

using WriteAclMap = TMap<Worker_ComponentId, WorkerRequirementSet>;

const Worker_ComponentId ENTITY_ACL_COMPONENT_ID = 50;
const Worker_ComponentId METADATA_COMPONENT_ID = 53;
const Worker_ComponentId POSITION_COMPONENT_ID = 54;
const Worker_ComponentId PERSISTENCE_COMPONENT_ID = 55;

namespace improbable
{
	struct Coordinates
	{
		double X;
		double Y;
		double Z;

		inline static Coordinates FromFVector(const FVector& Location)
		{
			Coordinates Coords;
			Coords.X = 0.01 * Location.Y;
			Coords.Y = 0.01 * Location.Z;
			Coords.Z = 0.01 * Location.X;

			return Coords;
		}

		inline static FVector ToFVector(const Coordinates& Coords)
		{
			FVector Location;
			Location.X = 100.0 * Coords.Z;
			Location.Y = 100.0 * Coords.X;
			Location.Z = 100.0 * Coords.Y;

			return Location;
		}
	};

	struct EntityAcl : improbable::Component
	{
		static const Worker_ComponentId ComponentId = ENTITY_ACL_COMPONENT_ID;

		EntityAcl() = default;

		EntityAcl(const WorkerRequirementSet& InReadAcl, const WriteAclMap& InComponentWriteAcl)
			: ReadAcl(InReadAcl), ComponentWriteAcl(InComponentWriteAcl) {}

		EntityAcl(const Worker_ComponentData& Data)
		{
			Schema_Object* ComponentObject = Schema_GetComponentDataFields(Data.schema_type);

			ReadAcl = Schema_GetWorkerRequirementSet(ComponentObject, 1);

			uint32 KVPairCount = Schema_GetObjectCount(ComponentObject, 2);
			for (uint32 i = 0; i < KVPairCount; i++)
			{
				Schema_Object* KVPairObject = Schema_IndexObject(ComponentObject, 2, i);
				uint32 Key = Schema_GetUint32(KVPairObject, SCHEMA_MAP_KEY_FIELD_ID);
				WorkerRequirementSet Value = Schema_GetWorkerRequirementSet(KVPairObject, SCHEMA_MAP_VALUE_FIELD_ID);

				ComponentWriteAcl.Add(Key, Value);
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
				Schema_AddUint32(KVPairObject, SCHEMA_MAP_KEY_FIELD_ID, KVPair.Key);
				Schema_AddWorkerRequirementSet(KVPairObject, SCHEMA_MAP_VALUE_FIELD_ID, KVPair.Value);
			}

			return Data;
		}

		WorkerRequirementSet ReadAcl;
		WriteAclMap ComponentWriteAcl;
	};

	struct Metadata : improbable::Component
	{
		static const Worker_ComponentId ComponentId = METADATA_COMPONENT_ID;

		Metadata() = default;

		Metadata(const FString& InEntityType)
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

		FString EntityType;
	};

	struct Position : improbable::Component
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

	struct Persistence : improbable::Component
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
}
