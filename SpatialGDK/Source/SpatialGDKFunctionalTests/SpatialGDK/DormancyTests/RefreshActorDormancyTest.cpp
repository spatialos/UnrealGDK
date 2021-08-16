// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "RefreshActorDormancyTest.h"

#include "RefreshActorDormancyTestActor.h"
#include "Engine/NetDriver.h"
#include "Engine/NetworkObjectList.h"
#include "EngineClasses/SpatialNetDriver.h"
#include "Net/UnrealNetwork.h"

// This test checks whether changes on a dormant actor are replicated when the actor becomes awake.

ARefreshActorDormancyTest::ARefreshActorDormancyTest()
{
	Author = TEXT("Ajanth Sutharsan");
}

void ARefreshActorDormancyTest::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, DormancyActor);
}

void ARefreshActorDormancyTest::PrepareTest()
{
	Super::PrepareTest();

	// Step 1 - Spawn dormancy actor which is non dormant and change NetDormancy to DORM_DormantAll after a tick
	AddStep(TEXT("ServerSpawnNonDormatActor"), FWorkerDefinition::Server(1), nullptr, [this]() {
		DormancyActor = SpawnActor<ARefreshActorDormancyTestActor>(FActorSpawnParameters(), /*bRegisterAsAutoDestroy*/ true);
		FTimerManager& TimerManager = GetWorld()->GetTimerManager();
		TimerManager.SetTimerForNextTick(
			[this]() {
				DormancyActor->SetNetDormancy(DORM_DormantAll);
				FinishStep();
			});
	});

	// Step 2 - Client check NetDormancy is DORM_DormantAll
	AddStep(("ClientAssertDormancyTestState"), FWorkerDefinition::AllClients, nullptr, [this]() {
		AssertEqual_Int(DormancyActor->NetDormancy, DORM_DormantAll, TEXT("Dormancy on ARefreshActorDormancyTestActor"));
		FinishStep();
	});
}
