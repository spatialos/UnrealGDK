// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "Schema/Component.h"
#include "SpatialCommonTypes.h"
#include "Utils/SchemaUtils.h"

#include "WorkerSDK/improbable/c_worker.h"

namespace SpatialGDK
{
inline void ApplyCollection(const Schema_Object& Object, TSet<Worker_EntityId_Key>& EntityIds, Schema_FieldId FieldId)
{
	const uint32 EntityIdCount = Schema_GetEntityIdCount(&Object, FieldId);
	EntityIds.Empty(EntityIdCount);
	for (uint32 EntityIdIndex = 0; EntityIdIndex < EntityIdCount; ++EntityIdIndex)
	{
		EntityIds.Emplace(Schema_IndexEntityId(&Object, FieldId, EntityIdIndex));
	}
}

struct FWorkingSetState
{
	static constexpr Schema_FieldId WORKING_SET_STATE_EPOCH_FIELD_ID = 1;
	static constexpr Schema_FieldId WORKING_SET_STATE_MEMBER_ENTITIES_FIELD_ID = 2;
	static constexpr Schema_FieldId WORKING_SET_STATE_LEADER_ENTITY_FIELD_ID = 3;

	void WriteSchema(Schema_Object& Object) const
	{
		Schema_AddInt64(&Object, WORKING_SET_STATE_EPOCH_FIELD_ID, Epoch);
		for (Worker_EntityId_Key EntityId : MemberEntities)
		{
			Schema_AddEntityId(&Object, WORKING_SET_STATE_MEMBER_ENTITIES_FIELD_ID, EntityId);
		}
		Schema_AddEntityId(&Object, WORKING_SET_STATE_LEADER_ENTITY_FIELD_ID, LeaderEntityId);
	}

	void ApplySchema(const Schema_Object& Object)
	{
		if (Schema_GetInt64Count(&Object, WORKING_SET_STATE_EPOCH_FIELD_ID) == 1)
		{
			Epoch = Schema_GetInt64(&Object, WORKING_SET_STATE_EPOCH_FIELD_ID);
		}
		ApplyCollection(Object, MemberEntities, WORKING_SET_STATE_MEMBER_ENTITIES_FIELD_ID);
		if (Schema_GetEntityIdCount(&Object, WORKING_SET_STATE_LEADER_ENTITY_FIELD_ID) == 1)
		{
			LeaderEntityId = Schema_GetEntityId(&Object, WORKING_SET_STATE_LEADER_ENTITY_FIELD_ID);
		}
	}

	int64 Epoch = 0;
	TSet<Worker_EntityId_Key> MemberEntities;
	Worker_EntityId LeaderEntityId = SpatialConstants::INVALID_ENTITY_ID;
};

struct FWorkingSetMarkerRequest
{
	static constexpr Worker_EntityId ComponentId = SpatialConstants::WORKING_SET_REQUEST_COMPONENT_ID;

	FWorkingSetMarkerRequest() = default;
	FWorkingSetMarkerRequest(FWorkingSetState InState)
		: RequestedState(InState)
	{
	}
	FWorkingSetMarkerRequest(const ComponentData& Component) { ApplySchema(*Component.GetFields()); }

	ComponentData CreateComponentData() const
	{
		ComponentData Component(ComponentId);
		WriteSchema(*Component.GetFields());
		return Component;
	}

	ComponentUpdate CreateComponentUpdate() const
	{
		ComponentUpdate Update(ComponentId);
		WriteSchema(*Update.GetFields());
		return Update;
	}

	void WriteSchema(Schema_Object& Object) const { RequestedState.WriteSchema(Object); }

	void ApplySchema(const Schema_Object& Object) { RequestedState.ApplySchema(Object); }

	FWorkingSetState RequestedState;
};

struct FWorkingSetMarkerResponse
{
	static constexpr Worker_EntityId ComponentId = SpatialConstants::WORKING_SET_RESPONSE_COMPONENT_ID;

	FWorkingSetMarkerResponse() = default;
	FWorkingSetMarkerResponse(FWorkingSetState InState)
		: ConfirmedState(InState)
	{
	}
	FWorkingSetMarkerResponse(const ComponentData& Component) { ApplySchema(*Component.GetFields()); }

	ComponentData CreateComponentData() const
	{
		ComponentData Component(ComponentId);
		WriteSchema(*Component.GetFields());
		return Component;
	}

	ComponentUpdate CreateComponentUpdate() const
	{
		ComponentUpdate Update(ComponentId);
		WriteSchema(*Update.GetFields());
		return Update;
	}

	void WriteSchema(Schema_Object& Object) const { ConfirmedState.WriteSchema(Object); }

	void ApplySchema(const Schema_Object& Object) { ConfirmedState.ApplySchema(Object); }

	FWorkingSetState ConfirmedState;
};

struct FWorkingSetMember
{
	static constexpr Worker_EntityId ComponentId = SpatialConstants::WORKING_SET_MEMBER_COMPONENT_ID;

	static constexpr Schema_FieldId WORKING_SET_MEMBER_MARKER_ENTITY_ID_FIELD_ID = 1;

	FWorkingSetMember() = default;
	FWorkingSetMember(const ComponentData& Component) { ApplySchema(*Component.GetFields()); }

	ComponentData CreateComponentData() const
	{
		ComponentData Component(ComponentId);
		WriteSchema(*Component.GetFields());
		return Component;
	}

	ComponentUpdate CreateComponentUpdate() const
	{
		ComponentUpdate Update(ComponentId);
		WriteSchema(*Update.GetFields());
		return Update;
	}

	void WriteSchema(Schema_Object& Object) const
	{
		Schema_AddEntityId(&Object, WORKING_SET_MEMBER_MARKER_ENTITY_ID_FIELD_ID, WorkingSetMarkerEntityId);
	}

	void ApplySchema(const Schema_Object& Object)
	{
		if (Schema_GetEntityIdCount(&Object, WORKING_SET_MEMBER_MARKER_ENTITY_ID_FIELD_ID) == 1)
		{
			WorkingSetMarkerEntityId = Schema_GetEntityId(&Object, WORKING_SET_MEMBER_MARKER_ENTITY_ID_FIELD_ID);
		}
	}

	Worker_EntityId WorkingSetMarkerEntityId = SpatialConstants::INVALID_ENTITY_ID;
};
} // namespace SpatialGDK
