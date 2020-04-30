// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "Interop/SpatialClassInfoManager.h"
#include "Schema/Interest.h"

/**
 * This class gives static functionality which returns spatial interest constraints mirroring the net cull distance
 * functionality of Unreal given the spatial class info manager.
 *
 * There are three different ways to generate the checkout radius constraint. The default is legacy NCD interest.
 * This generates a disjunct of radius bucket queries where each spatial bucket is conjoined with all the components representing the actors with that
 * net cull distance. There is also a minimum radius constraint which is not conjoined with any actor components. This is
 * set to the default NCD.
 *
 * If bEnableNetCullDistanceInterest is true, instead each radius bucket generated will only be conjoined with a single
 * marker component representing that net cull distance interest. These marker components are added to entities which represent
 * actors with that defined net cull distance. An important distinction between this and legacy NCD is that
 * all radius buckets now have a conjoined component.
 *
 * Further if also bEnableNetCullDistanceFrequency is true, then for each NCD, multiple queries will be generated.
 * Inside each NCD, there will be further radius buckets all conjoined with the same NCD marker component, but only a small radius
 * will receive the full frequency. More queries will be added representing bigger circles with lower frequencies depending on the
 * configured frequency <-> distance ratio pairs, until the final circle will be at the configured NCD. This approach will generate
 * n queries per client in total where n is the number of configured frequency buckets.
 */

DECLARE_LOG_CATEGORY_EXTERN(LogNetCullDistanceInterest, Log, All);

namespace SpatialGDK
{

class SPATIALGDK_API NetCullDistanceInterest
{
public:
	static FrequencyConstraints CreateCheckoutRadiusConstraints(USpatialClassInfoManager* InClassInfoManager);

	// visible for testing
	static TMap<float, TArray<UClass*>> DedupeDistancesAcrossActorTypes(const TMap<UClass*, float> ComponentSetToRadius);

private:

	static FrequencyConstraints CreateLegacyNetCullDistanceConstraint(USpatialClassInfoManager* InClassInfoManager);
	static FrequencyConstraints CreateNetCullDistanceConstraint(USpatialClassInfoManager* InClassInfoManager);
	static FrequencyConstraints CreateNetCullDistanceConstraintWithFrequency(USpatialClassInfoManager* InClassInfoManager);

	static QueryConstraint GetDefaultCheckoutRadiusConstraint();
	static TMap<UClass*, float> GetActorTypeToRadius();
	static TArray<QueryConstraint> BuildNonDefaultActorCheckoutConstraints(const TMap<float, TArray<UClass*>> DistanceToActorTypes, USpatialClassInfoManager* ClassInfoManager);
	static float NetCullDistanceSquaredToSpatialDistance(float NetCullDistanceSquared);

	static void AddToFrequencyConstraintMap(const float Frequency, const QueryConstraint& Constraint, FrequencyToConstraintsMap& OutFrequencyToConstraints);
	static void AddTypeHierarchyToConstraint(const UClass& BaseType, QueryConstraint& OutConstraint, USpatialClassInfoManager* ClassInfoManager);
};

} // namespace SpatialGDK
