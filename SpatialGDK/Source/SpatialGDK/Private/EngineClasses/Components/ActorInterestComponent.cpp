// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "EngineClasses/Components/ActorInterestComponent.h"

#include "Interop/SpatialClassInfoManager.h"
#include "Schema/Interest.h"

// NWX_BEGIN - https://improbableio.atlassian.net/browse/NWX-18843 - [IMPROVEMENT] Support for custom Interest components
UActorInterestComponent::UActorInterestComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PrimaryComponentTick.bCanEverTick = false;
}
// NWX_END

void UActorInterestComponent::PopulateFrequencyToConstraintsMap(const USpatialClassInfoManager& ClassInfoManager,
																SpatialGDK::FrequencyToConstraintsMap& OutFrequencyToQueryConstraints) const
{
	// Loop through the user specified queries to extract the constraints and frequencies.
	// We don't construct the actual query at this point because the interest factory enforces the result types.
	for (const auto& QueryData : Queries)
	{
		if (!QueryData.Constraint)
		{
			continue;
		}

		SpatialGDK::QueryConstraint NewQueryConstraint{};
		QueryData.Constraint->CreateConstraint(ClassInfoManager, NewQueryConstraint);

		// If there is already a query defined with this frequency, group them to avoid making too many queries down the line.
		// This avoids any extra cost due to duplicate result types across the network if they are large.
		if (OutFrequencyToQueryConstraints.Find(QueryData.Frequency))
		{
			OutFrequencyToQueryConstraints.Find(QueryData.Frequency)->Add(NewQueryConstraint);
			continue;
		}

		TArray<SpatialGDK::QueryConstraint> ConstraintList = { NewQueryConstraint };
		OutFrequencyToQueryConstraints.Add(QueryData.Frequency, ConstraintList);
	}
}
