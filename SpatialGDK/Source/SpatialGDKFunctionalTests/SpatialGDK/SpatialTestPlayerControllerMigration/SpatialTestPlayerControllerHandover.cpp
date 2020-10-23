// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "SpatialTestPlayerControllerHandover.h"
#include "EngineClasses/SpatialNetDriver.h"
#include "LoadBalancing/LayeredLBStrategy.h"
#include "SpatialFunctionalTestFlowController.h"

#include "GameFramework/Character.h"
#include "GameFramework/PlayerState.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"

ASpatialTestPlayerControllerHandoverGameMode::ASpatialTestPlayerControllerHandoverGameMode(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	DefaultPawnClass = ACharacter::StaticClass();
	bStartPlayersAsSpectators = true;
}

void ASpatialTestPlayerControllerHandover::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ASpatialTestPlayerControllerHandover, DestinationWorker);
}

/**
 * This test checks that APlayerController state is properly handed over when worker migration happens.
 *
 * We test alternating between possessing a pawn and spectating, and checking that no state is lots when migrating between workers.
 *
 * - The gamemode is set to start the player as a spectator.
 * - We make the player spawn for the client, migrate to another worker, and check that the "Playing" state has been handed over.
 * - Then we set the player as spectating only, which will also set the player as "not ready to spawn"
 * - We migrate to another worker, and check that the "spectating" state has been handed over.
 * - Then we try to respawn from the client, and check that no pawn has been spawned (because the player should still not be ready).
 * - We set the player as ready, migrate to another worker, and check that we can spawn.
 *
 */

ASpatialTestPlayerControllerHandover::ASpatialTestPlayerControllerHandover()
	: Super()
{
	Author = "Nicolas";
	Description = TEXT("Test player controller handover");
}

static const FName GDebugTag(TEXT("PlayerController"));

void ASpatialTestPlayerControllerHandover::OnRep_DestinationWorker()
{
	bReceivedNewDestination = true;
}

APlayerController* ASpatialTestPlayerControllerHandover::GetPlayerController()
{
	ASpatialFunctionalTestFlowController* FlowController = GetFlowController(ESpatialFunctionalTestWorkerType::Client, 1);
	check(FlowController != nullptr);
	return Cast<APlayerController>(FlowController->GetOwner());
}

void ASpatialTestPlayerControllerHandover::PrepareTest()
{
	Super::PrepareTest();

	FSpatialFunctionalTestStepDefinition NextDestination(true);
	NextDestination.StepName = TEXT("DetermineNextDestination");
	NextDestination.NativeStartEvent = FNativeStepStartDelegate::CreateLambda([this]() {
		if (HasAuthority())
		{
			TArray<VirtualWorkerId> Workers;
			WorkerPositions.GenerateKeyArray(Workers);
			int32 i = Workers.Find(DestinationWorker);
			check(i != -1);

			i = (i + 1) % Workers.Num();

			DestinationWorker = Workers[i];
		}

		FinishStep();
	});

	FSpatialFunctionalTestStepDefinition AuthMove(true);
	AuthMove.StepName = TEXT("PerformAuthMove");
	AuthMove.NativeTickEvent = FNativeStepTickDelegate::CreateLambda([this](float) {
		if (HasAuthority() || bReceivedNewDestination)
		{
			bReceivedNewDestination = false;
			SetTagDelegation(GDebugTag, DestinationWorker);
			FinishStep();
		}
	});

	FSpatialFunctionalTestStepDefinition WaitAuth(true);
	WaitAuth.StepName = TEXT("WaitForAuthChange");
	WaitAuth.NativeTickEvent = FNativeStepTickDelegate::CreateLambda([this](float) {
		if (LocalWorker == DestinationWorker)
		{
			APlayerController* PlayerController = GetPlayerController();
			if (PlayerController && PlayerController->HasAuthority())
			{
				FinishStep();
			}
		}
		else
		{
			FinishStep();
		}
	});

	auto AddStepChangePlayerControllerAuthWorker = [&] {
		AddStepFromDefinition(NextDestination, FWorkerDefinition::AllServers);
		AddStepFromDefinition(AuthMove, FWorkerDefinition::AllServers);
		AddStepFromDefinition(WaitAuth, FWorkerDefinition::AllServers);
	};

	FSpatialFunctionalTestStepDefinition ClientRespawn(true);
	ClientRespawn.StepName = TEXT("SpawnPlayerPawn");
	ClientRespawn.NativeStartEvent = FNativeStepStartDelegate::CreateLambda([this] {
		ASpatialFunctionalTestFlowController* FlowController = GetLocalFlowController();
		APlayerController* PlayerController = Cast<APlayerController>(FlowController->GetOwner());

		PlayerController->ServerRestartPlayer();

		FinishStep();
	});

	FSpatialFunctionalTestStepDefinition WaitPlayerSpawn(true);
	WaitPlayerSpawn.StepName = TEXT("WaitForPlayerPawn");
	WaitPlayerSpawn.NativeStartEvent = FNativeStepStartDelegate::CreateLambda([this] {
		APlayerController* PlayerController = GetPlayerController();
		if (PlayerController && PlayerController->HasAuthority())
		{
			if (PlayerController->GetPawn() != nullptr)
			{
				AssertTrue(PlayerController->GetStateName() == NAME_Playing, TEXT("State was changed on posession"));
				FinishStep();
			}
		}
		else
		{
			FinishStep();
		}
	});

	AddStep(
		TEXT("SetupStep"), FWorkerDefinition::AllServers, nullptr,
		[this]() {
			UWorld* World = GetWorld();

			WorkerPositions.Empty();
			bIsOnDefaultLayer = false;
			bReceivedNewDestination = false;

			ULayeredLBStrategy* RootStrategy = GetLoadBalancingStrategy();

			bIsOnDefaultLayer = RootStrategy->CouldHaveAuthority(ACharacter::StaticClass());
			if (bIsOnDefaultLayer)
			{
				FName LocalLayer = RootStrategy->GetLocalLayerName();
				UAbstractLBStrategy* LocalStrategy = RootStrategy->GetLBStrategyForLayer(LocalLayer);

				AssertTrue(LocalStrategy->IsA<UGridBasedLBStrategy>(), TEXT(""));

				UGridBasedLBStrategy* GridStrategy = Cast<UGridBasedLBStrategy>(LocalStrategy);

				for (auto& WorkerRegion : GridStrategy->GetLBStrategyRegions())
				{
					FVector2D RegionCenter = WorkerRegion.Value.GetCenter();
					WorkerPositions.Add(WorkerRegion.Key, FVector(RegionCenter.X, RegionCenter.Y, 0));
				}
				LocalWorker = GridStrategy->GetLocalVirtualWorkerId();
			}

			FinishStep();
		},
		nullptr);

	AddStep(TEXT("GetClientController"), FWorkerDefinition::AllServers, nullptr, [this] {
		if (APlayerController* PlayerController = GetPlayerController())
		{
			AssertTrue(PlayerController->GetStateName() == NAME_Spectating, TEXT("Client started in spectator mode"));

			if (PlayerController->HasAuthority())
			{
				AddDebugTag(PlayerController, GDebugTag);
				DestinationWorker = LocalWorker;
			}
		}
		FinishStep();
	});

	AddStepFromDefinition(ClientRespawn, FWorkerDefinition::Client(1));
	AddStepFromDefinition(WaitPlayerSpawn, FWorkerDefinition::AllServers);

	AddStepChangePlayerControllerAuthWorker();

	AddStep(TEXT("CheckStateHandover"), FWorkerDefinition::AllServers, nullptr, [this] {
		APlayerController* PlayerController = GetPlayerController();
		if (PlayerController && PlayerController->HasAuthority())
		{
			AssertTrue(PlayerController->GetStateName() == NAME_Playing, TEXT("State handed over"));
		}
		FinishStep();
	});

	AddStep(TEXT("SetToSpectateOnly"), FWorkerDefinition::AllServers, nullptr, [this] {
		APlayerController* PlayerController = GetPlayerController();
		if (PlayerController && PlayerController->HasAuthority())
		{
			// This will set the Player as "not ready"
			PlayerController->StartSpectatingOnly();
		}
		FinishStep();
	});

	AddStepChangePlayerControllerAuthWorker();

	AddStep(TEXT("CheckStateHandover"), FWorkerDefinition::AllServers, nullptr, [this] {
		APlayerController* PlayerController = GetPlayerController();
		if (PlayerController && PlayerController->HasAuthority())
		{
			AssertTrue(PlayerController->GetStateName() == NAME_Spectating, TEXT("State was handed over pn player controller"));
#if ENGINE_MINOR_VERSION <= 24
			AssertTrue(PlayerController->PlayerState->bOnlySpectator, TEXT("State was handed over on player state"));
			PlayerController->PlayerState->bOnlySpectator = false;
#else
			AssertTrue(PlayerController->PlayerState->IsOnlyASpectator(), TEXT("State was handed over on player state"));
			PlayerController->PlayerState->SetIsOnlyASpectator(false);
#endif
		}

		FinishStep();
	});

	AddStepFromDefinition(ClientRespawn, FWorkerDefinition::Client(1));

	AddStep(
		TEXT("CheckNoPlayerSpawned"), FWorkerDefinition::AllServers, nullptr,
		[this] {
			CheckNoPlayerSpawnTime = GetWorld()->GetTimeSeconds();
		},
		[this](float) {
			APlayerController* PlayerController = GetPlayerController();
			if (PlayerController && PlayerController->HasAuthority())
			{
				AssertTrue(PlayerController->GetPawn() == nullptr, TEXT("No pawn was spawned"));
				if (GetWorld()->GetTimeSeconds() - CheckNoPlayerSpawnTime > 3.0)
				{
					FinishStep();
				}
			}
			else
			{
				FinishStep();
			}
		});

	AddStep(TEXT("SetPlayerReady"), FWorkerDefinition::AllServers, nullptr, [this] {
		APlayerController* PlayerController = GetPlayerController();
		if (PlayerController && PlayerController->HasAuthority())
		{
			PlayerController->ServerSetSpectatorWaiting(true);
			PlayerController->ClientSetSpectatorWaiting(true);
		}
		FinishStep();
	});

	AddStepChangePlayerControllerAuthWorker();

	AddStepFromDefinition(ClientRespawn, FWorkerDefinition::Client(1));
	AddStepFromDefinition(WaitPlayerSpawn, FWorkerDefinition::AllServers);
}
