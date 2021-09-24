// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "EngineClasses/SpatialPartitionSystem.h"
#include "LoadBalancing/LBDataStorage.h"

namespace SpatialGDK
{
class FSubView;
class FSpatialHandoverManager;

class FPartitionSystemImpl
{
public:
	FPartitionSystemImpl(const FSubView& SubView)
		: PartitionData(SubView)
	{
	}

	void Advance();
	void ProcessHandoverEvents(FSpatialHandoverManager&);
	void ProcessDeletionEvents();

	FLBDataCollection PartitionData;
	TArray<SpatialGDK::FPartitionEvent> Events;
};
} // namespace SpatialGDK
