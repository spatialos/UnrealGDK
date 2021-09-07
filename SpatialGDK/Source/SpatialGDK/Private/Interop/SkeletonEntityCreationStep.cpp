// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "SkeletonEntityCreationStep.h"

#include "EngineClasses/SpatialNetDriver.h"
#include "Interop/GlobalStateManager.h"

namespace SpatialGDK
{
DEFINE_LOG_CATEGORY_STATIC(LogSpatialSkeletonEntityCreationStep, Log, All);

bool FSkeletonEntityCreationStartupStep::TryFinish()
{
	if (!bWasInitialized)
	{
		bWasInitialized = true;
		Initialize();
	}

	bool bDidFinish = true;

	if (ServerEntityCreator)
	{
		ServerEntityCreator->Advance();
		bDidFinish = bDidFinish && ServerEntityCreator->IsReady();
	}

	if (EntityPopulator)
	{
		EntityPopulator->Advance();
		bDidFinish = bDidFinish && EntityPopulator->IsReady();
	}

	return bDidFinish;
}

void FSkeletonEntityCreationStartupStep::Initialize()
{
	if (NetDriver->GlobalStateManager->HasAuthority())
	{
		UE_LOG(LogSpatialSkeletonEntityCreationStep, Log, TEXT("Starting GSM-auth skeleton entity creation step"));

		ServerEntityCreator.Emplace(*NetDriver);
		ServerEntityCreator->CreateSkeletonEntitiesForWorld(*NetDriver->GetWorld());
	}

	{
		UE_LOG(LogSpatialSkeletonEntityCreationStep, Log, TEXT("Starting skeleton entity population step"));

		EntityPopulator.Emplace(*NetDriver);
	}
}

} // namespace SpatialGDK
