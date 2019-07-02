// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "ActorInterestQueryComponent.h"

#include "Schema/Interest.h"
#include "Utils/SchemaDatabase.h"

UActorInterestQueryComponent::UActorInterestQueryComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UActorInterestQueryComponent::CreateQueries(const USchemaDatabase& SchemaDatabase, const SpatialGDK::QueryConstraint& AdditionalConstraints, TArray<SpatialGDK::Query>& OutQueries) const
{
	for (const auto& QueryData : Queries)
	{
		if (!QueryData.Constraint)
		{
			continue;
		}

		SpatialGDK::Query NewQuery{};
		if (AdditionalConstraints.IsValid())
		{
			SpatialGDK::QueryConstraint ComponentConstraints;
			QueryData.Constraint->CreateConstraint(SchemaDatabase, ComponentConstraints);
		
			NewQuery.Constraint.AndConstraint.Add(ComponentConstraints);
			NewQuery.Constraint.AndConstraint.Add(AdditionalConstraints);
		}
		else
		{
			QueryData.Constraint->CreateConstraint(SchemaDatabase, NewQuery.Constraint);
		}
		NewQuery.Frequency = QueryData.Frequency;

		if (NewQuery.Constraint.IsValid())
		{
			OutQueries.Push(NewQuery);
		}
	}

}
