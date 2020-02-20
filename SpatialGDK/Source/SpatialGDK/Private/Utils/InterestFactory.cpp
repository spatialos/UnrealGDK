// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "Utils/InterestFactory.h"

#include "Engine/World.h"
#include "Engine/Classes/GameFramework/Actor.h"
#include "GameFramework/PlayerController.h"
#include "UObject/UObjectIterator.h"

#include "EngineClasses/Components/ActorInterestComponent.h"
#include "EngineClasses/SpatialNetConnection.h"
#include "EngineClasses/SpatialPackageMapClient.h"
#include "SpatialGDKSettings.h"
#include "SpatialConstants.h"
#include "UObject/UObjectIterator.h"
#include "Interop/SpatialWorkerFlags.h"

DEFINE_LOG_CATEGORY(LogInterestFactory);

namespace
{
static TMap<UClass*, float> ClientInterestDistancesSquared;
}

namespace SpatialGDK
{
void GatherClientInterestDistances()
{
	ClientInterestDistancesSquared.Empty();

	const AActor* DefaultActor = Cast<AActor>(AActor::StaticClass()->GetDefaultObject());
	const float DefaultDistanceSquared = DefaultActor->NetCullDistanceSquared;
	const float MaxDistanceSquared = GetDefault<USpatialGDKSettings>()->MaxNetCullDistanceSquared;

	// Gather ClientInterestDistance settings, and add any larger than the default radius to a list for processing.
	TMap<UClass*, float> DiscoveredInterestDistancesSquared;
	for (TObjectIterator<UClass> It; It; ++It)
	{
		if (It->HasAnySpatialClassFlags(SPATIALCLASS_ServerOnly))
		{
			continue;
		}
		if (!It->HasAnySpatialClassFlags(SPATIALCLASS_SpatialType))
		{
			continue;
		}
		if (It->HasAnyClassFlags(CLASS_NewerVersionExists))
		{
			// This skips classes generated for hot reload etc (i.e. REINST_, SKEL_, TRASHCLASS_)
			continue;
		}
		if (!It->IsChildOf<AActor>())
		{
			continue;
		}

		const AActor* IteratedDefaultActor = Cast<AActor>(It->GetDefaultObject());
		if (IteratedDefaultActor->NetCullDistanceSquared > DefaultDistanceSquared)
		{
			float ActorNetCullDistanceSquared = IteratedDefaultActor->NetCullDistanceSquared;

			if (MaxDistanceSquared != 0.f && IteratedDefaultActor->NetCullDistanceSquared > MaxDistanceSquared)
			{
				UE_LOG(LogInterestFactory, Warning, TEXT("NetCullDistanceSquared for %s too large, clamping from %f to %f"),
					*It->GetName(), ActorNetCullDistanceSquared, MaxDistanceSquared);

				ActorNetCullDistanceSquared = MaxDistanceSquared;
			}

			DiscoveredInterestDistancesSquared.Add(*It, ActorNetCullDistanceSquared);
		}
	}

	// Sort the map for iteration so that parent classes are seen before derived classes. This lets us skip
	// derived classes that have a smaller interest distance than a parent class.
	DiscoveredInterestDistancesSquared.KeySort([](const UClass& LHS, const UClass& RHS) {
		return LHS.IsChildOf(&RHS);
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

InterestFactory::InterestFactory(AActor* InActor, const FClassInfo& InInfo, USpatialClassInfoManager* InClassInfoManager, USpatialPackageMapClient* InPackageMap)
	: Actor(InActor)
	, Info(InInfo)
	, ClassInfoManager(InClassInfoManager)
	, PackageMap(InPackageMap)
{
}

Worker_ComponentData InterestFactory::CreateInterestData() const
{
	return CreateInterest().CreateInterestData();
}

Worker_ComponentUpdate InterestFactory::CreateInterestUpdate() const
{
	return CreateInterest().CreateInterestUpdate();
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

Interest InterestFactory::CreateInterest() const
{
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

Interest InterestFactory::CreateActorInterest() const
{
	Interest NewInterest;
	ComponentInterest NewComponentInterest;

	if (GetDefault<USpatialGDKSettings>()->bDisableAutomaticQueryGeneration)
	{
		return NewInterest;
	}

	QueryConstraint SystemConstraints = CreateSystemDefinedConstraints();

	if (!SystemConstraints.IsValid())
	{
		return NewInterest;
	}

	// Separate the OrConstraints into multiple queries
	for (QueryConstraint constraint : SystemConstraints.OrConstraint)
	{
		Query NewQuery;
		NewQuery.Constraint = constraint;
		// TODO: Make result type handle components certain workers shouldn't see
		// e.g. Handover, OwnerOnly, etc.
		NewQuery.FullSnapshotResult = true;
		NewComponentInterest.Queries.Add(NewQuery);
	}

	// Server Interest
	NewInterest.ComponentInterestMap.Add(SpatialConstants::POSITION_COMPONENT_ID, NewComponentInterest);

	return NewInterest;
}

SpatialGDK::QueryConstraint InterestFactory::CreateRelevantActorConstraints(AActor* TargetActor) const
{
	bool bIsControllerOrPawn = TargetActor->IsA<APlayerController>() || TargetActor->IsA<APawn>();
	if (!bIsControllerOrPawn)
	{
		AActor* OwnerActor = TargetActor->GetOwner();
		if (OwnerActor == nullptr)
		{
			return QueryConstraint{};
		}
		return CreateRelevantActorConstraints(OwnerActor);
	}

	QueryConstraint Constraint;

	APlayerController* Controller = nullptr;
	APawn* Pawn = nullptr;

	if (TargetActor->IsA<APlayerController>())
	{
		Controller = Cast<APlayerController>(TargetActor);
		Pawn = Controller->GetPawn();
	}
	else
	{
		check(TargetActor->IsA<APawn>());
		Pawn = Cast<APawn>(TargetActor);
		Controller = Cast<APlayerController>(Pawn->GetController());
	}

	if (Controller != nullptr)
	{
		AddRelatedConstraints(Controller, Constraint);
	}

	if (Pawn != nullptr)
	{
		AddRelatedConstraints(Pawn, Constraint);
	}

	return Constraint;
}

void InterestFactory::AddRelatedConstraints(AActor* TargetActor, QueryConstraint& Constraint) const
{
	Worker_EntityId ActorEntityId = PackageMap->GetEntityIdFromObject(TargetActor);
	if (TargetActor != Actor && ActorEntityId != SpatialConstants::INVALID_ENTITY_ID)
	{
		QueryConstraint IdConstraint;
		IdConstraint.EntityIdConstraint = ActorEntityId;
		Constraint.OrConstraint.Add(IdConstraint);
	}
	for (auto* ChildActor : TargetActor->Children)
	{
		if (ChildActor == Actor) continue;
		QueryConstraint IdConstraint;
		Worker_EntityId ChildEntityId = PackageMap->GetEntityIdFromObject(ChildActor);
		if (ChildEntityId == SpatialConstants::INVALID_ENTITY_ID) continue;
		IdConstraint.EntityIdConstraint = ChildEntityId;
		Constraint.OrConstraint.Add(IdConstraint);
	}
}

Interest InterestFactory::CreatePlayerOwnedActorInterest() const
{
	Interest NewInterest;
	ComponentInterest ServerComponentInterest;
	ComponentInterest ClientComponentInterest;

	// Look through owner hierarchy and add any other relevant entities
	QueryConstraint RelatedConstraint = CreateRelevantActorConstraints(Actor);
	if (RelatedConstraint.OrConstraint.Num() > 0)
	{
		Query RelatedQuery;
		RelatedQuery.Constraint = RelatedConstraint;
		RelatedQuery.FullSnapshotResult = true;

		ServerComponentInterest.Queries.Add(RelatedQuery);
	}

	// If query generation is disabled, only continue if the actor has UActorInterestComponent defined
	if (GetDefault<USpatialGDKSettings>()->bDisableAutomaticQueryGeneration)
	{
		TArray<UActorInterestComponent*> ActorInterestComponents;
		Actor->GetComponents<UActorInterestComponent>(ActorInterestComponents);
		if (ActorInterestComponents.Num() == 0)
		{
			if (ServerComponentInterest.Queries.Num() > 0)
			{
				NewInterest.ComponentInterestMap.Add(SpatialConstants::POSITION_COMPONENT_ID, ServerComponentInterest);
			}
			return NewInterest;
		}
	}

	QueryConstraint SystemConstraints = CreateSystemDefinedConstraints();

	// Servers only need the system defined constraints, so they can separate these constraints into separate queries
	for (QueryConstraint constraint : SystemConstraints.OrConstraint)
	{
		Query ServerQuery;
		ServerQuery.Constraint = constraint;
		ServerQuery.FullSnapshotResult = true;
		ServerComponentInterest.Queries.Add(ServerQuery);
	}

	// Clients should only check out entities that are in loaded sublevels
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

	if (ClientConstraint.AndConstraint.Num() == 1)
	{
		QueryConstraint Constraint = ClientConstraint.AndConstraint[0];
		ClientConstraint = Constraint;
	}


	bool EnableScaledFrequencyConstraints = false;
	FString EnableScaledFrequency;
	if (USpatialWorkerFlags::GetWorkerFlag("enable_scaled_qbi_frequency", EnableScaledFrequency))
	{
		EnableScaledFrequencyConstraints = EnableScaledFrequency.Equals("true", ESearchCase::IgnoreCase);
	}

	if (EnableScaledFrequencyConstraints)
	{
		// TODO: Make configurable

		Query SmallClientQuery;
		SmallClientQuery.Constraint = CreateScaledFrequencyConstraint(ClientConstraint, 0.25f);
		SmallClientQuery.FullSnapshotResult = true;

		Query MediumClientQuery;
		MediumClientQuery.Constraint = CreateScaledFrequencyConstraint(ClientConstraint, 0.5f);
		MediumClientQuery.FullSnapshotResult = true;
		MediumClientQuery.Frequency = 5.f;

		Query LargeClientQuery;
		LargeClientQuery.Constraint = CreateScaledFrequencyConstraint(ClientConstraint, 1.0f);
		LargeClientQuery.FullSnapshotResult = true;
		LargeClientQuery.Frequency = 1.f;

		ClientComponentInterest.Queries.Add(LargeClientQuery);
		ClientComponentInterest.Queries.Add(MediumClientQuery);
		ClientComponentInterest.Queries.Add(SmallClientQuery);
	}
	else
	{
		Query ClientQuery;
		ClientQuery.Constraint = ClientConstraint;
		ClientQuery.FullSnapshotResult = true;
		ClientComponentInterest.Queries.Add(ClientQuery);
	}


	AddUserDefinedQueries(LevelConstraints, ClientComponentInterest.Queries);

	// Server Interest
	if (SystemConstraints.IsValid() && GetDefault<USpatialGDKSettings>()->bEnableServerQBI)
	{
		NewInterest.ComponentInterestMap.Add(SpatialConstants::POSITION_COMPONENT_ID, ServerComponentInterest);
	}
	// Client Interest
	if (ClientConstraint.IsValid())
	{
		NewInterest.ComponentInterestMap.Add(SpatialConstants::CLIENT_RPC_ENDPOINT_COMPONENT_ID_LEGACY, ClientComponentInterest);
	}

	return NewInterest;
}

QueryConstraint InterestFactory::CreateScaledFrequencyConstraint(QueryConstraint Constraint, float Scale) const
{
	QueryConstraint NewConstraint;
	if (Constraint.ComponentConstraint)
	{
		NewConstraint.ComponentConstraint = Constraint.ComponentConstraint;
	}
	if (Constraint.EntityIdConstraint)
	{
		NewConstraint.EntityIdConstraint = Constraint.EntityIdConstraint;
	}
	if (Constraint.SphereConstraint)
	{
		NewConstraint.SphereConstraint = Constraint.SphereConstraint;
	}
	if (Constraint.BoxConstraint)
	{
		NewConstraint.BoxConstraint = Constraint.BoxConstraint;
	}
	if (Constraint.BoxConstraint)
	{
		NewConstraint.BoxConstraint = Constraint.BoxConstraint;
	}
	if (Constraint.RelativeCylinderConstraint)
	{
		NewConstraint.RelativeCylinderConstraint = RelativeCylinderConstraint{ Constraint.RelativeCylinderConstraint->Radius * Scale };
	}
	if (Constraint.RelativeBoxConstraint)
	{
		const auto& Length = Constraint.RelativeBoxConstraint->EdgeLength;
		EdgeLength NewLength{ Length.X * Scale, Length.Y * Scale, Length.Z * Scale };
		NewConstraint.RelativeBoxConstraint = RelativeBoxConstraint{ NewLength };
	}
	if (Constraint.RelativeSphereConstraint)
	{
		NewConstraint.RelativeSphereConstraint = RelativeSphereConstraint{ Constraint.RelativeSphereConstraint->Radius * Scale };
	}
	for (const auto& OrConstraint : Constraint.OrConstraint)
	{
		NewConstraint.OrConstraint.Add(CreateScaledFrequencyConstraint(OrConstraint, Scale));
	}
	for (const auto& AndConstraint : Constraint.AndConstraint)
	{
		NewConstraint.AndConstraint.Add(CreateScaledFrequencyConstraint(AndConstraint, Scale));
	}
	return NewConstraint;
}

void InterestFactory::AddUserDefinedQueries(const QueryConstraint& LevelConstraints, TArray<SpatialGDK::Query>& OutQueries) const
{
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
	if (GetDefault<USpatialGDKSettings>()->bDisableAutomaticQueryGeneration)
	{
		return QueryConstraint{};
	}
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

	// Checkout Radius constraints are defined by the NetCullDistanceSquared property on actors.
	//   - Checkout radius is a RelativeCylinder constraint on the player controller.
	//   - NetCullDistanceSquared on AActor is used to define the default checkout radius with no other constraints.
	//   - NetCullDistanceSquared on other actor types is used to define additional constraints if needed.
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

	// For every interest distance that we still want, add a constraint with the distance for the actor type and all of its derived types.
	for (const auto& InterestDistanceSquared: ClientInterestDistancesSquared)
	{
		QueryConstraint CheckoutRadiusConstraint;

		QueryConstraint RadiusConstraint;
		const float CheckoutRadiusMeters = FMath::Sqrt(InterestDistanceSquared.Value / (100.0f * 100.0f));
		RadiusConstraint.RelativeCylinderConstraint = RelativeCylinderConstraint{ CheckoutRadiusMeters };
		CheckoutRadiusConstraint.AndConstraint.Add(RadiusConstraint);

		QueryConstraint ActorTypeConstraint;
		check(InterestDistanceSquared.Key);
		AddTypeHierarchyToConstraint(*InterestDistanceSquared.Key, ActorTypeConstraint);
		if (ActorTypeConstraint.IsValid())
		{
			CheckoutRadiusConstraint.AndConstraint.Add(ActorTypeConstraint);
			CheckoutRadiusConstraints.OrConstraint.Add(CheckoutRadiusConstraint);
		}
	}

	if (CheckoutRadiusConstraints.OrConstraint.Num() == 0)
	{
		return DefaultCheckoutRadiusConstraint;
	}
	else
	{
		CheckoutRadiusConstraints.OrConstraint.Add(DefaultCheckoutRadiusConstraint);
	}

	return CheckoutRadiusConstraints;
}

QueryConstraint InterestFactory::CreateAlwaysInterestedConstraint() const
{
	if (GetDefault<USpatialGDKSettings>()->bDisableAutomaticQueryGeneration)
	{
		return QueryConstraint{};
	}
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

void InterestFactory::AddTypeHierarchyToConstraint(const UClass& BaseType, QueryConstraint& OutConstraint) const
{
	check(ClassInfoManager);
	TArray<Worker_ComponentId> ComponentIds = ClassInfoManager->GetComponentIdsForClassHierarchy(BaseType);
	for (Worker_ComponentId ComponentId : ComponentIds)
	{
		QueryConstraint ComponentTypeConstraint;
		ComponentTypeConstraint.ComponentConstraint = ComponentId;
		OutConstraint.OrConstraint.Add(ComponentTypeConstraint);
	}
}

QueryConstraint InterestFactory::CreateLevelConstraints() const
{
	if (GetDefault<USpatialGDKSettings>()->bDisableAutomaticQueryGeneration)
	{
		return QueryConstraint{};
	}
	QueryConstraint LevelConstraint;

	QueryConstraint DefaultConstraint;
	DefaultConstraint.ComponentConstraint = SpatialConstants::NOT_STREAMED_COMPONENT_ID;

	UNetConnection* Connection = Actor->GetNetConnection();
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
			UE_LOG(LogInterestFactory, Error, TEXT("Error creating query constraints for Actor %s. "
				"Could not find Streaming Level Component for Level %s. Have you generated schema?"), *Actor->GetName(), *LevelPath.ToString());
		}
	}

	if (LevelConstraint.OrConstraint.Num() == 0)
	{
		return DefaultConstraint;
	}
	else
	{
		LevelConstraint.OrConstraint.Add(DefaultConstraint);
	}

	return LevelConstraint;
}

} // namespace SpatialGDK
