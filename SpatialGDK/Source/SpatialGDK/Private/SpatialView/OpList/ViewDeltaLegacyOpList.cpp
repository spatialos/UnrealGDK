// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "SpatialView/OpList/ViewDeltaLegacyOpList.h"

#include "Containers/StringConv.h"

namespace SpatialGDK
{
TArray<Worker_Op> GetOpsFromEntityDeltas(const TArray<EntityDelta>& Deltas)
{
	// The order of ops per entity should be:
	// Add entities
	// Add components
	// Authority lost (from lost and lost temporarily)
	// Component updates (complete updates and regular updates)
	// Remove components
	// Authority gained (from gained and lost temporarily)
	// Entities Removed (can be reordered with authority gained)

	TArray<Worker_Op> Ops;

	for (const EntityDelta& Entity : Deltas)
	{
		Worker_Op StartCriticalSection = {};
		StartCriticalSection.op_type = WORKER_OP_TYPE_CRITICAL_SECTION;
		StartCriticalSection.op.critical_section.in_critical_section = 1;
		Ops.Add(StartCriticalSection);

		if (Entity.Type == EntityDelta::ADD)
		{
			Worker_Op Op = {};
			Op.op_type = WORKER_OP_TYPE_ADD_ENTITY;
			Op.op.add_entity.entity_id = Entity.EntityId;
			Ops.Push(Op);
		}

		for (const ComponentChange& Change : Entity.ComponentsAdded)
		{
			Worker_Op Op = {};
			Op.op_type = WORKER_OP_TYPE_ADD_COMPONENT;
			Op.op.add_component.entity_id = Entity.EntityId;
			Op.op.add_component.data = Worker_ComponentData{ nullptr, Change.ComponentId, Change.Data, nullptr };
			Ops.Push(Op);
		}

		for (const AuthorityChange& Change : Entity.AuthorityLost)
		{
			Worker_Op Op = {};
			Op.op_type = WORKER_OP_TYPE_COMPONENT_SET_AUTHORITY_CHANGE;
			Op.op.component_set_authority_change.entity_id = Entity.EntityId;
			Op.op.component_set_authority_change.component_set_id = Change.ComponentId;
			Op.op.component_set_authority_change.authority = WORKER_AUTHORITY_NOT_AUTHORITATIVE;
			Ops.Push(Op);
		}

		for (const AuthorityChange& Change : Entity.AuthorityLostTemporarily)
		{
			Worker_Op Op = {};
			Op.op_type = WORKER_OP_TYPE_COMPONENT_SET_AUTHORITY_CHANGE;
			Op.op.component_set_authority_change.entity_id = Entity.EntityId;
			Op.op.component_set_authority_change.component_set_id = Change.ComponentId;
			Op.op.component_set_authority_change.authority = WORKER_AUTHORITY_NOT_AUTHORITATIVE;
			Ops.Push(Op);
		}

		for (const ComponentChange& Change : Entity.ComponentsRefreshed)
		{
			// We deliberately ignore the events update here to avoid breaking code that expects each update to contain data.
			Worker_Op AddOp = {};
			AddOp.op_type = WORKER_OP_TYPE_ADD_COMPONENT;
			AddOp.op.add_component.entity_id = Entity.EntityId;
			AddOp.op.add_component.data = Worker_ComponentData{ nullptr, Change.ComponentId, Change.CompleteUpdate.Data, nullptr };
			Ops.Push(AddOp);
		}

		for (const ComponentChange& Change : Entity.ComponentUpdates)
		{
			Worker_Op Op = {};
			Op.op_type = WORKER_OP_TYPE_COMPONENT_UPDATE;
			Op.op.component_update.entity_id = Entity.EntityId;
			Op.op.component_update.update = Worker_ComponentUpdate{ nullptr, Change.ComponentId, Change.Update, nullptr };
			Ops.Push(Op);
		}

		for (const ComponentChange& Change : Entity.ComponentsRemoved)
		{
			Worker_Op Op = {};
			Op.op_type = WORKER_OP_TYPE_REMOVE_COMPONENT;
			Op.op.remove_component.entity_id = Entity.EntityId;
			Op.op.remove_component.component_id = Change.ComponentId;
			Ops.Push(Op);
		}

		for (const AuthorityChange& Change : Entity.AuthorityLostTemporarily)
		{
			Worker_Op Op = {};
			Op.op_type = WORKER_OP_TYPE_COMPONENT_SET_AUTHORITY_CHANGE;
			Op.op.component_set_authority_change.entity_id = Entity.EntityId;
			Op.op.component_set_authority_change.component_set_id = Change.ComponentId;
			Op.op.component_set_authority_change.authority = WORKER_AUTHORITY_AUTHORITATIVE;
			Ops.Push(Op);
		}

		for (const AuthorityChange& Change : Entity.AuthorityGained)
		{
			Worker_Op Op = {};
			Op.op_type = WORKER_OP_TYPE_COMPONENT_SET_AUTHORITY_CHANGE;
			Op.op.component_set_authority_change.entity_id = Entity.EntityId;
			Op.op.component_set_authority_change.component_set_id = Change.ComponentId;
			Op.op.component_set_authority_change.authority = WORKER_AUTHORITY_AUTHORITATIVE;
			Ops.Push(Op);
		}

		if (Entity.Type == EntityDelta::REMOVE)
		{
			Worker_Op Op = {};
			Op.op_type = WORKER_OP_TYPE_REMOVE_ENTITY;
			Op.op.remove_entity.entity_id = Entity.EntityId;
			Ops.Push(Op);
		}

		Worker_Op EndCriticalSection = {};
		EndCriticalSection.op_type = WORKER_OP_TYPE_CRITICAL_SECTION;
		EndCriticalSection.op.critical_section.in_critical_section = 0;
		Ops.Add(EndCriticalSection);
	}

	return Ops;
}

} // namespace SpatialGDK
