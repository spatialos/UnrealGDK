// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "Subsystems/GameInstanceSubsystem.h"

#include "SpatialPartitionSystem.generated.h"

class ISpatialOSWorker;
class USpatialNetDriver;

namespace SpatialGDK
{
class FLBDataStorage;
class FPartitionSystemImpl;
} // namespace SpatialGDK

UCLASS(Abstract)
class SPATIALGDK_API USpatialPartitionSystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	USpatialPartitionSystem();
	USpatialPartitionSystem(FVTableHelper&);
	~USpatialPartitionSystem();

	virtual bool ShouldCreateSubsystem(UObject* Outer) const override;

	virtual TArray<SpatialGDK::FLBDataStorage*> GetData();

protected:
};
