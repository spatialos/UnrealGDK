// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "GameModeReplicationTest.h"

#include "Net/UnrealNetwork.h"

#include "GameFramework/GameStateBase.h"

AGameModeReplicationTestGameMode::AGameModeReplicationTestGameMode()
{
	NetCullDistanceSquared = 0.0f;
}

void AGameModeReplicationTestGameMode::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	DOREPLIFETIME(AGameModeReplicationTestGameMode, ReplicatedValue);
}

bool AGameModeReplicationTestGameMode::IsNetRelevantFor(const AActor* RealViewer, const AActor* ViewTarget,
														const FVector& SrcLocation) const
{
	return false;
}

/**
 * This test checks that GameModes:
 * * Only owned by a single worker
 * * Replicate between servers
 * * Don't replicate to clients
 */

AGameModeReplicationTest::AGameModeReplicationTest()
{
	Author = TEXT("Dmitrii");
	Description = TEXT("Test GameMode replication");
}

void AGameModeReplicationTest::MarkWorkerGameModeAuthority_Implementation(bool bHasGameModeAuthority)
{
	ServerResponsesCount++;

	if (bHasGameModeAuthority)
	{
		++AuthorityServersCount;
	}
}

void AGameModeReplicationTest::PrepareTest()
{
	Super::PrepareTest();

	check(GetWorld()->GetGameState()->GameModeClass == AGameModeReplicationTestGameMode::StaticClass());

	AuthorityServersCount = 0;

	AddStep(TEXT("Changing replicated value on the authoritative server"), FWorkerDefinition::AllServers, nullptr, [this]() {
		AGameModeReplicationTestGameMode* GameMode = Cast<AGameModeReplicationTestGameMode>(GetWorld()->GetAuthGameMode());

		check(IsValid(GameMode));

		AssertEqual_Int(GameMode->ReplicatedValue, AGameModeReplicationTestGameMode::StartingValue,
						TEXT("Value on the GameMode before changing it"));

		const bool bHasAuthorityOverGameMode = GameMode->HasAuthority();

		MarkWorkerGameModeAuthority(bHasAuthorityOverGameMode);

		if (bHasAuthorityOverGameMode)
		{
			const FVector GameModeFarawayLocation(1000 * 1000 * 1000, 1000 * 1000 * 1000, 1000 * 1000 * 1000);

			// GameModeBase hides this function so we upcast to AActor;
			// this probably doesn't do anything as GameModes generally don't have RootComponents
			// and no location as consequence
			AActor* GameModeActor = GameMode;

			GameModeActor->SetActorLocation(GameModeFarawayLocation);

			// actually change the replicated value from the authority server
			GameMode->ReplicatedValue = AGameModeReplicationTestGameMode::UpdatedValue;
		}

		FinishStep();
	});

	constexpr float CrossServerRpcExecutionTime = 1;

	AddStep(
		TEXT("Waiting for GameMode authority information"), FWorkerDefinition::AllServers, nullptr,
		[this]() {
			if (!HasAuthority())
			{
				FinishStep();
			}
		},
		[this](float) {
			if (ServerResponsesCount == GetNumberOfServerWorkers())
			{
				AssertEqual_Int(AuthorityServersCount, 1, TEXT("Count of servers holding authority over the GameMode"));

				FinishStep();
			}
		},
		CrossServerRpcExecutionTime);

	constexpr float ValueReplicationTime = 1;

	AddStep(
		TEXT("Waiting for the GameMode value to be received on all servers"), FWorkerDefinition::AllServers, nullptr, nullptr,
		[this](float DeltaTime) {
			AGameModeReplicationTestGameMode* GameMode = Cast<AGameModeReplicationTestGameMode>(GetWorld()->GetAuthGameMode());

			check(IsValid(GameMode));

			if (GameMode->ReplicatedValue == AGameModeReplicationTestGameMode::UpdatedValue)
			{
				FinishStep();
			}
		},
		ValueReplicationTime);

	AddStep(TEXT("Checking that no clients have a GameMode"), FWorkerDefinition::AllClients, nullptr, [this]() {
		for (AGameModeBase* GameModeActor : TActorRange<AGameModeBase>(GetWorld()))
		{
			AddError(FString::Printf(TEXT("Found a GameMode Actor %s on client!"), *GetNameSafe(GameModeActor)));
		}

		FinishStep();
	});
}
