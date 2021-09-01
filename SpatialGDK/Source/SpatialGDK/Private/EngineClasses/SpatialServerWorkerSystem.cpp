// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "EngineClasses/SpatialServerWorkerSystem.h"
#include "EngineClasses/SpatialGameInstance.h"
#include "Interop/SpatialServerWorkerSystemImpl.h"
#include "SpatialGDKSettings.h"

DEFINE_LOG_CATEGORY(LogSpatialServerWorkerSystem);

DEFINE_VTABLE_PTR_HELPER_CTOR(USpatialServerWorkerSystem);
USpatialServerWorkerSystem::USpatialServerWorkerSystem() = default;
USpatialServerWorkerSystem::~USpatialServerWorkerSystem() = default;

bool USpatialServerWorkerSystem::ShouldCreateSubsystem(UObject* Outer) const
{
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
	if (Settings->ServerWorkerSystemClass.Get() == GetClass())
	{
		return true;
	}

	return false;
}

void USpatialServerWorkerSystem::UpdateServerWorkerData(TArray<SpatialGDK::ComponentUpdate> Updates)
{
	if (Impl == nullptr)
	{
		return;
	}

	for (auto& Update : Updates)
	{
		if (!Impl->ServerWorkerComponents.Contains(Update.GetComponentId()))
		{
			UE_LOG(LogSpatialServerWorkerSystem, Error,
				   TEXT("Cannot update component not initially present on the server worker entity, ComponentId : %i"),
				   Update.GetComponentId());
			continue;
		}
		SpatialGDK::ComponentUpdate* ExistingUpdate =
			Impl->PendingComponentUpdates.FindByPredicate([&Update](const SpatialGDK::ComponentUpdate& PendingUpdate) {
				return PendingUpdate.GetComponentId() == Update.GetComponentId();
			});
		if (ExistingUpdate != nullptr)
		{
			ExistingUpdate->Merge(MoveTemp(Update));
		}
		else
		{
			Impl->PendingComponentUpdates.Add(MoveTemp(Update));
		}
	}
}

void USpatialServerWorkerSystem::SetImpl(SpatialGDK::FServerWorkerSystemImpl& InImpl)
{
	Impl = &InImpl;
	Impl->InitialData = GetServerWorkerInitialData();
}
