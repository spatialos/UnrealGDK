#include "LoadBalancing/LayeredLBStrategy.h"
#include "Tests/AutomationCommon.h"
#include "Tests/AutomationEditorCommon.h"
#include "Tests/TestDefinitions.h"
#include "Engine/Engine.h"
#include "GameFramework/GameStateBase.h"
#include "EngineClasses/SpatialWorldSettings.h"
#include "LoadBalancing/GridBasedLBStrategy.h"
#include <vector>
#include <utility>
#include "SpatialGDKTests/SpatialGDK/LoadBalancing/GridBasedLBStrategy/TestGridBasedLBStrategy.h"
#include <string>
#include "TestLayeredLBStrategy.h"
#include "GameFramework/DefaultPawn.h"
#include "Misc/Optional.h"


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

	DEFINE_LATENT_AUTOMATION_COMMAND_TWO_PARAMETER(FSetupStrategy,
		std::vector<StrategyConfig>, ComponentStrategyConfigs, TOptional<uint32>, NumVirtualWorkers);
	bool FSetupStrategy::Update()
	{
		ASpatialWorldSettings* WorldSettings = Cast<ASpatialWorldSettings>(TestWorld->GetWorldSettings());
		WorldSettings->DefaultLayerLoadBalanceStrategy = UGridBasedLBStrategy::StaticClass();
		WorldSettings->bEnableMultiWorker = true;

		Strat = NewObject<ULayeredLBStrategy>(TestWorld);
		int StratCounter = 0;
		for (auto& StrategyConfig : ComponentStrategyConfigs)
		{
			auto strat_name = FName{ *FString(std::to_string(StratCounter++).c_str()) };
			FLayerInfo layer_info;
			layer_info.Name = strat_name;
			layer_info.LoadBalanceStrategy = StrategyConfig.StrategyClass;

			for (const auto& TargetActors : StrategyConfig.TargetActorTypes) {
				layer_info.ActorClasses.Add(TargetActors);
			}

			WorldSettings->WorkerLayers.Add(strat_name, layer_info);
		}

		Strat->Init();


		if (!NumVirtualWorkers) {
			NumVirtualWorkers = Strat->GetMinimumRequiredWorkers();
		}

		Strat->SetVirtualWorkerIds(1, NumVirtualWorkers.GetValue());

		return true;
	}

	DEFINE_LATENT_AUTOMATION_COMMAND_ONE_PARAMETER(FSetupStrategysLocalWorker, VirtualWorkerId, worker_id);
	bool FSetupStrategysLocalWorker::Update()
	{
		Strat->SetLocalVirtualWorkerId(worker_id);
		return true;
	}

	DEFINE_LATENT_AUTOMATION_COMMAND_THREE_PARAMETER(FCheckWhoShouldHaveAuthority, FAutomationTestBase*, Test, FName, ActorName, VirtualWorkerId, Expected);
	bool FCheckWhoShouldHaveAuthority::Update()
	{
		VirtualWorkerId Actual = Strat->WhoShouldHaveAuthority(*TestActors[ActorName]);
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

	DEFINE_LATENT_AUTOMATION_COMMAND_TWO_PARAMETER(FCheckMinimumWorkers, FAutomationTestBase*, Test, int, Expected);
	bool FCheckMinimumWorkers::Update()
	{
		int Actual = Strat->GetMinimumRequiredWorkers();
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

	DEFINE_LATENT_AUTOMATION_COMMAND_TWO_PARAMETER(FSpawnDefaultPawnAtLocation, FName, Handle,
		FVector, Location);
	bool FSpawnDefaultPawnAtLocation::Update()
	{
		FActorSpawnParameters SpawnParams;
		SpawnParams.bNoFail = true;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

		AActor* NewActor = TestWorld->SpawnActor<ADefaultPawn>(Location, FRotator::ZeroRotator, SpawnParams);
		TestActors.Add(Handle, NewActor);

		return true;
	}

	DEFINE_LATENT_AUTOMATION_COMMAND_TWO_PARAMETER(FSpawnSpecialPawnAtLocation,
		FName, Handle, FVector, Location);
	bool FSpawnSpecialPawnAtLocation::Update()
	{
		FActorSpawnParameters SpawnParams;
		SpawnParams.bNoFail = true;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

		AActor* NewActor = TestWorld->SpawnActor<ASpecialPawn>(Location, FRotator::ZeroRotator, SpawnParams);
		TestActors.Add(Handle, NewActor);

		return true;
	}

	DEFINE_LATENT_AUTOMATION_COMMAND_THREE_PARAMETER(FCheckActorsHaveSameAuth,
		FAutomationTestBase*, Test, FName, FirstActorName, FName, SecondActorName);
	bool FCheckActorsHaveSameAuth::Update()
	{
		VirtualWorkerId FirstActorAuth = Strat->WhoShouldHaveAuthority(*TestActors[FirstActorName]);
		VirtualWorkerId SecondActorAuth = Strat->WhoShouldHaveAuthority(*TestActors[SecondActorName]);

		Test->TestEqual(
			FString::Printf(TEXT("Checking actors have the same auth. Actor1: %d, Actor2: %d"), FirstActorAuth, SecondActorAuth),
			FirstActorAuth, SecondActorAuth);
		return true;
	}

	DEFINE_LATENT_AUTOMATION_COMMAND_THREE_PARAMETER(FCheckActorsHaveDifferentAuth, FAutomationTestBase*, Test, FName, FirstActorName, FName, SecondActorName);
	bool FCheckActorsHaveDifferentAuth::Update()
	{
		VirtualWorkerId FirstActorAuth = Strat->WhoShouldHaveAuthority(*TestActors[FirstActorName]);
		VirtualWorkerId SecondActorAuth = Strat->WhoShouldHaveAuthority(*TestActors[SecondActorName]);

		Test->TestNotEqual(
			FString::Printf(TEXT("Checking actors have different auth. Actor1: %d, Actor2: %d"), FirstActorAuth, SecondActorAuth),
			FirstActorAuth, SecondActorAuth);

		return true;
	}

	DEFINE_LATENT_AUTOMATION_COMMAND_TWO_PARAMETER(FCheckRequiresHandover, FAutomationTestBase*, TEST, bool, Expected);
	bool FCheckRequiresHandover::Update()
	{
		bool Actual = Strat->RequiresHandoverData();
		TEST->TestEqual(
			FString::Printf(TEXT("Checking strategy requires handover data Expected: %b Actual: %b"), Expected, Actual),
			Expected, Actual);
		return true;
	}

	DEFINE_LATENT_AUTOMATION_COMMAND_TWO_PARAMETER(FCheckShouldHaveAuthMatchesWhoShouldHaveAuth, FAutomationTestBase*, Test, FName, ActorName);
	bool FCheckShouldHaveAuthMatchesWhoShouldHaveAuth::Update()
	{
		bool WeShouldHaveAuthority
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
		ADD_LATENT_AUTOMATION_COMMAND(FSetupStrategy({}, {}));
		ADD_LATENT_AUTOMATION_COMMAND(FCheckStratIsReady(this, false));
		ADD_LATENT_AUTOMATION_COMMAND(FSetupStrategysLocalWorker(1));
		ADD_LATENT_AUTOMATION_COMMAND(FCheckStratIsReady(this, true));
		ADD_LATENT_AUTOMATION_COMMAND(FCleanup());

		return true;
	}

	LAYEREDLBSTRATEGY_TEST(GIVEN_layered_strat_of_three_two_by_four_grid_strats_and_default_strat_WHEN_get_minimum_required_workers_called_THEN_twenty_five_returned)
	{
		AutomationOpenMap("/Engine/Maps/Entry");
		ADD_LATENT_AUTOMATION_COMMAND(FWaitForWorld());

		std::vector<StrategyConfig> ComponentStrategyConfig{ 3, {UTwoByFourLBGridStrategy::StaticClass(), {}} };
		ADD_LATENT_AUTOMATION_COMMAND(FSetupStrategy(ComponentStrategyConfig, {}));
		ADD_LATENT_AUTOMATION_COMMAND(FCheckMinimumWorkers(this, 25));
		ADD_LATENT_AUTOMATION_COMMAND(FCleanup());

		return true;
	}

	LAYEREDLBSTRATEGY_TEST(GIVEN_two_actors_of_different_types_and_same_positions_managed_by_different_layers_WHEN_who_has_auth_called_THEN_return_different_values)
	{
		AutomationOpenMap("/Engine/Maps/Entry");
		ADD_LATENT_AUTOMATION_COMMAND(FWaitForWorld());

		std::vector<StrategyConfig> ComponentStrategyConfig{
			{UTwoByFourLBGridStrategy::StaticClass(), {ADefaultPawn::StaticClass()}},
			{UTwoByFourLBGridStrategy::StaticClass(), {ASpecialPawn::StaticClass()}}
		};

		ADD_LATENT_AUTOMATION_COMMAND(FSetupStrategy(ComponentStrategyConfig, {}));
		ADD_LATENT_AUTOMATION_COMMAND(FSetupStrategysLocalWorker(1));

		ADD_LATENT_AUTOMATION_COMMAND(FSpawnDefaultPawnAtLocation(TEXT("Actor1"), FVector(0, 0, 0)));
		ADD_LATENT_AUTOMATION_COMMAND(FSpawnSpecialPawnAtLocation(TEXT("Actor2"), FVector(0, 0, 0)));

		ADD_LATENT_AUTOMATION_COMMAND(FCheckActorsHaveDifferentAuth(this, TEXT("Actor1"), TEXT("Actor2")));

		ADD_LATENT_AUTOMATION_COMMAND(FCleanup());
		return true;
	}

	LAYEREDLBSTRATEGY_TEST(Given_layered_strat_of_2_single_cell_strats_and_default_strat_WHEN_set_virtual_worker_ids_called_with_2_ids_THEN_error_is_logged)
	{
		AutomationOpenMap("/Engine/Maps/Entry");
		ADD_LATENT_AUTOMATION_COMMAND(FWaitForWorld());

		std::vector<StrategyConfig> ComponentStrategyConfig
		{
			{UGridBasedLBStrategy::StaticClass(), {ADefaultPawn::StaticClass()}},
			{UGridBasedLBStrategy::StaticClass(), {ASpecialPawn::StaticClass()}}
		};

		this->AddExpectedError("LayeredLBStrategy was not given enough VirtualWorkerIds to meet the demands of the layer strategies.",
			EAutomationExpectedErrorFlags::MatchType::Contains, 1);

		// The two single strategies plus the default strat require 3 vitual workers, but we only have 2.
		ADD_LATENT_AUTOMATION_COMMAND(FSetupStrategy(ComponentStrategyConfig, 2));
		ADD_LATENT_AUTOMATION_COMMAND(FCleanup());

		return true;
	}

	LAYEREDLBSTRATEGY_TEST(Given_layered_strat_of_2_single_cell_grid_strats_and_default_strat_WHEN_set_virtual_worker_ids_called_with_3_ids_THEN_no_error_is_logged)
	{
		AutomationOpenMap("/Engine/Maps/Entry");
		ADD_LATENT_AUTOMATION_COMMAND(FWaitForWorld());

		std::vector<StrategyConfig> ComponentStrategyConfig
		{
			{UGridBasedLBStrategy::StaticClass(), {ADefaultPawn::StaticClass()}},
			{UGridBasedLBStrategy::StaticClass(), {ASpecialPawn::StaticClass()}}
		};

		// The two single strategies plus the default strat require 3 vitual workers.
		ADD_LATENT_AUTOMATION_COMMAND(FSetupStrategy(ComponentStrategyConfig, 3));
		ADD_LATENT_AUTOMATION_COMMAND(FCleanup());

		return true;
	}

	LAYEREDLBSTRATEGY_TEST(Given_layered_strat_of_default_strat_WHEN_requires_handover_called_THEN_returns_false)
	{
		AutomationOpenMap("/Engine/Maps/Entry");
		ADD_LATENT_AUTOMATION_COMMAND(FWaitForWorld());
		ADD_LATENT_AUTOMATION_COMMAND(FSetupStrategy({}, {}));
		ADD_LATENT_AUTOMATION_COMMAND(FSetupStrategysLocalWorker(1));
		ADD_LATENT_AUTOMATION_COMMAND(FCheckRequiresHandover(this, false));
		ADD_LATENT_AUTOMATION_COMMAND(FCleanup());

		return true;
	}

	LAYEREDLBSTRATEGY_TEST(Given_layered_strat_of_single_cell_grid_strat_and_default_strat_WHEN_requires_handover_called_THEN_returns_true)
	{
		AutomationOpenMap("/Engine/Maps/Entry");
		ADD_LATENT_AUTOMATION_COMMAND(FWaitForWorld());

		std::vector<StrategyConfig> ComponentStrategyConfig
		{
			{UGridBasedLBStrategy::StaticClass(), {ADefaultPawn::StaticClass()}}
		};

		ADD_LATENT_AUTOMATION_COMMAND(FSetupStrategy(ComponentStrategyConfig, {}));
		ADD_LATENT_AUTOMATION_COMMAND(FSetupStrategysLocalWorker(1));
		ADD_LATENT_AUTOMATION_COMMAND(FCheckRequiresHandover(this, true));
		ADD_LATENT_AUTOMATION_COMMAND(FCleanup());

		return true;
	}

	LAYEREDLBSTRATEGY_TEST(Given_layered_strat_of_default_strat_WHEN_who_should_have_auth_called_THEN_return_1)
	{
		AutomationOpenMap("/Engine/Maps/Entry");
		ADD_LATENT_AUTOMATION_COMMAND(FWaitForWorld());
		ADD_LATENT_AUTOMATION_COMMAND(FSetupStrategy({}, {}));
		ADD_LATENT_AUTOMATION_COMMAND(FSetupStrategysLocalWorker(1));
		ADD_LATENT_AUTOMATION_COMMAND(FSpawnDefaultPawnAtLocation(TEXT("Actor1"), FVector(0, 0, 0)));
		ADD_LATENT_AUTOMATION_COMMAND(FCheckWhoShouldHaveAuthority(this, "Actor1", 1));
		ADD_LATENT_AUTOMATION_COMMAND(FCleanup());

		return true;
	}

	LAYEREDLBSTRATEGY_TEST(Given_two_actors_of_same_type_at_same_position_WHEN_who_should_have_auth_called_THEN_return_same_for_both)
	{
		AutomationOpenMap("/Engine/Maps/Entry");
		ADD_LATENT_AUTOMATION_COMMAND(FWaitForWorld());

		std::vector<StrategyConfig> ComponentStrategyConfig
		{
			{UTwoByFourLBGridStrategy::StaticClass(), {ADefaultPawn::StaticClass()}},
			{UTwoByFourLBGridStrategy::StaticClass(), {ASpecialPawn::StaticClass()}}
		};

		ADD_LATENT_AUTOMATION_COMMAND(FSetupStrategy(ComponentStrategyConfig, {}));
		ADD_LATENT_AUTOMATION_COMMAND(FSetupStrategysLocalWorker(1));
		ADD_LATENT_AUTOMATION_COMMAND(FSpawnDefaultPawnAtLocation(TEXT("Actor1"), FVector(0, 0, 0)));
		ADD_LATENT_AUTOMATION_COMMAND(FSpawnDefaultPawnAtLocation(TEXT("Actor2"), FVector(0, 0, 0)));
		ADD_LATENT_AUTOMATION_COMMAND(FSpawnSpecialPawnAtLocation(TEXT("Actor3"), FVector(0, 0, 0)));
		ADD_LATENT_AUTOMATION_COMMAND(FSpawnSpecialPawnAtLocation(TEXT("Actor4"), FVector(0, 0, 0)));

		ADD_LATENT_AUTOMATION_COMMAND(FCheckActorsHaveSameAuth(this, TEXT("Actor1"), TEXT("Actor2")));
		ADD_LATENT_AUTOMATION_COMMAND(FCheckActorsHaveSameAuth(this, TEXT("Actor3"), TEXT("Actor4")));
		ADD_LATENT_AUTOMATION_COMMAND(FCleanup());

		return true;
	}

	LAYEREDLBSTRATEGY_TEST(Given_layered_strat_of_default_and_2_2x4_grids_WHEN_should_have_auth_called_THEN_return_true_iff_who_should_have_auth_returns_local_worker_id)
	{
		AutomationOpenMap("/Engine/Maps/Entry");
		ADD_LATENT_AUTOMATION_COMMAND(FWaitForWorld());

		std::vector<StrategyConfig> ComponentStrategyConfig{
			{UTwoByFourLBGridStrategy::StaticClass(), {ADefaultPawn::StaticClass()}},
			{UTwoByFourLBGridStrategy::StaticClass(), {ASpecialPawn::StaticClass()}}
		};

		int num_virtual_workers = 17;
		ADD_LATENT_AUTOMATION_COMMAND(FSetupStrategy(ComponentStrategyConfig, num_virtual_workers));

		ADD_LATENT_AUTOMATION_COMMAND(FSpawnDefaultPawnAtLocation(TEXT("Actor1"), FVector(0, 0, 0)));
		ADD_LATENT_AUTOMATION_COMMAND(FSpawnSpecialPawnAtLocation(TEXT("Actor2"), FVector(0, 0, 0)));
		for (int i = 1; i <= num_virtual_workers; ++i) {
			ADD_LATENT_AUTOMATION_COMMAND(FSetupStrategysLocalWorker(i));
			ADD_LATENT_AUTOMATION_COMMAND(FCheckShouldHaveAuthMatchesWhoShouldHaveAuth(this, TEXT("Actor1")));
			ADD_LATENT_AUTOMATION_COMMAND(FCheckShouldHaveAuthMatchesWhoShouldHaveAuth(this, TEXT("Actor2")));
		}

		ADD_LATENT_AUTOMATION_COMMAND(FCleanup());

		return true;
	}
}

