// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "Math/Vector.h"

#include "Schema/Component.h"
#include "Schema/UnrealObjectRef.h"
#include "SpatialCommonTypes.h"
#include "UObject/UObjectGlobals.h"
#include "UObject/Package.h"
#include "Utils/SchemaUtils.h"

#include <WorkerSDK/improbable/c_schema.h>
#include <WorkerSDK/improbable/c_worker.h>

namespace SpatialGDK
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

	inline bool operator!=(const Coordinates& Right) const
	{
		return X != Right.X || Y != Right.Y || Z != Right.Z;
	}
};

static const Coordinates DeploymentOrigin{ 0, 0, 0 };

inline void AddCoordinateToSchema(Schema_Object* Object, Schema_FieldId Id, const Coordinates& Coordinate)
{
	Schema_Object* CoordsObject = Schema_AddObject(Object, Id);

	Schema_AddDouble(CoordsObject, 1, Coordinate.X);
	Schema_AddDouble(CoordsObject, 2, Coordinate.Y);
	Schema_AddDouble(CoordsObject, 3, Coordinate.Z);
}

inline Coordinates IndexCoordinateFromSchema(Schema_Object* Object, Schema_FieldId Id, uint32 Index)
{
	Schema_Object* CoordsObject = Schema_IndexObject(Object, Id, Index);

	Coordinates Coordinate;
	Coordinate.X = Schema_GetDouble(CoordsObject, 1);
	Coordinate.Y = Schema_GetDouble(CoordsObject, 2);
	Coordinate.Z = Schema_GetDouble(CoordsObject, 3);

	return Coordinate;
}

inline Coordinates GetCoordinateFromSchema(Schema_Object* Object, Schema_FieldId Id)
{
	return IndexCoordinateFromSchema(Object, Id, 0);
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
		Data.schema_type = Schema_CreateComponentData();
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
		ComponentUpdate.schema_type = Schema_CreateComponentUpdate();
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
		Data.schema_type = Schema_CreateComponentData();
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
		Data.schema_type = Schema_CreateComponentData();
		Schema_Object* ComponentObject = Schema_GetComponentDataFields(Data.schema_type);

		AddCoordinateToSchema(ComponentObject, 1, Coords);

		return Data;
	}

	static Worker_ComponentUpdate CreatePositionUpdate(const Coordinates& Coords)
	{
		Worker_ComponentUpdate ComponentUpdate = {};
		ComponentUpdate.component_id = ComponentId;
		ComponentUpdate.schema_type = Schema_CreateComponentUpdate();
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
		Data.schema_type = Schema_CreateComponentData();

		return Data;
	}
};

struct Connection
{
	enum ConnectionStatus
	{
		UNKNOWN = 0,
		// The worker requested a bridge from the receptionist, but the bridge has not yet had the worker connect to it.
		AWAITING_WORKER_CONNECTION = 1,
		// The worker is connected to the bridge as normal.
		CONNECTED = 2,
		// A worker was connected at one point, but is no longer connected. Currently, reconnecting is unsupported.
		DISCONNECTED = 3
	};

	void ReadConnectionData(Schema_Object* Object)
	{
		Status = (ConnectionStatus)Schema_GetUint32(Object, 1);
		DataLatencyMs = Schema_GetUint32(Object, 2);
		ConnectedSinceUtc = Schema_GetUint64(Object, 3);
	}

	ConnectionStatus Status;
	uint32 DataLatencyMs;
	uint64 ConnectedSinceUtc;
};

struct Worker : Component
{
	static const Worker_ComponentId ComponentId = SpatialConstants::WORKER_COMPONENT_ID;

	Worker() = default;
	Worker(const Worker_ComponentData& Data)
	{
		Schema_Object* ComponentObject = Schema_GetComponentDataFields(Data.schema_type);

		WorkerId = GetStringFromSchema(ComponentObject, 1);
		WorkerType = GetStringFromSchema(ComponentObject, 2);
		Connection.ReadConnectionData(Schema_GetObject(ComponentObject, 3));
	}

	FString WorkerId;
	FString WorkerType;
	Connection Connection;

	static Worker_CommandRequest CreateClaimPartitionRequest(Worker_PartitionId PartitionId)
	{
		Worker_CommandRequest CommandRequest = {};
		CommandRequest.component_id = SpatialConstants::WORKER_COMPONENT_ID;
		CommandRequest.command_index = SpatialConstants::WORKER_CLAIM_PARTITION_COMMAND_ID;
		CommandRequest.schema_type = Schema_CreateCommandRequest();
		Schema_Object* RequestObject = Schema_GetCommandRequestObject(CommandRequest.schema_type);

		Schema_AddInt64(RequestObject, 1, PartitionId);

		return CommandRequest;
	}
};

struct AuthorityDelegation : Component
{
	static const Worker_ComponentId ComponentId = SpatialConstants::AUTHORITY_DELEGATION_COMPONENT_ID;

	AuthorityDelegation() = default;

	AuthorityDelegation(AuthorityDelegationMap InDelegation)
		: Delegations(InDelegation) {}

	AuthorityDelegation(const Worker_ComponentData& Data)
	{
		Schema_Object* ComponentObject = Schema_GetComponentDataFields(Data.schema_type);

		const uint32 DelegationCount = Schema_GetObjectCount(ComponentObject, 1);
		for (uint32 i = 0; i < DelegationCount; i++)
		{
			Schema_Object* Delegation = Schema_IndexObject(ComponentObject, 1, i);
			Worker_ComponentId AssignedComponentId = Schema_GetUint32(Delegation, SCHEMA_MAP_KEY_FIELD_ID);
			Worker_PartitionId PartitionId = Schema_GetUint64(Delegation, SCHEMA_MAP_VALUE_FIELD_ID);

			Delegations.Add(AssignedComponentId, PartitionId);
		}
	}

	void ApplyComponentUpdate(const Worker_ComponentUpdate& Update)
	{
		Schema_Object* ComponentObject = Schema_GetComponentUpdateFields(Update.schema_type);

		// This is never emptied, so does not need an additional check for cleared fields
		const uint32 DelegationCount = Schema_GetObjectCount(ComponentObject, 1);
		if (DelegationCount > 0)
		{
			Delegations.Empty();
			for (uint32 i = 0; i < DelegationCount; i++)
			{
				Schema_Object* Delegation = Schema_IndexObject(ComponentObject, 1, i);
				Worker_ComponentId AssignedComponentId = Schema_GetUint32(Delegation, SCHEMA_MAP_KEY_FIELD_ID);
				Worker_PartitionId PartitionId = Schema_GetUint64(Delegation, SCHEMA_MAP_VALUE_FIELD_ID);

				Delegations.Add(AssignedComponentId, PartitionId);
			}
		}
	}

	Worker_ComponentData CreateAuthorityDelegationData()
	{
		Worker_ComponentData Data = {};
		Data.component_id = ComponentId;
		Data.schema_type = Schema_CreateComponentData();
		Schema_Object* ComponentObject = Schema_GetComponentDataFields(Data.schema_type);

		for (const auto& KVPair : Delegations)
		{
			Schema_Object* KVPairObject = Schema_AddObject(ComponentObject, 1);
			Schema_AddUint32(KVPairObject, SCHEMA_MAP_KEY_FIELD_ID, KVPair.Key);
			Schema_AddUint64(KVPairObject, SCHEMA_MAP_VALUE_FIELD_ID, KVPair.Value);
		}

		return Data;
	}

	Worker_ComponentUpdate CreateAuthorityDelegationUpdate()
	{
		Worker_ComponentUpdate ComponentUpdate = {};
		ComponentUpdate.component_id = ComponentId;
		ComponentUpdate.schema_type = Schema_CreateComponentUpdate();
		Schema_Object* ComponentObject = Schema_GetComponentUpdateFields(ComponentUpdate.schema_type);

		for (const auto& KVPair : Delegations)
		{
			Schema_Object* KVPairObject = Schema_AddObject(ComponentObject, 1);
			Schema_AddUint32(KVPairObject, SCHEMA_MAP_KEY_FIELD_ID, KVPair.Key);
			Schema_AddUint64(KVPairObject, SCHEMA_MAP_VALUE_FIELD_ID, KVPair.Value);
		}

		return ComponentUpdate;
	}

	AuthorityDelegationMap Delegations;
};

} // namespace SpatialGDK
