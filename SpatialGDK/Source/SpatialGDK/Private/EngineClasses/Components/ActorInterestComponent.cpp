// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "EngineClasses/Components/ActorInterestComponent.h"

#include "Schema/Interest.h"
#include "Interop/SpatialClassInfoManager.h"

void UActorInterestComponent::CreateQueries(const USpatialClassInfoManager& ClassInfoManager, const SpatialGDK::QueryConstraint& AdditionalConstraints, TArray<SpatialGDK::Query>& OutQueries) const
{
	for (const auto& QueryData : Queries)
	{
		if (!QueryData.Constraint)
		{
			continue;
		}

		SpatialGDK::Query NewQuery{};
		// Avoid creating an unnecessary AND constraint if there are no AdditionalConstraints to consider.
		if (AdditionalConstraints.IsValid())
		{
			SpatialGDK::QueryConstraint ComponentConstraints;
			QueryData.Constraint->CreateConstraint(ClassInfoManager, ComponentConstraints);

			NewQuery.Constraint.AndConstraint.Add(ComponentConstraints);
			NewQuery.Constraint.AndConstraint.Add(AdditionalConstraints);
		}
		else
		{
			QueryData.Constraint->CreateConstraint(ClassInfoManager, NewQuery.Constraint);
		}
		NewQuery.Frequency = QueryData.Frequency;
		NewQuery.FullSnapshotResult = true;

		if (NewQuery.Constraint.IsValid())
		{
			OutQueries.Push(NewQuery);
		}
	}

}
