// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "RefreshActorDormancyTest.h"

#include "Engine/NetDriver.h"
#include "Engine/NetworkObjectList.h"
#include "EngineClasses/SpatialNetDriver.h"
#include "Net/UnrealNetwork.h"
#include "RefreshActorDormancyTestActor.h"

ARefreshActorDormancyTest::ARefreshActorDormancyTest()
{
	Author = TEXT("Ajanth Sutharsan");
}

void ARefreshActorDormancyTest::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, DormantToAwakeActor);
	DOREPLIFETIME(ThisClass, AwakeToDormantActor);
}

void ARefreshActorDormancyTest::PrepareTest()
{
	Super::PrepareTest();

	// Step 1 - Spawn dormancy actor which is non dormant and change NetDormancy to DORM_DormantAll after a tick
	AddStep(TEXT("ServerSpawnNonDormatActor"), FWorkerDefinition::Server(1), nullptr, [this]() {
		DormantToAwakeActor = SpawnActor<ARefreshActorDormancyTestActor>(FActorSpawnParameters(), /*bRegisterAsAutoDestroy*/ true);
		AwakeToDormantActor = SpawnActor<ARefreshActorDormancyTestActor>(FActorSpawnParameters(), /*bRegisterAsAutoDestroy*/ true);
		DormantToAwakeActor->SetupForTest(true);
		AwakeToDormantActor->SetupForTest(false);
		FinishStep();
	});

	// Step 2 - Client check NetDormancy is DORM_DormantAll
	AddStep(("ClientAssertDormancyTestState"), FWorkerDefinition::AllClients, nullptr, nullptr,
		[this](float DeltaTime) {
			RequireEqual_Int(DormantToAwakeActor->NetDormancy, DORM_Awake, TEXT("DormantToAwakeActor is Awake"));
			RequireEqual_Int(AwakeToDormantActor->NetDormancy, DORM_DormantAll, TEXT("AwakeToDormantActor is Dormant"));
			FinishStep();
		}, 5.0f);
}
