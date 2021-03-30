// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "Utils/Interest/NetCullDistanceInterest.h"

#include "UObject/UObjectIterator.h"

#include "SpatialGDKSettings.h"

DEFINE_LOG_CATEGORY(LogNetCullDistanceInterest);

// Use 0 to represent "full" frequency here. Zero actually represents "never" when set in spatial, so this will be converted
// to an empty optional later.
const float FullFrequencyHz = 0.f;

namespace SpatialGDK
{
// And this the empty optional type it will be translated to.
const TSchemaOption<float> FullFrequencyOptional = TSchemaOption<float>();

FrequencyConstraints NetCullDistanceInterest::CreateCheckoutRadiusConstraints(USpatialClassInfoManager* InClassInfoManager)
{
	const USpatialGDKSettings* SpatialGDKSettings = GetDefault<USpatialGDKSettings>();

	if (!SpatialGDKSettings->bEnableNetCullDistanceInterest)
	{
		return NetCullDistanceInterest::CreateLegacyNetCullDistanceConstraint(InClassInfoManager);
	}

	if (!SpatialGDKSettings->bEnableNetCullDistanceFrequency)
	{
		return NetCullDistanceInterest::CreateNetCullDistanceConstraint(InClassInfoManager);
	}

	return NetCullDistanceInterest::CreateNetCullDistanceConstraintWithFrequency(InClassInfoManager);
}

FrequencyConstraints NetCullDistanceInterest::CreateLegacyNetCullDistanceConstraint(USpatialClassInfoManager* InClassInfoManager)
{
	// Checkout Radius constraints are defined by the NetCullDistanceSquared property on actors.
	//   - Checkout radius is a RelativeCylinder constraint on the player controller.
	//   - NetCullDistanceSquared on AActor is used to define the default checkout radius with no other constraints.
	//   - NetCullDistanceSquared on other actor types is used to define additional constraints if needed.
	//   - If a subtype defines a radius smaller than a parent type, then its requirements are already captured.
	//   - If a subtype defines a radius larger than all parent types, then it needs an additional constraint.
	//   - Other than the default from AActor, all radius constraints also include Component constraints to
	//     capture specific types, including all derived types of that actor.

	QueryConstraint CheckoutRadiusConstraint;

	CheckoutRadiusConstraint.OrConstraint.Add(NetCullDistanceInterest::GetDefaultCheckoutRadiusConstraint());

	// Get interest distances for each actor.
	TMap<UClass*, float> ActorComponentSetToRadius = NetCullDistanceInterest::GetActorTypeToRadius();

	// For every interest distance that we still want, build a map from radius to list of actor type components that match that radius.
	TMap<float, TArray<UClass*>> DistanceToActorTypeComponents =
		NetCullDistanceInterest::DedupeDistancesAcrossActorTypes(ActorComponentSetToRadius);

	// The previously built map removes duplicates of spatial constraints. Now the actual query constraints can be built of the form:
	// OR(AND(cylinder(radius), OR(actor 1 components, actor 2 components, ...)), ...)
	// which is equivalent to having a separate spatial query for each actor type if the radius is the same.
	TArray<QueryConstraint> CheckoutRadiusConstraints =
		NetCullDistanceInterest::BuildNonDefaultActorCheckoutConstraints(DistanceToActorTypeComponents, InClassInfoManager);

	// Add all the different actor queries to the overall checkout constraint.
	for (auto& ActorCheckoutConstraint : CheckoutRadiusConstraints)
	{
		CheckoutRadiusConstraint.OrConstraint.Add(ActorCheckoutConstraint);
	}

	return { { FullFrequencyOptional, CheckoutRadiusConstraint } };
}

FrequencyConstraints NetCullDistanceInterest::CreateNetCullDistanceConstraint(USpatialClassInfoManager* InClassInfoManager)
{
	QueryConstraint CheckoutRadiusConstraintRoot;

	const TMap<float, Worker_ComponentId>& NetCullDistancesToComponentIds = InClassInfoManager->GetNetCullDistanceToComponentIds();

	for (const auto& DistanceComponentPair : NetCullDistancesToComponentIds)
	{
		const float MaxCheckoutRadiusMeters = NetCullDistanceInterest::NetCullDistanceSquaredToSpatialDistance(DistanceComponentPair.Key);

		QueryConstraint ComponentConstraint;
		ComponentConstraint.ComponentConstraint = DistanceComponentPair.Value;

		QueryConstraint RadiusConstraint;
		RadiusConstraint.RelativeCylinderConstraint = RelativeCylinderConstraint{ MaxCheckoutRadiusMeters };

		QueryConstraint CheckoutRadiusConstraint;
		CheckoutRadiusConstraint.AndConstraint.Add(RadiusConstraint);
		CheckoutRadiusConstraint.AndConstraint.Add(ComponentConstraint);

		CheckoutRadiusConstraintRoot.OrConstraint.Add(CheckoutRadiusConstraint);
	}

	return { { FullFrequencyOptional, CheckoutRadiusConstraintRoot } };
}

FrequencyConstraints NetCullDistanceInterest::CreateNetCullDistanceConstraintWithFrequency(USpatialClassInfoManager* InClassInfoManager)
{
	const USpatialGDKSettings* SpatialGDKSettings = GetDefault<USpatialGDKSettings>();
	const TMap<float, Worker_ComponentId>& NetCullDistancesToComponentIds = InClassInfoManager->GetNetCullDistanceToComponentIds();

	FrequencyToConstraintsMap FrequencyToConstraints;

	for (const auto& DistanceComponentPair : NetCullDistancesToComponentIds)
	{
		const float MaxCheckoutRadiusMeters = NetCullDistanceInterest::NetCullDistanceSquaredToSpatialDistance(DistanceComponentPair.Key);

		QueryConstraint ComponentConstraint;
		ComponentConstraint.ComponentConstraint = DistanceComponentPair.Value;

		float FullFrequencyCheckoutRadius = MaxCheckoutRadiusMeters * SpatialGDKSettings->FullFrequencyNetCullDistanceRatio;

		QueryConstraint RadiusConstraint;
		RadiusConstraint.RelativeCylinderConstraint = RelativeCylinderConstraint{ FullFrequencyCheckoutRadius };

		QueryConstraint CheckoutRadiusConstraint;
		CheckoutRadiusConstraint.AndConstraint.Add(RadiusConstraint);
		CheckoutRadiusConstraint.AndConstraint.Add(ComponentConstraint);

		AddToFrequencyConstraintMap(FullFrequencyHz, CheckoutRadiusConstraint, FrequencyToConstraints);

		// Add interest query for specified distance/frequency pairs
		for (const auto& DistanceFrequencyPair : SpatialGDKSettings->InterestRangeFrequencyPairs)
		{
			float CheckoutRadius = MaxCheckoutRadiusMeters * DistanceFrequencyPair.DistanceRatio;

			QueryConstraint FrequencyRadiusConstraint;
			FrequencyRadiusConstraint.RelativeCylinderConstraint = RelativeCylinderConstraint{ CheckoutRadius };

			QueryConstraint FrequencyCheckoutRadiusConstraint;
			FrequencyCheckoutRadiusConstraint.AndConstraint.Add(FrequencyRadiusConstraint);
			FrequencyCheckoutRadiusConstraint.AndConstraint.Add(ComponentConstraint);

			AddToFrequencyConstraintMap(DistanceFrequencyPair.Frequency, FrequencyCheckoutRadiusConstraint, FrequencyToConstraints);
		}
	}

	FrequencyConstraints CheckoutConstraints;

	// De dupe across frequencies.
	for (auto& FrequencyConstraintsPair : FrequencyToConstraints)
	{
		TSchemaOption<float> SpatialFrequency =
			FrequencyConstraintsPair.Key == FullFrequencyHz ? FullFrequencyOptional : TSchemaOption<float>(FrequencyConstraintsPair.Key);
		if (FrequencyConstraintsPair.Value.Num() == 1)
		{
			CheckoutConstraints.Add({ SpatialFrequency, FrequencyConstraintsPair.Value[0] });
			continue;
		}
		QueryConstraint RadiusDisjunct;
		RadiusDisjunct.OrConstraint.Append(FrequencyConstraintsPair.Value);
		CheckoutConstraints.Add({ SpatialFrequency, RadiusDisjunct });
	}

	return CheckoutConstraints;
}

void NetCullDistanceInterest::AddToFrequencyConstraintMap(const float Frequency, const QueryConstraint& Constraint,
														  FrequencyToConstraintsMap& OutFrequencyToConstraints)
{
	// If there is already a query defined with this frequency, group them to avoid making too many queries down the line.
	// This avoids any extra cost due to duplicate result types across the network if they are large.
	TArray<QueryConstraint>& ConstraintList = OutFrequencyToConstraints.FindOrAdd(Frequency);
	ConstraintList.Add(Constraint);
}

QueryConstraint NetCullDistanceInterest::GetDefaultCheckoutRadiusConstraint()
{
	const float MaxDistanceSquared = GetDefault<USpatialGDKSettings>()->MaxNetCullDistanceSquared;

	// Use AActor's ClientInterestDistance for the default radius (all actors in that radius will be checked out)
	const AActor* DefaultActor = Cast<AActor>(AActor::StaticClass()->GetDefaultObject());

	float DefaultDistanceSquared = DefaultActor->NetCullDistanceSquared;

	if (MaxDistanceSquared > FLT_EPSILON && DefaultDistanceSquared > MaxDistanceSquared)
	{
		UE_LOG(LogNetCullDistanceInterest, Warning, TEXT("Default NetCullDistanceSquared is too large, clamping from %f to %f"),
			   DefaultDistanceSquared, MaxDistanceSquared);

		DefaultDistanceSquared = MaxDistanceSquared;
	}

	const float DefaultCheckoutRadius = NetCullDistanceSquaredToSpatialDistance(DefaultDistanceSquared);

	QueryConstraint DefaultCheckoutRadiusConstraint;
	DefaultCheckoutRadiusConstraint.RelativeCylinderConstraint = RelativeCylinderConstraint{ DefaultCheckoutRadius };

	return DefaultCheckoutRadiusConstraint;
}

TMap<UClass*, float> NetCullDistanceInterest::GetActorTypeToRadius()
{
	const AActor* DefaultActor = Cast<AActor>(AActor::StaticClass()->GetDefaultObject());
	const float DefaultDistanceSquared = DefaultActor->NetCullDistanceSquared;
	const float MaxDistanceSquared = GetDefault<USpatialGDKSettings>()->MaxNetCullDistanceSquared;

	// Gather ClientInterestDistance settings, and add any larger than the default radius to a list for processing.
	TMap<UClass*, float> DiscoveredInterestDistancesSquared;
	for (TObjectIterator<UClass> It; It; ++It)
	{
		if (It->HasAnySpatialClassFlags(SPATIALCLASS_ServerOnly))
		{
			continue;
		}
		if (!It->HasAnySpatialClassFlags(SPATIALCLASS_SpatialType))
		{
			continue;
		}
		if (It->HasAnyClassFlags(CLASS_NewerVersionExists))
		{
			// This skips classes generated for hot reload etc (i.e. REINST_, SKEL_, TRASHCLASS_)
			continue;
		}
		if (!It->IsChildOf<AActor>())
		{
			continue;
		}

		const AActor* IteratedDefaultActor = Cast<AActor>(It->GetDefaultObject());
		if (IteratedDefaultActor->NetCullDistanceSquared > DefaultDistanceSquared)
		{
			float ActorNetCullDistanceSquared = IteratedDefaultActor->NetCullDistanceSquared;

			if (MaxDistanceSquared > FLT_EPSILON && IteratedDefaultActor->NetCullDistanceSquared > MaxDistanceSquared)
			{
				UE_LOG(LogNetCullDistanceInterest, Warning, TEXT("NetCullDistanceSquared for %s too large, clamping from %f to %f"),
					   *It->GetName(), ActorNetCullDistanceSquared, MaxDistanceSquared);

				ActorNetCullDistanceSquared = MaxDistanceSquared;
			}

			DiscoveredInterestDistancesSquared.Add(*It, ActorNetCullDistanceSquared);
		}
	}

	// Sort the map for iteration so that parent classes are seen before derived classes. This lets us skip
	// derived classes that have a smaller interest distance than a parent class.
	DiscoveredInterestDistancesSquared.KeySort([](const UClass& LHS, const UClass& RHS) {
		return LHS.IsChildOf(&RHS);
	});

	TMap<UClass*, float> ActorTypeToDistance;

	// If an actor's interest distance is smaller than that of a parent class, there's no need to add interest for that actor.
	// Can't do inline removal since the sorted order is only guaranteed when the map isn't changed.
	for (const auto& ActorInterestDistance : DiscoveredInterestDistancesSquared)
	{
		check(ActorInterestDistance.Key);

		// Spatial distance works in meters, whereas unreal distance works in cm^2. Here we do the dimensionally strange conversion between
		// the two.
		float SpatialDistance = NetCullDistanceSquaredToSpatialDistance(ActorInterestDistance.Value);

		bool bShouldAdd = true;
		for (auto& OptimizedInterestDistance : ActorTypeToDistance)
		{
			if (ActorInterestDistance.Key->IsChildOf(OptimizedInterestDistance.Key) && SpatialDistance <= OptimizedInterestDistance.Value)
			{
				// No need to add this interest distance since it's captured in the optimized map already.
				bShouldAdd = false;
				break;
			}
		}
		if (bShouldAdd)
		{
			ActorTypeToDistance.Add(ActorInterestDistance.Key, SpatialDistance);
		}
	}

	return ActorTypeToDistance;
}

TMap<float, TArray<UClass*>> NetCullDistanceInterest::DedupeDistancesAcrossActorTypes(TMap<UClass*, float> ActorTypeToRadius)
{
	TMap<float, TArray<UClass*>> RadiusToActorTypes;
	for (const auto& InterestDistance : ActorTypeToRadius)
	{
		if (!RadiusToActorTypes.Contains(InterestDistance.Value))
		{
			TArray<UClass*> NewActorTypes;
			RadiusToActorTypes.Add(InterestDistance.Value, NewActorTypes);
		}

		auto& ActorTypes = RadiusToActorTypes[InterestDistance.Value];
		ActorTypes.Add(InterestDistance.Key);
	}
	return RadiusToActorTypes;
}

TArray<QueryConstraint> NetCullDistanceInterest::BuildNonDefaultActorCheckoutConstraints(TMap<float, TArray<UClass*>> DistanceToActorTypes,
																						 USpatialClassInfoManager* ClassInfoManager)
{
	TArray<QueryConstraint> CheckoutConstraints;
	for (const auto& DistanceActorsPair : DistanceToActorTypes)
	{
		QueryConstraint CheckoutRadiusConstraint;

		QueryConstraint RadiusConstraint;
		RadiusConstraint.RelativeCylinderConstraint = RelativeCylinderConstraint{ DistanceActorsPair.Key };
		CheckoutRadiusConstraint.AndConstraint.Add(RadiusConstraint);

		QueryConstraint ActorTypesConstraint;
		for (const auto ActorType : DistanceActorsPair.Value)
		{
			AddTypeHierarchyToConstraint(*ActorType, ActorTypesConstraint, ClassInfoManager);
		}
		CheckoutRadiusConstraint.AndConstraint.Add(ActorTypesConstraint);

		CheckoutConstraints.Add(CheckoutRadiusConstraint);
	}
	return CheckoutConstraints;
}

float NetCullDistanceInterest::NetCullDistanceSquaredToSpatialDistance(float NetCullDistanceSquared)
{
	// Spatial distance works in meters, whereas unreal distance works in cm^2. Here we do the dimensionally strange conversion between the
	// two.
	return FMath::Sqrt(NetCullDistanceSquared / (100.f * 100.f));
}

// The type hierarchy added here are the component IDs that represent the actor type hierarchy. These are added to the given constraint as:
// OR(actor type component IDs...)
void NetCullDistanceInterest::AddTypeHierarchyToConstraint(const UClass& BaseType, QueryConstraint& OutConstraint,
														   USpatialClassInfoManager* ClassInfoManager)
{
	check(ClassInfoManager);
	TArray<Worker_ComponentId> ComponentIds = ClassInfoManager->GetComponentIdsForClassHierarchy(BaseType);
	for (Worker_ComponentId ComponentId : ComponentIds)
	{
		QueryConstraint ComponentTypeConstraint;
		ComponentTypeConstraint.ComponentConstraint = ComponentId;
		OutConstraint.OrConstraint.Add(ComponentTypeConstraint);
	}
}

} // namespace SpatialGDK
