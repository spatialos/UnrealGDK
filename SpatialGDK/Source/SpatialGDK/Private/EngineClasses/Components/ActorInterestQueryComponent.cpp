// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "ActorInterestQueryComponent.h"

#include "Schema/Interest.h"
#include "Utils/SchemaDatabase.h"

UActorInterestQueryComponent::UActorInterestQueryComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

SpatialGDK::Query UActorInterestQueryComponent::CreateQuery(const USchemaDatabase& SchemaDatabase, const SpatialGDK::QueryConstraint& AdditionalConstraints) const
{
	SpatialGDK::Query InterestQuery{};
	if (AdditionalConstraints.IsValid())
	{
		SpatialGDK::QueryConstraint ComponentConstraints;
		Constraint->CreateConstraint(SchemaDatabase, ComponentConstraints);
		
		InterestQuery.Constraint.AndConstraint.Add(ComponentConstraints);
		InterestQuery.Constraint.AndConstraint.Add(AdditionalConstraints);
	}
	else
	{
		Constraint->CreateConstraint(SchemaDatabase, InterestQuery.Constraint);
	}

	InterestQuery.Frequency = Frequency;

	return InterestQuery;
}
