// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "Interop/SpatialClassInfoManager.h"
#include "Schema/Interest.h"

#include <WorkerSDK/improbable/c_worker.h>

class USpatialNetDriver;
class USpatialPackageMapClient;
class AActor;

DECLARE_LOG_CATEGORY_EXTERN(LogInterestFactory, Log, All);

namespace SpatialGDK
{

void GatherClientInterestDistances();

class SPATIALGDK_API InterestFactory
{
public:
	InterestFactory(AActor* InActor, const FClassInfo& InInfo, USpatialNetDriver* InNetDriver);

	Worker_ComponentData CreateInterestData() const;
	Worker_ComponentUpdate CreateInterestUpdate() const;

private:
	Interest CreateInterest() const;

	// Only uses Defined Constraint
	Interest CreateActorInterest() const;
	// Defined Constraint AND Level Constraint
	Interest CreatePlayerOwnedActorInterest() const;

	void AddUserDefinedQueries(const QueryConstraint& LevelConstraints, TArray<SpatialGDK::Query>& OutQueries) const;

	// Checkout Constraint OR AlwaysInterested Constraint
	QueryConstraint CreateSystemDefinedConstraints() const;

	// System Defined Constraints
	QueryConstraint CreateCheckoutRadiusConstraints() const;
	QueryConstraint CreateAlwaysInterestedConstraint() const;
	QueryConstraint CreateAlwaysRelevantConstraint() const;

	// Only checkout entities that are in loaded sublevels
	QueryConstraint CreateLevelConstraints() const;

	void AddObjectToConstraint(UObjectPropertyBase* Property, uint8* Data, QueryConstraint& OutConstraint) const;
	void AddTypeHierarchyToConstraint(const UClass& BaseType, QueryConstraint& OutConstraint) const;

	AActor* Actor;
	const FClassInfo& Info;
	USpatialNetDriver* NetDriver;
	USpatialPackageMapClient* PackageMap;
};

} // namespace SpatialGDK
