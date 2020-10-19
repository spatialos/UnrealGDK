#include "SpatialView/EntityQuery.h"

namespace SpatialGDK
{
EntityQuery::EntityQuery(const Worker_EntityQuery& Query)
	: ResultType(static_cast<Worker_ResultType>(Query.result_type))
{
	Constraints.Reserve(GetNestedConstraintCount(Query.constraint));
	Constraints.Add(Query.constraint);
	StoreChildConstraints(Query.constraint, 0);
	if (Query.result_type == WORKER_RESULT_TYPE_SNAPSHOT && Query.snapshot_result_type_component_ids)
	{
		SnapshotComponentIds.Reserve(Query.snapshot_result_type_component_id_count);
		SnapshotComponentIds.Append(Query.snapshot_result_type_component_ids, Query.snapshot_result_type_component_id_count);
	}
}

Worker_EntityQuery EntityQuery::GetWorkerQuery() const
{
	return Worker_EntityQuery{ Constraints[0], ResultType, static_cast<uint32>(SnapshotComponentIds.Num()),
							   ResultType == WORKER_RESULT_TYPE_SNAPSHOT ? SnapshotComponentIds.GetData() : nullptr };
}

int32 EntityQuery::GetNestedConstraintCount(const Worker_Constraint& Constraint)
{
	switch (static_cast<Worker_ConstraintType>(Constraint.constraint_type))
	{
	case WORKER_CONSTRAINT_TYPE_ENTITY_ID:
	case WORKER_CONSTRAINT_TYPE_COMPONENT:
	case WORKER_CONSTRAINT_TYPE_SPHERE:
		return 1;
	case WORKER_CONSTRAINT_TYPE_AND:
	{
		const Worker_AndConstraint& AndConstraint = Constraint.constraint.and_constraint;
		uint32 Sum = 1;
		for (uint32 i = 0; i < AndConstraint.constraint_count; ++i)
		{
			Sum += GetNestedConstraintCount(AndConstraint.constraints[i]);
		}
		return Sum;
	}
	case WORKER_CONSTRAINT_TYPE_OR:
	{
		const Worker_OrConstraint& OrConstraint = Constraint.constraint.or_constraint;
		uint32 Sum = 1;
		for (uint32 i = 0; i < OrConstraint.constraint_count; ++i)
		{
			Sum += GetNestedConstraintCount(OrConstraint.constraints[i]);
		}
		return Sum;
	}
	case WORKER_CONSTRAINT_TYPE_NOT:
		return 1 + GetNestedConstraintCount(*Constraint.constraint.not_constraint.constraint);
	default:
		check(false);
		return 0;
	}
}

void EntityQuery::StoreChildConstraints(const Worker_Constraint& Constraint, int32 Index)
{
	const int32 ChildIndex = Constraints.Num();
	switch (static_cast<Worker_ConstraintType>(Constraints[Index].constraint_type))
	{
	case WORKER_CONSTRAINT_TYPE_ENTITY_ID:
	case WORKER_CONSTRAINT_TYPE_COMPONENT:
	case WORKER_CONSTRAINT_TYPE_SPHERE:
		break;
	case WORKER_CONSTRAINT_TYPE_AND:
	{
		const Worker_AndConstraint& AndConstraint = Constraint.constraint.and_constraint;
		// Appends children to the array.
		Constraints.Append(AndConstraint.constraints, AndConstraint.constraint_count);
		// Adjust pointers to point at the child constraint.
		Constraints[Index].constraint.and_constraint.constraints = &Constraints[ChildIndex];

		// Do the same for child constraints.
		for (uint32 i = 0; i < AndConstraint.constraint_count; ++i)
		{
			StoreChildConstraints(AndConstraint.constraints[i], ChildIndex + i);
		}
		break;
	}
	case WORKER_CONSTRAINT_TYPE_OR:
	{
		const Worker_OrConstraint& OrConstraint = Constraint.constraint.or_constraint;
		// Appends children to the array.
		Constraints.Append(OrConstraint.constraints, OrConstraint.constraint_count);
		// Adjust pointers to point at the child constraint.
		Constraints[Index].constraint.or_constraint.constraints = &Constraints[ChildIndex];

		// Do the same for child constraints.
		for (uint32 i = 0; i < OrConstraint.constraint_count; ++i)
		{
			StoreChildConstraints(OrConstraint.constraints[i], ChildIndex + i);
		}
		break;
	}
	case WORKER_CONSTRAINT_TYPE_NOT:
	{
		const Worker_NotConstraint& NotConstraint = Constraint.constraint.not_constraint;
		// Insert child into the array.
		Constraints.Add(*NotConstraint.constraint);
		// Adjust pointers to point at the child constraint.
		Constraints[Index].constraint.not_constraint.constraint = &Constraints[ChildIndex];

		StoreChildConstraints(*NotConstraint.constraint, ChildIndex);
		break;
	}
	}
}

} // namespace SpatialGDK
