// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "SpatialCommonTypes.h"
#include "Subsystems/GameInstanceSubsystem.h"

#include "SpatialImposterSystem.generated.h"

class ISpatialOSWorker;
class USpatialNetDriver;

namespace SpatialGDK
{
class FLBDataStorage;
class FImposterSystemImpl;

struct FEntityEvent
{
	enum EventType
	{
		Created,
		Deleted,
		ToHiRes,
		ToLowRes
	};

	Worker_EntityId PartitionId;
	EventType Event;
};

} // namespace SpatialGDK

class USpatialNetDriver;

UCLASS(Abstract)
class SPATIALGDK_API USpatialImposterSystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	USpatialImposterSystem();
	USpatialImposterSystem(FVTableHelper&);
	~USpatialImposterSystem();

	virtual bool ShouldCreateSubsystem(UObject* Outer) const override;

	virtual TArray<SpatialGDK::FLBDataStorage*> GetData();

	TArray<SpatialGDK::FEntityEvent> ConsumeEntityEvents();

	void SetImpl(SpatialGDK::FImposterSystemImpl& InImpl) { Impl = &InImpl; }
	void ClearImpl() { Impl = nullptr; }

private:
	SpatialGDK::FImposterSystemImpl* Impl = nullptr;
};
