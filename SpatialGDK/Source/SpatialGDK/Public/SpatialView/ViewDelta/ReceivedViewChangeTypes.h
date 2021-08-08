// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "SpatialView/ComponentSetData.h"

#include <improbable/c_worker.h>

namespace SpatialGDK
{
struct ReceivedComponentChange
{
	ReceivedComponentChange(const Worker_AddComponentOp& Op)
		: EntityId(Op.entity_id)
		, ComponentId(Op.data.component_id)
		, Type(ADD)
		, ComponentAdded(Op.data.schema_type)
	{
	}

	ReceivedComponentChange(const Worker_ComponentUpdateOp& Op)
		: EntityId(Op.entity_id)
		, ComponentId(Op.update.component_id)
		, Type(UPDATE)
		, ComponentUpdate(Op.update.schema_type)
	{
	}

	ReceivedComponentChange(const Worker_RemoveComponentOp& Op)
		: EntityId(Op.entity_id)
		, ComponentId(Op.component_id)
		, Type(REMOVE)
	{
	}

	Worker_EntityId EntityId;
	Worker_ComponentId ComponentId;
	enum
	{
		ADD,
		UPDATE,
		REMOVE
	} Type;
	union
	{
		Schema_ComponentData* ComponentAdded;
		Schema_ComponentUpdate* ComponentUpdate;
	};
};

struct ReceivedEntityChange
{
	Worker_EntityId EntityId;
	bool bAdded;
};

struct ReceivedAuthorityChange
{
	Worker_EntityId EntityId;
	Worker_ComponentSetId ComponentSetId;
	bool bNewAuth;
};

// Comparator that will return true when the entity change in question is not for the same entity ID as stored.
struct DifferentEntity
{
	Worker_EntityId EntityId;
	bool operator()(const ReceivedEntityChange& Change) const { return Change.EntityId != EntityId; }

	bool operator()(const ReceivedComponentChange& Change) const { return Change.EntityId != EntityId; }

	bool operator()(const ReceivedAuthorityChange& Change) const { return Change.EntityId != EntityId; }
};

// Comparator that will return true when the entity change in question is not for the same entity-component as stored.
struct DifferentEntityComponent
{
	Worker_EntityId EntityId;
	Worker_ComponentId ComponentId;
	bool operator()(const ReceivedComponentChange& Op) const { return Op.ComponentId != ComponentId || Op.EntityId != EntityId; }
};

// Comparator that will return true when the entity change in question is not for the same entity-component-stated as stored.
struct DifferentEntityComponentSet
{
	Worker_EntityId EntityId;
	Worker_ComponentSetId ComponentSetId;
	bool operator()(const ReceivedAuthorityChange& Op) const { return Op.ComponentSetId != ComponentSetId || Op.EntityId != EntityId; }
};

// Comparator that will return true when the entity ID of Lhs is less than that of Rhs.
// If the entity IDs are the same it will return true when the component ID of Lhs is less than that of Rhs.
struct EntityComponentComparison
{
	bool operator()(const ReceivedComponentChange& Lhs, const ReceivedComponentChange& Rhs) const
	{
		if (Lhs.EntityId != Rhs.EntityId)
		{
			return Lhs.EntityId < Rhs.EntityId;
		}
		return Lhs.ComponentId < Rhs.ComponentId;
	}

	bool operator()(const ReceivedAuthorityChange& Lhs, const ReceivedAuthorityChange& Rhs) const
	{
		if (Lhs.EntityId != Rhs.EntityId)
		{
			return Lhs.EntityId < Rhs.EntityId;
		}
		return Lhs.ComponentSetId < Rhs.ComponentSetId;
	}
};

// Comparator that will return true when the entity ID of Lhs is less than that of Rhs.
struct EntityComparison
{
	bool operator()(const ReceivedEntityChange& Lhs, const ReceivedEntityChange& Rhs) const { return Lhs.EntityId < Rhs.EntityId; }
};

} // namespace SpatialGDK
