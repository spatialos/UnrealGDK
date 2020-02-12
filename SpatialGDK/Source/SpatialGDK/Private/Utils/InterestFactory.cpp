// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "Utils/InterestFactory.h"

#include "Engine/World.h"
#include "Engine/Classes/GameFramework/Actor.h"
#include "GameFramework/PlayerController.h"
#include "UObject/UObjectIterator.h"
#include "Utils/CheckoutRadiusConstraintUtils.h"

#include "EngineClasses/Components/ActorInterestComponent.h"
#include "EngineClasses/SpatialNetConnection.h"
#include "EngineClasses/SpatialNetDriver.h"
#include "EngineClasses/SpatialPackageMapClient.h"
#include "SpatialGDKSettings.h"
#include "SpatialConstants.h"
#include "UObject/UObjectIterator.h"

DEFINE_LOG_CATEGORY(LogInterestFactory);

DECLARE_STATS_GROUP(TEXT("InterestFactory"), STATGROUP_SpatialInterestFactory, STATCAT_Advanced);
DECLARE_CYCLE_STAT(TEXT("AddUserDefinedQueries"), STAT_InterestFactoryAddUserDefinedQueries, STATGROUP_SpatialInterestFactory);

namespace SpatialGDK
{
struct FrequencyConstraint
{
	float Frequency;
	SpatialGDK::QueryConstraint Constraint;
};
// Used to cache checkout radius constraints with frequency settings, so queries can be quickly recreated.
static TArray<FrequencyConstraint> CheckoutConstraints;

// The checkout radius constraint is built once for all actors in CreateCheckoutRadiusConstraint as it is equivalent for all actors.
// It is built once per net driver initialization.
static QueryConstraint ClientCheckoutRadiusConstraint;

// Cache the result types of queries.
static TArray<Worker_ComponentId> ClientNonAuthInterestResultType;
static TArray<Worker_ComponentId> ClientAuthInterestResultType;
static TArray<Worker_ComponentId> ServerNonAuthInterestResultType;
static TArray<Worker_ComponentId> ServerAuthInterestResultType;

InterestFactory::InterestFactory(AActor* InActor, const FClassInfo& InInfo, const Worker_EntityId InEntityId, USpatialClassInfoManager* InClassInfoManager, USpatialPackageMapClient* InPackageMap)
	: Actor(InActor)
	, Info(InInfo)
	, EntityId(InEntityId)
	, ClassInfoManager(InClassInfoManager)
	, PackageMap(InPackageMap)
{
}

void InterestFactory::CreateAndCacheInterestState(USpatialClassInfoManager* ClassInfoManager)
{
	ClientCheckoutRadiusConstraint = CreateClientCheckoutRadiusConstraint(ClassInfoManager);
	ClientNonAuthInterestResultType = CreateClientNonAuthInterestResultType(ClassInfoManager);
	ClientAuthInterestResultType = CreateClientAuthInterestResultType(ClassInfoManager);
	ServerNonAuthInterestResultType = CreateServerNonAuthInterestResultType(ClassInfoManager);
	ServerAuthInterestResultType = CreateServerAuthInterestResultType(ClassInfoManager);
}

QueryConstraint InterestFactory::CreateClientCheckoutRadiusConstraint(USpatialClassInfoManager* ClassInfoManager)
{
	const USpatialGDKSettings* SpatialGDKSettings = GetDefault<USpatialGDKSettings>();
	QueryConstraint CheckoutRadiusConstraint;
	CheckoutConstraints.Empty();

	if (!SpatialGDKSettings->bEnableNetCullDistanceInterest)
	{
		CheckoutRadiusConstraint = CreateLegacyNetCullDistanceConstraint(ClassInfoManager);
	}
	else
	{
		if (!SpatialGDKSettings->bEnableNetCullDistanceFrequency)
		{
			CheckoutRadiusConstraint = CreateNetCullDistanceConstraint(ClassInfoManager);
		}
		else
		{
			CheckoutRadiusConstraint = CreateNetCullDistanceConstraintWithFrequency(ClassInfoManager);
		}
	}

	return CheckoutRadiusConstraint;
}

QueryConstraint InterestFactory::CreateLegacyNetCullDistanceConstraint(USpatialClassInfoManager* ClassInfoManager)
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

	CheckoutRadiusConstraint.OrConstraint.Add(CheckoutRadiusConstraintUtils::GetDefaultCheckoutRadiusConstraint());

	// Get interest distances for each actor.
	TMap<UClass*, float> ActorComponentSetToRadius = CheckoutRadiusConstraintUtils::GetActorTypeToRadius();

	// For every interest distance that we still want, build a map from radius to list of actor type components that match that radius.
	TMap<float, TArray<UClass*>> DistanceToActorTypeComponents = CheckoutRadiusConstraintUtils::DedupeDistancesAcrossActorTypes(
		ActorComponentSetToRadius);

	// The previously built map removes duplicates of spatial constraints. Now the actual query constraints can be built of the form:
	// OR(AND(cylinder(radius), OR(actor 1 components, actor 2 components, ...)), ...)
	// which is equivalent to having a separate spatial query for each actor type if the radius is the same.
	TArray<QueryConstraint> CheckoutRadiusConstraints = CheckoutRadiusConstraintUtils::BuildNonDefaultActorCheckoutConstraints(
		DistanceToActorTypeComponents, ClassInfoManager);

	// Add all the different actor queries to the overall checkout constraint.
	for (auto& ActorCheckoutConstraint : CheckoutRadiusConstraints)
	{
		CheckoutRadiusConstraint.OrConstraint.Add(ActorCheckoutConstraint);
	}

	return CheckoutRadiusConstraint;
}

QueryConstraint InterestFactory::CreateNetCullDistanceConstraint(USpatialClassInfoManager* ClassInfoManager)
{
	QueryConstraint CheckoutRadiusConstraintRoot;

	const TMap<float, Worker_ComponentId>& NetCullDistancesToComponentIds = ClassInfoManager->GetNetCullDistanceToComponentIds();

	for (const auto& DistanceComponentPair : NetCullDistancesToComponentIds)
	{
		const float MaxCheckoutRadiusMeters = CheckoutRadiusConstraintUtils::NetCullDistanceSquaredToSpatialDistance(DistanceComponentPair.Key);

		QueryConstraint ComponentConstraint;
		ComponentConstraint.ComponentConstraint = DistanceComponentPair.Value;

		QueryConstraint RadiusConstraint;
		RadiusConstraint.RelativeCylinderConstraint = RelativeCylinderConstraint{ MaxCheckoutRadiusMeters };

		QueryConstraint CheckoutRadiusConstraint;
		CheckoutRadiusConstraint.AndConstraint.Add(RadiusConstraint);
		CheckoutRadiusConstraint.AndConstraint.Add(ComponentConstraint);

		CheckoutRadiusConstraintRoot.OrConstraint.Add(CheckoutRadiusConstraint);
	}

	return CheckoutRadiusConstraintRoot;
}

QueryConstraint InterestFactory::CreateNetCullDistanceConstraintWithFrequency(USpatialClassInfoManager* ClassInfoManager)
{
	QueryConstraint CheckoutRadiusConstraintRoot;

	const USpatialGDKSettings* SpatialGDKSettings = GetDefault<USpatialGDKSettings>();
	const TMap<float, Worker_ComponentId>& NetCullDistancesToComponentIds = ClassInfoManager->GetNetCullDistanceToComponentIds();

	for (const auto& DistanceComponentPair : NetCullDistancesToComponentIds)
	{
		const float MaxCheckoutRadiusMeters = CheckoutRadiusConstraintUtils::NetCullDistanceSquaredToSpatialDistance(DistanceComponentPair.Key);

		QueryConstraint ComponentConstraint;
		ComponentConstraint.ComponentConstraint = DistanceComponentPair.Value;

		{
			// Add default interest query which doesn't include a frequency
			float FullFrequencyCheckoutRadius = MaxCheckoutRadiusMeters * SpatialGDKSettings->FullFrequencyNetCullDistanceRatio;

			QueryConstraint RadiusConstraint;
			RadiusConstraint.RelativeCylinderConstraint = RelativeCylinderConstraint{ FullFrequencyCheckoutRadius };

			QueryConstraint CheckoutRadiusConstraint;
			CheckoutRadiusConstraint.AndConstraint.Add(RadiusConstraint);
			CheckoutRadiusConstraint.AndConstraint.Add(ComponentConstraint);

			CheckoutRadiusConstraintRoot.OrConstraint.Add(CheckoutRadiusConstraint);
		}

		// Add interest query for specified distance/frequency pairs
		for (const auto& DistanceFrequencyPair : SpatialGDKSettings->InterestRangeFrequencyPairs)
		{
			float CheckoutRadius = MaxCheckoutRadiusMeters * DistanceFrequencyPair.DistanceRatio;

			QueryConstraint RadiusConstraint;
			RadiusConstraint.RelativeCylinderConstraint = RelativeCylinderConstraint{ CheckoutRadius };

			QueryConstraint CheckoutRadiusConstraint;
			CheckoutRadiusConstraint.AndConstraint.Add(RadiusConstraint);
			CheckoutRadiusConstraint.AndConstraint.Add(ComponentConstraint);

			CheckoutConstraints.Add({ DistanceFrequencyPair.Frequency, CheckoutRadiusConstraint });
		}
	}

	return CheckoutRadiusConstraintRoot;
}

TArray<Worker_ComponentId> InterestFactory::CreateClientNonAuthInterestResultType(USpatialClassInfoManager* ClassInfoManager)
{
	TArray<Worker_ComponentId> ResultType;

	// Add the required unreal components
	ResultType.Append(SpatialConstants::REQUIRED_COMPONENTS_FOR_NON_AUTH_CLIENT_INTEREST);

	// Add all data components- clients don't need to see handover or owner only components on other entities.
	ResultType.Append(ClassInfoManager->GetComponentIdsForComponentType(ESchemaComponentType::SCHEMA_Data));

	// In direct disagreement with the above comment, we add the owner only components as well.
	// This is because GDK workers currently make assumptions about information being available at the point of possession.
	// TODO(jacques): fix (unr-2865)
	ResultType.Append(ClassInfoManager->GetComponentIdsForComponentType(ESchemaComponentType::SCHEMA_OwnerOnly));

	return ResultType;
}

TArray<Worker_ComponentId> InterestFactory::CreateClientAuthInterestResultType(USpatialClassInfoManager* ClassInfoManager)
{
	TArray<Worker_ComponentId> ResultType;

	// Add the required known components
	ResultType.Append(SpatialConstants::REQUIRED_COMPONENTS_FOR_AUTH_CLIENT_INTEREST);
	ResultType.Append(SpatialConstants::REQUIRED_COMPONENTS_FOR_NON_AUTH_CLIENT_INTEREST);

	// Add all the generated unreal components
	ResultType.Append(ClassInfoManager->GetComponentIdsForComponentType(ESchemaComponentType::SCHEMA_Data));
	ResultType.Append(ClassInfoManager->GetComponentIdsForComponentType(ESchemaComponentType::SCHEMA_OwnerOnly));

	return ResultType;
}

TArray<Worker_ComponentId> InterestFactory::CreateServerNonAuthInterestResultType(USpatialClassInfoManager* ClassInfoManager)
{
	TArray<Worker_ComponentId> ResultType;

	// Add the required unreal components
	ResultType.Append(SpatialConstants::REQUIRED_COMPONENTS_FOR_NON_AUTH_SERVER_INTEREST);

	// Add all data, owner only, and handover components
	ResultType.Append(ClassInfoManager->GetComponentIdsForComponentType(ESchemaComponentType::SCHEMA_Data));
	ResultType.Append(ClassInfoManager->GetComponentIdsForComponentType(ESchemaComponentType::SCHEMA_OwnerOnly));
	ResultType.Append(ClassInfoManager->GetComponentIdsForComponentType(ESchemaComponentType::SCHEMA_Handover));

	return ResultType;
}

TArray<Worker_ComponentId> InterestFactory::CreateServerAuthInterestResultType(USpatialClassInfoManager* ClassInfoManager)
{
	// Just the components that we won't have already checked out through authority
	return SpatialConstants::REQUIRED_COMPONENTS_FOR_AUTH_SERVER_INTEREST;
}

Worker_ComponentData InterestFactory::CreateInterestData() const
{
	return CreateInterest().CreateInterestData();
}

Worker_ComponentUpdate InterestFactory::CreateInterestUpdate() const
{
	return CreateInterest().CreateInterestUpdate();
}

Interest InterestFactory::CreateServerWorkerInterest(const USpatialNetDriver& NetDriver)
{
	QueryConstraint Constraint;

	const USpatialGDKSettings* SpatialGDKSettings = GetDefault<USpatialGDKSettings>();
	if (SpatialGDKSettings->bEnableServerQBI)
	{
		UE_LOG(LogInterestFactory, Warning, TEXT("For performance reasons, it's recommended to disable server QBI"));
	}

	if (!SpatialGDKSettings->bEnableServerQBI && SpatialGDKSettings->bEnableOffloading)
	{
		// In offloading scenarios, hijack the server worker entity to ensure each server has interest in all entities
		Constraint.ComponentConstraint = SpatialConstants::POSITION_COMPONENT_ID;
	}
	else
	{
		// Ensure server worker receives always relevant entities
		Constraint = CreateAlwaysRelevantConstraint();
	}

	Query Query;
	Query.Constraint = Constraint;
	if (SpatialGDKSettings->bEnableResultTypes)
	{
		Query.ResultComponentId = ServerNonAuthInterestResultType;
	}
	else
	{
		Query.FullSnapshotResult = true;
	}

	ComponentInterest Queries;
	Queries.Queries.Add(Query);

	if (SpatialGDKSettings->bEnableUnrealLoadBalancer && SpatialGDKSettings->bEnableServerQBI)
	{
		if (const UAbstractLBStrategy* LoadBalancer = NetDriver.LoadBalanceStrategy)
		{
			// The load balancer won't be ready when the worker initially connects to SpatialOS. It needs
			// to wait for the virtual worker mappings to be replicated.
			if (LoadBalancer->IsReady())
			{
				LoadBalancer->CreateWorkerInterestQueries(Queries.Queries);
			}
		}
	}

	Interest ServerInterest;
	ServerInterest.ComponentInterestMap.Add(SpatialConstants::POSITION_COMPONENT_ID, Queries);

	return ServerInterest;
}

Interest InterestFactory::CreateInterest() const
{
	const USpatialGDKSettings* Settings = GetDefault<USpatialGDKSettings>();
	Interest ResultInterest;

	if (Actor->IsA(APlayerController::StaticClass()))
	{
		// Put the "main" interest queries on the player controller
		AddPlayerControllerActorInterest(ResultInterest);
	}

	if (Actor->GetNetConnection() != nullptr && Settings->bEnableResultTypes)
	{
		// Clients need to see owner only and server RPC components on entities they have authority over
		AddClientSelfInterest(ResultInterest);
	}

	if (Settings->bEnableServerQBI)
	{
		return Interest{};
	}

	if (Settings->bEnableResultTypes)
	{
		// Every actor needs a self query for the server to the client RPC endpoint
		AddServerSelfInterest(ResultInterest);
	}

	return ResultInterest;
}

void InterestFactory::AddActorInterest(Interest& OutInterest) const
{
	QueryConstraint SystemConstraints = CreateSystemDefinedConstraints();

	if (!SystemConstraints.IsValid())
	{
		return;
	}

	Query NewQuery;
	NewQuery.Constraint = SystemConstraints;
	if (GetDefault<USpatialGDKSettings>()->bEnableResultTypes)
	{
		NewQuery.ResultComponentId = ServerNonAuthInterestResultType;
	}
	else
	{
		NewQuery.FullSnapshotResult = true;
	}

	AddComponentQueryPairToInterestComponent(OutInterest, SpatialConstants::POSITION_COMPONENT_ID, NewQuery);
}

void InterestFactory::AddPlayerControllerActorInterest(Interest& OutInterest) const
{
	const USpatialGDKSettings* SpatialGDKSettings = GetDefault<USpatialGDKSettings>();

	QueryConstraint SystemConstraints = CreateSystemDefinedConstraints();

	// Clients should only check out entities that are in loaded sub-levels
	QueryConstraint LevelConstraints = CreateLevelConstraints();

	QueryConstraint ClientConstraint;

	if (SystemConstraints.IsValid())
	{
		ClientConstraint.AndConstraint.Add(SystemConstraints);
	}

	if (LevelConstraints.IsValid())
	{
		ClientConstraint.AndConstraint.Add(LevelConstraints);
	}

	Query ClientQuery;
	ClientQuery.Constraint = ClientConstraint;

	if (SpatialGDKSettings->bEnableResultTypes)
	{
		ClientQuery.ResultComponentId = ClientNonAuthInterestResultType;
	}
	else
	{
		ClientQuery.FullSnapshotResult = true;
	}

	const Worker_ComponentId ClientEndpointComponentId = SpatialConstants::GetClientAuthorityComponent(SpatialGDKSettings->UseRPCRingBuffer());

	AddComponentQueryPairToInterestComponent(OutInterest, ClientEndpointComponentId, ClientQuery);

	TArray<Query> UserQueries = GetUserDefinedQueries(LevelConstraints);
	for (const auto& UserQuery : UserQueries)
	{
		AddComponentQueryPairToInterestComponent(OutInterest, ClientEndpointComponentId, UserQuery);
	}

	if (SpatialGDKSettings->bEnableNetCullDistanceFrequency)
	{
		for (const auto& RadiusCheckoutConstraints : CheckoutConstraints)
		{
			SpatialGDK::Query NewQuery{};

			NewQuery.Constraint.AndConstraint.Add(RadiusCheckoutConstraints.Constraint);

			if (LevelConstraints.IsValid())
			{
				NewQuery.Constraint.AndConstraint.Add(LevelConstraints);
			}

			NewQuery.Frequency = RadiusCheckoutConstraints.Frequency;

			if (SpatialGDKSettings->bEnableResultTypes)
			{
				NewQuery.ResultComponentId = ClientNonAuthInterestResultType;
			}
			else
			{
				NewQuery.FullSnapshotResult = true;
			}

			AddComponentQueryPairToInterestComponent(OutInterest, ClientEndpointComponentId, NewQuery);
		}
	}
}

void InterestFactory::AddClientSelfInterest(Interest& OutInterest) const
{
	Query NewQuery;
	// Just an entity ID constraint is fine, as clients should not become authoritative over entities outside their loaded levels
	NewQuery.Constraint.EntityIdConstraint = EntityId;

	NewQuery.ResultComponentId = ClientAuthInterestResultType;

	AddComponentQueryPairToInterestComponent(OutInterest, SpatialConstants::GetClientAuthorityComponent(GetDefault<USpatialGDKSettings>()->UseRPCRingBuffer()), NewQuery);
}

void InterestFactory::AddServerSelfInterest(Interest& OutInterest) const
{
	// Add a query for components all servers need to read client data
	Query ClientQuery;
	ClientQuery.Constraint.EntityIdConstraint = EntityId;
	ClientQuery.ResultComponentId = ServerAuthInterestResultType;
	AddComponentQueryPairToInterestComponent(OutInterest, SpatialConstants::POSITION_COMPONENT_ID, ClientQuery);

	// Add a query for the load balancing worker (whoever is delegated the ACL) to read the authority intent
	Query LoadBalanceQuery;
	LoadBalanceQuery.Constraint.EntityIdConstraint = EntityId;
	LoadBalanceQuery.ResultComponentId = TArray<Worker_ComponentId>{ SpatialConstants::AUTHORITY_INTENT_COMPONENT_ID };
	AddComponentQueryPairToInterestComponent(OutInterest, SpatialConstants::ENTITY_ACL_COMPONENT_ID, LoadBalanceQuery);
}

void InterestFactory::AddComponentQueryPairToInterestComponent(Interest& OutInterest, const Worker_ComponentId ComponentId, const Query& QueryToAdd)
{
	if (!OutInterest.ComponentInterestMap.Contains(ComponentId))
	{
		ComponentInterest NewComponentInterest;
		OutInterest.ComponentInterestMap.Add(ComponentId, NewComponentInterest);
	}
	OutInterest.ComponentInterestMap[ComponentId].Queries.Add(QueryToAdd);
}

void InterestFactory::GetActorUserDefinedQueries(const AActor* InActor, const QueryConstraint& LevelConstraints, TArray<SpatialGDK::Query>& OutQueries, bool bRecurseChildren) const
{
	check(ClassInfoManager);

	if (InActor == nullptr)
	{
		return;
	}

	TArray<UActorInterestComponent*> ActorInterestComponents;
	InActor->GetComponents<UActorInterestComponent>(ActorInterestComponents);
	if (ActorInterestComponents.Num() == 1)
	{
		ActorInterestComponents[0]->CreateQueries(*ClassInfoManager, LevelConstraints, OutQueries);
	}
	else if (ActorInterestComponents.Num() > 1)
	{
		UE_LOG(LogInterestFactory, Error, TEXT("%s has more than one ActorInterestComponent"), *InActor->GetPathName());
		checkNoEntry()
	}

	if (bRecurseChildren)
	{
		for (const auto& Child : InActor->Children)
		{
			GetActorUserDefinedQueries(Child, LevelConstraints, OutQueries, true);
		}
	}
}

TArray<Query> InterestFactory::GetUserDefinedQueries(const QueryConstraint& LevelConstraints) const
{
	SCOPE_CYCLE_COUNTER(STAT_InterestFactoryAddUserDefinedQueries);

	TArray<Query> Queries;

	if (APlayerController* PlayerController = Cast<APlayerController>(Actor))
	{
		GetActorUserDefinedQueries(Actor, LevelConstraints, Queries, true);
		GetActorUserDefinedQueries(PlayerController->GetPawn(), LevelConstraints, Queries, true);
	}
	else
	{
		GetActorUserDefinedQueries(Actor, LevelConstraints, Queries, false);
	}

	return Queries;
}

QueryConstraint InterestFactory::CreateSystemDefinedConstraints() const
{
	QueryConstraint CheckoutRadiusConstraint = CreateCheckoutRadiusConstraints();
	QueryConstraint AlwaysInterestedConstraint = CreateAlwaysInterestedConstraint();
	QueryConstraint AlwaysRelevantConstraint = CreateAlwaysRelevantConstraint();

	QueryConstraint SystemDefinedConstraints;

	if (CheckoutRadiusConstraint.IsValid())
	{
		SystemDefinedConstraints.OrConstraint.Add(CheckoutRadiusConstraint);
	}

	if (AlwaysInterestedConstraint.IsValid())
	{
		SystemDefinedConstraints.OrConstraint.Add(AlwaysInterestedConstraint);
	}

	if (AlwaysRelevantConstraint.IsValid())
	{
		SystemDefinedConstraints.OrConstraint.Add(AlwaysRelevantConstraint);
	}

	return SystemDefinedConstraints;
}

QueryConstraint InterestFactory::CreateCheckoutRadiusConstraints() const
{
	// If the actor has a component to specify interest and that indicates that we shouldn't generate
	// constraints based on NetCullDistanceSquared, abort. There is a check elsewhere to ensure that
	// there is at most one ActorInterestQueryComponent.
	TArray<UActorInterestComponent*> ActorInterestComponents;
	Actor->GetComponents<UActorInterestComponent>(ActorInterestComponents);
	if (ActorInterestComponents.Num() == 1)
	{
		const UActorInterestComponent* ActorInterest = ActorInterestComponents[0];
		check(ActorInterest);
		if (!ActorInterest->bUseNetCullDistanceSquaredForCheckoutRadius)
		{
			return QueryConstraint{};
		}
	}

	// Otherwise, return the previously computed checkout radius constraint.
	return ClientCheckoutRadiusConstraint;
}

QueryConstraint InterestFactory::CreateAlwaysInterestedConstraint() const
{
	QueryConstraint AlwaysInterestedConstraint;

	for (const FInterestPropertyInfo& PropertyInfo : Info.InterestProperties)
	{
		uint8* Data = (uint8*)Actor + PropertyInfo.Offset;
		if (UObjectPropertyBase* ObjectProperty = Cast<UObjectPropertyBase>(PropertyInfo.Property))
		{
			AddObjectToConstraint(ObjectProperty, Data, AlwaysInterestedConstraint);
		}
		else if (UArrayProperty* ArrayProperty = Cast<UArrayProperty>(PropertyInfo.Property))
		{
			FScriptArrayHelper ArrayHelper(ArrayProperty, Data);
			for (int i = 0; i < ArrayHelper.Num(); i++)
			{
				AddObjectToConstraint(Cast<UObjectPropertyBase>(ArrayProperty->Inner), ArrayHelper.GetRawPtr(i), AlwaysInterestedConstraint);
			}
		}
		else
		{
			checkNoEntry();
		}
	}

	return AlwaysInterestedConstraint;
}

QueryConstraint InterestFactory::CreateAlwaysRelevantConstraint()
{
	QueryConstraint AlwaysRelevantConstraint;

	Worker_ComponentId ComponentIds[] = {
		SpatialConstants::SINGLETON_COMPONENT_ID,
		SpatialConstants::SINGLETON_MANAGER_COMPONENT_ID,
		SpatialConstants::STARTUP_ACTOR_MANAGER_COMPONENT_ID,
		SpatialConstants::VIRTUAL_WORKER_TRANSLATION_COMPONENT_ID,
		SpatialConstants::ALWAYS_RELEVANT_COMPONENT_ID
	};

	for (Worker_ComponentId ComponentId : ComponentIds)
	{
		QueryConstraint Constraint;
		Constraint.ComponentConstraint = ComponentId;
		AlwaysRelevantConstraint.OrConstraint.Add(Constraint);
	}

	return AlwaysRelevantConstraint;
}

void InterestFactory::AddObjectToConstraint(UObjectPropertyBase* Property, uint8* Data, QueryConstraint& OutConstraint) const
{
	UObject* ObjectOfInterest = Property->GetObjectPropertyValue(Data);

	if (ObjectOfInterest == nullptr)
	{
		return;
	}

	FUnrealObjectRef UnrealObjectRef = PackageMap->GetUnrealObjectRefFromObject(ObjectOfInterest);

	if (!UnrealObjectRef.IsValid())
	{
		return;
	}

	QueryConstraint EntityIdConstraint;
	EntityIdConstraint.EntityIdConstraint = UnrealObjectRef.Entity;
	OutConstraint.OrConstraint.Add(EntityIdConstraint);
}

QueryConstraint InterestFactory::CreateLevelConstraints() const
{
	QueryConstraint LevelConstraint;

	QueryConstraint DefaultConstraint;
	DefaultConstraint.ComponentConstraint = SpatialConstants::NOT_STREAMED_COMPONENT_ID;
	LevelConstraint.OrConstraint.Add(DefaultConstraint);

	UNetConnection* Connection = Actor->GetNetConnection();
	check(Connection);
	APlayerController* PlayerController = Connection->GetPlayerController(nullptr);
	check(PlayerController);

	const TSet<FName>& LoadedLevels = PlayerController->NetConnection->ClientVisibleLevelNames;

	// Create component constraints for every loaded sublevel
	for (const auto& LevelPath : LoadedLevels)
	{
		const Worker_ComponentId ComponentId = ClassInfoManager->GetComponentIdFromLevelPath(LevelPath.ToString());
		if (ComponentId != SpatialConstants::INVALID_COMPONENT_ID)
		{
			QueryConstraint SpecificLevelConstraint;
			SpecificLevelConstraint.ComponentConstraint = ComponentId;
			LevelConstraint.OrConstraint.Add(SpecificLevelConstraint);
		}
		else
		{
			UE_LOG(LogInterestFactory, Error, TEXT("Error creating query constraints for Actor %s. "
				"Could not find Streaming Level Component for Level %s. Have you generated schema?"), *Actor->GetName(), *LevelPath.ToString());
		}
	}

	return LevelConstraint;
}

} // namespace SpatialGDK
