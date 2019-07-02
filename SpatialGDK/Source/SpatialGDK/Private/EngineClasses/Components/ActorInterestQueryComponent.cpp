// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "ActorInterestQueryComponent.h"

#include "Schema/Interest.h"
#include "Utils/SchemaDatabase.h"

UActorInterestQueryComponent::UActorInterestQueryComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

SpatialGDK::Query UActorInterestQueryComponent::CreateQuery(const USchemaDatabase& SchemaDatabase) const
{
	SpatialGDK::Query InterestQuery{};
	InterestQuery.Frequency = Frequency;
	Constraint->CreateConstraint(SchemaDatabase, InterestQuery.Constraint);
	return InterestQuery;
}
