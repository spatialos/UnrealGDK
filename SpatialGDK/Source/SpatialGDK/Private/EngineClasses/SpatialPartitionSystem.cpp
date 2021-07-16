// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "EngineClasses/SpatialPartitionSystem.h"
#include "Interop/SpatialPartitionSystemImpl.h"
#include "SpatialGDKSettings.h"

DEFINE_VTABLE_PTR_HELPER_CTOR(USpatialPartitionSystem);
USpatialPartitionSystem::USpatialPartitionSystem() = default;
USpatialPartitionSystem::~USpatialPartitionSystem() = default;

bool USpatialPartitionSystem::ShouldCreateSubsystem(UObject* Outer) const
{
	// UGameInstance* GameInstanceMaybe = Cast<UGameInstance>(Outer);
	// if (GameInstanceMaybe == nullptr)
	//{
	//	UE_LOG(LogTemp, Log, TEXT("What now ?"));
	//}
	// else
	//{
	//	if(GameInstanceMaybe->GetSpatialWorkerType() != SpatialConstants::DefaultServerWorkerType)
	//	{
	//		return false;
	//	}
	//}

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
