// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "Interop/SpatialClassInfoManager.h"
#include "Schema/Interest.h"

#include <WorkerSDK/improbable/c_worker.h>

/**
 * The SpatialInterestFactory is responsible for creating spatial Interest component state and updates for a GDK game.
 *
 * It has two dependencies:
 *   - the class info manager for finding level components and for creating user defined queries from ActorInterestComponents
 *   - the package map, for finding unreal object references as part of creating AlwaysInterested constraints
 *     (TODO) remove this dependency when/if we drop support for the AlwaysInterested constraint
 *
 * The interest factory is initialized within and has its lifecycle tied to the spatial net driver.
 *
 * There are two public types of functionality for this class.
 *
 * The first is actor interest. The factory takes information about an actor (the object, info and corresponding entity ID)
 * and produces an interest data/update for that entity. This interest contains anything specific to that actor, such as self constraints
 * for servers and clients, and if the actor is a player controller, the client worker's interest is also built for that actor.
 *
 * The other is server worker interest. Given a load balancing strategy, the factory will take the strategy's defined query constraint
 * and produce an interest component to exist on the server's worker entity. This interest component contains the primary interest query made
 * by that server worker.
 */

class UAbstractLBStrategy;
class USpatialClassInfoManager;
class USpatialPackageMapClient;

DECLARE_LOG_CATEGORY_EXTERN(LogInterestFactory, Log, All);

namespace SpatialGDK
{
class SPATIALGDK_API SpatialInterestFactory
{
public:
	SpatialInterestFactory(USpatialClassInfoManager* InClassInfoManager, USpatialPackageMapClient* InPackageMap);

	Worker_ComponentData CreateInterestData(AActor* InActor, const FClassInfo& InInfo, const Worker_EntityId InEntityId) const;
	Worker_ComponentUpdate CreateInterestUpdate(AActor* InActor, const FClassInfo& InInfo, const Worker_EntityId InEntityId) const;

	Interest CreateServerWorkerInterest(const UAbstractLBStrategy* LBStrategy);

private:
	// Shared constraints and result types are created at initialization and reused throughout the lifetime of the factory.
	void CreateAndCacheInterestState();

	// Build the checkout radius constraints for client workers
	// TODO: Pull out into checkout radius constraint utils
	static QueryConstraint CreateClientCheckoutRadiusConstraint(USpatialClassInfoManager* ClassInfoManager);
	static QueryConstraint CreateLegacyNetCullDistanceConstraint(USpatialClassInfoManager* ClassInfoManager);
	static QueryConstraint CreateNetCullDistanceConstraint(USpatialClassInfoManager* ClassInfoManager);
	static QueryConstraint CreateNetCullDistanceConstraintWithFrequency(USpatialClassInfoManager* ClassInfoManager);

	// Builds the result types of necessary components for clients
	// TODO: create and pull out into result types class
	static ResultType CreateClientNonAuthInterestResultType(USpatialClassInfoManager* ClassInfoManager);
	static ResultType CreateClientAuthInterestResultType(USpatialClassInfoManager* ClassInfoManager);
	static ResultType CreateServerNonAuthInterestResultType(USpatialClassInfoManager* ClassInfoManager);
	static ResultType CreateServerAuthInterestResultType(USpatialClassInfoManager* ClassInfoManager);

	Interest CreateInterest(AActor* InActor, const FClassInfo& InInfo, const Worker_EntityId InEntityId) const;

	// Defined Constraint AND Level Constraint
	void AddPlayerControllerActorInterest(Interest& OutInterest, const AActor* InActor, const FClassInfo& InInfo) const;
	// Self interests require the entity ID to know which entity is "self". This would no longer be required if there was a first class self constraint.
	// The components clients need to see on entities they are have authority over that they don't already see through authority.
	void AddClientSelfInterest(Interest& OutInterest, const Worker_EntityId& EntityId) const;
	// The components servers need to see on entities they have authority over that they don't already see through authority.
	void AddServerSelfInterest(Interest& OutInterest, const Worker_EntityId& EntityId) const;

	// Add the checkout radius, always relevant, or always interested query.
	void AddSystemQuery(Interest& OutInterest, const AActor* InActor, const FClassInfo& InInfo, const QueryConstraint& LevelConstraint) const;

	void AddUserDefinedQueries(Interest& OutInterest, const AActor* InActor, const QueryConstraint& LevelConstraint) const;
	FrequencyToConstraintsMap GetUserDefinedFrequencyToConstraintsMap(const AActor* InActor) const;
	void GetActorUserDefinedQueryConstraints(const AActor* InActor, FrequencyToConstraintsMap& OutFrequencyToConstraints, bool bRecurseChildren) const;

	void AddNetCullDistanceFrequencyQueries(Interest& OutInterest, const QueryConstraint& LevelConstraint) const;

	static void AddComponentQueryPairToInterestComponent(Interest& OutInterest, const Worker_ComponentId ComponentId, const Query& QueryToAdd);

	// System Defined Constraints
	QueryConstraint CreateCheckoutRadiusConstraints(const AActor* InActor) const;
	QueryConstraint CreateAlwaysInterestedConstraint(const AActor* InActor, const FClassInfo& InInfo) const;
	static QueryConstraint CreateAlwaysRelevantConstraint();

	// Only checkout entities that are in loaded sub-levels
	QueryConstraint CreateLevelConstraints(const AActor* InActor) const;

	void AddObjectToConstraint(UObjectPropertyBase* Property, uint8* Data, QueryConstraint& OutConstraint) const;

	// If the result types flag is flipped, set the specified result type.
	static void SetResultType(Query& OutQuery, const ResultType& InResultType);

	USpatialClassInfoManager* ClassInfoManager;
	USpatialPackageMapClient* PackageMap;
};

} // namespace SpatialGDK
