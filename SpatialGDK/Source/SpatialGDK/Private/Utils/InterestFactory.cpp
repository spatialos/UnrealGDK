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

DEFINE_LOG_CATEGORY(LogSpatialInterestFactory);

namespace SpatialGDK
{
// The checkout radius constraint is built once for all actors in CreateCheckoutRadiusConstraint as it is equivalent for all actors.
// It is built once per net driver initialisation.
static QueryConstraint ClientCheckoutRadiusConstraint;

SpatialInterestFactory::SpatialInterestFactory(USpatialClassInfoManager* InClassInfoManager)
	: ClassInfoManager(InClassInfoManager)
{
}

void SpatialInterestFactory::CreateClientCheckoutRadiusConstraint()
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

	// The previously built map dedupes spatial constraints. Now the actual query constraints can be built of the form:
	// OR(AND(cyl(radius), OR(actor 1 components, actor 2 components, ...)), ...)
	// which is equivalent to having a separate spatial query for each actor type if the radius is the same.
	TArray<QueryConstraint> CheckoutRadiusConstraints = CheckoutRadiusConstraintUtils::BuildNonDefaultActorCheckoutConstraints(
		DistanceToActorTypeComponents, ClassInfoManager);

	// Add all the different actor queries to the overall checkout constraint.
	for (auto& ActorCheckoutConstraint : CheckoutRadiusConstraints)
	{
		CheckoutRadiusConstraint.OrConstraint.Add(ActorCheckoutConstraint);
	}
	ClientCheckoutRadiusConstraint = CheckoutRadiusConstraint;
}

Worker_ComponentData SpatialInterestFactory::CreateInterestData(AActor* Actor, const FClassInfo& Info) const
{
	return CreateInterest({ Actor, Info }).CreateInterestData();
}

Worker_ComponentUpdate SpatialInterestFactory::CreateInterestUpdate(AActor* Actor) const
{
	return CreateInterestUpdate({ Actor, ClassInfoManager->GetOrCreateClassInfoByObject(Actor) });
}

Worker_ComponentUpdate SpatialInterestFactory::CreateInterestUpdate(AActor* Actor, const FClassInfo& Info) const
{
	return CreateInterestUpdate({ Actor, Info });
}

Worker_ComponentUpdate SpatialInterestFactory::CreateInterestUpdate(const InterestRequest& Request) const
{
	return CreateInterest(Request).CreateInterestUpdate();
}

Interest SpatialInterestFactory::CreateServerWorkerInterest()
{
	QueryConstraint Constraint;

	const USpatialGDKSettings* SpatialGDKSettings = GetDefault<USpatialGDKSettings>();
	if (SpatialGDKSettings->bEnableServerQBI && SpatialGDKSettings->bEnableOffloading)
	{
		UE_LOG(LogSpatialInterestFactory, Warning, TEXT("For performance reasons, it's recommended to disable server QBI when using offloading"));
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

Interest SpatialInterestFactory::CreateInterest(const InterestRequest& Request) const
{
	if (GetDefault<USpatialGDKSettings>()->bEnableServerQBI)
	{
		if (Request.Actor->GetNetConnection() != nullptr)
		{
			return CreatePlayerOwnedActorInterest(Request);
		}
		else
		{
			return CreateActorInterest(Request);
		}
	}
	else
	{
		if (Request.Actor->IsA(APlayerController::StaticClass()))
		{
			return CreatePlayerOwnedActorInterest(Request);
		}
		else
		{
			return Interest{};
		}
	}
}

Interest SpatialInterestFactory::CreateActorInterest(const InterestRequest& Request) const
{
	Interest NewInterest;

	QueryConstraint SystemConstraints = CreateSystemDefinedConstraints(Request);

	if (!SystemConstraints.IsValid())
	{
		return NewInterest;
	}

	Query NewQuery;
	NewQuery.Constraint = SystemConstraints;
	// TODO: Make result type handle components certain workers shouldn't see
	// e.g. Handover, OwnerOnly, etc.
	NewQuery.FullSnapshotResult = true;

	ComponentInterest NewComponentInterest;
	NewComponentInterest.Queries.Add(NewQuery);

	// Server Interest
	NewInterest.ComponentInterestMap.Add(SpatialConstants::POSITION_COMPONENT_ID, NewComponentInterest);

	return NewInterest;
}

Interest SpatialInterestFactory::CreatePlayerOwnedActorInterest(const InterestRequest& Request) const
{
	QueryConstraint SystemConstraints = CreateSystemDefinedConstraints(Request);

	// Servers only need the defined constraints
	Query ServerQuery;
	ServerQuery.Constraint = SystemConstraints;
	ServerQuery.FullSnapshotResult = true;

	ComponentInterest ServerComponentInterest;
	ServerComponentInterest.Queries.Add(ServerQuery);

	// Clients should only check out entities that are in loaded sublevels
	QueryConstraint LevelConstraints = CreateLevelConstraints(Request);

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
	ClientQuery.FullSnapshotResult = true;

	ComponentInterest ClientComponentInterest;
	ClientComponentInterest.Queries.Add(ClientQuery);

	AddUserDefinedQueries(Request, LevelConstraints, ClientComponentInterest.Queries);

	Interest NewInterest;
	// Server Interest
	if (SystemConstraints.IsValid() && GetDefault<USpatialGDKSettings>()->bEnableServerQBI)
	{
		NewInterest.ComponentInterestMap.Add(SpatialConstants::POSITION_COMPONENT_ID, ServerComponentInterest);
	}
	// Client Interest
	if (ClientConstraint.IsValid())
	{
		NewInterest.ComponentInterestMap.Add(SpatialConstants::GetClientAuthorityComponent(GetDefault<USpatialGDKSettings>()->bUseRPCRingBuffers), ClientComponentInterest);
	}

	return NewInterest;
}

void SpatialInterestFactory::AddUserDefinedQueries(const InterestRequest& Request, const QueryConstraint& LevelConstraints, TArray<SpatialGDK::Query>& OutQueries) const
{
	check(Request.Actor);
	check(ClassInfoManager);

	TArray<UActorInterestComponent*> ActorInterestComponents;
	Request.Actor->GetComponents<UActorInterestComponent>(ActorInterestComponents);
	if (ActorInterestComponents.Num() == 1)
	{
		ActorInterestComponents[0]->CreateQueries(*ClassInfoManager, LevelConstraints, OutQueries);
	}
	else if (ActorInterestComponents.Num() > 1)
	{
		UE_LOG(LogSpatialInterestFactory, Error, TEXT("%s has more than one ActorInterestQueryComponent"), *Request.Actor->GetPathName());
	}
}

QueryConstraint SpatialInterestFactory::CreateSystemDefinedConstraints(const InterestRequest& Request) const
{
	QueryConstraint CheckoutRadiusConstraint = CreateCheckoutRadiusConstraints(Request);
	QueryConstraint AlwaysInterestedConstraint = CreateAlwaysInterestedConstraint(Request);
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

QueryConstraint SpatialInterestFactory::CreateCheckoutRadiusConstraints(const InterestRequest& Request) const
{
	// If the actor has a component to specify interest and that indicates that we shouldn't generate
	// constraints based on NetCullDistanceSquared, abort. There is a check elsewhere to ensure that
	// there is at most one ActorInterestQueryComponent.
	TArray<UActorInterestComponent*> ActorInterestComponents;
	Request.Actor->GetComponents<UActorInterestComponent>(ActorInterestComponents);

	check(ActorInterestComponents.Num() <= 1, "Actor with more than one UActorInterestComponent component!");

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

QueryConstraint SpatialInterestFactory::CreateAlwaysInterestedConstraint(const InterestRequest& Request) const
{
	QueryConstraint AlwaysInterestedConstraint;

	for (const FInterestPropertyInfo& PropertyInfo : Request.Info.InterestProperties)
	{
		uint8* Data = (uint8*)Request.Actor + PropertyInfo.Offset;
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

QueryConstraint SpatialInterestFactory::CreateAlwaysRelevantConstraint()
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

void SpatialInterestFactory::AddObjectToConstraint(UObjectPropertyBase* Property, uint8* Data, QueryConstraint& OutConstraint) const
{
	if (PackageMap == nullptr)
	{
		UE_LOG(LogSpatialInterestFactory, Error, TEXT("Trying to add object to interest constraint without a valid PackageMap. Has SetPackageMap been called?"));
		return;
	}

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

QueryConstraint SpatialInterestFactory::CreateLevelConstraints(const InterestRequest& Request) const
{
	QueryConstraint LevelConstraint;

	QueryConstraint DefaultConstraint;
	DefaultConstraint.ComponentConstraint = SpatialConstants::NOT_STREAMED_COMPONENT_ID;
	LevelConstraint.OrConstraint.Add(DefaultConstraint);

	UNetConnection* Connection = Request.Actor->GetNetConnection();
	check(Connection);
	APlayerController* PlayerController = Connection->GetPlayerController(nullptr);
	check(PlayerController);

	const TSet<FName>& LoadedLevels = PlayerController->NetConnection->ClientVisibleLevelNames;

	// Create component constraints for every loaded sublevel
	for (const auto& LevelPath : LoadedLevels)
	{
		const uint32 ComponentId = ClassInfoManager->GetComponentIdFromLevelPath(LevelPath.ToString());
		if (ComponentId != SpatialConstants::INVALID_COMPONENT_ID)
		{
			QueryConstraint SpecificLevelConstraint;
			SpecificLevelConstraint.ComponentConstraint = ComponentId;
			LevelConstraint.OrConstraint.Add(SpecificLevelConstraint);
		}
		else
		{
			UE_LOG(LogSpatialInterestFactory, Error, TEXT("Error creating query constraints for Actor %s. "
				"Could not find Streaming Level Component for Level %s. Have you generated schema?"), *Request.Actor->GetName(), *LevelPath.ToString());
		}
	}

	return LevelConstraint;
}

} // namespace SpatialGDK
