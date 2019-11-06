// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "TestGridBasedLBStrategy.h"

UGridBasedLBStrategy* UTestGridBasedLBStrategy::Create(uint32 InRows, uint32 InCols, float WorldWidth, float WorldHeight)
{
	UTestGridBasedLBStrategy* Strat = NewObject<UTestGridBasedLBStrategy>();

	Strat->Rows = InRows;
	Strat->Cols = InCols;

	Strat->WorldWidth = WorldWidth;
	Strat->WorldHeight = WorldHeight;

	return Strat;
}
