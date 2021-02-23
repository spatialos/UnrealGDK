// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "Test1x2GridStrategy.h"

UTest1x2GridStrategy::UTest1x2GridStrategy()
{
	Cols = 2;
	// Maximum of the world size, so that the entire world is in the view of both the server-workers at all times
	InterestBorder = FMath::Max(WorldWidth, WorldHeight);
}
