// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "SpatialView/ComponentSetData.h"

#include <improbable/c_worker.h>

namespace SpatialGDK
{
struct FReceivedComponentChange
{
	FReceivedComponentChange(const Worker_AddComponentOp& Op)
		: EntityId(Op.entity_id)
		, ComponentId(Op.data.component_id)
		, Type(ADD)
		, ComponentAdded(Op.data.schema_type)
	{
	}

	FReceivedComponentChange(const Worker_ComponentUpdateOp& Op)
		: EntityId(Op.entity_id)
		, ComponentId(Op.update.component_id)
		, Type(UPDATE)
		, ComponentUpdate(Op.update.schema_type)
	{
	}

	FReceivedComponentChange(const Worker_RemoveComponentOp& Op)
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

struct FReceivedEntityChange
{
	Worker_EntityId EntityId;
	bool bAdded;
};

struct FReceivedAuthorityChange
{
	Worker_EntityId EntityId;
	Worker_ComponentSetId ComponentSetId;
	bool bNewAuth;
};

// Comparator that will return true when the entity change in question is not for the same entity ID as stored.
struct FDifferentEntity
{
	Worker_EntityId EntityId;
	bool operator()(const FReceivedEntityChange& Change) const { return Change.EntityId != EntityId; }

	bool operator()(const FReceivedComponentChange& Change) const { return Change.EntityId != EntityId; }

	bool operator()(const FReceivedAuthorityChange& Change) const { return Change.EntityId != EntityId; }
};

// Comparator that will return true when the entity change in question is not for the same entity-component as stored.
struct FDifferentEntityComponent
{
	Worker_EntityId EntityId;
	Worker_ComponentId ComponentId;
	bool operator()(const FReceivedComponentChange& Op) const { return Op.ComponentId != ComponentId || Op.EntityId != EntityId; }
};

// Comparator that will return true when the entity change in question is not for the same entity-component-set as stored.
struct FDifferentEntityComponentSet
{
	Worker_EntityId EntityId;
	Worker_ComponentSetId ComponentSetId;
	bool operator()(const FReceivedAuthorityChange& Op) const { return Op.ComponentSetId != ComponentSetId || Op.EntityId != EntityId; }
};

// Comparator that will return true when the entity ID of Lhs is less than that of Rhs.
// If the entity IDs are the same it will return true when the component ID of Lhs is less than that of Rhs.
struct FCompareByEntityComponent
{
	bool operator()(const FReceivedComponentChange& Lhs, const FReceivedComponentChange& Rhs) const
	{
		if (Lhs.EntityId != Rhs.EntityId)
		{
			return Lhs.EntityId < Rhs.EntityId;
		}
		return Lhs.ComponentId < Rhs.ComponentId;
	}

	bool operator()(const FReceivedAuthorityChange& Lhs, const FReceivedAuthorityChange& Rhs) const
	{
		if (Lhs.EntityId != Rhs.EntityId)
		{
			return Lhs.EntityId < Rhs.EntityId;
		}
		return Lhs.ComponentSetId < Rhs.ComponentSetId;
	}
};

// Comparator that will return true when the entity ID of Lhs is less than that of Rhs.
struct FCompareByEntityId
{
	bool operator()(const FReceivedEntityChange& Lhs, const FReceivedEntityChange& Rhs) const { return Lhs.EntityId < Rhs.EntityId; }
};

} // namespace SpatialGDK
