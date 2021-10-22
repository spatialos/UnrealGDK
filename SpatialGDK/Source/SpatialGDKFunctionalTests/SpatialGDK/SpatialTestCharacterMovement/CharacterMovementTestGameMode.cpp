// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "CharacterMovementTestGameMode.h"

#include "EngineUtils.h"
#if WITH_EDITOR
#include "GameFramework/PlayerStart.h"
#endif
#include "SpatialGDKFunctionalTests/SpatialGDK/TestActors/TestMovementCharacter.h"

ACharacterMovementTestGameMode::ACharacterMovementTestGameMode(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	DefaultPawnClass = ATestMovementCharacter::StaticClass();
}

void ACharacterMovementTestGameMode::BeginPlay()
{
	Super::BeginPlay();
#if WITH_EDITOR
	for (APlayerStart* PlayerStart : TActorRange<APlayerStart>(GetWorld()))
	{
		PlayerStarts.Add(PlayerStart);
	}
#endif
}

AActor* ACharacterMovementTestGameMode::FindPlayerStart_Implementation(AController* Player, const FString& IncomingName)
{
	SpawnRequests++;
#if WITH_EDITOR
	return PlayerStarts[SpawnRequests % PlayerStarts.Num()];
#endif
	return nullptr;
}
