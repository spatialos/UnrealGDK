// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "EngineClasses/SpatialImposterSystem.h"
#include "EngineClasses/SpatialGameInstance.h"
#include "Interop/SpatialImposterSystemImpl.h"
#include "SpatialGDKSettings.h"

DEFINE_VTABLE_PTR_HELPER_CTOR(USpatialImposterSystem);
USpatialImposterSystem::USpatialImposterSystem() = default;
USpatialImposterSystem::~USpatialImposterSystem() = default;

bool USpatialImposterSystem::ShouldCreateSubsystem(UObject* Outer) const
{
	USpatialGameInstance* GameInstance = Cast<USpatialGameInstance>(Outer);
	if (!ensure(GameInstance != nullptr))
	{
		return false;
	}

	if (GameInstance->GetSpatialWorkerType() != SpatialConstants::DefaultClientWorkerType)
	{
		return false;
	}

	const USpatialGDKSettings* Settings = GetDefault<USpatialGDKSettings>();
	if (Settings->ImposterSystemClass.Get() == GetClass())
	{
		return true;
	}

	return false;
}

TArray<SpatialGDK::FLBDataStorage*> USpatialImposterSystem::GetData()
{
	return TArray<SpatialGDK::FLBDataStorage*>();
}

TArray<SpatialGDK::FEntityEvent> USpatialImposterSystem::ConsumeEntityEvents()
{
	TArray<SpatialGDK::FEntityEvent> EventsToConsume;
	if (Impl != nullptr)
	{
		Swap(EventsToConsume, Impl->Events);
	}
	return EventsToConsume;
}
