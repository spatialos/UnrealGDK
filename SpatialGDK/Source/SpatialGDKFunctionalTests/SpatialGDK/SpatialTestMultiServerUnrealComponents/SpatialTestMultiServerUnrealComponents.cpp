// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "SpatialTestMultiServerUnrealComponents.h"

#include "SpatialFunctionalTestFlowController.h"
#include "SpatialGDKSettings.h"
#include "TestUnrealComponents.h"
#include "TestUnrealComponentsActor.h"

#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"

/**
 * This test tests that static and dynamic component are properly resolved on expectant workers.
 *
 * The test includes 2 Server and 2 Clients.
 * The flow is as follows:
 * - Setup:
 *  - The TestActor is spawned with both static and dynamic components.
 *  - The components are initialized with unique starting values.
 *  - The TestActor is possessed by Client1
 * - Test:
 *	- The owner client checks it can see the actor, components, and the values are correct.
 *  - The non-auth server checks it can see the actor, components, and the values are correct.
 *  - The non-owning client checks it can see the actor, components, and the values are correct.
 * - Clean-up:
 *	- The TestActor is destroyed.
 */
ASpatialTestMultiServerUnrealComponents::ASpatialTestMultiServerUnrealComponents()
	: Super()
{
	Author = "Mike";
	Description = TEXT("Test Unreal Component Replication in a MultiServer Context");
}

void ASpatialTestMultiServerUnrealComponents::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, TestActor);
}

void ASpatialTestMultiServerUnrealComponents::PrepareTest()
{
	Super::PrepareTest();

	bInitialOnlyEnabled = GetDefault<USpatialGDKSettings>()->bEnableInitialOnlyReplicationCondition;

	// The Server spawns the TestActor and immediately after it creates and attaches the dynamic components
	AddStep(TEXT("SpatialTestMultiServerUnrealComponents Spawn and Setup Test Actor"), FWorkerDefinition::Server(1), nullptr, [this]() {
		if (bInitialOnlyEnabled)
		{
			AddExpectedLogError(TEXT("Dynamic component using InitialOnly data. This data will not be sent."), 1, false);
		}

		AssertTrue(HasAuthority(), TEXT("Server requires auth"));
		TestActor = GetWorld()->SpawnActor<ATestUnrealComponentsActor>(ActorSpawnPosition, FRotator::ZeroRotator, FActorSpawnParameters());
		TestActor->SpawnDynamicComponents();

		const bool bWrite = true;
		ProcessActorProperties(bWrite);

		AController* PlayerController = Cast<AController>(GetFlowController(ESpatialFunctionalTestWorkerType::Client, 1)->GetOwner());
		AssertTrue(PlayerController->HasAuthority(), TEXT("Server requires authority over controller"));
		TestActor->SetOwner(PlayerController);

		RegisterAutoDestroyActor(TestActor);

		FinishStep();
	});

	// Check on owning client that actor exists and properties are correct
	AddStep(
		TEXT("SpatialTestMultiServerUnrealComponents Validate Test Actor On Owning Client"), FWorkerDefinition::Client(1),
		[this]() -> bool {
			return TestActor != nullptr;
		},
		[this]() {
			AssertFalse(TestActor->HasAuthority(), TEXT("This client shouldn't have authority"));
			if (AssertTrue(TestActor->AreAllDynamicComponentsValid(), TEXT("All dynamic components should have arrived")))
			{
				const bool bWrite = false;
				const bool bOwnerOnlyExpected = true;
				const bool bHandoverExpected = false;
				const bool bInitialOnlyExpected = true;
				ProcessActorProperties(bWrite, bOwnerOnlyExpected, bHandoverExpected, bInitialOnlyExpected);
			}

			FinishStep();
		});

	// Check on non-auth server that actor exists and properties are correct
	AddStep(
		TEXT("SpatialTestMultiServerUnrealComponents Validate Test Actor On Non-Auth Server"), FWorkerDefinition::Server(2),
		[this]() -> bool {
			return TestActor != nullptr;
		},
		[this]() {
			AssertFalse(TestActor->HasAuthority(), TEXT("This server shouldn't have authority"));
			if (AssertTrue(TestActor->AreAllDynamicComponentsValid(), TEXT("All dynamic components should have arrived")))
			{
				const bool bWrite = false;
				const bool bOwnerOnlyExpected = true;
				const bool bHandoverExpected = true;
				const bool bInitialOnlyExpected = true;
				ProcessActorProperties(bWrite, bOwnerOnlyExpected, bHandoverExpected, bInitialOnlyExpected);
			}

			FinishStep();
		});

	// Check on non-owning client that actor exists and properties are correct
	AddStep(
		TEXT("SpatialTestMultiServerUnrealComponents Validate Test Actor On Non-Owning Client"), FWorkerDefinition::Client(2),
		[this]() -> bool {
			return TestActor != nullptr;
		},
		[this]() {
			AssertFalse(TestActor->HasAuthority(), TEXT("This client shouldn't have authority"));
			if (AssertTrue(TestActor->AreAllDynamicComponentsValid(), TEXT("All dynamic components should have arrived")))
			{
				const bool bWrite = false;
				const bool bOwnerOnlyExpected = false;
				const bool bHandoverExpected = false;
				const bool bInitialOnlyExpected = true;
				ProcessActorProperties(bWrite, bOwnerOnlyExpected, bHandoverExpected, bInitialOnlyExpected);
			}

			FinishStep();
		});
}

void ASpatialTestMultiServerUnrealComponents::ProcessActorProperties(bool bWrite, bool bOwnerOnlyExpected, bool bHandoverExpected,
																	 bool bInitialOnlyExpected)
{
	auto Action = [&](int32& Source, int32 Expected) {
		if (bWrite)
		{
			Source = Expected;
		}
		else
		{
			AssertEqual_Int(Source, Expected, TEXT("Property replicated incorrectly"));
		}
	};

	Action(TestActor->StaticDataOnlyComponent->DataReplicatedVar, 10);

	Action(TestActor->DynamicDataOnlyComponent->DataReplicatedVar, 20);

	Action(TestActor->StaticDataAndHandoverComponent->DataReplicatedVar, 30);
	Action(TestActor->StaticDataAndHandoverComponent->HandoverReplicatedVar, (!bWrite && !bHandoverExpected) ? 0 : 31);

	Action(TestActor->DynamicDataAndHandoverComponent->DataReplicatedVar, 40);
	Action(TestActor->DynamicDataAndHandoverComponent->HandoverReplicatedVar, (!bWrite && !bHandoverExpected) ? 0 : 41);

	Action(TestActor->StaticDataAndOwnerOnlyComponent->DataReplicatedVar, 50);
	Action(TestActor->StaticDataAndOwnerOnlyComponent->OwnerOnlyReplicatedVar, (!bWrite && !bOwnerOnlyExpected) ? 0 : 51);

	Action(TestActor->DynamicDataAndOwnerOnlyComponent->DataReplicatedVar, 60);
	Action(TestActor->DynamicDataAndOwnerOnlyComponent->OwnerOnlyReplicatedVar, (!bWrite && !bOwnerOnlyExpected) ? 0 : 61);

	Action(TestActor->StaticDataAndInitialOnlyComponent->DataReplicatedVar, 70);
	Action(TestActor->StaticDataAndInitialOnlyComponent->InitialOnlyReplicatedVar, (!bWrite && !bInitialOnlyExpected) ? 0 : 71);

	Action(TestActor->DynamicDataAndInitialOnlyComponent->DataReplicatedVar, 80);
	Action(TestActor->DynamicDataAndInitialOnlyComponent->InitialOnlyReplicatedVar,
		   (!bWrite && (!bInitialOnlyExpected || bInitialOnlyEnabled)) ? 0 : 81);
}
