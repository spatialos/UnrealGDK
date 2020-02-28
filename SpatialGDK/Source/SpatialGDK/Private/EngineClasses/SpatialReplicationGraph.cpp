// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "EngineClasses/SpatialReplicationGraph.h"

#include "EngineClasses/SpatialActorChannel.h"
#include "EngineClasses/SpatialNetDriver.h"


UActorChannel* USpatialReplicationGraph::GetOrCreateSpatialActorChannel(UObject* TargetObject)
{
	if (TargetObject != nullptr)
	{
		if (USpatialNetDriver* SpatialNetDriver = Cast<USpatialNetDriver>(NetDriver))
		{
			return SpatialNetDriver->GetOrCreateSpatialActorChannel(TargetObject);
		}

		checkNoEntry();
	}

	return nullptr;
}
