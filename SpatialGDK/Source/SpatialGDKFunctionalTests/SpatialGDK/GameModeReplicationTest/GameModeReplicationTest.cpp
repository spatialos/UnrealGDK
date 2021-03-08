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
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AGameModeReplicationTestGameMode, ReplicatedValue);
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

	AuthorityServersCount = 0;

	AddStep(TEXT("Check initial replicated value correct on all server"), FWorkerDefinition::AllServers, nullptr, [this]() {
		AGameModeReplicationTestGameMode* GameMode = Cast<AGameModeReplicationTestGameMode>(GetWorld()->GetAuthGameMode());

		check(IsValid(GameMode));

		AssertEqual_Int(GameMode->ReplicatedValue, AGameModeReplicationTestGameMode::StartingValue,
						TEXT("Value on the GameMode before changing it"));

		FinishStep();
	});

	AddStep(TEXT("Changing replicated value on the authoritative server"), FWorkerDefinition::AllServers, nullptr, [this]() {
		AGameModeReplicationTestGameMode* GameMode = Cast<AGameModeReplicationTestGameMode>(GetWorld()->GetAuthGameMode());

		check(IsValid(GameMode));

		const bool bHasAuthorityOverGameMode = GameMode->HasAuthority();

		MarkWorkerGameModeAuthority(bHasAuthorityOverGameMode);

		if (bHasAuthorityOverGameMode)
		{
			// actually change the replicated value from the authority server
			GameMode->ReplicatedValue = AGameModeReplicationTestGameMode::UpdatedValue;
		}

		FinishStep();
	});

	constexpr float CrossServerRpcExecutionTime = 1.0f;

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

	constexpr float ValueReplicationTime = 1.0f;

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
