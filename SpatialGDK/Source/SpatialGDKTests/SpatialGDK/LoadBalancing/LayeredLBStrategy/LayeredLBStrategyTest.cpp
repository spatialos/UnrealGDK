// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "EngineClasses/SpatialWorldSettings.h"
#include "LoadBalancing/GridBasedLBStrategy.h"
#include "LoadBalancing/LayeredLBStrategy.h"
#include "SpatialGDKTests/SpatialGDK/LoadBalancing/GridBasedLBStrategy/TestGridBasedLBStrategy.h"
#include "TestLayeredLBStrategy.h"

#include "Engine/Engine.h"
#include "GameFramework/DefaultPawn.h"
#include "GameFramework/GameStateBase.h"
#include "Misc/Optional.h"
#include "Tests/AutomationCommon.h"
#include "Tests/AutomationEditorCommon.h"
#include "Tests/TestDefinitions.h"

#define LAYEREDLBSTRATEGY_TEST(TestName) \
	GDK_TEST(Core, ULayeredLBStrategy, TestName)

namespace
{
ULayeredLBStrategy* Strat;
UWorld* TestWorld;
TMap<FName, AActor*> TestActors;

// Copied from AutomationCommon::GetAnyGameWorld().
UWorld* GetAnyGameWorld()
{
	UWorld* World = nullptr;
	const TIndirectArray<FWorldContext>& WorldContexts = GEngine->GetWorldContexts();
	for (const FWorldContext& Context : WorldContexts)
	{
		if ((Context.WorldType == EWorldType::PIE || Context.WorldType == EWorldType::Game)
			&& (Context.World() != nullptr))
		{
			World = Context.World();
			break;
		}
	}

	return World;
}

DEFINE_LATENT_AUTOMATION_COMMAND(FWaitForWorld);
bool FWaitForWorld::Update()
{
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

struct StrategyConfig {
	TSubclassOf<UAbstractLBStrategy> StrategyClass;
	TSet<TSoftClassPtr<AActor>> TargetActorTypes;
};

DEFINE_LATENT_AUTOMATION_COMMAND(FCreateStrategy);
bool FCreateStrategy::Update()
{
	Strat = NewObject<ULayeredLBStrategy>(TestWorld);
	return true;
}

DEFINE_LATENT_AUTOMATION_COMMAND_ONE_PARAMETER(FSetDefaultLayer, TSubclassOf<UAbstractLBStrategy>, DefaultLayer);
bool FSetDefaultLayer::Update()
{
	ASpatialWorldSettings* WorldSettings = Cast<ASpatialWorldSettings>(TestWorld->GetWorldSettings());
	WorldSettings->DefaultLayerLoadBalanceStrategy = DefaultLayer;
	return true;
}

DEFINE_LATENT_AUTOMATION_COMMAND_ONE_PARAMETER(FAddLayer, StrategyConfig, Strategy);
bool FAddLayer::Update()
{
	ASpatialWorldSettings* WorldSettings = Cast<ASpatialWorldSettings>(TestWorld->GetWorldSettings());
	auto StratName = FName{ *FString::FromInt((WorldSettings->WorkerLayers.Num())) };
	FLayerInfo LayerInfo;
	LayerInfo.Name = StratName;
	LayerInfo.LoadBalanceStrategy = Strategy.StrategyClass;
	for (const auto& TargetActors : Strategy.TargetActorTypes) {
		LayerInfo.ActorClasses.Add(TargetActors);
	}
	WorldSettings->WorkerLayers.Add(StratName, LayerInfo);

	return true;
}

DEFINE_LATENT_AUTOMATION_COMMAND_ONE_PARAMETER(FSetupStrategy, TOptional<uint32>, NumVirtualWorkers);
bool FSetupStrategy::Update()
{
	ASpatialWorldSettings* WorldSettings = Cast<ASpatialWorldSettings>(TestWorld->GetWorldSettings());
	WorldSettings->DefaultLayerLoadBalanceStrategy = UGridBasedLBStrategy::StaticClass();
	WorldSettings->bEnableMultiWorker = true;

	Strat->Init();

	if (!NumVirtualWorkers.IsSet()) {
		NumVirtualWorkers = Strat->GetMinimumRequiredWorkers();
	}

	Strat->SetVirtualWorkerIds(1, NumVirtualWorkers.GetValue());

	return true;
}

DEFINE_LATENT_AUTOMATION_COMMAND_ONE_PARAMETER(FSetupStrategyLocalWorker, VirtualWorkerId, worker_id);
bool FSetupStrategyLocalWorker::Update()
{
	Strat->SetLocalVirtualWorkerId(worker_id);
	return true;
}

DEFINE_LATENT_AUTOMATION_COMMAND_THREE_PARAMETER(FCheckWhoShouldHaveAuthority, FAutomationTestBase*, Test, FName, ActorName, VirtualWorkerId, Expected);
bool FCheckWhoShouldHaveAuthority::Update()
{
	const VirtualWorkerId Actual = Strat->WhoShouldHaveAuthority(*TestActors[ActorName]);
	Test->TestEqual(
		FString::Printf(TEXT("Who Should Have Authority. Actual: %d, Expected: %d"), Actual, Expected),
		Actual, Expected);
	return true;
}

DEFINE_LATENT_AUTOMATION_COMMAND(FCleanup);
bool FCleanup::Update()
{
	ASpatialWorldSettings* WorldSettings = Cast<ASpatialWorldSettings>(TestWorld->GetWorldSettings());
	WorldSettings->WorkerLayers.Empty();
	WorldSettings->bEnableMultiWorker = false;
	TestWorld = nullptr;
	TestActors.Empty();
	Strat = nullptr;


	return true;
}

DEFINE_LATENT_AUTOMATION_COMMAND_TWO_PARAMETER(FCheckMinimumWorkers, FAutomationTestBase*, Test, uint32, Expected);
bool FCheckMinimumWorkers::Update()
{
	const uint32 Actual = Strat->GetMinimumRequiredWorkers();
	Test->TestEqual(
		FString::Printf(TEXT("Checking strategy for minimum required workers. Actual: %d, Expected: %d"), Actual, Expected),
		Actual, Expected);
	return true;
}

DEFINE_LATENT_AUTOMATION_COMMAND_TWO_PARAMETER(FCheckStratIsReady, FAutomationTestBase*, Test, bool, Expected);
bool FCheckStratIsReady::Update()
{
	Test->TestEqual(
		FString::Printf(TEXT("Checking strategy for minimum required workers. Actual: %d, Expected: %d"), Strat->IsReady(), Expected),
		Strat->IsReady(), Expected);
	return true;
}

DEFINE_LATENT_AUTOMATION_COMMAND_TWO_PARAMETER(FSpawnLayer1PawnAtLocation, FName, Handle,
	FVector, Location);
bool FSpawnLayer1PawnAtLocation::Update()
{
	FActorSpawnParameters SpawnParams;
	SpawnParams.bNoFail = true;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	AActor* NewActor = TestWorld->SpawnActor<ALayer1Pawn>(Location, FRotator::ZeroRotator, SpawnParams);
	TestActors.Add(Handle, NewActor);

	return true;
}

DEFINE_LATENT_AUTOMATION_COMMAND_TWO_PARAMETER(FSpawnLayer2PawnAtLocation,
	FName, Handle, FVector, Location);
bool FSpawnLayer2PawnAtLocation::Update()
{
	FActorSpawnParameters SpawnParams;
	SpawnParams.bNoFail = true;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	AActor* NewActor = TestWorld->SpawnActor<ALayer2Pawn>(Location, FRotator::ZeroRotator, SpawnParams);
	TestActors.Add(Handle, NewActor);

	return true;
}

DEFINE_LATENT_AUTOMATION_COMMAND_FOUR_PARAMETER(FCheckActorsAuth, FAutomationTestBase*, Test, FName, FirstActorName, FName, SecondActorName, bool, ExpectEqual);
bool FCheckActorsAuth::Update()
{
	const VirtualWorkerId FirstActorAuth = Strat->WhoShouldHaveAuthority(*TestActors[FirstActorName]);
	const VirtualWorkerId SecondActorAuth = Strat->WhoShouldHaveAuthority(*TestActors[SecondActorName]);

	if (ExpectEqual)
	{
		Test->TestEqual(
			FString::Printf(TEXT("Checking actors have the same auth. Actor1: %d, Actor2: %d"), FirstActorAuth, SecondActorAuth),
			FirstActorAuth, SecondActorAuth);
	}
	else {
		Test->TestNotEqual(
			FString::Printf(TEXT("Checking actors have different auth. Actor1: %d, Actor2: %d"), FirstActorAuth, SecondActorAuth),
			FirstActorAuth, SecondActorAuth);
	}

	return true;
}

DEFINE_LATENT_AUTOMATION_COMMAND_TWO_PARAMETER(FCheckRequiresHandover, FAutomationTestBase*, Test, bool, Expected);
bool FCheckRequiresHandover::Update()
{
	const bool Actual = Strat->RequiresHandoverData();
	Test->TestEqual(
		FString::Printf(TEXT("Checking strategy requires handover data Expected: %b Actual: %b"), Expected, Actual),
		Expected, Actual);
	return true;
}

DEFINE_LATENT_AUTOMATION_COMMAND_TWO_PARAMETER(FCheckShouldHaveAuthMatchesWhoShouldHaveAuth, FAutomationTestBase*, Test, FName, ActorName);
bool FCheckShouldHaveAuthMatchesWhoShouldHaveAuth::Update()
{
	const bool WeShouldHaveAuthority
		= Strat->WhoShouldHaveAuthority(*TestActors[ActorName]) == Strat->GetLocalVirtualWorkerId();

	Test->TestEqual(
		FString::Printf(TEXT("Expected: %b Actual: %b"), WeShouldHaveAuthority, Strat->ShouldHaveAuthority(*TestActors[ActorName])),
		WeShouldHaveAuthority, Strat->ShouldHaveAuthority(*TestActors[ActorName]));
	return true;
}

LAYEREDLBSTRATEGY_TEST(GIVEN_strat_is_not_ready_WHEN_local_virtual_worker_id_is_set_THEN_is_ready)
{
	AutomationOpenMap("/Engine/Maps/Entry");
	ADD_LATENT_AUTOMATION_COMMAND(FWaitForWorld());
	ADD_LATENT_AUTOMATION_COMMAND(FCreateStrategy);
	ADD_LATENT_AUTOMATION_COMMAND(FSetDefaultLayer(UGridBasedLBStrategy::StaticClass()));
	ADD_LATENT_AUTOMATION_COMMAND(FSetupStrategy({}));
	ADD_LATENT_AUTOMATION_COMMAND(FCheckStratIsReady(this, false));
	ADD_LATENT_AUTOMATION_COMMAND(FSetupStrategyLocalWorker(1));
	ADD_LATENT_AUTOMATION_COMMAND(FCheckStratIsReady(this, true));
	ADD_LATENT_AUTOMATION_COMMAND(FCleanup());

	return true;
}

LAYEREDLBSTRATEGY_TEST(GIVEN_layered_strat_of_two_by_four_grid_strat_singleton_strat_and_default_strat_WHEN_get_minimum_required_workers_called_THEN_ten_returned)
{
	AutomationOpenMap("/Engine/Maps/Entry");
	ADD_LATENT_AUTOMATION_COMMAND(FWaitForWorld());

	ADD_LATENT_AUTOMATION_COMMAND(FCreateStrategy);
	ADD_LATENT_AUTOMATION_COMMAND(FSetDefaultLayer(UGridBasedLBStrategy::StaticClass()));
	ADD_LATENT_AUTOMATION_COMMAND(FAddLayer({ UTwoByFourLBGridStrategy::StaticClass(), {} }));
	ADD_LATENT_AUTOMATION_COMMAND(FAddLayer({ UGridBasedLBStrategy::StaticClass(), {} }));
	ADD_LATENT_AUTOMATION_COMMAND(FSetupStrategy({}));
	ADD_LATENT_AUTOMATION_COMMAND(FCheckMinimumWorkers(this, 10));
	ADD_LATENT_AUTOMATION_COMMAND(FCleanup());

	return true;
}

LAYEREDLBSTRATEGY_TEST(GIVEN_two_actors_of_different_types_and_same_positions_managed_by_different_layers_WHEN_who_has_auth_called_THEN_return_different_values)
{
	AutomationOpenMap("/Engine/Maps/Entry");
	ADD_LATENT_AUTOMATION_COMMAND(FWaitForWorld());

	ADD_LATENT_AUTOMATION_COMMAND(FCreateStrategy);
	ADD_LATENT_AUTOMATION_COMMAND(FSetDefaultLayer(UGridBasedLBStrategy::StaticClass()));
	ADD_LATENT_AUTOMATION_COMMAND(FAddLayer({ UTwoByFourLBGridStrategy::StaticClass(), {ALayer1Pawn::StaticClass()} }));
	ADD_LATENT_AUTOMATION_COMMAND(FAddLayer({ UTwoByFourLBGridStrategy::StaticClass(), {ALayer2Pawn::StaticClass()} }));

	ADD_LATENT_AUTOMATION_COMMAND(FSetupStrategy({}));
	ADD_LATENT_AUTOMATION_COMMAND(FSetupStrategyLocalWorker(1));

	ADD_LATENT_AUTOMATION_COMMAND(FSpawnLayer1PawnAtLocation(TEXT("Layer1Actor"), FVector(0, 0, 0)));
	ADD_LATENT_AUTOMATION_COMMAND(FSpawnLayer2PawnAtLocation(TEXT("Layer2Actor"), FVector(0, 0, 0)));

	ADD_LATENT_AUTOMATION_COMMAND(FCheckActorsAuth(this, TEXT("Layer1Actor"), TEXT("Layer2Actor"), false));

	ADD_LATENT_AUTOMATION_COMMAND(FCleanup());
	return true;
}

LAYEREDLBSTRATEGY_TEST(Given_layered_strat_of_2_single_cell_strats_and_default_strat_WHEN_set_virtual_worker_ids_called_with_2_ids_THEN_error_is_logged)
{
	AutomationOpenMap("/Engine/Maps/Entry");
	ADD_LATENT_AUTOMATION_COMMAND(FWaitForWorld());
	ADD_LATENT_AUTOMATION_COMMAND(FCreateStrategy);
	ADD_LATENT_AUTOMATION_COMMAND(FSetDefaultLayer(UGridBasedLBStrategy::StaticClass()));
	ADD_LATENT_AUTOMATION_COMMAND(FAddLayer({ UGridBasedLBStrategy::StaticClass(), {ALayer1Pawn::StaticClass()} }));
	ADD_LATENT_AUTOMATION_COMMAND(FAddLayer({ UGridBasedLBStrategy::StaticClass(), {ALayer2Pawn::StaticClass()} }));
	this->AddExpectedError("LayeredLBStrategy was not given enough VirtualWorkerIds to meet the demands of the layer strategies.",
		EAutomationExpectedErrorFlags::MatchType::Contains, 1);

	// The two single strategies plus the default strat require 3 vitual workers, but we only have 2.
	ADD_LATENT_AUTOMATION_COMMAND(FSetupStrategy(2));
	ADD_LATENT_AUTOMATION_COMMAND(FCleanup());

	return true;
}

LAYEREDLBSTRATEGY_TEST(Given_layered_strat_of_2_single_cell_grid_strats_and_default_strat_WHEN_set_virtual_worker_ids_called_with_3_ids_THEN_no_error_is_logged)
{
	AutomationOpenMap("/Engine/Maps/Entry");
	ADD_LATENT_AUTOMATION_COMMAND(FWaitForWorld());
	ADD_LATENT_AUTOMATION_COMMAND(FCreateStrategy);
	ADD_LATENT_AUTOMATION_COMMAND(FSetDefaultLayer(UGridBasedLBStrategy::StaticClass()));
	ADD_LATENT_AUTOMATION_COMMAND(FAddLayer({ UGridBasedLBStrategy::StaticClass(), {ALayer1Pawn::StaticClass()} }));
	ADD_LATENT_AUTOMATION_COMMAND(FAddLayer({ UGridBasedLBStrategy::StaticClass(), {ALayer2Pawn::StaticClass()} }));

	// The two single strategies plus the default strat require 3 vitual workers.
	ADD_LATENT_AUTOMATION_COMMAND(FSetupStrategy(3));
	ADD_LATENT_AUTOMATION_COMMAND(FCleanup());

	return true;
}

LAYEREDLBSTRATEGY_TEST(Given_layered_strat_of_default_strat_WHEN_requires_handover_called_THEN_returns_false)
{
	AutomationOpenMap("/Engine/Maps/Entry");
	ADD_LATENT_AUTOMATION_COMMAND(FWaitForWorld());
	ADD_LATENT_AUTOMATION_COMMAND(FCreateStrategy);
	ADD_LATENT_AUTOMATION_COMMAND(FSetDefaultLayer(UGridBasedLBStrategy::StaticClass()));
	ADD_LATENT_AUTOMATION_COMMAND(FSetupStrategy({}));
	ADD_LATENT_AUTOMATION_COMMAND(FSetupStrategyLocalWorker(1));
	ADD_LATENT_AUTOMATION_COMMAND(FCheckRequiresHandover(this, false));
	ADD_LATENT_AUTOMATION_COMMAND(FCleanup());

	return true;
}

LAYEREDLBSTRATEGY_TEST(Given_layered_strat_of_single_cell_grid_strat_and_default_strat_WHEN_requires_handover_called_THEN_returns_true)
{
	AutomationOpenMap("/Engine/Maps/Entry");
	ADD_LATENT_AUTOMATION_COMMAND(FWaitForWorld());
	ADD_LATENT_AUTOMATION_COMMAND(FCreateStrategy);
	ADD_LATENT_AUTOMATION_COMMAND(FSetDefaultLayer(UGridBasedLBStrategy::StaticClass()));
	ADD_LATENT_AUTOMATION_COMMAND(FAddLayer({ UGridBasedLBStrategy::StaticClass(), {ADefaultPawn::StaticClass()} }));
	ADD_LATENT_AUTOMATION_COMMAND(FSetupStrategy({}));
	ADD_LATENT_AUTOMATION_COMMAND(FSetupStrategyLocalWorker(1));
	ADD_LATENT_AUTOMATION_COMMAND(FCheckRequiresHandover(this, true));
	ADD_LATENT_AUTOMATION_COMMAND(FCleanup());

	return true;
}

LAYEREDLBSTRATEGY_TEST(Given_layered_strat_of_default_strat_WHEN_who_should_have_auth_called_THEN_return_1)
{
	AutomationOpenMap("/Engine/Maps/Entry");
	ADD_LATENT_AUTOMATION_COMMAND(FWaitForWorld());
	ADD_LATENT_AUTOMATION_COMMAND(FCreateStrategy);
	ADD_LATENT_AUTOMATION_COMMAND(FSetupStrategy({}));
	ADD_LATENT_AUTOMATION_COMMAND(FSetupStrategyLocalWorker(1));
	ADD_LATENT_AUTOMATION_COMMAND(FSpawnLayer1PawnAtLocation(TEXT("DefaultLayerActor"), FVector(0, 0, 0)));
	ADD_LATENT_AUTOMATION_COMMAND(FCheckWhoShouldHaveAuthority(this, "DefaultLayerActor", 1));
	ADD_LATENT_AUTOMATION_COMMAND(FCleanup());

	return true;
}

LAYEREDLBSTRATEGY_TEST(Given_two_actors_of_same_type_at_same_position_WHEN_who_should_have_auth_called_THEN_return_same_for_both)
{
	AutomationOpenMap("/Engine/Maps/Entry");
	ADD_LATENT_AUTOMATION_COMMAND(FWaitForWorld());
	ADD_LATENT_AUTOMATION_COMMAND(FCreateStrategy);
	ADD_LATENT_AUTOMATION_COMMAND(FSetDefaultLayer(UGridBasedLBStrategy::StaticClass()));
	ADD_LATENT_AUTOMATION_COMMAND(FAddLayer({ UTwoByFourLBGridStrategy::StaticClass(), {ALayer1Pawn::StaticClass()} }));
	ADD_LATENT_AUTOMATION_COMMAND(FAddLayer({ UTwoByFourLBGridStrategy::StaticClass(), {ALayer2Pawn::StaticClass()} }));
	ADD_LATENT_AUTOMATION_COMMAND(FSetupStrategy({}));
	ADD_LATENT_AUTOMATION_COMMAND(FSetupStrategyLocalWorker(1));
	ADD_LATENT_AUTOMATION_COMMAND(FSpawnLayer1PawnAtLocation(TEXT("Layer1Actor1"), FVector(0, 0, 0)));
	ADD_LATENT_AUTOMATION_COMMAND(FSpawnLayer1PawnAtLocation(TEXT("Layer1Actor2"), FVector(0, 0, 0)));
	ADD_LATENT_AUTOMATION_COMMAND(FSpawnLayer2PawnAtLocation(TEXT("Layer2Actor1"), FVector(0, 0, 0)));
	ADD_LATENT_AUTOMATION_COMMAND(FSpawnLayer2PawnAtLocation(TEXT("Later2Actor2"), FVector(0, 0, 0)));

	ADD_LATENT_AUTOMATION_COMMAND(FCheckActorsAuth(this, TEXT("Layer1Actor1"), TEXT("Layer1Actor2"), true));
	ADD_LATENT_AUTOMATION_COMMAND(FCheckActorsAuth(this, TEXT("Layer2Actor1"), TEXT("Later2Actor2"), true));
	ADD_LATENT_AUTOMATION_COMMAND(FCleanup());

	return true;
}

LAYEREDLBSTRATEGY_TEST(Given_layered_strat_WHEN_set_local_worker_called_twice_THEN_an_error_is_logged)
{
	AutomationOpenMap("/Engine/Maps/Entry");
	ADD_LATENT_AUTOMATION_COMMAND(FWaitForWorld());
	ADD_LATENT_AUTOMATION_COMMAND(FCreateStrategy);
	ADD_LATENT_AUTOMATION_COMMAND(FSetDefaultLayer(UGridBasedLBStrategy::StaticClass()));
	ADD_LATENT_AUTOMATION_COMMAND(FSetupStrategy({}));
	ADD_LATENT_AUTOMATION_COMMAND(FSetupStrategyLocalWorker(1));
	ADD_LATENT_AUTOMATION_COMMAND(FSetupStrategyLocalWorker(2));

	this->AddExpectedError("The Local Virtual Worker Id cannot be set twice. Current value:",
		EAutomationExpectedErrorFlags::MatchType::Contains, 1);

	ADD_LATENT_AUTOMATION_COMMAND(FCleanup());

	return true;
}
}

