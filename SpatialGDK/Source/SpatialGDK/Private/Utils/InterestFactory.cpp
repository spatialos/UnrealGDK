// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "Utils/InterestFactory.h"

#include "Engine/World.h"
#include "Engine/Classes/GameFramework/Actor.h"
#include "GameFramework/PlayerController.h"

#include "EngineClasses/SpatialNetConnection.h"
#include "EngineClasses/SpatialNetDriver.h"
#include "EngineClasses/SpatialPackageMapClient.h"
#include "SpatialGDKSettings.h"
#include "SpatialConstants.h"
#include "UObject/UObjectIterator.h"

DEFINE_LOG_CATEGORY(LogInterestFactory);

namespace
{
static TMap<UClass*, float> ClientInterestDistancesSquared;
void GatherClientInterestDistancesOnce()
{
	if (ClientInterestDistancesSquared.Num() > 0)
	{
		return;
	}

	const AActor* DefaultActor = Cast<AActor>(AActor::StaticClass()->GetDefaultObject());
	const float DefaultDistanceSquared = DefaultActor->NetCullDistanceSquared;

	// Gather ClientInterestDistance settings, and add any larger than the default radius to a list for processing.
	TMap<UClass*, float> DiscoveredInterestDistancesSquared;
	for (TObjectIterator<UClass> It; It; ++It)
	{
		if (It->HasAnySpatialClassFlags(SPATIALCLASS_ServerOnly))
		{
			continue;
		}
		if (!It->IsChildOf<AActor>())
		{
			continue;
		}

		const AActor* IteratedDefaultActor = Cast<AActor>(It->GetDefaultObject());
		if (IteratedDefaultActor->NetCullDistanceSquared > DefaultDistanceSquared)
		{
			DiscoveredInterestDistancesSquared.Add(*It, IteratedDefaultActor->NetCullDistanceSquared);
		}
	}

	// Sort the map for iteration so that parent classes are seen before derived classes. This lets us skip
	// derived classes that have a smaller interest distance than a parent class.
	DiscoveredInterestDistancesSquared.KeySort([](const UClass& LHS, const UClass& RHS) {
		return
			LHS.IsChildOf(&RHS) ? -1 :
			RHS.IsChildOf(&LHS) ? 1 :
			0;
	});

	// If an actor's interest distance is smaller than that of a parent class, there's no need to add interest for that actor.
	// Can't do inline removal since the sorted order is only guaranteed when the map isn't changed.
	for (const auto& ActorInterestDistance : DiscoveredInterestDistancesSquared)
	{
		bool bShouldAdd = true;
		for (auto& OptimizedInterestDistance : ClientInterestDistancesSquared)
		{
			if (ActorInterestDistance.Key->IsChildOf(OptimizedInterestDistance.Key) && ActorInterestDistance.Value <= OptimizedInterestDistance.Value)
			{
				// No need to add this interest distance since it's captured in the optimized map already.
				bShouldAdd = false;
				break;
			}
		}
		if (bShouldAdd)
		{
			ClientInterestDistancesSquared.Add(ActorInterestDistance.Key, ActorInterestDistance.Value);
		}
	}
}
} // anonymous namespace

namespace SpatialGDK
{

InterestFactory::InterestFactory(AActor* InActor, const FClassInfo& InInfo, USpatialNetDriver* InNetDriver)
	: Actor(InActor)
	, Info(InInfo)
	, NetDriver(InNetDriver)
	, PackageMap(InNetDriver->PackageMap)
{
}

Worker_ComponentData InterestFactory::CreateInterestData()
{
	return CreateInterest().CreateInterestData();
}

Worker_ComponentUpdate InterestFactory::CreateInterestUpdate()
{
	return CreateInterest().CreateInterestUpdate();
}

Interest InterestFactory::CreateInterest()
{
	if (!GetDefault<USpatialGDKSettings>()->bUsingQBI)
	{
		return Interest{};
	}

	if (GetDefault<USpatialGDKSettings>()->bEnableServerQBI)
	{
		if (Actor->GetNetConnection() != nullptr)
		{
			return CreatePlayerOwnedActorInterest();
		}
		else
		{
			return CreateActorInterest();
		}
	}
	else
	{
		if (Actor->IsA(APlayerController::StaticClass()))
		{
			return CreatePlayerOwnedActorInterest();
		}
		else
		{
			return Interest{};
		}
	}
}

Interest InterestFactory::CreateActorInterest()
{
	Interest NewInterest;

	QueryConstraint DefinedConstraints = CreateSystemDefinedConstraints();

	if (!DefinedConstraints.IsValid())
	{
		return NewInterest;
	}

	Query NewQuery;
	NewQuery.Constraint = DefinedConstraints;
	// TODO: Make result type handle components certain workers shouldn't see
	// e.g. Handover, OwnerOnly, etc.
	NewQuery.FullSnapshotResult = true;

	ComponentInterest NewComponentInterest;
	NewComponentInterest.Queries.Add(NewQuery);

	// Server Interest
	NewInterest.ComponentInterestMap.Add(SpatialConstants::POSITION_COMPONENT_ID, NewComponentInterest);

	return NewInterest;
}

Interest InterestFactory::CreatePlayerOwnedActorInterest()
{
	QueryConstraint DefinedConstraints = CreateSystemDefinedConstraints();

	// Servers only need the defined constraints
	Query ServerQuery;
	ServerQuery.Constraint = DefinedConstraints;
	ServerQuery.FullSnapshotResult = true;

	ComponentInterest ServerComponentInterest;
	ServerComponentInterest.Queries.Add(ServerQuery);

	// Clients should only check out entities that are in loaded sublevels
	QueryConstraint LevelConstraints = CreateLevelConstraints();

	QueryConstraint ClientConstraint;

	if (DefinedConstraints.IsValid())
	{
		ClientConstraint.AndConstraint.Add(DefinedConstraints);
	}

	if (LevelConstraints.IsValid())
	{
		ClientConstraint.AndConstraint.Add(LevelConstraints);
	}

	Query ClientQuery;
	ClientQuery.Constraint = ClientConstraint;
	ClientQuery.FullSnapshotResult = true;

	ComponentInterest ClientComponentInterest;
	ClientComponentInterest.Queries.Add(ClientQuery);

	Interest NewInterest;
	// Server Interest
	if (DefinedConstraints.IsValid() && GetDefault<USpatialGDKSettings>()->bEnableServerQBI)
	{
		NewInterest.ComponentInterestMap.Add(SpatialConstants::POSITION_COMPONENT_ID, ServerComponentInterest);
	}
	// Client Interest
	if (ClientConstraint.IsValid())
	{
		NewInterest.ComponentInterestMap.Add(SpatialConstants::CLIENT_RPC_ENDPOINT_COMPONENT_ID, ClientComponentInterest);
	}

	return NewInterest;
}

QueryConstraint InterestFactory::CreateSystemDefinedConstraints()
{
	QueryConstraint CheckoutRadiusConstraint = CreateCheckoutRadiusConstraints();
	QueryConstraint AlwaysInterestedConstraint = CreateAlwaysInterestedConstraint();
	QueryConstraint SingletonConstraint = CreateSingletonConstraint();

	QueryConstraint SystemDefinedConstraints;

	if (CheckoutRadiusConstraint.IsValid())
	{
		SystemDefinedConstraints.OrConstraint.Add(CheckoutRadiusConstraint);
	}

	if (AlwaysInterestedConstraint.IsValid())
	{
		SystemDefinedConstraints.OrConstraint.Add(AlwaysInterestedConstraint);
	}

	if (SingletonConstraint.IsValid())
	{
		SystemDefinedConstraints.OrConstraint.Add(SingletonConstraint);
	}

	return SystemDefinedConstraints;
}

QueryConstraint InterestFactory::CreateCheckoutRadiusConstraints()
{
	GatherClientInterestDistancesOnce();

	// Checkout Radius constraints are defined by the ClientInterestDistance property on actors.
	//   - Checkout radius is a RelativeCylinder constraint on the player controller.
	//   - ClientInterestDistance on AActor is used to define the default checkout radius with no other constraints.
	//   - ClientInterestDistance on other actor types is used to define additional constraints if needed.
	//   - If a subtype defines a radius smaller than a parent type, then its requirements are already captured.
	//   - If a subtype defines a radius larger than all parent types, then it needs an additional constraint.
	//   - Other than the default from AActor, all radius constraints also include Component constraints to
	//     capture specific types, including all derived types of that actor.

	const AActor* DefaultActor = Cast<AActor>(AActor::StaticClass()->GetDefaultObject());
	const float DefaultDistanceSquared = DefaultActor->NetCullDistanceSquared;

	QueryConstraint CheckoutRadiusConstraints;

	// Use AActor's ClientInterestDistance for the default radius (all actors in that radius will be checked out)
	const float DefaultCheckoutRadiusMeters = FMath::Sqrt(DefaultDistanceSquared / (100.0f * 100.0f));
	QueryConstraint DefaultCheckoutRadiusConstraint;
	DefaultCheckoutRadiusConstraint.RelativeCylinderConstraint = RelativeCylinderConstraint{ DefaultCheckoutRadiusMeters };
	CheckoutRadiusConstraints.OrConstraint.Add(DefaultCheckoutRadiusConstraint);

	// For every interest distance that we still want, add a constraint with the distance for the actor type and all of its derived types.
	for (const auto& InterestDistanceSquared: ClientInterestDistancesSquared)
	{
		QueryConstraint CheckoutRadiusConstraint;

		QueryConstraint RadiusConstraint;
		const float CheckoutRadiusMeters = FMath::Sqrt(InterestDistanceSquared.Value / (100.0f * 100.0f));
		RadiusConstraint.RelativeCylinderConstraint = RelativeCylinderConstraint{ CheckoutRadiusMeters };
		CheckoutRadiusConstraint.AndConstraint.Add(RadiusConstraint);

		QueryConstraint ActorTypeConstraint;
		AddTypeHierarchyToConstraint(InterestDistanceSquared.Key, ActorTypeConstraint);
		CheckoutRadiusConstraint.AndConstraint.Add(ActorTypeConstraint);

		CheckoutRadiusConstraints.OrConstraint.Add(CheckoutRadiusConstraint);
	}

	return CheckoutRadiusConstraints;
}

QueryConstraint InterestFactory::CreateAlwaysInterestedConstraint()
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


QueryConstraint InterestFactory::CreateSingletonConstraint()
{
	QueryConstraint SingletonConstraint;

	Worker_ComponentId SingletonComponentIds[] = {
		SpatialConstants::SINGLETON_COMPONENT_ID,
		SpatialConstants::SINGLETON_MANAGER_COMPONENT_ID };

	for (Worker_ComponentId ComponentId : SingletonComponentIds)
	{
		QueryConstraint Constraint;
		Constraint.ComponentConstraint = ComponentId;
		SingletonConstraint.OrConstraint.Add(Constraint);
	}

	return SingletonConstraint;
}

void InterestFactory::AddObjectToConstraint(UObjectPropertyBase* Property, uint8* Data, QueryConstraint& OutConstraint)
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

void InterestFactory::AddTypeHierarchyToConstraint(const UClass* BaseType, QueryConstraint& OutConstraint)
{
	const UClass* AuthoritativeBaseType = BaseType->GetAuthoritativeClass();

	check(NetDriver && NetDriver->ClassInfoManager && NetDriver->ClassInfoManager->SchemaDatabase);
	const USchemaDatabase* SchemaDatabase = NetDriver->ClassInfoManager->SchemaDatabase;
	for (TObjectIterator<UClass> ClassIt; ClassIt; ++ClassIt)
	{
		UClass* Class = *ClassIt;
		check(Class);
		if (Class->IsChildOf(AuthoritativeBaseType))
		{
			const uint32 ComponentId = SchemaDatabase->GetComponentIdForClass(*Class);
			if (ComponentId != SpatialConstants::INVALID_COMPONENT_ID)
			{
				QueryConstraint ComponentTypeConstraint;
				ComponentTypeConstraint.ComponentConstraint = ComponentId;
				OutConstraint.OrConstraint.Add(ComponentTypeConstraint);
			}
		}
	}
	check(OutConstraint.IsValid());
}

QueryConstraint InterestFactory::CreateLevelConstraints()
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
		const uint32 ComponentId = NetDriver->ClassInfoManager->SchemaDatabase->GetComponentIdFromLevelPath(LevelPath.ToString());
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
