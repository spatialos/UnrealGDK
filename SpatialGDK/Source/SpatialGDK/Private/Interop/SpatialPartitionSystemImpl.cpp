// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "Interop/SpatialPartitionSystemImpl.h"
#include "EngineClasses/SpatialHandoverManager.h"

namespace SpatialGDK
{
void FPartitionSystemImpl::Advance()
{
	PartitionData.Advance();
	for (Worker_EntityId PartitionAdded : PartitionData.EntitiesAdded)
	{
		SpatialGDK::FPartitionEvent Event = { PartitionAdded, SpatialGDK::FPartitionEvent::Created };
		Events.Add(Event);
	}
	PartitionData.EntitiesAdded.Empty();
}

void FPartitionSystemImpl::ProcessHandoverEvents(FSpatialHandoverManager& HandoverManager)
{
	for (Worker_EntityId Partition : HandoverManager.GetDelegationLost())
	{
		SpatialGDK::FPartitionEvent Event = { Partition, SpatialGDK::FPartitionEvent::DelegationLost };
		Events.Add(Event);
	}
	for (Worker_EntityId Partition : HandoverManager.GetDelegatedPartitions())
	{
		SpatialGDK::FPartitionEvent Event = { Partition, SpatialGDK::FPartitionEvent::Delegated };
		Events.Add(Event);
	}
}

void FPartitionSystemImpl::ProcessDeletionEvents()
{
	for (Worker_EntityId PartitionAdded : PartitionData.EntitiesRemoved)
	{
		SpatialGDK::FPartitionEvent Event = { PartitionAdded, SpatialGDK::FPartitionEvent::Deleted };
		Events.Add(Event);
	}
	PartitionData.EntitiesRemoved.Empty();
}

} // namespace SpatialGDK
