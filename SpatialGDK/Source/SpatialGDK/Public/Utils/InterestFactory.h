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
	InterestFactory(AActor* InActor, const FClassInfo& InInfo, const Worker_EntityId InEntityId, USpatialClassInfoManager* InClassInfoManager, USpatialPackageMapClient* InPackageMap);

	static void CreateAndCacheInterestState(USpatialClassInfoManager* ClassInfoManager);

	Worker_ComponentData CreateInterestData() const;
	Worker_ComponentUpdate CreateInterestUpdate() const;

	static Interest CreateServerWorkerInterest();

private:
	// Build the checkout radius constraints for client workers
	static QueryConstraint CreateClientCheckoutRadiusConstraint(USpatialClassInfoManager* ClassInfoManager);
	static QueryConstraint CreateLegacyNetCullDistanceConstraint(USpatialClassInfoManager* ClassInfoManager);
	static QueryConstraint CreateNetCullDistanceConstraint(USpatialClassInfoManager* ClassInfoManager);
	static QueryConstraint CreateNetCullDistanceConstraintWithFrequency(USpatialClassInfoManager* ClassInfoManager);

	// Builds the result types of necessary components for clients
	static TArray<Worker_ComponentId> CreateClientNonAuthInterestResultType(USpatialClassInfoManager* ClassInfoManager);
	static TArray<Worker_ComponentId> CreateClientAuthInterestResultType(USpatialClassInfoManager* ClassInfoManager);

	Interest CreateInterest() const;

	// Only uses Defined Constraint
	void AddActorInterest(Interest& OutInterest) const;
	// Defined Constraint AND Level Constraint
	void AddPlayerControllerActorInterest(Interest& OutInterest) const;
	// The components clients need to see on entities they are have authority over.
	void AddClientSelfInterest(Interest& OutInterest) const;

	void GetActorUserDefinedQueries(const AActor* InActor, const QueryConstraint& LevelConstraints, TArray<SpatialGDK::Query>& OutQueries, bool bRecurseChildren) const;
	TArray<Query> GetUserDefinedQueries(const QueryConstraint& LevelConstraints) const;

	static void AddComponentQueryPairToInterestComponent(Interest& OutInterest, const Worker_ComponentId ComponentId, const Query& QueryToAdd);

	// Checkout Constraint OR AlwaysInterested OR AlwaysRelevant Constraint
	QueryConstraint CreateSystemDefinedConstraints() const;

	// System Defined Constraints
	QueryConstraint CreateCheckoutRadiusConstraints() const;
	QueryConstraint CreateAlwaysInterestedConstraint() const;
	static QueryConstraint CreateAlwaysRelevantConstraint();

	// Only checkout entities that are in loaded sub-levels
	QueryConstraint CreateLevelConstraints() const;	

	void AddObjectToConstraint(UObjectPropertyBase* Property, uint8* Data, QueryConstraint& OutConstraint) const;

	AActor* Actor;
	const FClassInfo& Info;
	const Worker_EntityId EntityId;
	USpatialClassInfoManager* ClassInfoManager;
	USpatialPackageMapClient* PackageMap;
};

} // namespace SpatialGDK
