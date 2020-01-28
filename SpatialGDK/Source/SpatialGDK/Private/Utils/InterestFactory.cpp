// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "Utils/InterestFactory.h"

#include "Engine/World.h"
#include "Engine/Classes/GameFramework/Actor.h"
#include "GameFramework/PlayerController.h"
#include "UObject/UObjectIterator.h"
#include "Utils/CheckoutRadiusConstraintUtils.h"

#include "EngineClasses/Components/ActorInterestComponent.h"
#include "EngineClasses/SpatialNetConnection.h"
#include "EngineClasses/SpatialPackageMapClient.h"
#include "SpatialGDKSettings.h"
#include "SpatialConstants.h"
#include "UObject/UObjectIterator.h"

DEFINE_LOG_CATEGORY(LogInterestFactory);

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

// Cache the result type of client Interest queries.
static TArray<Worker_ComponentId> ClientResultType;

InterestFactory::InterestFactory(AActor* InActor, const FClassInfo& InInfo, USpatialClassInfoManager* InClassInfoManager, USpatialPackageMapClient* InPackageMap)
	: Actor(InActor)
	, Info(InInfo)
	, ClassInfoManager(InClassInfoManager)
	, PackageMap(InPackageMap)
{
}

void InterestFactory::CreateAndCacheInterestState(USpatialClassInfoManager* ClassInfoManager)
{
	ClientCheckoutRadiusConstraint = CreateClientCheckoutRadiusConstraint(ClassInfoManager);
	ClientResultType = CreateClientResultType(ClassInfoManager);
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

TArray<Worker_ComponentId> InterestFactory::CreateClientResultType(USpatialClassInfoManager* ClassInfoManager)
{
	TArray<Worker_ComponentId> ResultType;

	// Add the required unreal components
	ResultType.Append(SpatialConstants::REQUIRED_COMPONENTS_FOR_CLIENT_INTEREST);

	// Add all data components- clients don't need to see handover or owner only components on other entities.
	ResultType.Append(ClassInfoManager->GetComponentIdsForComponentType(ESchemaComponentType::SCHEMA_Data));

	return ResultType;
}

Worker_ComponentData InterestFactory::CreateInterestData(Worker_EntityId EntityId) const
{
	return CreateInterest(EntityId).CreateInterestData();
}

Worker_ComponentUpdate InterestFactory::CreateInterestUpdate(Worker_EntityId EntityId) const
{
	return CreateInterest(EntityId).CreateInterestUpdate();
}

Interest InterestFactory::CreateServerWorkerInterest()
{
	QueryConstraint Constraint;

	const USpatialGDKSettings* SpatialGDKSettings = GetDefault<USpatialGDKSettings>();
	if (SpatialGDKSettings->bEnableServerQBI && SpatialGDKSettings->bEnableOffloading)
	{
		UE_LOG(LogInterestFactory, Warning, TEXT("For performance reasons, it's recommended to disable server QBI when using offloading"));
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
	Query.FullSnapshotResult = true;

	ComponentInterest Queries;
	Queries.Queries.Add(Query);

	Interest ServerInterest;
	ServerInterest.ComponentInterestMap.Add(SpatialConstants::POSITION_COMPONENT_ID, Queries);

	return ServerInterest;
}

Interest InterestFactory::CreateInterest(Worker_EntityId EntityId) const
{
	const auto Settings = GetDefault<USpatialGDKSettings>();
	Interest ResultInterest;
	if (Actor->IsA(APlayerController::StaticClass()))
	{
		// Put the "main" interest queries on the player controller
		AddPlayerControllerActorInterest(ResultInterest);
	}
	if (Actor->GetNetConnection() != nullptr && Settings->bEnableClientResultTypes)
	{
		AddClientSelfInterest(ResultInterest, EntityId);
	}
	if (Settings->bEnableServerQBI)
	{
		// If we have server QBI, every actor needs a query for the server.
		AddActorInterest(ResultInterest);
	}
	return ResultInterest;
}

void InterestFactory::AddActorInterest(Interest& InInterest) const
{
	QueryConstraint SystemConstraints = CreateSystemDefinedConstraints();

	if (!SystemConstraints.IsValid())
	{
		return;
	}

	Query NewQuery;
	NewQuery.Constraint = SystemConstraints;
	// TODO: Make result type handle components certain workers shouldn't see
	// e.g. Handover, OwnerOnly, etc.
	NewQuery.FullSnapshotResult = true;

	AddComponentQueryPairToInterestComponent(InInterest, SpatialConstants::POSITION_COMPONENT_ID, NewQuery);
}

void InterestFactory::AddPlayerControllerActorInterest(Interest& InInterest) const
{
	const USpatialGDKSettings* SpatialGDKSettings = GetDefault<USpatialGDKSettings>();

	const Worker_ComponentId ClientEndpointComponentId = SpatialConstants::GetClientAuthorityComponent(SpatialGDKSettings->bUseRPCRingBuffers);

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

	if (SpatialGDKSettings->bEnableClientResultTypes)
	{
		ClientQuery.ResultComponentId = ClientResultType;
	}
	else
	{
		ClientQuery.FullSnapshotResult = true;
	}

	AddComponentQueryPairToInterestComponent(InInterest, ClientEndpointComponentId, ClientQuery);

	TArray<Query> UserQueries = GetUserDefinedQueries(LevelConstraints);
	for (const auto UserQuery : UserQueries) {
		AddComponentQueryPairToInterestComponent(InInterest, ClientEndpointComponentId, UserQuery);
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

			if (SpatialGDKSettings->bEnableClientResultTypes)
			{
				NewQuery.ResultComponentId = ClientResultType;
			}
			else
			{
				NewQuery.FullSnapshotResult = true;
			}

			AddComponentQueryPairToInterestComponent(InInterest, ClientEndpointComponentId, NewQuery);
		}
	}
}

void InterestFactory::AddClientSelfInterest(Interest InInterest, Worker_EntityId EntityId) const
{
	Query NewQuery;
	NewQuery.Constraint.EntityIdConstraint = EntityId;

	NewQuery.ResultComponentId = SpatialConstants::REQUIRED_COMPONENTS_FOR_CLIENT_AUTH;

	AddComponentQueryPairToInterestComponent(InInterest, SpatialConstants::GetClientAuthorityComponent(GetDefault<USpatialGDKSettings>()->bUseRPCRingBuffers), NewQuery);
}

void InterestFactory::AddComponentQueryPairToInterestComponent(Interest& InInterest, const Worker_ComponentId ComponentId, const Query QueryToAdd)
{
	if (InInterest.ComponentInterestMap.Contains(ComponentId))
	{
		InInterest.ComponentInterestMap[ComponentId].Queries.Add(QueryToAdd);
	}
	else
	{
		ComponentInterest NewComponentInterest;
		NewComponentInterest.Queries.Add(QueryToAdd);
		InInterest.ComponentInterestMap.Add(ComponentId, NewComponentInterest);
	}
}

TArray<Query> InterestFactory::GetUserDefinedQueries(const QueryConstraint& LevelConstraints) const
{
	TArray<Query> OutQueries;
	check(Actor);
	check(ClassInfoManager);

	TArray<UActorInterestComponent*> ActorInterestComponents;
	Actor->GetComponents<UActorInterestComponent>(ActorInterestComponents);
	if (ActorInterestComponents.Num() == 1)
	{
		ActorInterestComponents[0]->CreateQueries(*ClassInfoManager, LevelConstraints, OutQueries);
	}
	else if (ActorInterestComponents.Num() > 1)
	{
		UE_LOG(LogInterestFactory, Error, TEXT("%s has more than one ActorInterestQueryComponent"), *Actor->GetPathName());
	}

	return OutQueries;
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
