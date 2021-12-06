// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "EngineClasses/SpatialPartitionSystem.h"
#include "EngineClasses/SpatialGameInstance.h"
#include "Interop/SpatialPartitionSystemImpl.h"
#include "SpatialGDKSettings.h"

#include "GeneralProjectSettings.h"

DEFINE_VTABLE_PTR_HELPER_CTOR(USpatialPartitionSystem);
USpatialPartitionSystem::USpatialPartitionSystem() = default;
USpatialPartitionSystem::~USpatialPartitionSystem() = default;

bool USpatialPartitionSystem::ShouldCreateSubsystem(UObject* Outer) const
{
	if (!GetDefault<UGeneralProjectSettings>()->UsesSpatialNetworking())
	{
		return false;
	}

	USpatialGameInstance* GameInstance = Cast<USpatialGameInstance>(Outer);
	if (!ensure(GameInstance != nullptr))
	{
		return false;
	}

	if (GameInstance->GetSpatialWorkerType() != SpatialConstants::DefaultServerWorkerType)
	{
		return false;
	}

	const USpatialGDKSettings* Settings = GetDefault<USpatialGDKSettings>();
	if (Settings->PartitionSystemClass.Get() == GetClass())
	{
		return true;
	}

	return false;
}

TArray<SpatialGDK::FLBDataStorage*> USpatialPartitionSystem::GetData()
{
	return TArray<SpatialGDK::FLBDataStorage*>();
}

TArray<SpatialGDK::FPartitionEvent> USpatialPartitionSystem::ConsumePartitionEvents()
{
	TArray<SpatialGDK::FPartitionEvent> EventsToConsume;
	if (Impl != nullptr)
	{
		Swap(EventsToConsume, Impl->Events);
	}
	return EventsToConsume;
}
