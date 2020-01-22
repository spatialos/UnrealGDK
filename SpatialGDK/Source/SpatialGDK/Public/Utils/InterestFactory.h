// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "Interop/SpatialClassInfoManager.h"
#include "Schema/Interest.h"

#include <WorkerSDK/improbable/c_worker.h>

class USpatialClassInfoManager;
class USpatialPackageMapClient;
class AActor;

DECLARE_LOG_CATEGORY_EXTERN(LogSpatialInterestFactory, Log, All);

namespace SpatialGDK
{
class SPATIALGDK_API SpatialInterestFactory
{
public:
	SpatialInterestFactory(USpatialClassInfoManager* InClassInfoManager);

	void SetPackageMap(USpatialPackageMapClient* InPackageMap) { PackageMap = InPackageMap; }

	void CreateClientCheckoutRadiusConstraint();

	Worker_ComponentData CreateInterestData(AActor* Actor, const FClassInfo& Info) const;
	Worker_ComponentUpdate CreateInterestUpdate(AActor* Actor) const;
	Worker_ComponentUpdate CreateInterestUpdate(AActor* Actor, const FClassInfo& Info) const;

	static Interest CreateServerWorkerInterest();

private:

	struct InterestRequest
	{
		AActor* Actor;
		const FClassInfo& Info;
	};

	Interest CreateInterest(const InterestRequest& Request) const;

	Worker_ComponentUpdate CreateInterestUpdate(const InterestRequest& Request) const;

	// Only uses Defined Constraint
	Interest CreateActorInterest(const InterestRequest& Request) const;
	// Defined Constraint AND Level Constraint
	Interest CreatePlayerOwnedActorInterest(const InterestRequest& Request) const;

	void AddUserDefinedQueries(const InterestRequest& Request, const QueryConstraint& LevelConstraints, TArray<SpatialGDK::Query>& OutQueries) const;

	// Checkout Constraint OR AlwaysInterested OR AlwaysRelevant Constraint
	QueryConstraint CreateSystemDefinedConstraints(const InterestRequest& Request) const;

	// System Defined Constraints
	QueryConstraint CreateCheckoutRadiusConstraints(const InterestRequest& Request) const;
	QueryConstraint CreateAlwaysInterestedConstraint(const InterestRequest& Request) const;
	static QueryConstraint CreateAlwaysRelevantConstraint();

	// Only checkout entities that are in loaded sublevels
	QueryConstraint CreateLevelConstraints(const InterestRequest& Request) const;

	void AddObjectToConstraint(UObjectPropertyBase* Property, uint8* Data, QueryConstraint& OutConstraint) const;

	USpatialClassInfoManager* ClassInfoManager;
	USpatialPackageMapClient* PackageMap;
};

} // namespace SpatialGDK
