// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "LoadBalancing/GridBasedLBStrategy.h"
#include "Schema/StandardLibrary.h"
#include "TestGridBasedLBStrategy.h"

#include "CoreMinimal.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/DefaultPawn.h"
#include "GameFramework/GameStateBase.h"
#include "Tests/AutomationCommon.h"
#include "Tests/AutomationEditorCommon.h"
#include "Tests/TestDefinitions.h"

#define GRIDBASEDLBSTRATEGY_TEST(TestName) GDK_TEST(Core, UGridBasedLBStrategy, TestName)

// Test Globals
namespace
{
UWorld* TestWorld;
TMap<FName, AActor*> TestActors;
UGridBasedLBStrategy* Strat;

// Copied from AutomationCommon::GetAnyGameWorld()
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

void CreateStrategy(uint32 Rows, uint32 Cols, float WorldWidth, float WorldHeight, uint32 LocalWorkerId)
{
	Strat = UTestGridBasedLBStrategy::Create(Rows, Cols, WorldWidth, WorldHeight);
	Strat->Init();
	Strat->SetVirtualWorkerIds(1, Strat->GetMinimumRequiredWorkers());
	Strat->SetLocalVirtualWorkerId(LocalWorkerId);
}

DEFINE_LATENT_AUTOMATION_COMMAND(FCleanup);
bool FCleanup::Update()
{
	TestWorld = nullptr;
	TestActors.Empty();
	Strat = nullptr;

	return true;
}

DEFINE_LATENT_AUTOMATION_COMMAND_FIVE_PARAMETER(FCreateStrategy, uint32, Rows, uint32, Cols, float, WorldWidth, float, WorldHeight, uint32,
												LocalWorkerId);
bool FCreateStrategy::Update()
{
	CreateStrategy(Rows, Cols, WorldWidth, WorldHeight, LocalWorkerId);
	return true;
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

DEFINE_LATENT_AUTOMATION_COMMAND_TWO_PARAMETER(FSpawnActorAtLocation, FName, Handle, FVector, Location);
bool FSpawnActorAtLocation::Update()
{
	FActorSpawnParameters SpawnParams;
	SpawnParams.bNoFail = true;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	AActor* NewActor = TestWorld->SpawnActor<ADefaultPawn>(Location, FRotator::ZeroRotator, SpawnParams);
	TestActors.Add(Handle, NewActor);

	return true;
}

DEFINE_LATENT_AUTOMATION_COMMAND_TWO_PARAMETER(FMoveActor, FName, Handle, FVector, Location);
bool FMoveActor::Update()
{
	TestActors[Handle]->SetActorLocation(Location);
	return true;
}

DEFINE_LATENT_AUTOMATION_COMMAND_ONE_PARAMETER(FWaitForActor, FName, Handle);
bool FWaitForActor::Update()
{
	AActor* TestActor = TestActors[Handle];
	return (IsValid(TestActor) && TestActor->IsActorInitialized() && TestActor->HasActorBegunPlay());
}

DEFINE_LATENT_AUTOMATION_COMMAND_THREE_PARAMETER(FCheckShouldRelinquishAuthority, FAutomationTestBase*, Test, FName, Handle, bool,
												 bExpected);
bool FCheckShouldRelinquishAuthority::Update()
{
	bool bActual = !Strat->ShouldHaveAuthority(*TestActors[Handle]);

	Test->TestEqual(FString::Printf(TEXT("Should Relinquish Authority. Actual: %d, Expected: %d"), bActual, bExpected), bActual, bExpected);

	return true;
}

DEFINE_LATENT_AUTOMATION_COMMAND_THREE_PARAMETER(FCheckWhoShouldHaveAuthority, FAutomationTestBase*, Test, FName, Handle, uint32,
												 ExpectedVirtualWorker);
bool FCheckWhoShouldHaveAuthority::Update()
{
	uint32 Actual = Strat->WhoShouldHaveAuthority(*TestActors[Handle]);

	Test->TestEqual(FString::Printf(TEXT("Who Should Have Authority. Actual: %d, Expected: %d"), Actual, ExpectedVirtualWorker), Actual,
					ExpectedVirtualWorker);

	return true;
}

DEFINE_LATENT_AUTOMATION_COMMAND_TWO_PARAMETER(FCheckVirtualWorkersDiffer, FAutomationTestBase*, Test, TArray<FName>, Handles);
bool FCheckVirtualWorkersDiffer::Update()
{
	TMap<uint32, FName> WorkerIdToHandle;
	for (int i = 0; i < Handles.Num(); i++)
	{
		const uint32 WorkerId = Strat->WhoShouldHaveAuthority(*TestActors[Handles[i]]);
		if (WorkerIdToHandle.Contains(WorkerId))
		{
			Test->AddError(FString::Printf(TEXT("%s and %s both belong to virtual worker %d"), *WorkerIdToHandle[WorkerId].ToString(),
										   *Handles[i].ToString(), WorkerId));
		}
		else
		{
			WorkerIdToHandle.Add(WorkerId, Handles[i]);
		}
	}

	return true;
}

DEFINE_LATENT_AUTOMATION_COMMAND_TWO_PARAMETER(FCheckVirtualWorkersMatch, FAutomationTestBase*, Test, TArray<FName>, Handles);
bool FCheckVirtualWorkersMatch::Update()
{
	uint32 VirtualWorkerId = Strat->WhoShouldHaveAuthority(*TestActors[Handles[0]]);

	for (int i = 1; i < Handles.Num(); i++)
	{
		uint32 NextVirtualWorkerId = Strat->WhoShouldHaveAuthority(*TestActors[Handles[i]]);
		Test->TestEqual(FString::Printf(TEXT("Should Have Authority %s(%d) and %s(%d)"), *Handles[0].ToString(), VirtualWorkerId,
										*Handles[i].ToString(), NextVirtualWorkerId),
						VirtualWorkerId, NextVirtualWorkerId);
	}

	return true;
}

GRIDBASEDLBSTRATEGY_TEST(GIVEN_2_rows_3_cols_WHEN_get_minimum_required_workers_is_called_THEN_it_returns_6)
{
	CreateStrategy(2, 3, 10000.f, 10000.f, 1);

	uint32 NumVirtualWorkers = Strat->GetMinimumRequiredWorkers();
	TestEqual("Number of Virtual Workers", NumVirtualWorkers, 6);

	return true;
}

GRIDBASEDLBSTRATEGY_TEST(GIVEN_grid_is_not_ready_WHEN_local_virtual_worker_id_is_set_THEN_is_ready)
{
	Strat = UTestGridBasedLBStrategy::Create(1, 1, 10000.f, 10000.f);
	Strat->Init();
	Strat->SetVirtualWorkerIds(1, Strat->GetMinimumRequiredWorkers());

	TestFalse("IsReady Before LocalVirtualWorkerId Set", Strat->IsReady());

	Strat->SetLocalVirtualWorkerId(123);

	TestTrue("IsReady After LocalVirtualWorkerId Set", Strat->IsReady());

	return true;
}

GRIDBASEDLBSTRATEGY_TEST(GIVEN_four_cells_WHEN_get_worker_interest_for_virtual_worker_THEN_returns_correct_constraint)
{
	// Take the top right corner, as then all our testing numbers can be positive.
	// Create the Strategy manually so we can set an interest border.
	Strat = UTestGridBasedLBStrategy::Create(2, 2, 10000.f, 10000.f, 1000.f);
	Strat->Init();
	Strat->SetVirtualWorkerIds(1, Strat->GetMinimumRequiredWorkers());
	Strat->SetLocalVirtualWorkerId(4);

	SpatialGDK::QueryConstraint StratConstraint = Strat->GetWorkerInterestQueryConstraint();

	SpatialGDK::BoxConstraint Box = StratConstraint.BoxConstraint.GetValue();

	// y is the vertical axis in SpatialOS coordinates.
	SpatialGDK::Coordinates TestCentre = SpatialGDK::Coordinates{ 25.0, 0.0, 25.0 };
	// The constraint will be a 50x50 box around the centre, expanded by 10 in every direction because of the interest border, so +20 to x
	// and z.
	double TestEdgeLength = 70;

	TestEqual("Centre of the interest grid is as expected", Box.Center, TestCentre);
	TestEqual("Edge length in x is as expected", Box.EdgeLength.X, TestEdgeLength);
	TestEqual("Edge length in z is as expected", Box.EdgeLength.Z, TestEdgeLength);

	// The height of the box is "some very large number which is effectively infinite", so just sanity check it here.
	TestTrue("Edge length in y is greater than 0", Box.EdgeLength.Y > 0);

	return true;
}

GRIDBASEDLBSTRATEGY_TEST(GIVEN_four_cells_WHEN_get_worker_entity_position_for_virtual_worker_THEN_returns_correct_position)
{
	// Take the top right corner, as then all our testing numbers can be positive.
	CreateStrategy(2, 2, 10000.f, 10000.f, 4);

	FVector WorkerPosition = Strat->GetWorkerEntityPosition();

	FVector TestPosition = FVector{ 2500.0f, 2500.0f, 0.0f };

	TestEqual("Worker entity position is as expected", WorkerPosition, TestPosition);

	return true;
}

GRIDBASEDLBSTRATEGY_TEST(GIVEN_one_cell_WHEN_requires_handover_data_called_THEN_returns_false)
{
	CreateStrategy(1, 1, 10000.f, 10000.f, 1);
	TestFalse("Strategy doesn't require handover data", Strat->RequiresHandoverData());
	return true;
}

GRIDBASEDLBSTRATEGY_TEST(GIVEN_more_than_one_row_WHEN_requires_handover_data_called_THEN_returns_true)
{
	CreateStrategy(2, 1, 10000.f, 10000.f, 1);
	TestTrue("Strategy doesn't require handover data", Strat->RequiresHandoverData());
	return true;
}

GRIDBASEDLBSTRATEGY_TEST(GIVEN_more_than_one_column_WHEN_requires_handover_data_called_THEN_returns_true)
{
	CreateStrategy(1, 2, 10000.f, 10000.f, 1);
	TestTrue("Strategy doesn't require handover data", Strat->RequiresHandoverData());
	return true;
}

} // anonymous namespace

GRIDBASEDLBSTRATEGY_TEST(GIVEN_a_single_cell_and_valid_local_id_WHEN_should_relinquish_called_THEN_returns_false)
{
	AutomationOpenMap("/Engine/Maps/Entry");

	ADD_LATENT_AUTOMATION_COMMAND(FCreateStrategy(1, 1, 10000.f, 10000.f, 1));
	ADD_LATENT_AUTOMATION_COMMAND(FWaitForWorld());
	ADD_LATENT_AUTOMATION_COMMAND(FSpawnActorAtLocation("Actor", FVector::ZeroVector));
	ADD_LATENT_AUTOMATION_COMMAND(FWaitForActor("Actor"));
	ADD_LATENT_AUTOMATION_COMMAND(FCheckShouldRelinquishAuthority(this, "Actor", false));
	ADD_LATENT_AUTOMATION_COMMAND(FCleanup());

	return true;
}

GRIDBASEDLBSTRATEGY_TEST(GIVEN_four_cells_WHEN_actors_in_each_cell_THEN_should_return_different_virtual_workers)
{
	AutomationOpenMap("/Engine/Maps/Entry");

	ADD_LATENT_AUTOMATION_COMMAND(FCreateStrategy(2, 2, 10000.f, 10000.f, 1));
	ADD_LATENT_AUTOMATION_COMMAND(FWaitForWorld());
	ADD_LATENT_AUTOMATION_COMMAND(FSpawnActorAtLocation("Actor1", FVector(-2500.f, -2500.f, 0.f)));
	ADD_LATENT_AUTOMATION_COMMAND(FSpawnActorAtLocation("Actor2", FVector(2500.f, -2500.f, 0.f)));
	ADD_LATENT_AUTOMATION_COMMAND(FSpawnActorAtLocation("Actor3", FVector(-2500.f, 2500.f, 0.f)));
	ADD_LATENT_AUTOMATION_COMMAND(FSpawnActorAtLocation("Actor4", FVector(2500.f, 2500.f, 0.f)));
	ADD_LATENT_AUTOMATION_COMMAND(FWaitForActor("Actor1"));
	ADD_LATENT_AUTOMATION_COMMAND(FWaitForActor("Actor2"));
	ADD_LATENT_AUTOMATION_COMMAND(FWaitForActor("Actor3"));
	ADD_LATENT_AUTOMATION_COMMAND(FWaitForActor("Actor4"));
	ADD_LATENT_AUTOMATION_COMMAND(FCheckVirtualWorkersDiffer(this, { "Actor1", "Actor2", "Actor3", "Actor4" }));
	ADD_LATENT_AUTOMATION_COMMAND(FCleanup());

	return true;
}

GRIDBASEDLBSTRATEGY_TEST(GIVEN_moving_actor_WHEN_actor_crosses_boundary_THEN_should_relinquish_authority)
{
	AutomationOpenMap("/Engine/Maps/Entry");

	ADD_LATENT_AUTOMATION_COMMAND(FCreateStrategy(2, 1, 10000.f, 10000.f, 1));
	ADD_LATENT_AUTOMATION_COMMAND(FWaitForWorld());
	ADD_LATENT_AUTOMATION_COMMAND(FSpawnActorAtLocation("Actor1", FVector(-2.f, 0.f, 0.f)));
	ADD_LATENT_AUTOMATION_COMMAND(FWaitForActor("Actor1"));
	ADD_LATENT_AUTOMATION_COMMAND(FCheckShouldRelinquishAuthority(this, "Actor1", false));
	ADD_LATENT_AUTOMATION_COMMAND(FMoveActor("Actor1", FVector(0.f, 0.f, 0.f)));
	ADD_LATENT_AUTOMATION_COMMAND(FCheckShouldRelinquishAuthority(this, "Actor1", true));
	ADD_LATENT_AUTOMATION_COMMAND(FMoveActor("Actor1", FVector(2.f, 0.f, 0.f)));
	ADD_LATENT_AUTOMATION_COMMAND(FCheckShouldRelinquishAuthority(this, "Actor1", true));
	ADD_LATENT_AUTOMATION_COMMAND(FCleanup());

	return true;
}

GRIDBASEDLBSTRATEGY_TEST(GIVEN_two_actors_WHEN_actors_are_in_same_cell_THEN_should_belong_to_same_worker_id)
{
	AutomationOpenMap("/Engine/Maps/Entry");

	ADD_LATENT_AUTOMATION_COMMAND(FCreateStrategy(1, 2, 10000.f, 10000.f, 1));
	ADD_LATENT_AUTOMATION_COMMAND(FWaitForWorld());
	ADD_LATENT_AUTOMATION_COMMAND(FSpawnActorAtLocation("Actor1", FVector(-2.f, 100.f, 0.f)));
	ADD_LATENT_AUTOMATION_COMMAND(FSpawnActorAtLocation("Actor2", FVector(-500.f, 0.f, 0.f)));
	ADD_LATENT_AUTOMATION_COMMAND(FWaitForActor("Actor1"));
	ADD_LATENT_AUTOMATION_COMMAND(FWaitForActor("Actor2"));
	ADD_LATENT_AUTOMATION_COMMAND(FCheckVirtualWorkersMatch(this, { "Actor1", "Actor2" }));
	ADD_LATENT_AUTOMATION_COMMAND(FCleanup());

	return true;
}

GRIDBASEDLBSTRATEGY_TEST(GIVEN_two_cells_WHEN_actor_in_one_cell_THEN_strategy_relinquishes_based_on_local_id)
{
	AutomationOpenMap("/Engine/Maps/Entry");

	ADD_LATENT_AUTOMATION_COMMAND(FWaitForWorld());
	ADD_LATENT_AUTOMATION_COMMAND(FSpawnActorAtLocation("Actor1", FVector(0.f, -2500.f, 0.f)));
	ADD_LATENT_AUTOMATION_COMMAND(FWaitForActor("Actor1"));
	ADD_LATENT_AUTOMATION_COMMAND(FCreateStrategy(1, 2, 10000.f, 10000.f, 1));
	ADD_LATENT_AUTOMATION_COMMAND(FCheckShouldRelinquishAuthority(this, "Actor1", false));
	ADD_LATENT_AUTOMATION_COMMAND(FCreateStrategy(1, 2, 10000.f, 10000.f, 2));
	ADD_LATENT_AUTOMATION_COMMAND(FCheckShouldRelinquishAuthority(this, "Actor1", true));
	ADD_LATENT_AUTOMATION_COMMAND(FCleanup());

	return true;
}
