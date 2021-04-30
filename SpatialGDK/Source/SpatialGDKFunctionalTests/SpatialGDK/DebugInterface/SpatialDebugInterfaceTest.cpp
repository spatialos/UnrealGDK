// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "SpatialDebugInterfaceTest.h"

#include "EngineClasses/SpatialWorldSettings.h"
#include "LoadBalancing/GridBasedLBStrategy.h"
#include "LoadBalancing/LayeredLBStrategy.h"
#include "SpatialFunctionalTestFlowController.h"
#include "SpatialGDKFunctionalTests/SpatialGDK/TestActors/ReplicatedTestActorBase_RepGraphAlwaysReplicate.h"
#include "TestWorkerSettings.h"

#include "Kismet/GameplayStatics.h"

/**
 * Test for coverage of the USpatialGDKDebugInterface.
 * The debug interface allows you to manipulate interest and load balancing by tagging actors and declaring interest and delegation over
 * these tags The goal is to have an easier time writing load balancing test to not have to derive an additional load balancing strategy for
 * each new test.
 *
 * The test walks through a couple of situations that one can setup through the debug interface :
 * - Add interest over all actors having some tags
 * - Delegate tags to specific workers, forcing load balancing
 * - Create new actors with the tag and check that extra interest and delegation is properly applied
 * - Remove extra interest
 * - Remove tags from actors and see interest and load-balancing revert to their default.
 * - Add tags again and clear delegation
 * - Clear all debug information
 *
 * Most of these tests are performed in a two-step way, that is setting some debug behaviour and waiting for it to happen on the next step
 * Delegation commands expect consensus between workers to behave properly, so separating change steps from observation steps helps avoiding
 * races that could happen around interest or load balancing.
 */

ASpatialDebugInterfaceTest::ASpatialDebugInterfaceTest()
	: Super()
{
	Author = "Nicolas";
	Description = TEXT("Test Debug interface");
}

namespace
{
FName GetTestTag()
{
	static const FName TestTag(TEXT("TestActorToFollow"));
	return TestTag;
}
} // namespace

bool ASpatialDebugInterfaceTest::WaitToSeeActors(UClass* ActorClass, int32 NumActors)
{
	if (bIsOnDefaultLayer)
	{
		UWorld* World = GetWorld();

		TArray<AActor*> TestActors;
		UGameplayStatics::GetAllActorsOfClass(World, ActorClass, TestActors);
		if (TestActors.Num() != NumActors)
		{
			return false;
		}
	}
	return true;
}

void ASpatialDebugInterfaceTest::PrepareTest()
{
	Super::PrepareTest();

	AddStep(TEXT("SetupStep"), FWorkerDefinition::AllServers, nullptr, nullptr, [this](float DeltaTime) {
		UWorld* World = GetWorld();
		DelegationStep = 0;
		Workers.Empty();
		bIsOnDefaultLayer = false;

		ULayeredLBStrategy* RootStrategy = GetLoadBalancingStrategy();

		bIsOnDefaultLayer = RootStrategy->CouldHaveAuthority(AReplicatedTestActorBase_RepGraphAlwaysReplicate::StaticClass());
		if (bIsOnDefaultLayer)
		{
			FName LocalLayer = RootStrategy->GetLocalLayerName();
			UAbstractLBStrategy* LocalStrategy = RootStrategy->GetLBStrategyForLayer(LocalLayer);

			AssertTrue(LocalStrategy->IsA<UGridBasedLBStrategy>(), TEXT(""));

			UGridBasedLBStrategy* GridStrategy = Cast<UGridBasedLBStrategy>(LocalStrategy);

			for (auto& WorkerRegion : GridStrategy->GetLBStrategyRegions())
			{
				Workers.Add(WorkerRegion.Key);
			}
			LocalWorker = GridStrategy->GetLocalVirtualWorkerId();

			WorkerEntityPosition = GridStrategy->GetWorkerEntityPosition();
			AReplicatedTestActorBase_RepGraphAlwaysReplicate* Actor =
				World->SpawnActor<AReplicatedTestActorBase_RepGraphAlwaysReplicate>(WorkerEntityPosition, FRotator());
			AddDebugTag(Actor, GetTestTag());
			RegisterAutoDestroyActor(Actor);
			TimeStampSpinning = FPlatformTime::Cycles64();
		}

		FinishStep();
	});

	AddStep(
		TEXT("Wait for actor ready and add extra interest"), FWorkerDefinition::AllServers,
		[this]() -> bool {
			if (double(FPlatformTime::Cycles64() - TimeStampSpinning) * FPlatformTime::GetSecondsPerCycle() < 2.0)
			{
				return false;
			}

			if (!bIsOnDefaultLayer)
			{
				return true;
			}

			UWorld* World = GetWorld();

			TArray<AActor*> TestActors;
			UGameplayStatics::GetAllActorsOfClass(World, AReplicatedTestActorBase_RepGraphAlwaysReplicate::StaticClass(), TestActors);
			if (!AssertTrue(TestActors.Num() == 1, "We should only see a single actor at this point!!"))
			{
				return false;
			}
			return TestActors[0]->IsActorReady();
		},
		[this]() {
			if (!bIsOnDefaultLayer)
			{
				FinishStep();
			}

			AddInterestOnTag(GetTestTag());
			FinishStep();
		},
		nullptr, 10.0f);

	AddStep(
		TEXT("Wait for extra actors"), FWorkerDefinition::AllServers,
		[this]() -> bool {
			return WaitToSeeActors(AReplicatedTestActorBase_RepGraphAlwaysReplicate::StaticClass(), Workers.Num());
		},
		[this]() {
			if (!bIsOnDefaultLayer)
			{
				FinishStep();
			}
			UWorld* World = GetWorld();

			TArray<AActor*> TestActors;
			UGameplayStatics::GetAllActorsOfClass(World, AReplicatedTestActorBase_RepGraphAlwaysReplicate::StaticClass(), TestActors);

			AssertTrue(TestActors.Num() == Workers.Num(), TEXT("Not the expected number of actors"));

			FinishStep();
		},
		nullptr, 5.0f);

	AddStep(
		TEXT("Force actor delegation"), FWorkerDefinition::AllServers, nullptr, nullptr,
		[this](float DeltaTime) {
			if (!bIsOnDefaultLayer)
			{
				FinishStep();
			}
			int32 CurAuthWorker = DelegationStep / 2;
			int32 WorkerSubStep = DelegationStep % 2;
			switch (WorkerSubStep)
			{
			case 0:
				SetTagDelegation(GetTestTag(), Workers[CurAuthWorker]);
				++DelegationStep;
				break;
			case 1:
				bool bExpectedAuth = Workers[CurAuthWorker] == LocalWorker;
				bool bExpectedResult = true;

				TArray<AActor*> TestActors;
				UGameplayStatics::GetAllActorsOfClass(GetWorld(), AReplicatedTestActorBase_RepGraphAlwaysReplicate::StaticClass(), TestActors);
				for (AActor* Actor : TestActors)
				{
					bExpectedResult &= Actor->HasAuthority() == bExpectedAuth;
				}

				if (bExpectedResult)
				{
					++DelegationStep;
				}
				break;
			}

			if (DelegationStep >= Workers.Num() * 2)
			{
				FinishStep();
			}
		},
		5.0f);

	AddStep(
		TEXT("Create new actors"), FWorkerDefinition::AllServers, nullptr,
		[this] {
			UWorld* World = GetWorld();

			AReplicatedTestActorBase_RepGraphAlwaysReplicate* Actor =
				World->SpawnActor<AReplicatedTestActorBase_RepGraphAlwaysReplicate>(WorkerEntityPosition, FRotator());
			AddDebugTag(Actor, GetTestTag());
			RegisterAutoDestroyActor(Actor);
			FinishStep();
		},
		nullptr, 5.0f);

	AddStep(
		TEXT("Check new actors interest and delegation"), FWorkerDefinition::AllServers,
		[this]() -> bool {
			return WaitToSeeActors(AReplicatedTestActorBase_RepGraphAlwaysReplicate::StaticClass(), Workers.Num() * 2);
		},
		nullptr,
		[this](float DeltaTime) {
			if (!bIsOnDefaultLayer)
			{
				FinishStep();
			}
			int32_t CurAuthWorker = Workers.Num() - 1;

			bool bExpectedAuth = Workers[CurAuthWorker] == LocalWorker;
			bool bExpectedResult = true;

			TArray<AActor*> TestActors;
			UGameplayStatics::GetAllActorsOfClass(GetWorld(), AReplicatedTestActorBase_RepGraphAlwaysReplicate::StaticClass(), TestActors);
			for (AActor* Actor : TestActors)
			{
				bExpectedResult &= Actor->HasAuthority() == bExpectedAuth;
			}

			if (bExpectedResult)
			{
				FinishStep();
			}
		},
		5.0f);

	AddStep(
		TEXT("Remove extra interest"), FWorkerDefinition::AllServers, nullptr,
		[this]() {
			if (!bIsOnDefaultLayer)
			{
				FinishStep();
			}

			RemoveInterestOnTag(GetTestTag());
			FinishStep();
		},
		nullptr);

	AddStep(
		TEXT("Check extra interest removed"), FWorkerDefinition::AllServers,
		[this] {
			int32_t CurAuthWorker = Workers.Num() - 1;
			bool bExpectedAuth = Workers[CurAuthWorker] == LocalWorker;
			return WaitToSeeActors(AReplicatedTestActorBase_RepGraphAlwaysReplicate::StaticClass(), bExpectedAuth ? Workers.Num() * 2 : 2);
		},
		[this] {
			FinishStep();
		},
		nullptr, 5.0f);

	AddStep(
		TEXT("Add extra interest again"), FWorkerDefinition::AllServers, nullptr,
		[this]() {
			if (!bIsOnDefaultLayer)
			{
				FinishStep();
			}
			AddInterestOnTag(GetTestTag());
			FinishStep();
		},
		nullptr, 5.0f);

	AddStep(
		TEXT("Wait for extra interest to come back"), FWorkerDefinition::AllServers,
		[this] {
			return WaitToSeeActors(AReplicatedTestActorBase_RepGraphAlwaysReplicate::StaticClass(), Workers.Num() * 2);
		},
		[this] {
			FinishStep();
		},
		nullptr, 5.0f);

	AddStep(
		TEXT("Remove actor tags"), FWorkerDefinition::AllServers, nullptr,
		[this] {
			if (!bIsOnDefaultLayer)
			{
				FinishStep();
			}

			TArray<AActor*> TestActors;
			UGameplayStatics::GetAllActorsOfClass(GetWorld(), AReplicatedTestActorBase_RepGraphAlwaysReplicate::StaticClass(), TestActors);
			for (AActor* Actor : TestActors)
			{
				if (Actor->HasAuthority())
				{
					RemoveDebugTag(Actor, GetTestTag());
				}
			}

			FinishStep();
		},
		nullptr, 5.0f);

	AddStep(
		TEXT("Check state after tags removed"), FWorkerDefinition::AllServers,
		[this]() -> bool {
			return WaitToSeeActors(AReplicatedTestActorBase_RepGraphAlwaysReplicate::StaticClass(), 2);
		},
		nullptr,
		[this](float DeltaTime) {
			if (!bIsOnDefaultLayer)
			{
				FinishStep();
			}

			bool bExpectedResult = true;
			uint32 NumAuth = 0;

			TArray<AActor*> TestActors;
			UGameplayStatics::GetAllActorsOfClass(GetWorld(), AReplicatedTestActorBase_RepGraphAlwaysReplicate::StaticClass(), TestActors);
			for (AActor* Actor : TestActors)
			{
				bExpectedResult &= Actor->HasAuthority();
				NumAuth += Actor->HasAuthority() ? 1 : 0;
			}

			if (bExpectedResult)
			{
				AssertTrue(NumAuth == 2, TEXT("We see the expected number of authoritative actors"));

				FinishStep();
			}
		},
		5.0f);

	AddStep(
		TEXT("Add tag and remove delegation"), FWorkerDefinition::AllServers, nullptr,
		[this]() {
			if (!bIsOnDefaultLayer)
			{
				FinishStep();
			}

			uint32 NumUpdated = 0;

			TArray<AActor*> TestActors;
			UGameplayStatics::GetAllActorsOfClass(GetWorld(), AReplicatedTestActorBase_RepGraphAlwaysReplicate::StaticClass(), TestActors);
			for (AActor* Actor : TestActors)
			{
				if (Actor->HasAuthority())
				{
					AddDebugTag(Actor, GetTestTag());
					NumUpdated++;
				}
			}

			AssertTrue(NumUpdated == 2, TEXT("We updated the expected number of authoritative actors"));

			ClearTagDelegation(GetTestTag());
			FinishStep();
		},
		nullptr, 5.0f);

	AddStep(
		TEXT("Check state after delegation removal"), FWorkerDefinition::AllServers,
		[this] {
			return WaitToSeeActors(AReplicatedTestActorBase_RepGraphAlwaysReplicate::StaticClass(), Workers.Num() * 2);
		},
		[this]() {
			if (!bIsOnDefaultLayer)
			{
				FinishStep();
			}

			bool bExpectedResult = true;
			TArray<AActor*> TestActors;
			UGameplayStatics::GetAllActorsOfClass(GetWorld(), AReplicatedTestActorBase_RepGraphAlwaysReplicate::StaticClass(), TestActors);
			for (AActor* Actor : TestActors)
			{
				bExpectedResult &= (Actor->HasAuthority() == WorkerEntityPosition.Equals(Actor->GetActorLocation()));
			}

			if (bExpectedResult)
			{
				FinishStep();
			}
		},
		nullptr, 5.0f);

	AddStep(
		TEXT("Shutdown debugging"), FWorkerDefinition::AllServers, nullptr,
		[this]() {
			if (bIsOnDefaultLayer)
			{
				ClearTagDelegationAndInterest();
				FinishStep();
			}
		},
		nullptr, 5.0f);

	AddStep(
		TEXT("Check state after debug reset"), FWorkerDefinition::AllServers,
		[this]() -> bool {
			return WaitToSeeActors(AReplicatedTestActorBase_RepGraphAlwaysReplicate::StaticClass(), 2);
		},
		nullptr,
		[this](float DeltaTime) {
			if (!bIsOnDefaultLayer)
			{
				FinishStep();
			}

			bool bExpectedResult = true;

			TArray<AActor*> TestActors;
			UGameplayStatics::GetAllActorsOfClass(GetWorld(), AReplicatedTestActorBase_RepGraphAlwaysReplicate::StaticClass(), TestActors);
			for (AActor* Actor : TestActors)
			{
				bExpectedResult &= Actor->HasAuthority();
			}

			if (bExpectedResult)
			{
				FinishStep();
			}
		},
		5.0f);
}

USpatialDebugInterfaceMap::USpatialDebugInterfaceMap()
	: UGeneratedTestMap(EMapCategory::CI_PREMERGE_SPATIAL_ONLY, TEXT("SpatialDebugInterfaceMap"))
{
}

void USpatialDebugInterfaceMap::CreateCustomContentForMap()
{
	ULevel* CurrentLevel = World->GetCurrentLevel();

	// Add the tests
	AddActorToLevel<ASpatialDebugInterfaceTest>(CurrentLevel, FTransform::Identity);

	ASpatialWorldSettings* WorldSettings = CastChecked<ASpatialWorldSettings>(World->GetWorldSettings());
	WorldSettings->SetMultiWorkerSettingsClass(UTest1x2NoInterestWorkerSettings::StaticClass());
	WorldSettings->bEnableDebugInterface = true;
}
