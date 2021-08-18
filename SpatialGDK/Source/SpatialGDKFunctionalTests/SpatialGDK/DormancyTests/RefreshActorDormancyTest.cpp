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

	// Step 1 - Spawn dormancy actor which is non dormant and dormant initially and then become dormant and non dormant respectively
	AddStep(TEXT("ServerSpawnDormantAndNonDormatActors"), FWorkerDefinition::Server(1), nullptr, [this]() {
		DormantToAwakeActor = SpawnActor<ARefreshActorDormancyTestActor>(FActorSpawnParameters(), true);
		AwakeToDormantActor = SpawnActor<ARefreshActorDormancyTestActor>(FActorSpawnParameters(), true);
		DormantToAwakeActor->SetInitiallyDormant(true);
		AwakeToDormantActor->SetInitiallyDormant(false);
		FinishStep();
	});

	// Step 2 - Client check NetDormancy is DORM_Awake for DormantToAwakeActor and DORM_DormantAll for AwakeToDormantActor
	AddStep(("ClientAssertDormancyTestStates"), FWorkerDefinition::AllClients, nullptr, nullptr,
			[this](float DeltaTime) {
				if (DormantToAwakeActor != nullptr && AwakeToDormantActor != nullptr)
				{
					RequireEqual_Int(DormantToAwakeActor->NetDormancy, DORM_Awake, TEXT("DormantToAwakeActor is Awake"));
					RequireEqual_Int(AwakeToDormantActor->NetDormancy, DORM_DormantAll, TEXT("AwakeToDormantActor is Dormant"));
					FinishStep();
				}
			},
			5.0f);
}
