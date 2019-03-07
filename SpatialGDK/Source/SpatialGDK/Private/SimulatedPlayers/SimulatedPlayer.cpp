// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "SimulatedPlayer.h"

#include "Misc/Parse.h"
#include "Misc/CommandLine.h"
#include "SpatialConstants.h"


bool USimulatedPlayer::IsSimulatedPlayer()
{
	// TODO(wouter) add support for having a PIE simulated player (can't use command line)
	return FParse::Param(FCommandLine::Get(), *SpatialConstants::SimulatedPlayerArg);
}
