// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "SkeletonEntityCreationStep.h"

#include "EngineClasses/SpatialNetDriver.h"
#include "Interop/GlobalStateManager.h"

namespace SpatialGDK
{
DEFINE_LOG_CATEGORY_STATIC(LogSpatialSkeletonEntityCreationStep, Log, All);

bool FSkeletonEntityCreationStep::TryFinish()
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

	if (bDidFinish)
	{
		UE_LOG(LogSpatialSkeletonEntityCreationStep, Log, TEXT("Finished skeleton entity creation"));
	}

	return bDidFinish;
}

void FSkeletonEntityCreationStep::Initialize()
{
	const ViewCoordinator& Coordinator = NetDriver->Connection->GetCoordinator();
	const Worker_EntityId GlobalStateManagerEntityId = NetDriver->GlobalStateManager->GlobalStateManagerEntityId;
	const EntityViewElement* GlobalStateManagerEntityPtr = Coordinator.GetView().Find(GlobalStateManagerEntityId);
	check(GlobalStateManagerEntityPtr != nullptr);
	if (GlobalStateManagerEntityPtr->Authority.Contains(SpatialConstants::GDK_KNOWN_ENTITY_AUTH_COMPONENT_SET_ID))
	{
		UE_LOG(LogSpatialSkeletonEntityCreationStep, Log, TEXT("Starting GSM-auth skeleton entity creation step"));

		ServerEntityCreator.Emplace(*NetDriver);
		ServerEntityCreator->CreateSkeletonEntitiesForWorld(*NetDriver->GetWorld());
	}

	{
		UE_LOG(LogSpatialSkeletonEntityCreationStep, Log, TEXT("Starting skeleton entity population step"));

		if (ServerEntityCreator)
		{
			EntityPopulator.Emplace(*NetDriver, [this](Worker_EntityId EntityId, const FSkeletonEntityManifest& UpdatedManifest) {
				ServerEntityCreator->HackAddManifest(EntityId, UpdatedManifest);
			});
		}
		else
		{
			EntityPopulator.Emplace(*NetDriver);
		}
	}
}

} // namespace SpatialGDK
