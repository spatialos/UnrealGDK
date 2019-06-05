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

// TODO - timgibson - change API so that we can avoid attaching empty Interest components.
Interest InterestFactory::CreateInterest()
{
	if (!GetDefault<USpatialGDKSettings>()->bUsingQBI)
	{
		return Interest{};
	}

	// TODO - timgibson - validate that there's a connection.
	if (Actor->IsA(APlayerController::StaticClass()))
	{
		return CreateClientWorkerInterest();
	}

	// TODO - timgibson - need a better way to attach server interest.
	if (Actor->IsA(AWorldSettings::StaticClass()))
	{
		return CreateServerWorkerInterest();
	}

	return Interest{};
}

Interest InterestFactory::CreateClientWorkerInterest()
{
	QueryConstraint ClientConstraint;
	{
		QueryConstraint NearnessConstraint;
		const QueryConstraint CheckoutRadiusConstraint = CreateCheckoutRadiusConstraint();
		const QueryConstraint AlwaysInterestedConstraint = CreateAlwaysInterestedConstraint();
		check(CheckoutRadiusConstraint.IsValid());
		if (AlwaysInterestedConstraint.IsValid())
		{
			NearnessConstraint.OrConstraint.Add(CheckoutRadiusConstraint);
			NearnessConstraint.OrConstraint.Add(AlwaysInterestedConstraint);
		}
		else 
		{
			NearnessConstraint = CheckoutRadiusConstraint;
		}
		
		QueryConstraint StreamingConstraint;
		const QueryConstraint LevelConstraints = CreateLevelConstraints();
		check(LevelConstraints.IsValid());
		check(NearnessConstraint.IsValid());
		StreamingConstraint.AndConstraint.Add(LevelConstraints);
		StreamingConstraint.AndConstraint.Add(NearnessConstraint);

		QueryConstraint SingletonConstraint = CreateSingletonConstraint();
		check(SingletonConstraint.IsValid());
		check(SingletonConstraint.IsValid());
		ClientConstraint.OrConstraint.Add(SingletonConstraint);
		ClientConstraint.OrConstraint.Add(StreamingConstraint);
	}

	Query ClientQuery;
	ClientQuery.Constraint = ClientConstraint;
	ClientQuery.FullSnapshotResult = true;

	// TODO - timgibson - add actor-based queries defined in userland. These need to conjoin with level streaming constraints.

	ComponentInterest ClientComponentInterest;
	ClientComponentInterest.Queries.Add(ClientQuery);

	Interest ClientInterest;
	ClientInterest.ComponentInterestMap.Add(SpatialConstants::CLIENT_RPC_ENDPOINT_COMPONENT_ID, ClientComponentInterest);
	return ClientInterest;
}

Interest InterestFactory::CreateServerWorkerInterest()
{
	// TODO - timgibson - Solve zoning and offloading.
	// With single-server, no interest is needed since the sole server worker has authority
	// over basically everything.
	//
	// Server worker interest should be
	// Singleton OR (Levels AND Buffer around my zone)

	Interest ServerInterest;
	// TODO - timgibson - add constraints.
	return ServerInterest;
}

QueryConstraint InterestFactory::CreateCheckoutRadiusConstraint()
{
	QueryConstraint CheckoutRadiusConstraint;

	const float CheckoutRadius = static_cast<float>(GetDefault<USpatialGDKSettings>()->DefaultClientCheckoutRadius) / 100.0f; // Convert to meters
	CheckoutRadiusConstraint.RelativeCylinderConstraint = RelativeCylinderConstraint{ CheckoutRadius };

	return CheckoutRadiusConstraint;
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

	// TODO - timgibson - Client workers don't need the singleton manager.
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
