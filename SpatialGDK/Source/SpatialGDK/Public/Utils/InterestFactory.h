// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "Interop/SpatialClassInfoManager.h"
#include "Schema/Interest.h"

#include <WorkerSDK/improbable/c_worker.h>

class UAbstractLBStrategy;
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

	static Interest CreateServerWorkerInterest(const UAbstractLBStrategy* LBStrategy);

private:
	// Build the checkout radius constraints for client workers
	static QueryConstraint CreateClientCheckoutRadiusConstraint(USpatialClassInfoManager* ClassInfoManager);
	static QueryConstraint CreateLegacyNetCullDistanceConstraint(USpatialClassInfoManager* ClassInfoManager);
	static QueryConstraint CreateNetCullDistanceConstraint(USpatialClassInfoManager* ClassInfoManager);
	static QueryConstraint CreateNetCullDistanceConstraintWithFrequency(USpatialClassInfoManager* ClassInfoManager);

	// Builds the result types of necessary components for clients
	static TArray<Worker_ComponentId> CreateClientNonAuthInterestResultType(USpatialClassInfoManager* ClassInfoManager);
	static TArray<Worker_ComponentId> CreateClientAuthInterestResultType(USpatialClassInfoManager* ClassInfoManager);
	static TArray<Worker_ComponentId> CreateServerNonAuthInterestResultType(USpatialClassInfoManager* ClassInfoManager);
	static TArray<Worker_ComponentId> CreateServerAuthInterestResultType(USpatialClassInfoManager* ClassInfoManager);

	Interest CreateInterest() const;

	// Defined Constraint AND Level Constraint
	void AddPlayerControllerActorInterest(Interest& OutInterest) const;
	// The components clients need to see on entities they are have authority over that they don't already see through authority.
	void AddClientSelfInterest(Interest& OutInterest) const;
	// The components servers need to see on entities they have authority over that they don't already see through authority.
	void AddServerSelfInterest(Interest& OutInterest) const;

	void AddUserDefinedQueries(Interest& OutInterest, const AActor* InActor, const QueryConstraint& LevelConstraint) const;
	FrequencyToConstraintsMap GetUserDefinedFrequencyToConstraintsMap(const AActor* InActor) const;
	void GetActorUserDefinedQueryConstraints(const AActor* InActor, FrequencyToConstraintsMap& OutFrequencyToConstraints, bool bRecurseChildren) const;

	TArray<Query> GetNetCullDistanceFrequencyQueries(const QueryConstraint& LevelConstraint) const;

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
