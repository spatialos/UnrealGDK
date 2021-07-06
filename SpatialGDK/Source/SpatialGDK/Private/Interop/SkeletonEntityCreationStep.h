// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "Interop/SkeletonEntities.h"

class USpatialNetDriver;

namespace SpatialGDK
{
class FSkeletonEntityCreationStep
{
public:
	explicit FSkeletonEntityCreationStep(USpatialNetDriver& InNetDriver)
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
