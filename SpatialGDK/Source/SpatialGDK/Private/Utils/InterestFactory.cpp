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

namespace SpatialGDK
{

InterestFactory::InterestFactory(AActor* InActor, const FClassInfo& InInfo, USpatialNetDriver* InNetDriver)
	: Actor(InActor)
	, Info(InInfo)
	, NetDriver(InNetDriver)
	, PackageMap(InNetDriver->PackageMap)
{}

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

	QueryConstraint DefinedConstraints = CreateDefinedConstraints();

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
	QueryConstraint DefinedConstraints = CreateDefinedConstraints();

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

QueryConstraint InterestFactory::CreateDefinedConstraints()
{
	QueryConstraint SystemDefinedConstraints = CreateSystemDefinedConstraints();
	QueryConstraint UserDefinedConstraints = CreateUserDefinedConstraints();

	QueryConstraint DefinedConstraints;

	if (SystemDefinedConstraints.IsValid())
	{
		DefinedConstraints.OrConstraint.Add(SystemDefinedConstraints);
	}

	if (UserDefinedConstraints.IsValid())
	{
		DefinedConstraints.OrConstraint.Add(UserDefinedConstraints);
	}

	return DefinedConstraints;
}

QueryConstraint InterestFactory::CreateSystemDefinedConstraints()
{
	QueryConstraint CheckoutRadiusConstraint = CreateCheckoutRadiusConstraint();
	QueryConstraint NetCullDistanceConstraints = CreateCullDistanceConstraints();
	QueryConstraint AlwaysInterestedConstraint = CreateAlwaysInterestedConstraint();
	QueryConstraint SingletonConstraint = CreateSingletonConstraint();

	QueryConstraint SystemDefinedConstraints;

	if (CheckoutRadiusConstraint.IsValid())
	{
		SystemDefinedConstraints.OrConstraint.Add(CheckoutRadiusConstraint);
	}

	if (NetCullDistanceConstraints.IsValid())
	{
		SystemDefinedConstraints.OrConstraint.Add(CreateCullDistanceConstraints());
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

QueryConstraint InterestFactory::CreateUserDefinedConstraints()
{
	return QueryConstraint{};
}

QueryConstraint InterestFactory::CreateCheckoutRadiusConstraint()
{
	QueryConstraint CheckoutRadiusConstraint;

#if 0
	// TODO - timgibson - Add separate setting for server? Or rename setting to be agnostic.
	const float CheckoutRadius = static_cast<float>(GetDefault<USpatialGDKSettings>()->DefaultClientInterestRadius) / 100.0f; // Convert to meters
	CheckoutRadiusConstraint.RelativeCylinderConstraint = RelativeCylinderConstraint{ CheckoutRadius };
#endif

	return CheckoutRadiusConstraint;
}

QueryConstraint InterestFactory::CreateCullDistanceConstraints()
{
	const AActor* DefaultActor = Cast<AActor>(AActor::StaticClass()->GetDefaultObject());

	// Gather NetCullDistance settings, and add any larger than the default radius.
	TMap<UClass*, float> ActorCullDistances;
	for (TObjectIterator<UClass> It; It; ++It)
	{
		if (It->IsChildOf<AActor>())
		{
			if (!ActorCullDistances.Find(*It))
			{
				const AActor* IteratedDefaultActor = Cast<AActor>(It->GetDefaultObject());
				check(IteratedDefaultActor);
				if (IteratedDefaultActor->NetCullDistanceSquared > DefaultActor->NetCullDistanceSquared)
				{
					ActorCullDistances.Add(*It, IteratedDefaultActor->NetCullDistanceSquared);
				}
			}
		}
	}

	// Sort the map for the next iteration so that base classes are seen before derived classes. This lets us skip
	// derived classes that have a smaller cull distance than a parent class.
	ActorCullDistances.KeySort([](UClass& LHS, UClass& RHS) {
		return
			LHS.IsChildOf(&RHS) ? -1 :
			RHS.IsChildOf(&LHS) ? 1 :
			0;
	});

	// If an actor's cull distance is smaller than that of a parent class, there's no need to add interest for that actor.
	// Can't do inline removal since the sorted order is only guaranteed when the map isn't changed.
	TMap<UClass*, float> OptimizedActorCullDistances;
	for (const auto& CullDistancePair : ActorCullDistances)
	{
		bool bShouldAdd = true;
		for (auto& OptimizedDistancePair : OptimizedActorCullDistances)
		{
			if (CullDistancePair.Key->IsChildOf(OptimizedDistancePair.Key) && CullDistancePair.Value <= OptimizedDistancePair.Value)
			{
				// No need to add this cull distance since it's captured in the optimized map already.
				bShouldAdd = false;
				break;
			}
		}
		if (bShouldAdd)
		{
			OptimizedActorCullDistances.Add(CullDistancePair.Key, CullDistancePair.Value);
		}
	}
	
	QueryConstraint CullDistanceConstraints;

	// Use AActor's NetCullDistance for the default radius (all actors in that radius will be checked out)
	const float DefaultCheckoutRadius= FMath::Sqrt(DefaultActor->NetCullDistanceSquared / (100.0f * 100.0f));
	QueryConstraint DefaultCheckoutRadiusConstraint;
	DefaultCheckoutRadiusConstraint.RelativeCylinderConstraint = RelativeCylinderConstraint{ DefaultCheckoutRadius };
	CullDistanceConstraints.OrConstraint.Add(DefaultCheckoutRadiusConstraint);

	// For every cull distance that we still want, add a constraint with the distance for the actor type and all of its derived types.
	for (const auto& NetCullDistancePair : OptimizedActorCullDistances)
	{
		QueryConstraint ActorDistanceConstraint;

		QueryConstraint RadiusConstraint;
		const float CheckoutRadius = FMath::Sqrt(NetCullDistancePair.Value / (100.0f * 100.0f));
		RadiusConstraint.RelativeCylinderConstraint = RelativeCylinderConstraint{ CheckoutRadius };
		ActorDistanceConstraint.AndConstraint.Add(RadiusConstraint);

		QueryConstraint ActorTypeConstraint;
		const UClass* ActorType = NetCullDistancePair.Key;
		const USchemaDatabase* SchemaDatabase = NetDriver->ClassInfoManager->SchemaDatabase;
		for (TObjectIterator<UClass> ClassIt; ClassIt; ++ClassIt)
		{
			UClass* Class = *ClassIt;
			check(Class);
			if (Class->IsChildOf(ActorType))
			{
				const uint32 ComponentId = SchemaDatabase->GetComponentIdForClass(*Class);
				if (ComponentId != SpatialConstants::INVALID_COMPONENT_ID)
				{
					QueryConstraint ComponentTypeConstraint;
					ComponentTypeConstraint.ComponentConstraint = ComponentId;
					ActorTypeConstraint.OrConstraint.Add(ComponentTypeConstraint);
				}
			}
		}
		ActorDistanceConstraint.AndConstraint.Add(ActorTypeConstraint);

		CullDistanceConstraints.OrConstraint.Add(ActorDistanceConstraint);
	}

	return CullDistanceConstraints;
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

