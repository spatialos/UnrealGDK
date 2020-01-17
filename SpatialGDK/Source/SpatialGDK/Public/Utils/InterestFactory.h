// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "Interop/SpatialClassInfoManager.h"
#include "Schema/Interest.h"

#include <WorkerSDK/improbable/c_worker.h>

class USpatialClassInfoManager;
class USpatialPackageMapClient;
class AActor;

DECLARE_LOG_CATEGORY_EXTERN(LogInterestFactory, Log, All);

namespace SpatialGDK
{
class SPATIALGDK_API InterestFactory
{
public:
	InterestFactory(AActor* InActor, const FClassInfo& InInfo, USpatialClassInfoManager* InClassInfoManager, USpatialPackageMapClient* InPackageMap);

	static void CreateClientCheckoutRadiusConstraint(USpatialClassInfoManager* ClassInfoManager);

	Worker_ComponentData CreateInterestData() const;
	Worker_ComponentUpdate CreateInterestUpdate() const;

	static Interest CreateServerWorkerInterest();

private:
	static QueryConstraint GetDefaultCheckoutRadiusConstraint();
	static TMap<UClass*, float> GetActorTypeToRadius();
	static TMap<float, TArray<UClass*>> DedupeDistancesAcrossActorTypes(const TMap<UClass*, float> ComponentSetToRadius);
	static TArray<QueryConstraint> BuildNonDefaultActorCheckoutConstraints(const TMap<float, TArray<UClass*>> DistanceToActorTypes, USpatialClassInfoManager* ClassInfoManager);

	Interest CreateInterest() const;

	// Only uses Defined Constraint
	Interest CreateActorInterest() const;
	// Defined Constraint AND Level Constraint
	Interest CreatePlayerOwnedActorInterest() const;

	void AddUserDefinedQueries(const QueryConstraint& LevelConstraints, TArray<SpatialGDK::Query>& OutQueries) const;

	// Checkout Constraint OR AlwaysInterested OR AlwaysRelevant Constraint
	QueryConstraint CreateSystemDefinedConstraints() const;

	// System Defined Constraints
	QueryConstraint CreateCheckoutRadiusConstraints() const;
	QueryConstraint CreateAlwaysInterestedConstraint() const;
	static QueryConstraint CreateAlwaysRelevantConstraint();

	// Only checkout entities that are in loaded sublevels
	QueryConstraint CreateLevelConstraints() const;

	void AddObjectToConstraint(UObjectPropertyBase* Property, uint8* Data, QueryConstraint& OutConstraint) const;
	static void AddTypeHierarchyToConstraint(const UClass& BaseType, QueryConstraint& OutConstraint, USpatialClassInfoManager* ClassInfoManager);

	AActor* Actor;
	const FClassInfo& Info;
	USpatialClassInfoManager* ClassInfoManager;
	USpatialPackageMapClient* PackageMap;
};

} // namespace SpatialGDK
