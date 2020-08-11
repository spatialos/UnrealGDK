// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "EngineClasses/SpatialWorldSettings.h"
#include "LoadBalancing/GridBasedLBStrategy.h"
#include "LoadBalancing/LayeredLBStrategy.h"
#include "LoadBalancing/SpatialMultiWorkerSettings.h"
#include "TestLayeredLBStrategy.h"
#include "Utils/LayerInfo.h"

#include "Engine/Engine.h"
#include "GameFramework/DefaultPawn.h"
#include "GameFramework/GameStateBase.h"
#include "Misc/Optional.h"
#include "Tests/AutomationCommon.h"
#include "Tests/AutomationEditorCommon.h"
#include "Tests/TestDefinitions.h"

#define LAYEREDLBSTRATEGY_TEST(TestName) GDK_TEST(Core, ULayeredLBStrategy, TestName)

namespace
{
struct TestData
{
	ULayeredLBStrategy* Strat{ nullptr };
	UWorld* TestWorld{ nullptr };
	TMap<FName, AActor*> TestActors{};
};

// Copied from AutomationCommon::GetAnyGameWorld().
UWorld* GetAnyGameWorld()
{
	UWorld* World = nullptr;
	const TIndirectArray<FWorldContext>& WorldContexts = GEngine->GetWorldContexts();
	for (const FWorldContext& Context : WorldContexts)
	{
		if ((Context.WorldType == EWorldType::PIE || Context.WorldType == EWorldType::Game) && (Context.World() != nullptr))
		{
			World = Context.World();
			break;
		}
	}

	return World;
}

} // anonymous namespace

DEFINE_LATENT_AUTOMATION_COMMAND_ONE_PARAMETER(FWaitForWorld, TSharedPtr<TestData>, TestData);
bool FWaitForWorld::Update()
{
	auto& TestWorld = TestData->TestWorld;
	TestWorld = GetAnyGameWorld();

	if (TestWorld && TestWorld->AreActorsInitialized())
	{
		AGameStateBase* GameState = TestWorld->GetGameState();
		if (GameState && GameState->HasMatchStarted())
		{
			return true;
		}
	}

	return false;
}

DEFINE_LATENT_AUTOMATION_COMMAND_THREE_PARAMETER(FCreateStrategy, TSharedPtr<TestData>, TestData, UAbstractSpatialMultiWorkerSettings*,
												 MultiWorkerSettings, TOptional<uint32>, NumVirtualWorkers);
bool FCreateStrategy::Update()
{
	TestData->Strat = NewObject<ULayeredLBStrategy>(TestData->TestWorld);
	TestData->Strat->Init();
	TestData->Strat->SetLayers(MultiWorkerSettings->WorkerLayers);

	if (!NumVirtualWorkers.IsSet())
	{
		NumVirtualWorkers = TestData->Strat->GetMinimumRequiredWorkers();
	}

	TestData->Strat->SetVirtualWorkerIds(1, NumVirtualWorkers.GetValue());

	return true;
}

DEFINE_LATENT_AUTOMATION_COMMAND_TWO_PARAMETER(FSetLocalVirtualWorker, TSharedPtr<TestData>, TestData, VirtualWorkerId, WorkerId);
bool FSetLocalVirtualWorker::Update()
{
	TestData->Strat->SetLocalVirtualWorkerId(WorkerId);
	return true;
}

DEFINE_LATENT_AUTOMATION_COMMAND_FOUR_PARAMETER(FCheckWhoShouldHaveAuthority, TSharedPtr<TestData>, TestData, FAutomationTestBase*, Test,
												FName, ActorName, VirtualWorkerId, Expected);
bool FCheckWhoShouldHaveAuthority::Update()
{
	const VirtualWorkerId Actual = TestData->Strat->WhoShouldHaveAuthority(*TestData->TestActors[ActorName]);
	Test->TestEqual(FString::Printf(TEXT("Who Should Have Authority. Actual: %d, Expected: %d"), Actual, Expected), Actual, Expected);
	return true;
}

DEFINE_LATENT_AUTOMATION_COMMAND_THREE_PARAMETER(FCheckMinimumWorkers, TSharedPtr<TestData>, TestData, FAutomationTestBase*, Test, uint32,
												 Expected);
bool FCheckMinimumWorkers::Update()
{
	const uint32 Actual = TestData->Strat->GetMinimumRequiredWorkers();
	Test->TestEqual(FString::Printf(TEXT("Strategy for minimum required workers. Actual: %d, Expected: %d"), Actual, Expected), Actual,
					Expected);
	return true;
}

DEFINE_LATENT_AUTOMATION_COMMAND_THREE_PARAMETER(FCheckStratIsReady, TSharedPtr<TestData>, TestData, FAutomationTestBase*, Test, bool,
												 Expected);
bool FCheckStratIsReady::Update()
{
	const UAbstractLBStrategy* Strat = TestData->Strat;
	Test->TestEqual(FString::Printf(TEXT("Strategy is ready. Actual: %d, Expected: %d"), Strat->IsReady(), Expected), Strat->IsReady(),
					Expected);
	return true;
}

DEFINE_LATENT_AUTOMATION_COMMAND_THREE_PARAMETER(FSpawnLayer1PawnAtLocation, TSharedPtr<TestData>, TestData, FName, Handle, FVector,
												 Location);
bool FSpawnLayer1PawnAtLocation::Update()
{
	FActorSpawnParameters SpawnParams;
	SpawnParams.bNoFail = true;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	AActor* NewActor = TestData->TestWorld->SpawnActor<ALayer1Pawn>(Location, FRotator::ZeroRotator, SpawnParams);
	TestData->TestActors.Add(Handle, NewActor);

	return true;
}

DEFINE_LATENT_AUTOMATION_COMMAND_THREE_PARAMETER(FSpawnLayer2PawnAtLocation, TSharedPtr<TestData>, TestData, FName, Handle, FVector,
												 Location);
bool FSpawnLayer2PawnAtLocation::Update()
{
	FActorSpawnParameters SpawnParams;
	SpawnParams.bNoFail = true;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	AActor* NewActor = TestData->TestWorld->SpawnActor<ALayer2Pawn>(Location, FRotator::ZeroRotator, SpawnParams);
	TestData->TestActors.Add(Handle, NewActor);

	return true;
}

DEFINE_LATENT_AUTOMATION_COMMAND_FIVE_PARAMETER(FCheckActorsAuth, TSharedPtr<TestData>, TestData, FAutomationTestBase*, Test, FName,
												FirstActorName, FName, SecondActorName, bool, ExpectEqual);
bool FCheckActorsAuth::Update()
{
	const auto& Strat = TestData->Strat;
	const auto& TestActors = TestData->TestActors;

	const VirtualWorkerId FirstActorAuth = Strat->WhoShouldHaveAuthority(*TestActors[FirstActorName]);
	const VirtualWorkerId SecondActorAuth = Strat->WhoShouldHaveAuthority(*TestActors[SecondActorName]);

	if (ExpectEqual)
	{
		Test->TestEqual(FString::Printf(TEXT("Actors should have the same auth. Actor1: %d, Actor2: %d"), FirstActorAuth, SecondActorAuth),
						FirstActorAuth, SecondActorAuth);
	}
	else
	{
		Test->TestNotEqual(
			FString::Printf(TEXT("Actors should have different auth. Actor1: %d, Actor2: %d"), FirstActorAuth, SecondActorAuth),
			FirstActorAuth, SecondActorAuth);
	}

	return true;
}

DEFINE_LATENT_AUTOMATION_COMMAND_THREE_PARAMETER(FCheckRequiresHandover, TSharedPtr<TestData>, TestData, FAutomationTestBase*, Test, bool,
												 Expected);
bool FCheckRequiresHandover::Update()
{
	const bool Actual = TestData->Strat->RequiresHandoverData();
	Test->TestEqual(FString::Printf(TEXT("Strategy requires handover data. Expected: %c Actual: %c"), Expected, Actual), Expected, Actual);
	return true;
}

DEFINE_LATENT_AUTOMATION_COMMAND_THREE_PARAMETER(FCheckShouldHaveAuthMatchesWhoShouldHaveAuth, TSharedPtr<TestData>, TestData,
												 FAutomationTestBase*, Test, FName, ActorName);
bool FCheckShouldHaveAuthMatchesWhoShouldHaveAuth::Update()
{
	const auto Strat = TestData->Strat;
	const auto& TestActors = TestData->TestActors;

	const bool WeShouldHaveAuthority = Strat->WhoShouldHaveAuthority(*TestActors[ActorName]) == Strat->GetLocalVirtualWorkerId();
	const bool DoWeActuallyHaveAuthority = Strat->ShouldHaveAuthority(*TestActors[ActorName]);

	Test->TestEqual(FString::Printf(TEXT("WhoShouldHaveAuthority should match ShouldHaveAuthority. Expected: %b Actual: %b"),
									WeShouldHaveAuthority, DoWeActuallyHaveAuthority),
					WeShouldHaveAuthority, DoWeActuallyHaveAuthority);
	return true;
}

LAYEREDLBSTRATEGY_TEST(GIVEN_strat_is_not_ready_WHEN_local_virtual_worker_id_is_set_THEN_is_ready)
{
	AutomationOpenMap("/Engine/Maps/Entry");

	TSharedPtr<TestData> Data = TSharedPtr<TestData>(new TestData);

	USpatialMultiWorkerSettings* MultiWorkerSettings = NewObject<USpatialMultiWorkerSettings>();

	ADD_LATENT_AUTOMATION_COMMAND(FWaitForWorld(Data));
	ADD_LATENT_AUTOMATION_COMMAND(FCreateStrategy(Data, MultiWorkerSettings, {}));
	ADD_LATENT_AUTOMATION_COMMAND(FCheckStratIsReady(Data, this, false));
	ADD_LATENT_AUTOMATION_COMMAND(FSetLocalVirtualWorker(Data, 1));
	ADD_LATENT_AUTOMATION_COMMAND(FCheckStratIsReady(Data, this, true));

	return true;
}

LAYEREDLBSTRATEGY_TEST(
	GIVEN_layered_strat_of_two_by_four_grid_strat_singleton_strat_and_default_strat_WHEN_get_minimum_required_workers_called_THEN_ten_returned)
{
	AutomationOpenMap("/Engine/Maps/Entry");

	TSharedPtr<TestData> Data = TSharedPtr<TestData>(new TestData);

	USpatialMultiWorkerSettings* MultiWorkerSettings = NewObject<USpatialMultiWorkerSettings>();
	MultiWorkerSettings->WorkerLayers.Add(FLayerInfo{ TEXT("LayerOne"), {}, UTwoByFourLBGridStrategy::StaticClass() });
	MultiWorkerSettings->WorkerLayers.Add(FLayerInfo{ TEXT("LayerTwo"), {}, USingleWorkerStrategy::StaticClass() });

	ADD_LATENT_AUTOMATION_COMMAND(FWaitForWorld(Data));
	ADD_LATENT_AUTOMATION_COMMAND(FCreateStrategy(Data, MultiWorkerSettings, {}));
	ADD_LATENT_AUTOMATION_COMMAND(FCheckMinimumWorkers(Data, this, 10));

	return true;
}

LAYEREDLBSTRATEGY_TEST(
	Given_layered_strat_of_2_single_cell_strats_and_default_strat_WHEN_set_virtual_worker_ids_called_with_2_ids_THEN_error_is_logged)
{
	AutomationOpenMap("/Engine/Maps/Entry");

	TSharedPtr<TestData> Data = TSharedPtr<TestData>(new TestData);

	USpatialMultiWorkerSettings* MultiWorkerSettings = NewObject<USpatialMultiWorkerSettings>();
	MultiWorkerSettings->WorkerLayers.Add(
		FLayerInfo{ TEXT("LayerOne"), { ALayer1Pawn::StaticClass() }, USingleWorkerStrategy::StaticClass() });
	MultiWorkerSettings->WorkerLayers.Add(
		FLayerInfo{ TEXT("LayerTwo"), { ALayer2Pawn::StaticClass() }, USingleWorkerStrategy::StaticClass() });

	this->AddExpectedError("LayeredLBStrategy was not given enough VirtualWorkerIds to meet the demands of the layer strategies.",
						   EAutomationExpectedErrorFlags::MatchType::Contains, 1);

	// The two single strategies plus the default strategy require 3 virtual workers, but we only explicitly provide 2.
	ADD_LATENT_AUTOMATION_COMMAND(FWaitForWorld(Data));
	ADD_LATENT_AUTOMATION_COMMAND(FCreateStrategy(Data, MultiWorkerSettings, 2));

	return true;
}

LAYEREDLBSTRATEGY_TEST(
	Given_layered_strat_of_2_single_cell_grid_strats_and_default_strat_WHEN_set_virtual_worker_ids_called_with_3_ids_THEN_no_error_is_logged)
{
	AutomationOpenMap("/Engine/Maps/Entry");

	TSharedPtr<TestData> Data = TSharedPtr<TestData>(new TestData);

	USpatialMultiWorkerSettings* MultiWorkerSettings = NewObject<USpatialMultiWorkerSettings>();
	MultiWorkerSettings->WorkerLayers.Add(
		FLayerInfo{ TEXT("LayerOne"), { ALayer1Pawn::StaticClass() }, USingleWorkerStrategy::StaticClass() });
	MultiWorkerSettings->WorkerLayers.Add(
		FLayerInfo{ TEXT("LayerTwo"), { ALayer2Pawn::StaticClass() }, USingleWorkerStrategy::StaticClass() });

	// The two single strategies plus the default strategy require 3 virtual workers.
	ADD_LATENT_AUTOMATION_COMMAND(FWaitForWorld(Data));
	ADD_LATENT_AUTOMATION_COMMAND(FCreateStrategy(Data, MultiWorkerSettings, 3));

	return true;
}

LAYEREDLBSTRATEGY_TEST(Given_layered_strat_of_default_strat_WHEN_requires_handover_called_THEN_returns_false)
{
	AutomationOpenMap("/Engine/Maps/Entry");

	TSharedPtr<TestData> Data = TSharedPtr<TestData>(new TestData);

	USpatialMultiWorkerSettings* MultiWorkerSettings = NewObject<USpatialMultiWorkerSettings>();

	ADD_LATENT_AUTOMATION_COMMAND(FWaitForWorld(Data));
	ADD_LATENT_AUTOMATION_COMMAND(FCreateStrategy(Data, MultiWorkerSettings, {}));
	ADD_LATENT_AUTOMATION_COMMAND(FSetLocalVirtualWorker(Data, 1));
	ADD_LATENT_AUTOMATION_COMMAND(FCheckRequiresHandover(Data, this, false));

	return true;
}

LAYEREDLBSTRATEGY_TEST(Given_layered_strat_of_single_cell_grid_strat_and_default_strat_WHEN_requires_handover_called_THEN_returns_true)
{
	AutomationOpenMap("/Engine/Maps/Entry");

	TSharedPtr<TestData> Data = TSharedPtr<TestData>(new TestData);

	USpatialMultiWorkerSettings* MultiWorkerSettings = NewObject<USpatialMultiWorkerSettings>();
	MultiWorkerSettings->WorkerLayers.Add(
		FLayerInfo{ TEXT("LayerOne"), { ADefaultPawn::StaticClass() }, USingleWorkerStrategy::StaticClass() });

	ADD_LATENT_AUTOMATION_COMMAND(FWaitForWorld(Data));
	ADD_LATENT_AUTOMATION_COMMAND(FCreateStrategy(Data, MultiWorkerSettings, {}));
	ADD_LATENT_AUTOMATION_COMMAND(FSetLocalVirtualWorker(Data, 1));
	ADD_LATENT_AUTOMATION_COMMAND(FCheckRequiresHandover(Data, this, true));

	return true;
}

LAYEREDLBSTRATEGY_TEST(Given_layered_strat_of_default_strat_WHEN_who_should_have_auth_called_THEN_return_1)
{
	AutomationOpenMap("/Engine/Maps/Entry");

	TSharedPtr<TestData> Data = TSharedPtr<TestData>(new TestData);

	USpatialMultiWorkerSettings* MultiWorkerSettings = NewObject<USpatialMultiWorkerSettings>();

	ADD_LATENT_AUTOMATION_COMMAND(FWaitForWorld(Data));
	ADD_LATENT_AUTOMATION_COMMAND(FCreateStrategy(Data, MultiWorkerSettings, {}));
	ADD_LATENT_AUTOMATION_COMMAND(FSetLocalVirtualWorker(Data, 1));
	ADD_LATENT_AUTOMATION_COMMAND(FSpawnLayer1PawnAtLocation(Data, TEXT("DefaultLayerActor"), FVector::ZeroVector));
	ADD_LATENT_AUTOMATION_COMMAND(FCheckWhoShouldHaveAuthority(Data, this, "DefaultLayerActor", 1));

	return true;
}

LAYEREDLBSTRATEGY_TEST(Given_layered_strat_WHEN_set_local_worker_called_twice_THEN_an_error_is_logged)
{
	AutomationOpenMap("/Engine/Maps/Entry");

	TSharedPtr<TestData> Data = TSharedPtr<TestData>(new TestData);

	USpatialMultiWorkerSettings* MultiWorkerSettings = NewObject<USpatialMultiWorkerSettings>();

	ADD_LATENT_AUTOMATION_COMMAND(FWaitForWorld(Data));
	ADD_LATENT_AUTOMATION_COMMAND(FCreateStrategy(Data, MultiWorkerSettings, {}));
	ADD_LATENT_AUTOMATION_COMMAND(FSetLocalVirtualWorker(Data, 1));
	ADD_LATENT_AUTOMATION_COMMAND(FSetLocalVirtualWorker(Data, 2));

	this->AddExpectedError(
		"The Local Virtual Worker Id cannot be set twice. Current value:", EAutomationExpectedErrorFlags::MatchType::Contains, 1);

	return true;
}

LAYEREDLBSTRATEGY_TEST(Given_two_actors_of_same_type_at_same_position_WHEN_who_should_have_auth_called_THEN_return_same_for_both)
{
	AutomationOpenMap("/Engine/Maps/Entry");

	TSharedPtr<TestData> Data = TSharedPtr<TestData>(new TestData);

	USpatialMultiWorkerSettings* MultiWorkerSettings = NewObject<USpatialMultiWorkerSettings>();
	MultiWorkerSettings->WorkerLayers.Add(
		FLayerInfo{ TEXT("LayerOne"), { ALayer1Pawn::StaticClass() }, UTwoByFourLBGridStrategy::StaticClass() });
	MultiWorkerSettings->WorkerLayers.Add(
		FLayerInfo{ TEXT("LayerTwo"), { ALayer2Pawn::StaticClass() }, UTwoByFourLBGridStrategy::StaticClass() });

	ADD_LATENT_AUTOMATION_COMMAND(FWaitForWorld(Data));
	ADD_LATENT_AUTOMATION_COMMAND(FCreateStrategy(Data, MultiWorkerSettings, {}));
	ADD_LATENT_AUTOMATION_COMMAND(FSetLocalVirtualWorker(Data, 1));
	ADD_LATENT_AUTOMATION_COMMAND(FSpawnLayer1PawnAtLocation(Data, TEXT("Layer1Actor1"), FVector::ZeroVector));
	ADD_LATENT_AUTOMATION_COMMAND(FSpawnLayer1PawnAtLocation(Data, TEXT("Layer1Actor2"), FVector::ZeroVector));
	ADD_LATENT_AUTOMATION_COMMAND(FSpawnLayer2PawnAtLocation(Data, TEXT("Layer2Actor1"), FVector::ZeroVector));
	ADD_LATENT_AUTOMATION_COMMAND(FSpawnLayer2PawnAtLocation(Data, TEXT("Later2Actor2"), FVector::ZeroVector));

	ADD_LATENT_AUTOMATION_COMMAND(FCheckActorsAuth(Data, this, TEXT("Layer1Actor1"), TEXT("Layer1Actor2"), true));
	ADD_LATENT_AUTOMATION_COMMAND(FCheckActorsAuth(Data, this, TEXT("Layer2Actor1"), TEXT("Later2Actor2"), true));

	return true;
}

LAYEREDLBSTRATEGY_TEST(
	GIVEN_two_actors_of_different_types_and_same_positions_managed_by_different_layers_WHEN_who_has_auth_called_THEN_return_different_values)
{
	AutomationOpenMap("/Engine/Maps/Entry");

	TSharedPtr<TestData> Data = TSharedPtr<TestData>(new TestData);

	USpatialMultiWorkerSettings* MultiWorkerSettings = NewObject<USpatialMultiWorkerSettings>();
	MultiWorkerSettings->WorkerLayers.Add(
		FLayerInfo{ TEXT("LayerOne"), { ALayer1Pawn::StaticClass() }, UTwoByFourLBGridStrategy::StaticClass() });
	MultiWorkerSettings->WorkerLayers.Add(
		FLayerInfo{ TEXT("LayerTwo"), { ALayer2Pawn::StaticClass() }, UTwoByFourLBGridStrategy::StaticClass() });

	ADD_LATENT_AUTOMATION_COMMAND(FWaitForWorld(Data));
	ADD_LATENT_AUTOMATION_COMMAND(FCreateStrategy(Data, MultiWorkerSettings, {}));
	ADD_LATENT_AUTOMATION_COMMAND(FSetLocalVirtualWorker(Data, 1));

	ADD_LATENT_AUTOMATION_COMMAND(FSpawnLayer1PawnAtLocation(Data, TEXT("Layer1Actor"), FVector::ZeroVector));
	ADD_LATENT_AUTOMATION_COMMAND(FSpawnLayer2PawnAtLocation(Data, TEXT("Layer2Actor"), FVector::ZeroVector));

	ADD_LATENT_AUTOMATION_COMMAND(FCheckActorsAuth(Data, this, TEXT("Layer1Actor"), TEXT("Layer2Actor"), false));

	return true;
}
