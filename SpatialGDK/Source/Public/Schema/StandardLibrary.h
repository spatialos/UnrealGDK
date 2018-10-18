// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "Math/Vector.h"
#include "Platform.h"

#include "improbable/UnrealObjectRef.h"
#include "Schema/Component.h"
#include "SpatialConstants.h"
#include "UObject/UObjectGlobals.h"
#include "UObject/Package.h"
#include "Utils/SchemaUtils.h"

#include <WorkerSDK/improbable/c_schema.h>
#include <WorkerSDK/improbable/c_worker.h>

using WriteAclMap = TMap<Worker_ComponentId, WorkerRequirementSet>;
//using ComponentInterestMap = TMap<uint32, 

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

using EdgeLength = Coordinates;

inline void AddCoordinateToSchema(Schema_Object* Object, Schema_FieldId Id, const Coordinates& Coordinate)
{
	Schema_Object* CoordsObject = Schema_AddObject(Object, Id);

	Schema_AddDouble(CoordsObject, 1, Coordinate.X);
	Schema_AddDouble(CoordsObject, 2, Coordinate.Y);
	Schema_AddDouble(CoordsObject, 3, Coordinate.Z);
}

inline Coordinates GetCoordinateFromSchema(Schema_Object* Object, Schema_FieldId Id)
{
	Schema_Object* CoordsObject = Schema_GetObject(Object, Id);

	Coordinates Coordinate;
	Coordinate.X = Schema_GetDouble(CoordsObject, 1);
	Coordinate.Y = Schema_GetDouble(CoordsObject, 2);
	Coordinate.Z = Schema_GetDouble(CoordsObject, 3);

	return Coordinate;
}

struct ComponentInterest
{
	struct SphereConstraint
	{
		Coordinates Center;
		double Radius;
	};

	struct CylinderConstraint 
	{
		Coordinates Center;
		double Radius;
	};

	struct BoxConstraint
	{
		Coordinates Center;
		EdgeLength EdgeLength;
	};

	struct RelativeSphereConstraint
	{
		double Radius;
	};

	struct RelativeCylinderConstraint
	{
		double Radius;
	};

	struct RelativeBoxConstraint
	{
		EdgeLength EdgeLength;
	};

	struct QueryConstraint
	{
		TSchemaOption<SphereConstraint> SphereConstraint;
		TSchemaOption<CylinderConstraint> CylinderConstraint;
		TSchemaOption<BoxConstraint> BoxConstraint;
		TSchemaOption<RelativeSphereConstraint> RelativeSphereConstraint;
		TSchemaOption<RelativeCylinderConstraint> RelativeCylinderConstraint;
		TSchemaOption<RelativeBoxConstraint> RelativeBoxConstraint;
		TSchemaOption<int64> EntityIdConstraint;
		TSchemaOption<uint32> ComponentConstraint;
		TArray<QueryConstraint> AndConstraint;
		TArray<QueryConstraint> OrConstraint;
	};

	struct Query
	{
		QueryConstraint Constraint;
		TSchemaOption<bool> FullSnapshotResult;
		TArray<uint32> ResultComponentId;
		TSchemaOption<float> Frequency;
	};

	TArray<Query> Queries;
};

inline void AddQueryConstraintToSchema(Schema_Object* Object, Schema_FieldId Id, const ComponentInterest::QueryConstraint& Constraint)
{
	Schema_Object* QueryConstraintObject = Schema_AddObject(Object, Id);

	//option<SphereConstraint> sphere_constraint = 1;
	if (Constraint.SphereConstraint)
	{
		Schema_Object* SphereConstraintObject = Schema_AddObject(QueryConstraintObject, 1);

		AddCoordinateToSchema(SphereConstraintObject, 1, Constraint.SphereConstraint->Center);
		Schema_AddDouble(SphereConstraintObject, 2, Constraint.SphereConstraint->Radius);
	}

	//option<CylinderConstraint> cylinder_constraint = 2;
	if (Constraint.CylinderConstraint)
	{
		Schema_Object* CylinderConstraintObject = Schema_AddObject(QueryConstraintObject, 2);

		AddCoordinateToSchema(CylinderConstraintObject, 1, Constraint.CylinderConstraint->Center);
		Schema_AddDouble(CylinderConstraintObject, 2, Constraint.CylinderConstraint->Radius);
	}
	//option<BoxConstraint> box_constraint = 3;
	if (Constraint.BoxConstraint)
	{
		Schema_Object* BoxConstraintObject = Schema_AddObject(QueryConstraintObject, 3);
		AddCoordinateToSchema(BoxConstraintObject, 1, Constraint.BoxConstraint->Center);
		AddCoordinateToSchema(BoxConstraintObject, 2, Constraint.BoxConstraint->EdgeLength);
	}
	//option<RelativeSphereConstraint> relative_sphere_constraint = 4;
	if (Constraint.RelativeSphereConstraint)
	{
		Schema_Object* RelativeSphereConstraintObject = Schema_AddObject(QueryConstraintObject, 4);

		Schema_AddDouble(RelativeSphereConstraintObject, 1, Constraint.RelativeSphereConstraint->Radius);
	}

	//option<RelativeCylinderConstraint> relative_cylinder_constraint = 5;
	if (Constraint.RelativeCylinderConstraint)
	{
		Schema_Object* RelativeCylinderConstraintObject = Schema_AddObject(QueryConstraintObject, 5);
		Schema_AddDouble(RelativeCylinderConstraintObject, 1, Constraint.RelativeCylinderConstraint->Radius);
	}
	//option<RelativeBoxConstraint> relative_box_constraint = 6;
	if (Constraint.RelativeBoxConstraint)
	{
		Schema_Object* RelativeBoxConstraintObject = Schema_AddObject(QueryConstraintObject, 6);
		AddCoordinateToSchema(RelativeBoxConstraintObject, 1, Constraint.RelativeBoxConstraint->EdgeLength);
	}
	//option<int64> entity_id_constraint = 7;
	if (Constraint.EntityIdConstraint)
	{
		Schema_Object* EntityIdConstraintObject = Schema_AddObject(QueryConstraintObject, 7);
		Schema_AddInt64(EntityIdConstraintObject, 1, *Constraint.EntityIdConstraint);
	}

	//option<uint32> component_constraint = 8;
	if (Constraint.ComponentConstraint)
	{
		Schema_Object* ComponentConstraintObject = Schema_AddObject(QueryConstraintObject, 8);
		Schema_AddUint32(ComponentConstraintObject, 1, *Constraint.ComponentConstraint);
	}

	//list<QueryConstraint> and_constraint = 9;
	if (Constraint.AndConstraint.Num() > 0)
	{
		Schema_Object* AndConstraintObject = Schema_AddObject(QueryConstraintObject, 9);

		for (const ComponentInterest::QueryConstraint& AndConstraintEntry : Constraint.AndConstraint)
		{
			AddQueryConstraintToSchema(AndConstraintObject, 1, AndConstraintEntry);
		}
	}

	//list<QueryConstraint> or_constraint = 10;
	if (Constraint.OrConstraint.Num() > 0)
	{
		Schema_Object* OrConstraintObject = Schema_AddObject(QueryConstraintObject, 10);

		for (const ComponentInterest::QueryConstraint& OrConstraintEntry : Constraint.OrConstraint)
		{
			AddQueryConstraintToSchema(OrConstraintObject, 1, OrConstraintEntry);
		}
	}
}

inline void AddQueryToSchema(Schema_Object* Object, Schema_FieldId Id, const ComponentInterest::Query& Query)
{
	Schema_Object* QueryObject = Schema_AddObject(Object, Id);

	// Write the query constraint
	AddQueryConstraintToSchema(QueryObject, 1, Query.Constraint);

	//write the snapshot option
	if (Query.FullSnapshotResult)
	{
		Schema_Object* FullSnapshotResultObject = Schema_AddObject(QueryObject, 2);
		Schema_AddBool(FullSnapshotResultObject, 1, *Query.FullSnapshotResult);
	}

	// write the resulting component if
	Schema_Object* ResultComponentIdObject = Schema_AddObject(QueryObject, 3);

	for (uint32 ComponentId : Query.ResultComponentId)
	{
		Schema_AddUint32(ResultComponentIdObject, 1, ComponentId);
	}
	// write option for frequency
	if (Query.Frequency)
	{
		Schema_Object* FrequencyObject = Schema_AddObject(QueryObject, 4);
		Schema_AddFloat(FrequencyObject, 1, *Query.Frequency);
	}
}

struct EntityAcl : Component
{
	static const Worker_ComponentId ComponentId = SpatialConstants::ENTITY_ACL_COMPONENT_ID;

	EntityAcl() = default;

	EntityAcl(const WorkerRequirementSet& InReadAcl, const WriteAclMap& InComponentWriteAcl)
		: ReadAcl(InReadAcl), ComponentWriteAcl(InComponentWriteAcl) {}

	EntityAcl(const Worker_ComponentData& Data)
	{
		Schema_Object* ComponentObject = Schema_GetComponentDataFields(Data.schema_type);

		ReadAcl = GetWorkerRequirementSetFromSchema(ComponentObject, 1);

		uint32 KVPairCount = Schema_GetObjectCount(ComponentObject, 2);
		for (uint32 i = 0; i < KVPairCount; i++)
		{
			Schema_Object* KVPairObject = Schema_IndexObject(ComponentObject, 2, i);
			uint32 Key = Schema_GetUint32(KVPairObject, SCHEMA_MAP_KEY_FIELD_ID);
			WorkerRequirementSet Value = GetWorkerRequirementSetFromSchema(KVPairObject, SCHEMA_MAP_VALUE_FIELD_ID);

			ComponentWriteAcl.Add(Key, Value);
		}
	}

	void ApplyComponentUpdate(const Worker_ComponentUpdate& Update)
	{
		Schema_Object* ComponentObject = Schema_GetComponentUpdateFields(Update.schema_type);

		if (Schema_GetObjectCount(ComponentObject, 1) > 0)
		{
			ReadAcl = GetWorkerRequirementSetFromSchema(ComponentObject, 1);
		}

		// This is never emptied, so does not need an additional check for cleared fields
		uint32 KVPairCount = Schema_GetObjectCount(ComponentObject, 2);
		if (KVPairCount > 0)
		{
			ComponentWriteAcl.Empty();
			for (uint32 i = 0; i < KVPairCount; i++)
			{
				Schema_Object* KVPairObject = Schema_IndexObject(ComponentObject, 2, i);
				uint32 Key = Schema_GetUint32(KVPairObject, SCHEMA_MAP_KEY_FIELD_ID);
				WorkerRequirementSet Value = GetWorkerRequirementSetFromSchema(KVPairObject, SCHEMA_MAP_VALUE_FIELD_ID);

				ComponentWriteAcl.Add(Key, Value);
			}
		}
	}

	Worker_ComponentData CreateEntityAclData()
	{
		Worker_ComponentData Data = {};
		Data.component_id = ComponentId;
		Data.schema_type = Schema_CreateComponentData(ComponentId);
		Schema_Object* ComponentObject = Schema_GetComponentDataFields(Data.schema_type);

		AddWorkerRequirementSetToSchema(ComponentObject, 1, ReadAcl);

		for (const auto& KVPair : ComponentWriteAcl)
		{
			Schema_Object* KVPairObject = Schema_AddObject(ComponentObject, 2);
			Schema_AddUint32(KVPairObject, SCHEMA_MAP_KEY_FIELD_ID, KVPair.Key);
			AddWorkerRequirementSetToSchema(KVPairObject, SCHEMA_MAP_VALUE_FIELD_ID, KVPair.Value);
		}

		return Data;
	}

	Worker_ComponentUpdate CreateEntityAclUpdate()
	{
		Worker_ComponentUpdate ComponentUpdate = {};
		ComponentUpdate.component_id = ComponentId;
		ComponentUpdate.schema_type = Schema_CreateComponentUpdate(ComponentId);
		Schema_Object* ComponentObject = Schema_GetComponentUpdateFields(ComponentUpdate.schema_type);

		AddWorkerRequirementSetToSchema(ComponentObject, 1, ReadAcl);

		for (const auto& KVPair : ComponentWriteAcl)
		{
			Schema_Object* KVPairObject = Schema_AddObject(ComponentObject, 2);
			Schema_AddUint32(KVPairObject, SCHEMA_MAP_KEY_FIELD_ID, KVPair.Key);
			AddWorkerRequirementSetToSchema(KVPairObject, SCHEMA_MAP_VALUE_FIELD_ID, KVPair.Value);
		}

		return ComponentUpdate;
	}

	WorkerRequirementSet ReadAcl;
	WriteAclMap ComponentWriteAcl;
};

struct Metadata : Component
{
	static const Worker_ComponentId ComponentId = SpatialConstants::METADATA_COMPONENT_ID;

	Metadata() = default;

	Metadata(const FString& InEntityType)
		: EntityType(InEntityType) {}

	Metadata(const Worker_ComponentData& Data)
	{
		Schema_Object* ComponentObject = Schema_GetComponentDataFields(Data.schema_type);

		EntityType = GetStringFromSchema(ComponentObject, 1);
	}

	Worker_ComponentData CreateMetadataData()
	{
		Worker_ComponentData Data = {};
		Data.component_id = ComponentId;
		Data.schema_type = Schema_CreateComponentData(ComponentId);
		Schema_Object* ComponentObject = Schema_GetComponentDataFields(Data.schema_type);

		AddStringToSchema(ComponentObject, 1, EntityType);

		return Data;
	}

	FString EntityType;
};

struct Position : Component
{
	static const Worker_ComponentId ComponentId = SpatialConstants::POSITION_COMPONENT_ID;

	Position() = default;

	Position(const Coordinates& InCoords)
		: Coords(InCoords) {}

	Position(const Worker_ComponentData& Data)
	{
		Schema_Object* ComponentObject = Schema_GetComponentDataFields(Data.schema_type);

		Coords = GetCoordinateFromSchema(ComponentObject, 1);
	}

	Worker_ComponentData CreatePositionData()
	{
		Worker_ComponentData Data = {};
		Data.component_id = ComponentId;
		Data.schema_type = Schema_CreateComponentData(ComponentId);
		Schema_Object* ComponentObject = Schema_GetComponentDataFields(Data.schema_type);

		AddCoordinateToSchema(ComponentObject, 1, Coords);

		return Data;
	}

	static Worker_ComponentUpdate CreatePositionUpdate(const Coordinates& Coords)
	{
		Worker_ComponentUpdate ComponentUpdate = {};
		ComponentUpdate.component_id = ComponentId;
		ComponentUpdate.schema_type = Schema_CreateComponentUpdate(ComponentId);
		Schema_Object* ComponentObject = Schema_GetComponentUpdateFields(ComponentUpdate.schema_type);

		AddCoordinateToSchema(ComponentObject, 1, Coords);

		return ComponentUpdate;
	}

	void ApplyComponentUpdate(const Worker_ComponentUpdate& Update)
	{
		Schema_Object* ComponentObject = Schema_GetComponentUpdateFields(Update.schema_type);
		Schema_Object* CoordsObject = Schema_GetObject(ComponentObject, 1);
		Coords.X = Schema_GetDouble(CoordsObject, 1);
		Coords.Y = Schema_GetDouble(CoordsObject, 2);
		Coords.Z = Schema_GetDouble(CoordsObject, 3);
	}

	Coordinates Coords;
};

struct Persistence : Component
{
	static const Worker_ComponentId ComponentId = SpatialConstants::PERSISTENCE_COMPONENT_ID;

	Persistence() = default;
	Persistence(const Worker_ComponentData& Data)
	{
	}

	FORCEINLINE Worker_ComponentData CreatePersistenceData()
	{
		Worker_ComponentData Data = {};
		Data.component_id = ComponentId;
		Data.schema_type = Schema_CreateComponentData(ComponentId);

		return Data;
	}
};

struct Interest : Component
{
	static const Worker_ComponentId ComponentId = SpatialConstants::INTEREST_COMPONENT_ID;

	Interest() = default;

	Interest(const Worker_ComponentData& Data)
	{
		Schema_Object* ComponentObject = Schema_GetComponentDataFields(Data.schema_type);

		//TODO TODO TODO
	}

	Worker_ComponentData CreateInterestData()
	{
		Worker_ComponentData Data = {};
		Data.component_id = ComponentId;
		Data.schema_type = Schema_CreateComponentData(ComponentId);
		Schema_Object* ComponentObject = Schema_GetComponentDataFields(Data.schema_type);

		for (const auto& KVPair : ComponentInterest)
		{
			Schema_Object* KVPairObject = Schema_AddObject(ComponentObject, 1);
			Schema_AddUint32(KVPairObject, SCHEMA_MAP_KEY_FIELD_ID, KVPair.Key);
			AddComponentInterestToSchema(KVPairObject, SCHEMA_MAP_VALUE_FIELD_ID, KVPair.Value);
		}

		return Data;
	}

	Worker_ComponentUpdate CreateInterestUpdate()
	{
		Worker_ComponentUpdate ComponentUpdate = {};
		ComponentUpdate.component_id = ComponentId;
		ComponentUpdate.schema_type = Schema_CreateComponentUpdate(ComponentId);
		Schema_Object* ComponentObject = Schema_GetComponentUpdateFields(ComponentUpdate.schema_type);

		for (const auto& KVPair : ComponentInterest)
		{
			Schema_Object* KVPairObject = Schema_AddObject(ComponentObject, 2);
			Schema_AddUint32(KVPairObject, SCHEMA_MAP_KEY_FIELD_ID, KVPair.Key);
			AddComponentInterestToSchema(KVPairObject, SCHEMA_MAP_VALUE_FIELD_ID, KVPair.Value);
		}

		return ComponentUpdate;
	}

	inline void AddComponentInterestToSchema(Schema_Object* Object, Schema_FieldId Id, const ComponentInterest& Value)
	{
		Schema_Object* ComponentInterestObject = Schema_AddObject(Object, Id);

		for (const ComponentInterest::Query& QueryEntry : Value.Queries)
		{
			AddQueryToSchema(ComponentInterestObject, 1, QueryEntry);
		}
	}

	inline WorkerRequirementSet GetComponentInterestFromSchema(Schema_Object* Object, Schema_FieldId Id)
	{

	}

	TMap<uint32, ComponentInterest> ComponentInterest;
};

}
