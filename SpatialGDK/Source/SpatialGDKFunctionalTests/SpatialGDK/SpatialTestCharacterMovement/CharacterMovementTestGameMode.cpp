// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "CharacterMovementTestGameMode.h"
#include "TestMovementCharacter.h"

ACharacterMovementTestGameMode::ACharacterMovementTestGameMode(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	DefaultPawnClass = ATestMovementCharacter::StaticClass();
}
