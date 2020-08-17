// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include <Containers/Array.h>
#include <improbable/c_worker.h>

namespace SpatialGDK
{
// A wrapper around a Worker_EntityQuery that allows it to be stored and moved.
class EntityQuery
{
public:
	explicit EntityQuery(const Worker_EntityQuery& Query);

	~EntityQuery() = default;

	// Moveable, not copyable.
	EntityQuery(const EntityQuery&) = delete;
	EntityQuery(EntityQuery&&) = default;
	EntityQuery& operator=(const EntityQuery&) = delete;
	EntityQuery& operator=(EntityQuery&&) = default;

	// Returns the stored entity query.
	// The value is only valid until the EntityQuery object is moved or goes out of scope.
	Worker_EntityQuery GetWorkerQuery() const;

private:
	// Returns the total number of Worker_Constraint objects in the tree.
	static int32 GetNestedConstraintCount(const Worker_Constraint& Constraint);
	// Recursively re-points the constraint to its children and stores those children in the array.
	// `Constraint` should refer to the same query that Constraints[Index] was copied from.
	void StoreChildConstraints(const Worker_Constraint& Constraint, int32 Index);

	TArray<Worker_ComponentId> SnapshotComponentIds;
	TArray<Worker_Constraint> Constraints; // Stable pointer storage.
	uint8 ResultType;
};

} // namespace SpatialGDK
