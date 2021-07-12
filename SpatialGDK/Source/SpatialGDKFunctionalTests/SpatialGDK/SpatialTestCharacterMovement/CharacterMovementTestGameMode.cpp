// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "CharacterMovementTestGameMode.h"

#include "EngineUtils.h"
#include "GameFramework/PlayerStart.h"
#include "SpatialGDKFunctionalTests/SpatialGDK/TestActors/TestMovementCharacter.h"

ACharacterMovementTestGameMode::ACharacterMovementTestGameMode(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	DefaultPawnClass = ATestMovementCharacter::StaticClass();
}

void ACharacterMovementTestGameMode::BeginPlay()
{
	Super::BeginPlay();
	for (TActorIterator<APlayerStart> Iter(GetWorld()); Iter; ++Iter)
	{
		PlayerStarts.Add(*Iter);
	}
}

AActor* ACharacterMovementTestGameMode::FindPlayerStart_Implementation(AController* Player, const FString& IncomingName)
{
	SpawnRequests++;
	return PlayerStarts[SpawnRequests % PlayerStarts.Num()];
}
