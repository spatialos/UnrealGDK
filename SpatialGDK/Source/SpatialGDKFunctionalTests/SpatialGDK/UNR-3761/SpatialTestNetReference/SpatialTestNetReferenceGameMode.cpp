// Copyright (c) Improbable Worlds Ltd, All Rights Reserved


#include "SpatialTestNetReferenceGameMode.h"
#include "SpatialGDKFunctionalTests/SpatialGDK/SpatialTestCharacterMovement/TestMovementCharacter.h"

ASpatialTestNetReferenceGameMode::ASpatialTestNetReferenceGameMode(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	DefaultPawnClass = ATestMovementCharacter::StaticClass();
}
