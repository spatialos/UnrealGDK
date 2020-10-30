// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "ComponentTestUtils.h"
#include "SpatialView/OutgoingMessages.h"

namespace SpatialGDK
{
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

	return Lhs.TimeoutMillis != Rhs.TimeoutMillis;
	// TODO span comparison?
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

	return Lhs.TimeoutMillis != Rhs.TimeoutMillis;
}

inline bool CompareEntityQueryRequests(const EntityQueryRequest& Lhs, const EntityQueryRequest& Rhs)
{
	if (Lhs.RequestId != Rhs.RequestId)
	{
		return false;
	}

	if (Lhs.Query.GetWorkerQuery().result_type != Rhs.Query.GetWorkerQuery().result_type)
	{
		return false;
	}

	if (Lhs.Query.GetWorkerQuery().snapshot_result_type_component_id_count
		!= Rhs.Query.GetWorkerQuery().snapshot_result_type_component_id_count)
	{
		return false;
	}

	if (Lhs.Query.GetWorkerQuery().constraint.constraint_type != Rhs.Query.GetWorkerQuery().constraint.constraint_type)
	{
		return false;
	}

	if (Lhs.Query.GetWorkerQuery().constraint.constraint.entity_id_constraint.entity_id
		!= Rhs.Query.GetWorkerQuery().constraint.constraint.entity_id_constraint.entity_id)
	{
		return false;
	}

	// TODO

	return Lhs.TimeoutMillis != Rhs.TimeoutMillis;
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

	return Lhs.Message == Rhs.Message;
}
} // namespace SpatialGDK
