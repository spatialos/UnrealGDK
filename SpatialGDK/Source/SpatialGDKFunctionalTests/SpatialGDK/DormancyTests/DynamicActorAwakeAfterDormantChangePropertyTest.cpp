// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "DynamicActorAwakeAfterDormantChangePropertyTest.h"

#include "DormancyTestActor.h"
#include "Engine/NetDriver.h"
#include "Engine/NetworkObjectList.h"
#include "EngineClasses/SpatialNetDriver.h"
#include "Net/UnrealNetwork.h"

// This test checks whether changes on a dormant actor are replicated when the actor becomes awake.

ADynamicActorAwakeAfterDormantChangePropertyTest::ADynamicActorAwakeAfterDormantChangePropertyTest()
{
	Author = TEXT("Matthew Sandford");
}

void ADynamicActorAwakeAfterDormantChangePropertyTest::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, DormancyActor);
}

void ADynamicActorAwakeAfterDormantChangePropertyTest::PrepareTest()
{
	Super::PrepareTest();

	// Step 1 - Spawn dormancy actor and change NetDormancy to DORM_DormantAll
	AddStep(TEXT("ServerSpawnDormancyActor"), FWorkerDefinition::Server(1), nullptr, [this]() {
		DormancyActor = SpawnActor<ADormancyTestActor>(FActorSpawnParameters(), /*bRegisterAsAutoDestroy*/ true);
		// Going from DORM_DormInitial to DORM_DormantAll
		DormancyActor->SetNetDormancy(DORM_DormantAll);
		FinishStep();
	});

	// Step 2 Require channel to actually be dormant
	AddStep(TEXT("ServerAssertActuallyDormant"), FWorkerDefinition::Server(1), nullptr, nullptr, [this](float) {
		const TSet<TSharedPtr<FNetworkObjectInfo>, FNetworkObjectKeyFuncs>& Actives =
			DormancyActor->GetNetDriver()->GetNetworkObjectList().GetActiveObjects();
		const TSet<TSharedPtr<FNetworkObjectInfo>, FNetworkObjectKeyFuncs>& Dormants =
			DormancyActor->GetNetDriver()->GetNetworkObjectList().GetDormantObjectsOnAllConnections();

		RequireEqual_Bool(Actives.Contains(DormancyActor), false, TEXT("Require test actor to no longer be in ActiveObjects"));
		RequireEqual_Bool(Dormants.Contains(DormancyActor), true, TEXT("Require test actor to be in ObjectsDormantOnAllConnections"));
		FinishStep();
	});

	// Step 3 - Client check NetDormancy is DORM_DormantAll
	AddStep(("ClientAssertDormancyTestState"), FWorkerDefinition::AllClients, nullptr, [this]() {
		AssertEqual_Int(DormancyActor->NetDormancy, DORM_DormantAll, TEXT("Dormancy on ADormancyTestActor"));
		AssertEqual_Int(DormancyActor->TestIntProp, 0, TEXT("TestIntProp on ADormancyTestActor"));
		FinishStep();
	});

	// Step 4 - Server set TestIntProp to 1
	AddStep(TEXT("ServerModifyNetDormancy"), FWorkerDefinition::Server(1), nullptr, [this]() {
		DormancyActor->TestIntProp = 1;
		FinishStep();
	});

	// Step 5 - Give chance for property to be replicated to clients, if we don't do this the
	// next step will assert the 0 value
	AddStep(
		TEXT("ClientWaitForReplication"), FWorkerDefinition::AllClients, nullptr,
		[this]() {
			FTimerHandle DelayTimerHandle;
			FTimerManager& TimerManager = GetWorld()->GetTimerManager();
			TimerManager.SetTimer(
				DelayTimerHandle,
				[this]() {
					FinishStep();
				},
				0.5f, false);
		},
		nullptr, 5.0f);

	// Step 6 - Client check TestIntProp is still 0
	AddStep(
		TEXT("ClientRequireDormancyTestState"), FWorkerDefinition::AllClients, nullptr, nullptr,
		[this](float DeltaTime) {
			AssertEqual_Int(DormancyActor->NetDormancy, DORM_DormantAll, TEXT("Dormancy on ADormancyTestActor"));
			AssertEqual_Int(DormancyActor->TestIntProp, 0, TEXT("TestIntProp on ADormancyTestActor"));
			FinishStep();
		},
		5.0f);

	// Step 7 - Server set NetDormancy to DORM_Awake
	AddStep(TEXT("ServerModifyNetDormancy"), FWorkerDefinition::Server(1), nullptr, [this]() {
		DormancyActor->SetNetDormancy(DORM_Awake);
		FinishStep();
	});

	// Step 8 Require channel to actually be DORM_Awake
	AddStep(TEXT("ServerRequireActuallyAwake"), FWorkerDefinition::Server(1), nullptr, nullptr, [this](float) {
		const TSet<TSharedPtr<FNetworkObjectInfo>, FNetworkObjectKeyFuncs>& Actives =
			DormancyActor->GetNetDriver()->GetNetworkObjectList().GetActiveObjects();
		const TSet<TSharedPtr<FNetworkObjectInfo>, FNetworkObjectKeyFuncs>& Dormants =
			DormancyActor->GetNetDriver()->GetNetworkObjectList().GetDormantObjectsOnAllConnections();

		RequireEqual_Bool(Actives.Contains(DormancyActor), true, TEXT("Require test actor to be in ActiveObjects"));
		RequireEqual_Bool(Dormants.Contains(DormancyActor), false,
						  TEXT("Require test actor to no longer be in ObjectsDormantOnAllConnections"));
		FinishStep();
	});

	// Step 9 - Client check NetDormancy is DORM_Awake and TestIntProp is 1
	AddStep(
		TEXT("ClientRequireDormancyTestState"), FWorkerDefinition::AllClients, nullptr, nullptr,
		[this](float DeltaTime) {
			RequireEqual_Int(DormancyActor->NetDormancy, DORM_Awake, TEXT("Dormancy on ADormancyTestActor"));
			RequireEqual_Int(DormancyActor->TestIntProp, 1, TEXT("TestIntProp on ADormancyTestActor"));
			FinishStep();
		},
		5.0f);
}
