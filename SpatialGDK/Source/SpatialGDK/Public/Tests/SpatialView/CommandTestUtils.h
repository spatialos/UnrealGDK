// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "ComponentTestUtils.h"
#include "SpatialView/OutgoingMessages.h"

namespace SpatialGDK
{
bool CompareListOfWorkerConstraints(const Worker_Constraint* Lhs, const Worker_Constraint* Rhs, const int32 LhsNum, const int32 RhsNum);
inline bool CompareReseverEntityIdsRequests(const ReserveEntityIdsRequest& Lhs, const ReserveEntityIdsRequest& Rhs)
{
	if (Lhs.RequestId != Rhs.RequestId)
	{
		return false;
	}

	if (Lhs.NumberOfEntityIds != Rhs.NumberOfEntityIds)
	{
		return false;
	}

	return Lhs.TimeoutMillis == Rhs.TimeoutMillis;
}

inline bool CompareCreateEntityRequests(const CreateEntityRequest& Lhs, const CreateEntityRequest& Rhs)
{
	if (Lhs.EntityId != Rhs.EntityId)
	{
		return false;
	}

	if (Lhs.RequestId != Rhs.RequestId)
	{
		return false;
	}

	if (Lhs.TimeoutMillis != Rhs.TimeoutMillis)
	{
		return false;
	}

	return AreEquivalent(Lhs.EntityComponents, Rhs.EntityComponents, CompareComponentData);
}

inline bool CompareDeleteEntityRequests(const DeleteEntityRequest& Lhs, const DeleteEntityRequest& Rhs)
{
	if (Lhs.EntityId != Rhs.EntityId)
	{
		return false;
	}

	if (Lhs.RequestId != Rhs.RequestId)
	{
		return false;
	}

	return Lhs.TimeoutMillis == Rhs.TimeoutMillis;
}

inline bool CompareWorkerConstraints(const Worker_Constraint& Lhs, const Worker_Constraint Rhs)
{
	if (Lhs.constraint_type != Rhs.constraint_type)
	{
		return false;
	}

	switch (Lhs.constraint_type)
	{
	case WORKER_CONSTRAINT_TYPE_ENTITY_ID:
		return Lhs.constraint.entity_id_constraint.entity_id == Rhs.constraint.entity_id_constraint.entity_id;
	case WORKER_CONSTRAINT_TYPE_COMPONENT:
		return Lhs.constraint.component_constraint.component_id == Rhs.constraint.component_constraint.component_id;
	case WORKER_CONSTRAINT_TYPE_SPHERE:
		return Lhs.constraint.sphere_constraint.radius == Rhs.constraint.sphere_constraint.radius
			   && Lhs.constraint.sphere_constraint.x == Rhs.constraint.sphere_constraint.x
			   && Lhs.constraint.sphere_constraint.y == Rhs.constraint.sphere_constraint.y
			   && Lhs.constraint.sphere_constraint.z == Rhs.constraint.sphere_constraint.z;
	case WORKER_CONSTRAINT_TYPE_AND:
		return CompareListOfWorkerConstraints(Lhs.constraint.and_constraint.constraints, Rhs.constraint.and_constraint.constraints,
											  Lhs.constraint.and_constraint.constraint_count,
											  Rhs.constraint.and_constraint.constraint_count);
	case WORKER_CONSTRAINT_TYPE_OR:
		return CompareListOfWorkerConstraints(Lhs.constraint.or_constraint.constraints, Rhs.constraint.or_constraint.constraints,
											  Lhs.constraint.or_constraint.constraint_count, Rhs.constraint.or_constraint.constraint_count);
	case WORKER_CONSTRAINT_TYPE_NOT:
		return CompareWorkerConstraints(*Lhs.constraint.not_constraint.constraint, *Rhs.constraint.not_constraint.constraint);
	default:
		return false;
	}
}

inline bool CompareListOfWorkerConstraints(const Worker_Constraint* Lhs, const Worker_Constraint* Rhs, const int32 LhsNum,
										   const int32 RhsNum)
{
	if (LhsNum != RhsNum)
	{
		return false;
	}

	return std::is_permutation(Lhs, Lhs + LhsNum, Rhs, CompareWorkerConstraints);
}

inline bool CompareEntityQueryRequests(const EntityQueryRequest& Lhs, const EntityQueryRequest& Rhs)
{
	if (Lhs.RequestId != Rhs.RequestId)
	{
		return false;
	}

	if (Lhs.Query.GetWorkerQuery().snapshot_result_type_component_id_count
		!= Rhs.Query.GetWorkerQuery().snapshot_result_type_component_id_count)
	{
		return false;
	}

	if (Lhs.TimeoutMillis != Rhs.TimeoutMillis)
	{
		return false;
	}

	return CompareWorkerConstraints(Lhs.Query.GetWorkerQuery().constraint, Rhs.Query.GetWorkerQuery().constraint);
}

inline bool CompareCommandRequests(const CommandRequest& Lhs, const CommandRequest& Rhs)
{
	if (Lhs.GetComponentId() != Rhs.GetComponentId())
	{
		return false;
	}

	if (Lhs.GetCommandIndex() != Rhs.GetCommandIndex())
	{
		return false;
	}

	return CompareSchemaObjects(Lhs.GetRequestObject(), Rhs.GetRequestObject());
}

inline bool CompareCommandResponses(const CommandResponse& Lhs, const CommandResponse& Rhs)
{
	if (Lhs.GetComponentId() != Rhs.GetComponentId())
	{
		return false;
	}

	if (Lhs.GetCommandIndex() != Rhs.GetCommandIndex())
	{
		return false;
	}

	return CompareSchemaObjects(Lhs.GetResponseObject(), Rhs.GetResponseObject());
}

inline bool CompareEntityCommandRequests(const EntityCommandRequest& Lhs, const EntityCommandRequest& Rhs)
{
	if (Lhs.EntityId != Rhs.EntityId)
	{
		return false;
	}

	if (Lhs.RequestId != Rhs.RequestId)
	{
		return false;
	}

	if (Lhs.TimeoutMillis != Rhs.TimeoutMillis)
	{
		return false;
	}

	return CompareCommandRequests(Lhs.Request, Rhs.Request);
}

inline bool CompareEntityCommandResponses(const EntityCommandResponse& Lhs, const EntityCommandResponse& Rhs)
{
	if (Lhs.RequestId != Rhs.RequestId)
	{
		return false;
	}

	return CompareCommandResponses(Lhs.Response, Rhs.Response);
}

inline bool CompareEntityCommandFailuers(const EntityCommandFailure& Lhs, const EntityCommandFailure& Rhs)
{
	if (Lhs.RequestId != Rhs.RequestId)
	{
		return false;
	}

	return Lhs.Message.Equals(Rhs.Message);
}
} // namespace SpatialGDK
