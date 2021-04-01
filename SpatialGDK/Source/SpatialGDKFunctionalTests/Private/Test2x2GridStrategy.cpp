// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "Test2x2GridStrategy.h"

UTest2x2GridStrategy::UTest2x2GridStrategy()
{
	Rows = 2;
	Cols = 2;
	// Maximum of the world size, so that the entire world is in the view of both the server-workers at all times
	InterestBorder = FMath::Max(WorldWidth, WorldHeight);
}
