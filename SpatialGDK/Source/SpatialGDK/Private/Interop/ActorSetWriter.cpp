// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "Interop/ActorSetWriter.h"

#include "EngineClasses/SpatialPackageMapClient.h"
#include "Utils/SpatialActorUtils.h"

namespace SpatialGDK
{
ActorSetMember ActorSetWriter::GetActorSetData(AActor* Actor) const
{
	const AActor* LeaderActor = GetReplicatedHierarchyRoot(Actor);
	check(IsValid(LeaderActor));

	const Worker_EntityId LeaderEntityId = PackageMap.GetEntityIdFromObject(LeaderActor);
	check(LeaderEntityId != SpatialConstants::INVALID_ENTITY_ID);

	return ActorSetMember(LeaderEntityId);
}

void ActorSetWriter::UpdateActorSetComponent(Worker_EntityId ActorEntityId, AActor* Actor) const
{
	const ActorSetMember ActorSetData = GetActorSetData(Actor);

	ComponentUpdate ComponentUpdate(ActorSetData.ComponentId);
	Schema_Object* ComponentUpdateFields = Schema_GetComponentUpdateFields(ComponentUpdate.GetUnderlying());
	ActorSetData.WriteSchema(ComponentUpdateFields);

	Coordinator.SendComponentUpdate(ActorEntityId, MoveTemp(ComponentUpdate), {});
}
} // namespace SpatialGDK
