// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "SpatialGDK/SpatialTestLoadBalancingData/SpatialTestLoadBalancingData.h"

#include "EngineClasses/SpatialNetDriver.h"
#include "EngineClasses/SpatialPackageMapClient.h"
#include "EngineClasses/SpatialWorldSettings.h"
#include "Interop/Connection/SpatialWorkerConnection.h"
#include "SpatialCommonTypes.h"
#include "SpatialView/EntityComponentTypes.h"
#include "TestWorkerSettings.h"

#include "Kismet/GameplayStatics.h"

USpatialTestLoadBalancingDataTestMap::USpatialTestLoadBalancingDataTestMap()
	: Super(EMapCategory::CI_PREMERGE_SPATIAL_ONLY, TEXT("SpatialTestLoadBalancingData"))
{
	// clang-format off
	SetCustomConfig(
		TEXT("[/Script/SpatialGDK.SpatialGDKSettings]") LINE_TERMINATOR
		TEXT("bEnableStrategyLoadBalancingComponents=True")
		);
	// clang-format on
}

void USpatialTestLoadBalancingDataTestMap::CreateCustomContentForMap()
{
	Super::CreateCustomContentForMap();

	CastChecked<ASpatialWorldSettings>(World->GetWorldSettings())
		->SetMultiWorkerSettingsClass(USpatialTestLoadBalancingDataMultiWorkerSettings::StaticClass());

	AddActorToLevel<ASpatialTestLoadBalancingData>(World->GetCurrentLevel(), FTransform::Identity);
}

USpatialTestLoadBalancingDataMultiWorkerSettings::USpatialTestLoadBalancingDataMultiWorkerSettings()
{
	WorkerLayers[0].ActorClasses.Add(ASpatialTestLoadBalancingDataActor::StaticClass());
	WorkerLayers.Add(
		{ TEXT("Offloaded"), { ASpatialTestLoadBalancingDataOffloadedActor::StaticClass() }, USingleWorkerStrategy::StaticClass() });
	WorkerLayers.Add(
		{ TEXT("Grid"), { ASpatialTestLoadBalancingDataZonedActor::StaticClass() }, UTest1x2FullInterestGridStrategy::StaticClass() });
}

ASpatialTestLoadBalancingDataActor::ASpatialTestLoadBalancingDataActor()
{
	bReplicates = true;

	bAlwaysRelevant = true;
}

ASpatialTestLoadBalancingDataOffloadedActor::ASpatialTestLoadBalancingDataOffloadedActor()
{
	bReplicates = true;

	bAlwaysRelevant = true;
}

ASpatialTestLoadBalancingDataZonedActor::ASpatialTestLoadBalancingDataZonedActor()
{
	bReplicates = true;

	bAlwaysRelevant = true;

	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));
}

template <class T>
T* GetWorldActor(UWorld* World)
{
	TArray<AActor*> DiscoveredActors;
	UGameplayStatics::GetAllActorsOfClass(World, T::StaticClass(), DiscoveredActors);
	if (DiscoveredActors.Num() == 1)
	{
		return Cast<T>(DiscoveredActors[0]);
	}
	return nullptr;
}

template <class T, int Count, class U>
void GetWorldActors(UWorld* World, TArray<U>& OutActors)
{
	TArray<AActor*> DiscoveredActors;
	UGameplayStatics::GetAllActorsOfClass(World, T::StaticClass(), DiscoveredActors);
	if (DiscoveredActors.Num() == Count)
	{
		DiscoveredActors.Sort([](const AActor& Lhs, const AActor& Rhs) {
			return Lhs.GetActorLocation().Y < Rhs.GetActorLocation().Y;
		});
		Algo::Transform(DiscoveredActors, OutActors, [](AActor* Actor) -> U {
			return Cast<T>(Actor);
		});
	}
}

/*
 * The expected worker setup is as follows:
 * * Worker 1 is the "main" worker that handles ASpatialTestLoadBalancingDataActor
 * * Worker 2 is the "offloaded" worker that handles ASpatialTestLoadBalancingDataOffloadedActor
 * * Workers 3 and 4 are zoned workers that handle ASpatialTestLoadBalancingDataZonedActor; these workers
 *   are referred to as "zoned server 1" and "zoned server 2" respectively
 */
void ASpatialTestLoadBalancingData::PrepareTest()
{
	Super::PrepareTest();

	const FWorkerDefinition MainServer = FWorkerDefinition::Server(1);
	const FWorkerDefinition OffloadedServer = FWorkerDefinition::Server(2);
	const FWorkerDefinition ZonedServers[]{ FWorkerDefinition::Server(3), FWorkerDefinition::Server(4) };

	AddStep(TEXT("Create an actor on the main server"), MainServer, nullptr, [this] {
		ASpatialTestLoadBalancingDataActor* SpawnedActor = GetWorld()->SpawnActor<ASpatialTestLoadBalancingDataActor>();
		check(SpawnedActor->HasAuthority());
		RegisterAutoDestroyActor(SpawnedActor);
		FinishStep();
	});

	AddStep(TEXT("Create an offloaded actor on the offloaded server"), OffloadedServer, nullptr, [this] {
		ASpatialTestLoadBalancingDataOffloadedActor* SpawnedActor = GetWorld()->SpawnActor<ASpatialTestLoadBalancingDataOffloadedActor>();
		check(SpawnedActor->HasAuthority());
		RegisterAutoDestroyActor(SpawnedActor);
		FinishStep();
	});

	// One to the left of the boundary, one to the right.
	const static FVector ZonedActorsPositions[]{ { 100, -100, 100 }, { 100, 100, 100 } };

	AddStep(TEXT("Create zoned actor on zoned server 1"), ZonedServers[0], nullptr, [this] {
		ASpatialTestLoadBalancingDataZonedActor* SpawnedActor =
			GetWorld()->SpawnActor<ASpatialTestLoadBalancingDataZonedActor>(ZonedActorsPositions[0], FRotator::ZeroRotator);
		check(SpawnedActor->HasAuthority());
		RegisterAutoDestroyActor(SpawnedActor);
		FinishStep();
	});

	AddStep(TEXT("Create zoned actor on zoned server 2"), ZonedServers[1], nullptr, [this] {
		ASpatialTestLoadBalancingDataZonedActor* SpawnedActor =
			GetWorld()->SpawnActor<ASpatialTestLoadBalancingDataZonedActor>(ZonedActorsPositions[1], FRotator::ZeroRotator);
		check(SpawnedActor->HasAuthority());
		RegisterAutoDestroyActor(SpawnedActor);
		FinishStep();
	});

	constexpr float ActorReceiptTimeout = 5.0f;
	AddStep(
		TEXT("Retrieve actors on all workers"), FWorkerDefinition::AllWorkers, nullptr, nullptr,
		[this](float) {
			TargetActor = GetWorldActor<ASpatialTestLoadBalancingDataActor>(GetWorld());
			RequireTrue(TargetActor.IsValid(), TEXT("Received main actor"));

			TargetOffloadedActor = GetWorldActor<ASpatialTestLoadBalancingDataOffloadedActor>(GetWorld());
			RequireTrue(TargetOffloadedActor.IsValid(), TEXT("Received offloaded actor"));

			GetWorldActors<ASpatialTestLoadBalancingDataZonedActor, 2>(GetWorld(), TargetZonedActors);
			RequireTrue(TargetZonedActors.Num() == 2, TEXT("Received zoned actors"));

			FinishStep();
		},
		ActorReceiptTimeout);

	const float LoadBalancingDataReceiptTimeout = 5.0f;
	AddStep(
		TEXT("Confirm LB group IDs on the server"), FWorkerDefinition::AllServers, nullptr, nullptr,
		[this](float) {
			// ServerWorkers should have interest in LoadBalancingData so it should be available
			TOptional<SpatialGDK::ActorGroupMember> LoadBalancingData = GetActorGroupData(TargetActor.Get());
			RequireTrue(LoadBalancingData.IsSet(), TEXT("Main actor entity has LoadBalancingData"));

			TOptional<SpatialGDK::ActorGroupMember> OffloadedLoadBalancingData = GetActorGroupData(TargetOffloadedActor.Get());
			RequireTrue(OffloadedLoadBalancingData.IsSet(), TEXT("Offloaded actor entity has LoadBalancingData"));

			const bool bIsValid = LoadBalancingData.IsSet() && OffloadedLoadBalancingData.IsSet()
								  && LoadBalancingData->ActorGroupId != OffloadedLoadBalancingData->ActorGroupId;

			RequireTrue(bIsValid, TEXT("Load balancing group ids are different for the main server and for the offloaded server"));

			FinishStep();
		},
		LoadBalancingDataReceiptTimeout);

	AddStep(TEXT("Put zoned actors to a single ownership group"), ZonedServers[0], nullptr, [this]() {
		AssertTrue(TargetZonedActors[0]->HasAuthority(), TEXT("Zoned actor 1 is owned by the zoned server 1"));
		TargetZonedActors[0]->SetOwner(TargetZonedActors[1].Get());
		FinishStep();
	});

	AddStep(
		TEXT("Wait until actor set IDs are the same"), FWorkerDefinition::AllServers, nullptr, nullptr,
		[this](float) {
			RequireTrue(GetActorSetData(TargetZonedActors[0].Get())->ActorSetId == GetActorSetData(TargetZonedActors[1].Get())->ActorSetId,
						TEXT("Actor set IDs are the same for the two zoned actors"));
			const bool bShouldHaveAuthority = GetLocalWorkerId() == 4;
			RequireTrue(TargetZonedActors[0]->HasAuthority() == bShouldHaveAuthority, TEXT("Authority was moved correctly"));
			FinishStep();
		},
		LoadBalancingDataReceiptTimeout);

	AddStep(TEXT("Put main server actor and offloaded server actor into different ownership groups"), ZonedServers[1], nullptr, [this]() {
		AssertTrue(TargetZonedActors[0]->HasAuthority(), TEXT("Zoned actor 1 is owned by the zoned server 2"));
		TargetZonedActors[0]->SetOwner(nullptr);
		FinishStep();
	});

	AddStep(
		TEXT("Wait until actor set IDs different again"), FWorkerDefinition::AllServers, nullptr, nullptr,
		[this](float) {
			RequireTrue(GetActorSetData(TargetZonedActors[0].Get())->ActorSetId != GetActorSetData(TargetZonedActors[1].Get())->ActorSetId,
						TEXT("Actor set IDs are different for the two zoned actors"));

			FinishStep();
		},
		LoadBalancingDataReceiptTimeout);
}

template <class TComponent>
TOptional<TComponent> ASpatialTestLoadBalancingData::GetSpatialComponent(const AActor* Actor) const
{
	USpatialNetDriver* SpatialNetDriver = Cast<USpatialNetDriver>(GetNetDriver());
	const Worker_EntityId ActorEntityId = SpatialNetDriver->PackageMap->GetEntityIdFromObject(Actor);

	if (ensure(ActorEntityId != SpatialConstants::INVALID_ENTITY_ID))
	{
		const SpatialGDK::EntityViewElement& ActorEntity = SpatialNetDriver->Connection->GetView()[ActorEntityId];

		const SpatialGDK::ComponentData* ComponentData =
			ActorEntity.Components.FindByPredicate(SpatialGDK::ComponentIdEquality{ TComponent::ComponentId });

		if (ComponentData != nullptr)
		{
			return TComponent(*ComponentData);
		}
	}

	return {};
}

TOptional<SpatialGDK::ActorSetMember> ASpatialTestLoadBalancingData::GetActorSetData(const AActor* Actor) const
{
	return GetSpatialComponent<SpatialGDK::ActorSetMember>(Actor);
}

TOptional<SpatialGDK::ActorGroupMember> ASpatialTestLoadBalancingData::GetActorGroupData(const AActor* Actor) const
{
	return GetSpatialComponent<SpatialGDK::ActorGroupMember>(Actor);
}
