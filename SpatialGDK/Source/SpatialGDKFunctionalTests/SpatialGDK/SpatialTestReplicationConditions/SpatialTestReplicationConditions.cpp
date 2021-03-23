// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "SpatialTestReplicationConditions.h"

#include "SpatialFunctionalTestFlowController.h"
#include "SpatialGDKSettings.h"
#include "TestReplicationConditionsActor.h"

#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"

/**
 * This tests that Unreal replication conditions.
 * This includes test for all replication conditions as defined in by ELifetimeCondition expect -
 * COND_InitialOnly - tested in SpatialTestInitialOnly
 * COND_InitialOrOwner - not supported in Spatial
 * COND_ReplayOnly - not relevant to Spatial
 *
 * The test includes 2 Server and 2 Clients.
 * The flow is as follows:
 * - Setup:
 *  - The test actors are spawned with both static and dynamic components.
 *  - The test actors are possessed by Client1
 * - Test:
 *  - The test actors are their properties initialized with unique starting values.
 *  - The non-auth server, owning client and non-owning client checks it can see the actors, components, and the values are correct.
 *  - The test actors have their properties updated to new unique values.
 *  - The non-auth server, owning client and non-owning client checks it can see the actors, components, and the values are correct.
 * - Clean-up:
 *	- The test actors are destroyed.
 */
ASpatialTestReplicationConditions::ASpatialTestReplicationConditions()
	: Super()
{
	Author = "Mike";
	Description = TEXT("Test Unreal Replication Conditions in a MultiServer Context");
	static_assert(COND_Max == 16, TEXT("New replication condition added - add more tests!"))
}

void ASpatialTestReplicationConditions::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, TestActor_Common);
	DOREPLIFETIME(ThisClass, TestActor_CustomEnabled);
	DOREPLIFETIME(ThisClass, TestActor_CustomDisabled);
	DOREPLIFETIME(ThisClass, TestActor_AutonomousOnly);
	DOREPLIFETIME(ThisClass, TestActor_PhysicsEnabled);
	DOREPLIFETIME(ThisClass, TestActor_PhysicsDisabled);
}

void ASpatialTestReplicationConditions::PrepareTest()
{
	Super::PrepareTest();

	bSpatialEnabled = GetDefault<UGeneralProjectSettings>()->UsesSpatialNetworking();
	ReplicationStage = STAGE_InitialReplication;
	PropertyOffset = 0;

	// Reusable steps
	FSpatialFunctionalTestStepDefinition CheckOnNonAuthServerStepDefinition(/*bIsNativeDefinition*/ true);
	CheckOnNonAuthServerStepDefinition.StepName = TEXT("ASpatialTestReplicationConditions Validate Test Actors On Non-Auth Server");
	CheckOnNonAuthServerStepDefinition.TimeLimit = 5.0f;
	CheckOnNonAuthServerStepDefinition.NativeIsReadyEvent.BindLambda([this]() -> bool {
		return ActorsReady();
	});
	CheckOnNonAuthServerStepDefinition.NativeStartEvent.BindLambda([this]() {
		AssertFalse(TestActor_Common->HasAuthority(), TEXT("This server shouldn't have authority"));
		AssertFalse(TestActor_CustomEnabled->HasAuthority(), TEXT("This server shouldn't have authority"));
		AssertFalse(TestActor_CustomDisabled->HasAuthority(), TEXT("This server shouldn't have authority"));
		AssertFalse(TestActor_AutonomousOnly->HasAuthority(), TEXT("This server shouldn't have authority"));
		AssertFalse(TestActor_PhysicsEnabled->HasAuthority(), TEXT("This server shouldn't have authority"));
		AssertFalse(TestActor_PhysicsDisabled->HasAuthority(), TEXT("This server shouldn't have authority"));

		if (AssertTrue(TestActor_Common->AreAllDynamicComponentsValid(),
					   TEXT("TestActor_Common - All dynamic components should have arrived")))
		{
			const bool bWrite = false;
			bool bCondIgnore[COND_Max]{};
			ProcessCommonActorProperties(bWrite, bCondIgnore);
		}

		if (AssertTrue(TestActor_CustomEnabled->AreAllDynamicComponentsValid(),
					   TEXT("TestActor_CustomEnabled - All dynamic components should have arrived")))
		{
			const bool bWrite = false;
			const bool bEnabled = true;
			ProcessCustomActorProperties(TestActor_CustomEnabled, bWrite, bEnabled);
		}

		if (!bSpatialEnabled) // TODO: UNR-5212 - fix DOREPLIFETIME_ACTIVE_OVERRIDE replication
		{
			if (AssertTrue(TestActor_CustomDisabled->AreAllDynamicComponentsValid(),
						   TEXT("TestActor_CustomDisabled - All dynamic components should have arrived")))
			{
				const bool bWrite = false;
				const bool bEnabled = false;
				ProcessCustomActorProperties(TestActor_CustomDisabled, bWrite, bEnabled);
			}
		}

		if (AssertTrue(TestActor_AutonomousOnly->AreAllDynamicComponentsValid(),
					   TEXT("TestActor_AutonomousOnly - All dynamic components should have arrived")))
		{
			const bool bWrite = false;
			const bool bAutonomousExpected = true;
			ProcessAutonomousOnlyActorProperties(bWrite, bAutonomousExpected);
		}

		if (AssertTrue(TestActor_PhysicsEnabled->AreAllDynamicComponentsValid(),
					   TEXT("TestActor_PhysicsEnabled - All dynamic components should have arrived")))
		{
			const bool bWrite = false;
			const bool bPhysicsEnabled = true;
			const bool bPhysicsExpected = true;
			ProcessPhysicsActorProperties(TestActor_PhysicsEnabled, bWrite, bPhysicsEnabled, bPhysicsExpected);
		}

		if (AssertTrue(TestActor_PhysicsDisabled->AreAllDynamicComponentsValid(),
					   TEXT("TestActor_PhysicsDisabled - All dynamic components should have arrived")))
		{
			const bool bWrite = false;
			const bool bPhysicsEnabled = false;
			const bool bPhysicsExpected = true; // Gets replicated through simulated status
			ProcessPhysicsActorProperties(TestActor_PhysicsDisabled, bWrite, bPhysicsEnabled, bPhysicsExpected);
		}

		FinishStep();
	});

	FSpatialFunctionalTestStepDefinition CheckOnOwningClientStepDefinition(/*bIsNativeDefinition*/ true);
	CheckOnOwningClientStepDefinition.StepName = TEXT("ASpatialTestReplicationConditions Validate Test Actors On Owning Client");
	CheckOnOwningClientStepDefinition.TimeLimit = 5.0f;
	CheckOnOwningClientStepDefinition.NativeIsReadyEvent.BindLambda([this]() -> bool {
		return ActorsReady();
	});
	CheckOnOwningClientStepDefinition.NativeStartEvent.BindLambda([this]() {
		AssertFalse(TestActor_Common->HasAuthority(), TEXT("This client shouldn't have authority"));
		AssertFalse(TestActor_CustomEnabled->HasAuthority(), TEXT("This client shouldn't have authority"));
		AssertFalse(TestActor_CustomDisabled->HasAuthority(), TEXT("This client shouldn't have authority"));
		AssertFalse(TestActor_AutonomousOnly->HasAuthority(), TEXT("This client shouldn't have authority"));
		AssertFalse(TestActor_PhysicsEnabled->HasAuthority(), TEXT("This client shouldn't have authority"));
		AssertFalse(TestActor_PhysicsDisabled->HasAuthority(), TEXT("This client shouldn't have authority"));

		AssertTrue(TestActor_Common->IsOwnedBy(GetLocalFlowController()->GetOwner()), TEXT("This client should own the actor"));
		AssertTrue(TestActor_AutonomousOnly->IsOwnedBy(GetLocalFlowController()->GetOwner()), TEXT("This client should own the actor"));
		AssertTrue(TestActor_PhysicsEnabled->IsOwnedBy(GetLocalFlowController()->GetOwner()), TEXT("This client should own the actor"));
		AssertTrue(TestActor_PhysicsDisabled->IsOwnedBy(GetLocalFlowController()->GetOwner()), TEXT("This client should own the actor"));

		if (AssertTrue(TestActor_Common->AreAllDynamicComponentsValid(),
					   TEXT("TestActor_Common - All dynamic components should have arrived")))
		{
			const bool bWrite = false;
			bool bCondIgnore[COND_Max]{};
			bCondIgnore[COND_AutonomousOnly] = true;
			bCondIgnore[COND_SkipOwner] = true;
			ProcessCommonActorProperties(bWrite, bCondIgnore);
		}

		if (AssertTrue(TestActor_CustomEnabled->AreAllDynamicComponentsValid(),
					   TEXT("TestActor_CustomEnabled - All dynamic components should have arrived")))
		{
			const bool bWrite = false;
			const bool bEnabled = true;
			ProcessCustomActorProperties(TestActor_CustomEnabled, bWrite, bEnabled);
		}

		if (!bSpatialEnabled) // TODO: UNR-5212 - fix DOREPLIFETIME_ACTIVE_OVERRIDE replication
		{
			if (AssertTrue(TestActor_CustomDisabled->AreAllDynamicComponentsValid(),
						   TEXT("TestActor_CustomDisabled - All dynamic components should have arrived")))
			{
				const bool bWrite = false;
				const bool bEnabled = false;
				ProcessCustomActorProperties(TestActor_CustomDisabled, bWrite, bEnabled);
			}
		}

		if (!bSpatialEnabled) // TODO: UNR-5213 - fix COND_AutonomousOnly replication
		{
			if (AssertTrue(TestActor_AutonomousOnly->AreAllDynamicComponentsValid(),
						   TEXT("TestActor_AutonomousOnly - All dynamic components should have arrived")))
			{
				const bool bWrite = false;
				const bool bAutonomousExpected = true;
				ProcessAutonomousOnlyActorProperties(bWrite, bAutonomousExpected);
			}
		}

		if (AssertTrue(TestActor_PhysicsEnabled->AreAllDynamicComponentsValid(),
					   TEXT("TestActor_PhysicsEnabled - All dynamic components should have arrived")))
		{
			const bool bWrite = false;
			const bool bPhysicsEnabled = true;
			const bool bPhysicsExpected = true;
			ProcessPhysicsActorProperties(TestActor_PhysicsEnabled, bWrite, bPhysicsEnabled, bPhysicsExpected);
		}

		if (!bSpatialEnabled) // TODO: UNR-5214 - fix physics condition replications
		{
			if (AssertTrue(TestActor_PhysicsDisabled->AreAllDynamicComponentsValid(),
						   TEXT("TestActor_PhysicsDisabled - All dynamic components should have arrived")))
			{
				const bool bWrite = false;
				const bool bPhysicsEnabled = false;
				const bool bPhysicsExpected = false; // Won't be replicated as no physics or simulated status
				ProcessPhysicsActorProperties(TestActor_PhysicsDisabled, bWrite, bPhysicsEnabled, bPhysicsExpected);
			}
		}

		FinishStep();
	});

	FSpatialFunctionalTestStepDefinition CheckOnNonOwningClientStepDefinition(/*bIsNativeDefinition*/ true);
	CheckOnNonOwningClientStepDefinition.StepName = TEXT("ASpatialTestReplicationConditions Validate Test Actors On Non-Owning Client");
	CheckOnNonOwningClientStepDefinition.TimeLimit = 5.0f;
	CheckOnNonOwningClientStepDefinition.NativeIsReadyEvent.BindLambda([this]() -> bool {
		return ActorsReady();
	});
	CheckOnNonOwningClientStepDefinition.NativeStartEvent.BindLambda([this]() {
		AssertFalse(TestActor_Common->HasAuthority(), TEXT("This client shouldn't have authority"));
		AssertFalse(TestActor_CustomEnabled->HasAuthority(), TEXT("This client shouldn't have authority"));
		AssertFalse(TestActor_CustomDisabled->HasAuthority(), TEXT("This client shouldn't have authority"));
		AssertFalse(TestActor_AutonomousOnly->HasAuthority(), TEXT("This client shouldn't have authority"));
		AssertFalse(TestActor_PhysicsEnabled->HasAuthority(), TEXT("This client shouldn't have authority"));
		AssertFalse(TestActor_PhysicsDisabled->HasAuthority(), TEXT("This client shouldn't have authority"));

		AssertFalse(TestActor_Common->IsOwnedBy(GetLocalFlowController()->GetOwner()), TEXT("This client should not own the actor"));
		AssertFalse(TestActor_AutonomousOnly->IsOwnedBy(GetLocalFlowController()->GetOwner()),
					TEXT("This client should not own the actor"));
		AssertFalse(TestActor_PhysicsEnabled->IsOwnedBy(GetLocalFlowController()->GetOwner()),
					TEXT("This client should not own the actor"));
		AssertFalse(TestActor_PhysicsDisabled->IsOwnedBy(GetLocalFlowController()->GetOwner()),
					TEXT("This client should not own the actor"));

		if (AssertTrue(TestActor_Common->AreAllDynamicComponentsValid(),
					   TEXT("TestActor_Common - All dynamic components should have arrived")))
		{
			const bool bWrite = false;
			bool bCondIgnore[COND_Max]{};
			bCondIgnore[COND_OwnerOnly] = true;
			bCondIgnore[COND_AutonomousOnly] = true;
			bCondIgnore[COND_ReplayOrOwner] = true;
			ProcessCommonActorProperties(bWrite, bCondIgnore);
		}

		if (AssertTrue(TestActor_CustomEnabled->AreAllDynamicComponentsValid(),
					   TEXT("TestActor_CustomEnabled - All dynamic components should have arrived")))
		{
			const bool bWrite = false;
			const bool bEnabled = true;
			ProcessCustomActorProperties(TestActor_CustomEnabled, bWrite, bEnabled);
		}

		if (!bSpatialEnabled) // TODO: UNR-5212 - fix DOREPLIFETIME_ACTIVE_OVERRIDE replication
		{
			if (AssertTrue(TestActor_CustomDisabled->AreAllDynamicComponentsValid(),
						   TEXT("TestActor_CustomDisabled - All dynamic components should have arrived")))
			{
				const bool bWrite = false;
				const bool bEnabled = false;
				ProcessCustomActorProperties(TestActor_CustomDisabled, bWrite, bEnabled);
			}
		}

		if (AssertTrue(TestActor_AutonomousOnly->AreAllDynamicComponentsValid(),
					   TEXT("TestActor_AutonomousOnly - All dynamic components should have arrived")))
		{
			const bool bWrite = false;
			const bool bAutonomousExpected = false;
			ProcessAutonomousOnlyActorProperties(bWrite, bAutonomousExpected);
		}

		if (AssertTrue(TestActor_PhysicsEnabled->AreAllDynamicComponentsValid(),
					   TEXT("TestActor_PhysicsEnabled - All dynamic components should have arrived")))
		{
			const bool bWrite = false;
			const bool bPhysicsEnabled = true;
			const bool bPhysicsExpected = true;
			ProcessPhysicsActorProperties(TestActor_PhysicsEnabled, bWrite, bPhysicsEnabled, bPhysicsExpected);
		}

		if (AssertTrue(TestActor_PhysicsDisabled->AreAllDynamicComponentsValid(),
					   TEXT("TestActor_PhysicsDisabled - All dynamic components should have arrived")))
		{
			const bool bWrite = false;
			const bool bPhysicsEnabled = false;
			const bool bPhysicsExpected = true; // Gets replicated through simulated status
			ProcessPhysicsActorProperties(TestActor_PhysicsDisabled, bWrite, bPhysicsEnabled, bPhysicsExpected);
		}

		FinishStep();
	});

	// The Server spawns the TestActors and immediately after it creates and attaches the dynamic components
	AddStep(TEXT("ASpatialTestReplicationConditions Spawn and Setup Test Actor"), FWorkerDefinition::Server(1), nullptr, [this]() {
		AssertTrue(HasAuthority(), TEXT("Server 1 requires authority over the test actor"));

		TestActor_Common = GetWorld()->SpawnActor<ATestReplicationConditionsActor_Common>(ActorSpawnPosition, FRotator::ZeroRotator,
																						  FActorSpawnParameters());
		if (!AssertTrue(TestActor_Common != nullptr, TEXT("Failed to spawn TestActor_Common")))
		{
			return;
		}

		TestActor_CustomEnabled = GetWorld()->SpawnActor<ATestReplicationConditionsActor_Custom>(ActorSpawnPosition, FRotator::ZeroRotator,
																								 FActorSpawnParameters());
		if (!AssertTrue(TestActor_CustomEnabled != nullptr, TEXT("Failed to spawn TestActor_CustomEnabled")))
		{
			return;
		}

		TestActor_CustomDisabled = GetWorld()->SpawnActor<ATestReplicationConditionsActor_Custom>(ActorSpawnPosition, FRotator::ZeroRotator,
																								  FActorSpawnParameters());
		if (!AssertTrue(TestActor_CustomDisabled != nullptr, TEXT("Failed to spawn TestActor_CustomDisabled")))
		{
			return;
		}

		TestActor_AutonomousOnly = GetWorld()->SpawnActor<ATestReplicationConditionsActor_AutonomousOnly>(
			ActorSpawnPosition, FRotator::ZeroRotator, FActorSpawnParameters());
		if (!AssertTrue(TestActor_AutonomousOnly != nullptr, TEXT("Failed to spawn TestActor_AutonomousOnly")))
		{
			return;
		}

		TestActor_PhysicsEnabled = GetWorld()->SpawnActor<ATestReplicationConditionsActor_Physics>(
			ActorSpawnPosition, FRotator::ZeroRotator, FActorSpawnParameters());
		if (!AssertTrue(TestActor_PhysicsEnabled != nullptr, TEXT("Failed to spawn TestActor_PhysicsEnabled")))
		{
			return;
		}

		TestActor_PhysicsDisabled = GetWorld()->SpawnActor<ATestReplicationConditionsActor_Physics>(
			ActorSpawnPosition, FRotator::ZeroRotator, FActorSpawnParameters());
		if (!AssertTrue(TestActor_PhysicsDisabled != nullptr, TEXT("Failed to spawn TestActor_PhysicsDisabled")))
		{
			return;
		}

		TestActor_Common->SpawnDynamicComponents();
		TestActor_CustomEnabled->SpawnDynamicComponents();
		TestActor_CustomDisabled->SpawnDynamicComponents();
		TestActor_AutonomousOnly->SpawnDynamicComponents();
		TestActor_PhysicsEnabled->SpawnDynamicComponents();
		TestActor_PhysicsDisabled->SpawnDynamicComponents();

		TestActor_CustomEnabled->SetCustomReplicationEnabled(true);
		TestActor_CustomDisabled->SetCustomReplicationEnabled(false);

		TestActor_PhysicsEnabled->SetPhysicsEnabled(true);
		TestActor_PhysicsDisabled->SetPhysicsEnabled(false);

		const bool bWrite = true;
		bool bCondIgnore[COND_Max]{};
		ProcessCommonActorProperties(bWrite, bCondIgnore);

		ProcessCustomActorProperties(TestActor_CustomEnabled, bWrite, /*bCustomEnabled*/ true);
		ProcessCustomActorProperties(TestActor_CustomDisabled, bWrite, /*bCustomEnabled*/ false);

		ProcessAutonomousOnlyActorProperties(bWrite, /*bAutonomousExpected*/ false);

		ProcessPhysicsActorProperties(TestActor_PhysicsEnabled, bWrite, /*bPhysicsEnabled*/ true, /*bPhysicsExpected*/ true);
		ProcessPhysicsActorProperties(TestActor_PhysicsDisabled, bWrite, /*bPhysicsEnabled*/ false, /*bPhysicsExpected*/ false);

		AController* PlayerController = Cast<AController>(GetFlowController(ESpatialFunctionalTestWorkerType::Client, 1)->GetOwner());
		if (!AssertTrue(PlayerController != nullptr, TEXT("Failed to retrieve player controller")))
		{
			return;
		}

		AssertTrue(PlayerController->HasAuthority(), TEXT("Server 1 requires authority over controller"));
		TestActor_Common->SetOwner(PlayerController);

		TestActor_AutonomousOnly->SetOwner(PlayerController);
		TestActor_AutonomousOnly->SetAutonomousProxy(true);

		// Set both physics actor to autonomous so properties won't be replicated through simulated condition
		TestActor_PhysicsEnabled->SetOwner(PlayerController);
		TestActor_PhysicsDisabled->SetOwner(PlayerController);
		TestActor_PhysicsEnabled->SetAutonomousProxy(true);
		TestActor_PhysicsDisabled->SetAutonomousProxy(true);

		RegisterAutoDestroyActor(TestActor_Common);
		RegisterAutoDestroyActor(TestActor_CustomEnabled);
		RegisterAutoDestroyActor(TestActor_CustomDisabled);
		RegisterAutoDestroyActor(TestActor_AutonomousOnly);
		RegisterAutoDestroyActor(TestActor_PhysicsEnabled);
		RegisterAutoDestroyActor(TestActor_PhysicsDisabled);

		FinishStep();
	});

	// Check on non-auth server that actor exists and properties are correct
	if (bSpatialEnabled)
	{
		AddStepFromDefinition(CheckOnNonAuthServerStepDefinition, FWorkerDefinition::Server(2));
	}

	// Check on owning client that actor exists and properties are correct
	AddStepFromDefinition(CheckOnOwningClientStepDefinition, FWorkerDefinition::Client(1));

	// Check on non-owning client that actor exists and properties are correct
	AddStepFromDefinition(CheckOnNonOwningClientStepDefinition, FWorkerDefinition::Client(2));

	AddStep(TEXT("ASpatialTestReplicationConditions Update Test Actors To Expect New Properties"), FWorkerDefinition::AllWorkers, nullptr,
			[this]() {
				ReplicationStage = STAGE_UpdateReplication;
				PropertyOffset = 10000;

				FinishStep();
			});

	// The Server updates TestActors with new replicated properties
	AddStep(TEXT("ASpatialTestReplicationConditions Update Test Actors With New Properties"), FWorkerDefinition::Server(1), nullptr,
			[this]() {
				const bool bWrite = true;
				bool bCondIgnore[COND_Max]{};
				ProcessCommonActorProperties(bWrite, bCondIgnore);

				ProcessCustomActorProperties(TestActor_CustomEnabled, bWrite, /*bCustomEnabled*/ true);
				ProcessCustomActorProperties(TestActor_CustomDisabled, bWrite, /*bCustomEnabled*/ false);

				ProcessAutonomousOnlyActorProperties(bWrite, /*bAutonomousExpected*/ false);

				ProcessPhysicsActorProperties(TestActor_PhysicsEnabled, bWrite, /*bPhysicsEnabled*/ true, /*bPhysicsExpected*/ true);
				ProcessPhysicsActorProperties(TestActor_PhysicsDisabled, bWrite, /*bPhysicsEnabled*/ false, /*bPhysicsExpected*/ false);

				FinishStep();
			});

	// Check on non-auth server that actor exists and properties are correct
	if (bSpatialEnabled)
	{
		AddStepFromDefinition(CheckOnNonAuthServerStepDefinition, FWorkerDefinition::Server(2));
	}

	// Check on owning client that actor exists and properties are correct
	AddStepFromDefinition(CheckOnOwningClientStepDefinition, FWorkerDefinition::Client(1));

	// Check on non-owning client that actor exists and properties are correct
	AddStepFromDefinition(CheckOnNonOwningClientStepDefinition, FWorkerDefinition::Client(2));
}

bool ASpatialTestReplicationConditions::ActorsReady() const
{
	bool bReady = true;

	bReady &= TestActor_Common != nullptr && TestActor_Common->AreAllDynamicComponentsValid();
	bReady &= TestActor_CustomEnabled != nullptr && TestActor_CustomEnabled->AreAllDynamicComponentsValid();
	bReady &= TestActor_CustomDisabled != nullptr && TestActor_CustomDisabled->AreAllDynamicComponentsValid();
	bReady &= TestActor_AutonomousOnly != nullptr && TestActor_AutonomousOnly->AreAllDynamicComponentsValid();
	bReady &= TestActor_PhysicsEnabled != nullptr && TestActor_PhysicsEnabled->AreAllDynamicComponentsValid();
	bReady &= TestActor_PhysicsDisabled != nullptr && TestActor_PhysicsDisabled->AreAllDynamicComponentsValid();

	return bReady;
}

void ASpatialTestReplicationConditions::ProcessCommonActorProperties(bool bWrite, bool bCondIgnore[COND_Max])
{
	// This function encapsulates the logic for both writing to, and reading from (and asserting) the properties of TestActor_Common.
	// When bWrite is true, the Action lambda just writes the expected property values to the Actor.
	// When bWrite is false, the Action lambda asserts that the each property has the expectant value.
	// As not all properties are replicated to all workers, we need to also know which property types to expect.

	auto Action = [&](int32& Source, int32 Expected, ELifetimeCondition Cond) {
		if (bWrite)
		{
			Source = Expected + PropertyOffset;
		}
		else if (Cond == COND_SkipOwner && bSpatialEnabled)
		{
			// UNR-3714 - COND_SkipOwner broken on spatial in initial replication
		}
		else if (bCondIgnore[Cond])
		{
			AssertEqual_Int(Source, 0, *FString::Printf(TEXT("Property replicated incorrectly on %s"), *GetLocalWorkerString()));
		}
		else
		{
			AssertEqual_Int(Source, Expected + PropertyOffset,
							*FString::Printf(TEXT("Property replicated incorrectly on %s"), *GetLocalWorkerString()));
		}
	};

	Action(TestActor_Common->CondNone_Var, 10, COND_None);
	Action(TestActor_Common->CondOwnerOnly_Var, 30, COND_OwnerOnly);
	Action(TestActor_Common->CondSkipOwner_Var, 40, COND_SkipOwner);
	Action(TestActor_Common->CondSimulatedOnly_Var, 50, COND_SimulatedOnly);
	Action(TestActor_Common->CondAutonomousOnly_Var, 60, COND_AutonomousOnly);
	Action(TestActor_Common->CondSimulatedOrPhysics_Var, 70, COND_SimulatedOrPhysics);
	Action(TestActor_Common->CondReplayOrOwner_Var, 100, COND_ReplayOrOwner);
	Action(TestActor_Common->CondSimulatedOnlyNoReplay_Var, 120, COND_SimulatedOnlyNoReplay);
	Action(TestActor_Common->CondSimulatedOrPhysicsNoReplay_Var, 130, COND_SimulatedOrPhysicsNoReplay);
	Action(TestActor_Common->CondSkipReplay_Var, 140, COND_SkipReplay);

	Action(TestActor_Common->StaticComponent->CondNone_Var, 210, COND_None);
	Action(TestActor_Common->StaticComponent->CondOwnerOnly_Var, 230, COND_OwnerOnly);
	Action(TestActor_Common->StaticComponent->CondSkipOwner_Var, 240, COND_SkipOwner);
	Action(TestActor_Common->StaticComponent->CondSimulatedOnly_Var, 250, COND_SimulatedOnly);
	Action(TestActor_Common->StaticComponent->CondAutonomousOnly_Var, 260, COND_AutonomousOnly);
	Action(TestActor_Common->StaticComponent->CondSimulatedOrPhysics_Var, 270, COND_SimulatedOrPhysics);
	Action(TestActor_Common->StaticComponent->CondReplayOrOwner_Var, 300, COND_ReplayOrOwner);
	Action(TestActor_Common->StaticComponent->CondSimulatedOnlyNoReplay_Var, 320, COND_SimulatedOnlyNoReplay);
	Action(TestActor_Common->StaticComponent->CondSimulatedOrPhysicsNoReplay_Var, 330, COND_SimulatedOrPhysicsNoReplay);
	Action(TestActor_Common->StaticComponent->CondSkipReplay_Var, 340, COND_SkipReplay);

	Action(TestActor_Common->DynamicComponent->CondNone_Var, 410, COND_None);
	Action(TestActor_Common->DynamicComponent->CondOwnerOnly_Var, 430, COND_OwnerOnly);
	Action(TestActor_Common->DynamicComponent->CondSkipOwner_Var, 440, COND_SkipOwner);
	Action(TestActor_Common->DynamicComponent->CondSimulatedOnly_Var, 450, COND_SimulatedOnly);
	Action(TestActor_Common->DynamicComponent->CondAutonomousOnly_Var, 460, COND_AutonomousOnly);
	Action(TestActor_Common->DynamicComponent->CondSimulatedOrPhysics_Var, 470, COND_SimulatedOrPhysics);
	Action(TestActor_Common->DynamicComponent->CondReplayOrOwner_Var, 500, COND_ReplayOrOwner);
	Action(TestActor_Common->DynamicComponent->CondSimulatedOnlyNoReplay_Var, 520, COND_SimulatedOnlyNoReplay);
	Action(TestActor_Common->DynamicComponent->CondSimulatedOrPhysicsNoReplay_Var, 530, COND_SimulatedOrPhysicsNoReplay);
	Action(TestActor_Common->DynamicComponent->CondSkipReplay_Var, 540, COND_SkipReplay);
}

void ASpatialTestReplicationConditions::ProcessCustomActorProperties(ATestReplicationConditionsActor_Custom* Actor, bool bWrite,
																	 bool bCustomEnabled)
{
	auto Action = [&](int32& Source, int32 Expected) {
		if (bWrite)
		{
			Source = Expected + PropertyOffset;
		}
		else if (bCustomEnabled)
		{
			AssertEqual_Int(Source, Expected + PropertyOffset,
							*FString::Printf(TEXT("Property replicated incorrectly on %s"), *GetLocalWorkerString()));
		}
		else
		{
			AssertEqual_Int(Source, 0, *FString::Printf(TEXT("Property replicated incorrectly on %s"), *GetLocalWorkerString()));
		}
	};

	Action(Actor->CondCustom_Var, bCustomEnabled ? 1010 : 2010);
	Action(Actor->StaticComponent->CondCustom_Var, bCustomEnabled ? 1020 : 2020);
	Action(Actor->DynamicComponent->CondCustom_Var, bCustomEnabled ? 1030 : 2030);
}

void ASpatialTestReplicationConditions::ProcessAutonomousOnlyActorProperties(bool bWrite, bool bAutonomousExpected)
{
	auto Action = [&](int32& Source, int32 Expected) {
		if (bWrite)
		{
			Source = Expected + PropertyOffset;
		}
		else if (bAutonomousExpected)
		{
			AssertEqual_Int(Source, Expected + PropertyOffset,
							*FString::Printf(TEXT("Property replicated incorrectly on %s"), *GetLocalWorkerString()));
		}
		else
		{
			AssertEqual_Int(Source, 0, *FString::Printf(TEXT("Property replicated incorrectly on %s"), *GetLocalWorkerString()));
		}
	};

	Action(TestActor_AutonomousOnly->CondAutonomousOnly_Var, 3010);
	Action(TestActor_AutonomousOnly->StaticComponent->CondAutonomousOnly_Var, 3020);
	Action(TestActor_AutonomousOnly->DynamicComponent->CondAutonomousOnly_Var, 3030);
}

void ASpatialTestReplicationConditions::ProcessPhysicsActorProperties(ATestReplicationConditionsActor_Physics* Actor, bool bWrite,
																	  bool bPhysicsEnabled, bool bPhysicsExpected)
{
	auto Action = [&](int32& Source, int32 Expected) {
		if (bWrite)
		{
			Source = Expected + PropertyOffset;
		}
		else if (bPhysicsExpected)
		{
			AssertEqual_Int(Source, Expected + PropertyOffset,
							*FString::Printf(TEXT("Property replicated incorrectly on %s"), *GetLocalWorkerString()));
		}
		else
		{
			AssertEqual_Int(Source, 0, *FString::Printf(TEXT("Property replicated incorrectly on %s"), *GetLocalWorkerString()));
		}
	};

	Action(Actor->CondSimulatedOrPhysics_Var, (bPhysicsEnabled) ? 4010 : 5010);
	Action(Actor->CondSimulatedOrPhysicsNoReplay_Var, (bPhysicsEnabled) ? 4020 : 5020);
	Action(Actor->StaticComponent->CondSimulatedOrPhysics_Var, (bPhysicsEnabled) ? 4030 : 5030);
	Action(Actor->StaticComponent->CondSimulatedOrPhysicsNoReplay_Var, (bPhysicsEnabled) ? 4040 : 5040);
	Action(Actor->DynamicComponent->CondSimulatedOrPhysics_Var, (bPhysicsEnabled) ? 4050 : 5050);
	Action(Actor->DynamicComponent->CondSimulatedOrPhysicsNoReplay_Var, (bPhysicsEnabled) ? 4060 : 5060);
}
