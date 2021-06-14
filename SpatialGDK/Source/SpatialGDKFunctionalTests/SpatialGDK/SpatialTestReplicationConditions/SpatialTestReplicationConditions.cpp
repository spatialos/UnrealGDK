// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "SpatialTestReplicationConditions.h"

#include "SpatialFunctionalTestFlowController.h"
#include "SpatialGDKSettings.h"
#include "TestReplicationConditionsActor.h"

#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"

/**
 * This test tests Unreal replication conditions.
 * This includes tests for all replication conditions as defined by ELifetimeCondition, except:
 * COND_InitialOnly - tested in SpatialTestInitialOnly
 * COND_InitialOrOwner - not supported in Spatial
 * COND_ReplayOnly - not relevant to Spatial
 *
 * There are 6 test actors used in this test.
 * TestActor_Common, non-autonomous, physics disabled, used for all common replication conditions.
 * TestActor_CustomEnabled, non-autonomous, physics disabled, used for COND_Custom.
 * TestActor_CustomDisabled, non-autonomous, physics disabled, used for COND_Custom.
 * TestActor_AutonomousOnly, autonomous, physics disabled, used for autonomous related conditions.
 * TestActor_PhysicsEnabled, non-autonomous, physics enabled, used for physics related conditions.
 * TestActor_PhysicsDisabled, non-autonomous, physics disabled, used for physics related conditions.
 *
 * The test includes 2 Server and 2 Clients.
 * The flow is as follows:
 * - Setup:
 *  - The test actors are spawned with both static and dynamic components.
 *  - The test actors are possessed by Client1
 * - Test:
 *  - The test actors have their properties initialized with unique starting values.
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
	static_assert(COND_Max == 16, TEXT("New replication condition added - add more tests!"));
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

	// Reusable steps
	FSpatialFunctionalTestStepDefinition CheckOnNonAuthServerStepDefinition(/*bIsNativeDefinition*/ true);
	CheckOnNonAuthServerStepDefinition.StepName = TEXT("ASpatialTestReplicationConditions Validate Test Actors On Non-Auth Server");
	CheckOnNonAuthServerStepDefinition.TimeLimit = 5.0f;
	CheckOnNonAuthServerStepDefinition.NativeIsReadyEvent.BindLambda([this]() -> bool {
		return ActorsReady();
	});
	CheckOnNonAuthServerStepDefinition.NativeTickEvent.BindLambda([this](float /*DeltaTime*/) {
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
			const bool bSimulatedExpected = true;
			ProcessAutonomousOnlyActorProperties(bWrite, bAutonomousExpected, bSimulatedExpected);
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
	CheckOnOwningClientStepDefinition.NativeTickEvent.BindLambda([this](float /*DeltaTime*/) {
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
			bCondIgnore[COND_ServerOnly] = true;
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
			const bool bSimulatedExpected = false;
			ProcessAutonomousOnlyActorProperties(bWrite, bAutonomousExpected, bSimulatedExpected);
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
			const bool bPhysicsExpected = false; // Won't be replicated as no physics or simulated status
			ProcessPhysicsActorProperties(TestActor_PhysicsDisabled, bWrite, bPhysicsEnabled, bPhysicsExpected);
		}

		FinishStep();
	});

	FSpatialFunctionalTestStepDefinition CheckOnNonOwningClientStepDefinition(/*bIsNativeDefinition*/ true);
	CheckOnNonOwningClientStepDefinition.StepName = TEXT("ASpatialTestReplicationConditions Validate Test Actors On Non-Owning Client");
	CheckOnNonOwningClientStepDefinition.TimeLimit = 5.0f;
	CheckOnNonOwningClientStepDefinition.NativeIsReadyEvent.BindLambda([this]() -> bool {
		return ActorsReady();
	});
	CheckOnNonOwningClientStepDefinition.NativeTickEvent.BindLambda([this](float /*DeltaTime*/) {
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
			bCondIgnore[COND_ServerOnly] = true;
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
			const bool bSimulatedExpected = true;
			ProcessAutonomousOnlyActorProperties(bWrite, bAutonomousExpected, bSimulatedExpected);
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

	// Setup initial test variables
	AddStep(TEXT("ASpatialTestReplicationConditions Initial Test Setup"), FWorkerDefinition::AllWorkers, nullptr, [this]() {
		ReplicationStage = STAGE_InitialReplication;
		PropertyOffset = 0;

		FinishStep();
	});

	// The Server spawns the TestActors and immediately after it creates and attaches the dynamic components
	AddStep(TEXT("ASpatialTestReplicationConditions Spawn and Setup Test Actor"), FWorkerDefinition::Server(1), nullptr, [this]() {
		AssertTrue(HasAuthority(), TEXT("Server 1 requires authority over the test actor"));

		TestActor_Common = GetWorld()->SpawnActor<ATestReplicationConditionsActor_Common>(ActorSpawnPosition, FRotator::ZeroRotator,
																						  FActorSpawnParameters());
		if (!AssertTrue(IsValid(TestActor_Common), TEXT("Failed to spawn TestActor_Common")))
		{
			return;
		}

		TestActor_CustomEnabled = GetWorld()->SpawnActor<ATestReplicationConditionsActor_Custom>(ActorSpawnPosition, FRotator::ZeroRotator,
																								 FActorSpawnParameters());
		if (!AssertTrue(IsValid(TestActor_CustomEnabled), TEXT("Failed to spawn TestActor_CustomEnabled")))
		{
			return;
		}

		TestActor_CustomDisabled = GetWorld()->SpawnActor<ATestReplicationConditionsActor_Custom>(ActorSpawnPosition, FRotator::ZeroRotator,
																								  FActorSpawnParameters());
		if (!AssertTrue(IsValid(TestActor_CustomDisabled), TEXT("Failed to spawn TestActor_CustomDisabled")))
		{
			return;
		}

		TestActor_AutonomousOnly = GetWorld()->SpawnActor<ATestReplicationConditionsActor_AutonomousOnly>(
			ActorSpawnPosition, FRotator::ZeroRotator, FActorSpawnParameters());
		if (!AssertTrue(IsValid(TestActor_AutonomousOnly), TEXT("Failed to spawn TestActor_AutonomousOnly")))
		{
			return;
		}

		TestActor_PhysicsEnabled = GetWorld()->SpawnActor<ATestReplicationConditionsActor_Physics>(
			ActorSpawnPosition, FRotator::ZeroRotator, FActorSpawnParameters());
		if (!AssertTrue(IsValid(TestActor_PhysicsEnabled), TEXT("Failed to spawn TestActor_PhysicsEnabled")))
		{
			return;
		}

		TestActor_PhysicsDisabled = GetWorld()->SpawnActor<ATestReplicationConditionsActor_Physics>(
			ActorSpawnPosition, FRotator::ZeroRotator, FActorSpawnParameters());
		if (!AssertTrue(IsValid(TestActor_PhysicsDisabled), TEXT("Failed to spawn TestActor_PhysicsDisabled")))
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

		ProcessAutonomousOnlyActorProperties(bWrite, /*bAutonomousExpected*/ false, /*bSimulatedExpected*/ false);

		ProcessPhysicsActorProperties(TestActor_PhysicsEnabled, bWrite, /*bPhysicsEnabled*/ true, /*bPhysicsExpected*/ true);
		ProcessPhysicsActorProperties(TestActor_PhysicsDisabled, bWrite, /*bPhysicsEnabled*/ false, /*bPhysicsExpected*/ false);

		AController* PlayerController = Cast<AController>(GetFlowController(ESpatialFunctionalTestWorkerType::Client, 1)->GetOwner());
		if (!AssertTrue(IsValid(PlayerController), TEXT("Failed to retrieve player controller")))
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

		// Set both custom actors to be owned by the PlayerController, just so we guarantee that they stay on server 1
		TestActor_CustomEnabled->SetOwner(PlayerController);
		TestActor_CustomDisabled->SetOwner(PlayerController);

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

				ProcessAutonomousOnlyActorProperties(bWrite, /*bAutonomousExpected*/ false, /*bSimulatedExpected*/ false);

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

	bReady &= IsValid(TestActor_Common) && TestActor_Common->AreAllDynamicComponentsValid();
	bReady &= IsValid(TestActor_CustomEnabled) && TestActor_CustomEnabled->AreAllDynamicComponentsValid();
	bReady &= IsValid(TestActor_CustomDisabled) && TestActor_CustomDisabled->AreAllDynamicComponentsValid();
	bReady &= IsValid(TestActor_AutonomousOnly) && TestActor_AutonomousOnly->AreAllDynamicComponentsValid();
	bReady &= IsValid(TestActor_PhysicsEnabled) && TestActor_PhysicsEnabled->AreAllDynamicComponentsValid();
	bReady &= IsValid(TestActor_PhysicsDisabled) && TestActor_PhysicsDisabled->AreAllDynamicComponentsValid();

	return bReady;
}

FString CondAsString(ELifetimeCondition Condition)
{
	const UEnum* EnumClass = FindObject<UEnum>(ANY_PACKAGE, TEXT("ELifetimeCondition"), true);
	FName ConditionName = EnumClass->GetNameByValue((int64)Condition);
	ensureAlwaysMsgf(ConditionName != NAME_None, TEXT("Could not find ELifetimeCondition"));
	return ConditionName.ToString();
}

FString StaticCompText = TEXT(" on static component");
FString DynamicCompText = TEXT(" on dynamic component");

// This function encapsulates the logic for both writing to, and reading from (and asserting) the properties of TestActor_Common.
// When bWrite is true, the Action lambda just writes the expected property values to the Actor.
// When bWrite is false, the Action lambda asserts that the each property has the expectant value.
// As not all properties are replicated to all workers, we need to also know which property types to expect.
void ASpatialTestReplicationConditions::Action(int32& Source, int32 Expected, ELifetimeCondition Cond, bool bWrite,
											   bool bCondIgnore[COND_Max], FString AdditionalText)
{
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
		RequireEqual_Int(Source, 0,
						 *FString::Printf(TEXT("Property%s with condition %s should not be replicated to %s"), *AdditionalText,
										  *CondAsString(Cond), *GetLocalWorkerString()));
	}
	else
	{
		RequireEqual_Int(Source, Expected + PropertyOffset,
						 *FString::Printf(TEXT("Property%s with condition %s should be replicated to %s"), *AdditionalText,
										  *CondAsString(Cond), *GetLocalWorkerString()));
	}
}

void ASpatialTestReplicationConditions::ProcessCommonActorProperties(bool bWrite, bool bCondIgnore[COND_Max])
{
	auto WrappedAction = [&](int32& Source, int32 Expected, ELifetimeCondition Cond, FString AdditionalText = TEXT("")) {
		Action(Source, Expected, Cond, bWrite, bCondIgnore, AdditionalText);
	};

	WrappedAction(TestActor_Common->CondNone_Var, 10, COND_None);
	WrappedAction(TestActor_Common->CondOwnerOnly_Var, 30, COND_OwnerOnly);
	WrappedAction(TestActor_Common->CondSkipOwner_Var, 40, COND_SkipOwner);
	WrappedAction(TestActor_Common->CondSimulatedOnly_Var, 50, COND_SimulatedOnly);
	WrappedAction(TestActor_Common->CondAutonomousOnly_Var, 60, COND_AutonomousOnly);
	WrappedAction(TestActor_Common->CondSimulatedOrPhysics_Var, 70, COND_SimulatedOrPhysics);
	WrappedAction(TestActor_Common->CondReplayOrOwner_Var, 100, COND_ReplayOrOwner);
	WrappedAction(TestActor_Common->CondSimulatedOnlyNoReplay_Var, 120, COND_SimulatedOnlyNoReplay);
	WrappedAction(TestActor_Common->CondSimulatedOrPhysicsNoReplay_Var, 130, COND_SimulatedOrPhysicsNoReplay);
	WrappedAction(TestActor_Common->CondSkipReplay_Var, 140, COND_SkipReplay);
	WrappedAction(TestActor_Common->CondServerOnly_Var, 150, COND_ServerOnly);

	WrappedAction(TestActor_Common->StaticComponent->CondNone_Var, 210, COND_None, StaticCompText);
	WrappedAction(TestActor_Common->StaticComponent->CondOwnerOnly_Var, 230, COND_OwnerOnly, StaticCompText);
	WrappedAction(TestActor_Common->StaticComponent->CondSkipOwner_Var, 240, COND_SkipOwner, StaticCompText);
	WrappedAction(TestActor_Common->StaticComponent->CondSimulatedOnly_Var, 250, COND_SimulatedOnly, StaticCompText);
	WrappedAction(TestActor_Common->StaticComponent->CondAutonomousOnly_Var, 260, COND_AutonomousOnly, StaticCompText);
	WrappedAction(TestActor_Common->StaticComponent->CondSimulatedOrPhysics_Var, 270, COND_SimulatedOrPhysics, StaticCompText);
	WrappedAction(TestActor_Common->StaticComponent->CondReplayOrOwner_Var, 300, COND_ReplayOrOwner, StaticCompText);
	WrappedAction(TestActor_Common->StaticComponent->CondSimulatedOnlyNoReplay_Var, 320, COND_SimulatedOnlyNoReplay, StaticCompText);
	WrappedAction(TestActor_Common->StaticComponent->CondSimulatedOrPhysicsNoReplay_Var, 330, COND_SimulatedOrPhysicsNoReplay,
				  StaticCompText);
	WrappedAction(TestActor_Common->StaticComponent->CondSkipReplay_Var, 340, COND_SkipReplay, StaticCompText);
	WrappedAction(TestActor_Common->StaticComponent->CondServerOnly_Var, 350, COND_ServerOnly, StaticCompText);

	WrappedAction(TestActor_Common->DynamicComponent->CondNone_Var, 410, COND_None, DynamicCompText);
	WrappedAction(TestActor_Common->DynamicComponent->CondOwnerOnly_Var, 430, COND_OwnerOnly, DynamicCompText);
	WrappedAction(TestActor_Common->DynamicComponent->CondSkipOwner_Var, 440, COND_SkipOwner, DynamicCompText);
	WrappedAction(TestActor_Common->DynamicComponent->CondSimulatedOnly_Var, 450, COND_SimulatedOnly, DynamicCompText);
	WrappedAction(TestActor_Common->DynamicComponent->CondAutonomousOnly_Var, 460, COND_AutonomousOnly, DynamicCompText);
	WrappedAction(TestActor_Common->DynamicComponent->CondSimulatedOrPhysics_Var, 470, COND_SimulatedOrPhysics, DynamicCompText);
	WrappedAction(TestActor_Common->DynamicComponent->CondReplayOrOwner_Var, 500, COND_ReplayOrOwner, DynamicCompText);
	WrappedAction(TestActor_Common->DynamicComponent->CondSimulatedOnlyNoReplay_Var, 520, COND_SimulatedOnlyNoReplay, DynamicCompText);
	WrappedAction(TestActor_Common->DynamicComponent->CondSimulatedOrPhysicsNoReplay_Var, 530, COND_SimulatedOrPhysicsNoReplay,
				  DynamicCompText);
	WrappedAction(TestActor_Common->DynamicComponent->CondSkipReplay_Var, 540, COND_SkipReplay, DynamicCompText);
	WrappedAction(TestActor_Common->DynamicComponent->CondServerOnly_Var, 550, COND_ServerOnly, DynamicCompText);
}

void ASpatialTestReplicationConditions::ProcessCustomActorProperties(ATestReplicationConditionsActor_Custom* Actor, bool bWrite,
																	 bool bCustomEnabled)
{
	auto WrappedAction = [&](int32& Source, int32 Expected, FString AdditionalText = TEXT("")) {
		bool CondIgnore[COND_Max]{};
		Action(Source, Expected, COND_Custom, bWrite, CondIgnore, AdditionalText);
	};

	WrappedAction(Actor->CondCustom_Var, bCustomEnabled ? 1010 : 2010);
	WrappedAction(Actor->StaticComponent->CondCustom_Var, bCustomEnabled ? 1020 : 2020, StaticCompText);
	WrappedAction(Actor->DynamicComponent->CondCustom_Var, bCustomEnabled ? 1030 : 2030, DynamicCompText);
}

void ASpatialTestReplicationConditions::ProcessAutonomousOnlyActorProperties(bool bWrite, bool bAutonomousExpected, bool bSimulatedExpected)
{
	auto WrappedAction = [&](int32& Source, bool bExpected, int32 Expected, ELifetimeCondition Cond, FString AdditionalText = TEXT("")) {
		bool CondIgnore[COND_Max]{};
		CondIgnore[Cond] = !bExpected;
		Action(Source, Expected, Cond, bWrite, CondIgnore, AdditionalText);
	};

	WrappedAction(TestActor_AutonomousOnly->CondAutonomousOnly_Var, bAutonomousExpected, 3010, COND_AutonomousOnly);
	WrappedAction(TestActor_AutonomousOnly->CondSimulatedOnly_Var, bSimulatedExpected, 3020, COND_SimulatedOnly);
	WrappedAction(TestActor_AutonomousOnly->StaticComponent->CondAutonomousOnly_Var, bAutonomousExpected, 3030, COND_AutonomousOnly,
				  StaticCompText);
	WrappedAction(TestActor_AutonomousOnly->StaticComponent->CondSimulatedOnly_Var, bSimulatedExpected, 3040, COND_SimulatedOnly,
				  StaticCompText);
	WrappedAction(TestActor_AutonomousOnly->DynamicComponent->CondAutonomousOnly_Var, bAutonomousExpected, 3050, COND_AutonomousOnly,
				  DynamicCompText);
	WrappedAction(TestActor_AutonomousOnly->DynamicComponent->CondSimulatedOnly_Var, bSimulatedExpected, 3060, COND_SimulatedOnly,
				  DynamicCompText);
}

void ASpatialTestReplicationConditions::ProcessPhysicsActorProperties(ATestReplicationConditionsActor_Physics* Actor, bool bWrite,
																	  bool bPhysicsEnabled, bool bPhysicsExpected)
{
	auto WrappedAction = [&](int32& Source, int32 Expected, ELifetimeCondition Cond, FString AdditionalText = TEXT("")) {
		bool CondIgnore[COND_Max]{};
		CondIgnore[Cond] = !bPhysicsExpected;
		Action(Source, Expected, Cond, bWrite, CondIgnore, AdditionalText);
	};

	WrappedAction(Actor->CondSimulatedOrPhysics_Var, (bPhysicsEnabled) ? 4010 : 5010, COND_SimulatedOrPhysics);
	WrappedAction(Actor->CondSimulatedOrPhysicsNoReplay_Var, (bPhysicsEnabled) ? 4020 : 5020, COND_SimulatedOrPhysicsNoReplay);
	WrappedAction(Actor->StaticComponent->CondSimulatedOrPhysics_Var, (bPhysicsEnabled) ? 4030 : 5030, COND_SimulatedOrPhysics,
				  StaticCompText);
	WrappedAction(Actor->StaticComponent->CondSimulatedOrPhysicsNoReplay_Var, (bPhysicsEnabled) ? 4040 : 5040,
				  COND_SimulatedOrPhysicsNoReplay, StaticCompText);
	WrappedAction(Actor->DynamicComponent->CondSimulatedOrPhysics_Var, (bPhysicsEnabled) ? 4050 : 5050, COND_SimulatedOrPhysics,
				  DynamicCompText);
	WrappedAction(Actor->DynamicComponent->CondSimulatedOrPhysicsNoReplay_Var, (bPhysicsEnabled) ? 4060 : 5060,
				  COND_SimulatedOrPhysicsNoReplay, DynamicCompText);
}
