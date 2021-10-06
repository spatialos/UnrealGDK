// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "PartiallyStablePathGameMode.h"

#include "PartiallyStablePathPawn.h"

APartiallyStablePathGameMode::APartiallyStablePathGameMode()
{
	DefaultPawnClass = APartiallyStablePathPawn::StaticClass();
}
