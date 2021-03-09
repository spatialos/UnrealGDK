// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "Interop/LoadBalancingWriter.h"

#include "EngineClasses/SpatialActorChannel.h"
#include "EngineClasses/SpatialNetDriver.h"
#include "EngineClasses/SpatialPackageMapClient.h"
#include "LoadBalancing/AbstractLBStrategy.h"
#include "Utils/SpatialActorUtils.h"

namespace SpatialGDK
{
ActorSetMember LoadBalancingWriterActorSet::GetActorSetData(AActor* Actor) const
{
	const AActor* LeaderActor = GetReplicatedHierarchyRoot(Actor);
	check(IsValid(LeaderActor));

	const Worker_EntityId LeaderEntityId = PackageMap.GetEntityIdFromObject(LeaderActor);
	check(LeaderEntityId != SpatialConstants::INVALID_ENTITY_ID);

	return ActorSetMember(LeaderEntityId);
}

void LoadBalancingWriterActorSet::UpdateActorSetComponent(Worker_EntityId ActorEntityId, AActor* Actor) const
{
	Schema_ComponentUpdate* ptr = Schema_CopyComponentUpdate(GetActorSetData(Actor).CreateComponentUpdate().schema_type);
	SendComponentUpdate(ActorEntityId, ComponentUpdate(OwningComponentUpdatePtr(ptr), ActorSetMember::ComponentId));
}

void LoadBalancingWriterActorSet::SendComponentUpdate(Worker_EntityId EntityId, const ComponentUpdate& ComponentUpdate) const
{
	Coordinator.SendComponentUpdate(EntityId, ComponentUpdate.DeepCopy(), FSpatialGDKSpanId());
}

ActorGroupMember LoadBalancingWriterActorGroup::GetActorGroupData(AActor* Actor) const
{
	return ActorGroupMember(LoadBalancingStrategy.GetActorGroupId(*Actor));
}
} // namespace SpatialGDK
