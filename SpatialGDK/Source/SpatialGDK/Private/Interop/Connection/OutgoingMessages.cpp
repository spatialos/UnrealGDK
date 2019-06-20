// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "Interop/Connection/OutgoingMessages.h"

namespace SpatialGDK
{

void FEntityQueryRequest::TraverseConstraint(Worker_Constraint* Constraint)
{
	switch (Constraint->constraint_type)
	{
	case WORKER_CONSTRAINT_TYPE_AND:
	{
		TUniquePtr<Worker_Constraint[]> NewConstraints = MakeUnique<Worker_Constraint[]>(Constraint->and_constraint.constraint_count);

		for (unsigned int i = 0; i < Constraint->and_constraint.constraint_count; i++)
		{
			NewConstraints[i] = Constraint->and_constraint.constraints[i];
			TraverseConstraint(&NewConstraints[i]);
		}
		Constraint->and_constraint.constraints = NewConstraints.Get();

		ConstraintStorage.Add(MoveTemp(NewConstraints));
		break;
	}
	case WORKER_CONSTRAINT_TYPE_OR:
	{
		TUniquePtr<Worker_Constraint[]> NewConstraints = MakeUnique<Worker_Constraint[]>(Constraint->or_constraint.constraint_count);

		for (unsigned int i = 0; i < Constraint->or_constraint.constraint_count; i++)
		{
			NewConstraints[i] = Constraint->or_constraint.constraints[i];
			TraverseConstraint(&NewConstraints[i]);
		}
		Constraint->or_constraint.constraints = NewConstraints.Get();

		ConstraintStorage.Add(MoveTemp(NewConstraints));
		break;
	}
	case WORKER_CONSTRAINT_TYPE_NOT:
	{
		TUniquePtr<Worker_Constraint[]> NewConstraint = MakeUnique<Worker_Constraint[]>(1);

		NewConstraint[0] = *Constraint->not_constraint.constraint;
		TraverseConstraint(&NewConstraint[0]);

		Constraint->not_constraint.constraint = NewConstraint.Get();

		ConstraintStorage.Add(MoveTemp(NewConstraint));
		break;
	}
	}
}

} // namespace SpatialGDK
