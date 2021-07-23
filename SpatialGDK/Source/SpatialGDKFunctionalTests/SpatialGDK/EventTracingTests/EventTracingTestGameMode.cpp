// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "EventTracingTestGameMode.h"

#include "SpatialGDKFunctionalTests/SpatialGDK/TestActors/EventTracingCharacter.h"

AEventTracingTestGameMode::AEventTracingTestGameMode(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	DefaultPawnClass = AEventTracingCharacter::StaticClass();
}
