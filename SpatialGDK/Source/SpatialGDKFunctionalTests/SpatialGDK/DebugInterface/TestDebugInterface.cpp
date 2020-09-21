// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "TestDebugInterface.h"
#include "SpatialFunctionalTestFlowController.h"

#include "LoadBalancing/GridBasedLBStrategy.h"
#include "LoadBalancing/LayeredLBStrategy.h"

#include "Kismet/GameplayStatics.h"
#include "SpatialGDKFunctionalTests/SpatialGDK/TestActors/ReplicatedTestActorBase.h"

/*
Test for coverage of the USpatialGDKDebugInterface.


*/

ATestDebugInterface::ATestDebugInterface()
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

bool ATestDebugInterface::WaitToSeeActors(UClass* ActorClass, int32 NumActors)
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

void ATestDebugInterface::PrepareTest()
{
	Super::PrepareTest();

	AddStep(TEXT("SetupStep"), FWorkerDefinition::AllServers, nullptr, nullptr, [this](float DeltaTime) {
		UWorld* World = GetWorld();
		DelegationStep = 0;
		Workers.Empty();
		bIsOnDefaultLayer = false;

		ULayeredLBStrategy* RootStrategy = GetLoadBalancingStrategy();

		bIsOnDefaultLayer = RootStrategy->CouldHaveAuthority(AReplicatedTestActorBase::StaticClass());
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
			AReplicatedTestActorBase* Actor = World->SpawnActor<AReplicatedTestActorBase>(WorkerEntityPosition, FRotator());
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
			UGameplayStatics::GetAllActorsOfClass(World, AReplicatedTestActorBase::StaticClass(), TestActors);
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
			return WaitToSeeActors(AReplicatedTestActorBase::StaticClass(), Workers.Num());
		},
		[this]() {
			if (!bIsOnDefaultLayer)
			{
				FinishStep();
			}
			UWorld* World = GetWorld();

			TArray<AActor*> TestActors;
			UGameplayStatics::GetAllActorsOfClass(World, AReplicatedTestActorBase::StaticClass(), TestActors);

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
				DelegateTagToWorker(GetTestTag(), Workers[CurAuthWorker]);
				++DelegationStep;
				break;
			case 1:
				bool bExpectedAuth = Workers[CurAuthWorker] == LocalWorker;
				bool bExpectedResult = true;

				TArray<AActor*> TestActors;
				UGameplayStatics::GetAllActorsOfClass(GetWorld(), AReplicatedTestActorBase::StaticClass(), TestActors);
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
				UWorld* World = GetWorld();

				AReplicatedTestActorBase* Actor = World->SpawnActor<AReplicatedTestActorBase>(WorkerEntityPosition, FRotator());
				AddDebugTag(Actor, GetTestTag());
				RegisterAutoDestroyActor(Actor);

				FinishStep();
			}
		},
		5.0f);

	AddStep(
		TEXT("Check new actors interest and delegation"), FWorkerDefinition::AllServers,
		[this]() -> bool {
			return WaitToSeeActors(AReplicatedTestActorBase::StaticClass(), Workers.Num() * 2);
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
			UGameplayStatics::GetAllActorsOfClass(GetWorld(), AReplicatedTestActorBase::StaticClass(), TestActors);
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
			return WaitToSeeActors(AReplicatedTestActorBase::StaticClass(), bExpectedAuth ? Workers.Num() * 2 : 2);
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
		TEXT("Remove actor tags"), FWorkerDefinition::AllServers,
		[this] {
			return WaitToSeeActors(AReplicatedTestActorBase::StaticClass(), Workers.Num() * 2);
		},
		[this] {
			if (!bIsOnDefaultLayer)
			{
				FinishStep();
			}

			TArray<AActor*> TestActors;
			UGameplayStatics::GetAllActorsOfClass(GetWorld(), AReplicatedTestActorBase::StaticClass(), TestActors);
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
			return WaitToSeeActors(AReplicatedTestActorBase::StaticClass(), 2);
		},
		nullptr,
		[this](float DeltaTime) {
			if (!bIsOnDefaultLayer)
			{
				FinishStep();
			}

			bool bExpectedResult = true;

			TArray<AActor*> TestActors;
			UGameplayStatics::GetAllActorsOfClass(GetWorld(), AReplicatedTestActorBase::StaticClass(), TestActors);
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

	AddStep(
		TEXT("Add tag and remove delegation"), FWorkerDefinition::AllServers, nullptr,
		[this]() {
			if (!bIsOnDefaultLayer)
			{
				FinishStep();
			}
			bool bExpectedResult = true;

			TArray<AActor*> TestActors;
			UGameplayStatics::GetAllActorsOfClass(GetWorld(), AReplicatedTestActorBase::StaticClass(), TestActors);
			for (AActor* Actor : TestActors)
			{
				if (Actor->HasAuthority())
				{
					AddDebugTag(Actor, GetTestTag());
				}
			}

			RemoveTagDelegation(GetTestTag());
			FinishStep();
		},
		nullptr, 5.0f);

	AddStep(
		TEXT("Check state after delegation removal"), FWorkerDefinition::AllServers,
		[this] {
			return WaitToSeeActors(AReplicatedTestActorBase::StaticClass(), Workers.Num() * 2);
		},
		[this]() {
			if (!bIsOnDefaultLayer)
			{
				FinishStep();
			}

			bool bExpectedResult = true;
			TArray<AActor*> TestActors;
			UGameplayStatics::GetAllActorsOfClass(GetWorld(), AReplicatedTestActorBase::StaticClass(), TestActors);
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
			return WaitToSeeActors(AReplicatedTestActorBase::StaticClass(), 2);
		},
		nullptr,
		[this](float DeltaTime) {
			if (!bIsOnDefaultLayer)
			{
				FinishStep();
			}

			bool bExpectedResult = true;

			TArray<AActor*> TestActors;
			UGameplayStatics::GetAllActorsOfClass(GetWorld(), AReplicatedTestActorBase::StaticClass(), TestActors);
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
