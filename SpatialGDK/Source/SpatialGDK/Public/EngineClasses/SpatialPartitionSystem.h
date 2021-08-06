// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "SpatialCommonTypes.h"
#include "Subsystems/GameInstanceSubsystem.h"

#include "SpatialPartitionSystem.generated.h"

class ISpatialOSWorker;
class USpatialNetDriver;

namespace SpatialGDK
{
class FLBDataStorage;
class FPartitionSystemImpl;

struct FPartitionEvent
{
	enum EventType
	{
		Created,
		Deleted,
		Delegated,
		DelegationLost
	};

	Worker_EntityId PartitionId;
	EventType Event;
};

} // namespace SpatialGDK

class USpatialNetDriver;

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

	TArray<SpatialGDK::FPartitionEvent> ConsumePartitionEvents();

	void SetImpl(SpatialGDK::FPartitionSystemImpl& InImpl) { Impl = &InImpl; }
	void ClearImpl() { Impl = nullptr; }

private:
	SpatialGDK::FPartitionSystemImpl* Impl = nullptr;
};
