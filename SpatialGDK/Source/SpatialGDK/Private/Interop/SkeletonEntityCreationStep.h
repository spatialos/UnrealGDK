// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "Interop/SkeletonEntities.h"

class USpatialNetDriver;

namespace SpatialGDK
{
// This class controls skeleton entity creation and population, intended to be called from
// USpatialNetDriver::TryFinishStartup after the GSM authority has been established.
class FSkeletonEntityCreationStartupStep
{
public:
	explicit FSkeletonEntityCreationStartupStep(USpatialNetDriver& InNetDriver)
		: NetDriver(&InNetDriver)
	{
	}

	bool TryFinish();

private:
	void Initialize();

	USpatialNetDriver* NetDriver;
	bool bWasInitialized = false;
	TOptional<FDistributedStartupActorSkeletonEntityCreator> ServerEntityCreator;
	TOptional<FSkeletonEntityPopulator> EntityPopulator;
};
} // namespace SpatialGDK
