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

	static void CreateAndCacheInterestState(USpatialClassInfoManager* ClassInfoManager);

	Worker_ComponentData CreateInterestData(Worker_EntityId EntityId) const;
	Worker_ComponentUpdate CreateInterestUpdate(Worker_EntityId EntityId) const;

	static Interest CreateServerWorkerInterest();

private:
	// Build the checkout radius constraints for client workers
	static QueryConstraint CreateClientCheckoutRadiusConstraint(USpatialClassInfoManager* ClassInfoManager);
	static QueryConstraint CreateLegacyNetCullDistanceConstraint(USpatialClassInfoManager* ClassInfoManager);
	static QueryConstraint CreateNetCullDistanceConstraint(USpatialClassInfoManager* ClassInfoManager);
	static QueryConstraint CreateNetCullDistanceConstraintWithFrequency(USpatialClassInfoManager* ClassInfoManager);

	// Builds the result type of necessary components for clients to see on NON-AUTHORITATIVE entities
	static TArray<Worker_ComponentId> CreateClientResultType(USpatialClassInfoManager* ClassInfoManager);
	


	Interest CreateInterest(Worker_EntityId EntityId) const;

	// Only uses Defined Constraint
	void AddActorInterest(Interest& InInterest) const;
	// Defined Constraint AND Level Constraint
	void AddPlayerControllerActorInterest(Interest& InInterest) const;
	// The components clients need to see on entities they are have authority over.
	void AddClientSelfInterest(Interest& InInterest, Worker_EntityId EntityId) const;

	static void AddComponentQueryPairToInterestComponent(Interest& InInterest, const Worker_ComponentId ComponentId, const Query QueryToAdd);

	TArray<Query> GetUserDefinedQueries(const QueryConstraint& LevelConstraints) const;

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
	USpatialClassInfoManager* ClassInfoManager;
	USpatialPackageMapClient* PackageMap;
};

} // namespace SpatialGDK
