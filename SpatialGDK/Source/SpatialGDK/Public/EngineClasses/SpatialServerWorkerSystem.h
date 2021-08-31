// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "SpatialCommonTypes.h"
#include "SpatialView/ComponentData.h"
#include "Subsystems/GameInstanceSubsystem.h"

#include "SpatialServerWorkerSystem.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogSpatialServerWorkerSystem, Log, All);

namespace SpatialGDK
{
class FServerWorkerSystemImpl;
} // namespace SpatialGDK

class USpatialNetDriver;

UCLASS(Abstract)
class SPATIALGDK_API USpatialServerWorkerSystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	USpatialServerWorkerSystem();
	USpatialServerWorkerSystem(FVTableHelper&);
	~USpatialServerWorkerSystem();

	virtual bool ShouldCreateSubsystem(UObject* Outer) const override;

	void UpdateServerWorkerData(TArray<SpatialGDK::ComponentUpdate> Updates);

	void SetImpl(SpatialGDK::FServerWorkerSystemImpl& InImpl);
	void ClearImpl() { Impl = nullptr; }

private:
	virtual TArray<SpatialGDK::ComponentData> GetServerWorkerInitialData()
		PURE_VIRTUAL(GetServerWorkerData, return TArray<SpatialGDK::ComponentData>();)

			SpatialGDK::FServerWorkerSystemImpl* Impl = nullptr;
};