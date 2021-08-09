// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "LoadBalancing/LBDataStorage.h"
#include "Schema/ActorSetMember.h"

namespace SpatialGDK
{
class FActorSetSystem
{
public:
	void Update(TLBDataStorage<ActorSetMember>& Data, TSet<Worker_EntityId_Key>& DeletedEntities);

	Worker_EntityId GetSetLeader(Worker_EntityId Entity) const
	{
		const Worker_EntityId* Entry = ActorSetMembership.Find(Entity);
		return Entry != nullptr ? *Entry : SpatialConstants::INVALID_ENTITY_ID;
	}

	const TSet<Worker_EntityId_Key>* GetSet(Worker_EntityId Entity) const { return ActorSets.Find(Entity); }

	const TSet<Worker_EntityId_Key>& GetEntitiesToEvaluate() const { return EntitiesToEvaluate; }
	const TSet<Worker_EntityId_Key>& GetEntitiesToAttach() const { return EntitiesToAttach; }
	void Clear()
	{
		EntitiesToEvaluate.Empty();
		EntitiesToAttach.Empty();
	}

protected:
	TMap<Worker_EntityId_Key, TSet<Worker_EntityId_Key>> ActorSets;
	TMap<Worker_EntityId_Key, Worker_EntityId_Key> ActorSetMembership;
	TSet<Worker_EntityId_Key> EntitiesToEvaluate;
	TSet<Worker_EntityId_Key> EntitiesToAttach;
};
} // namespace SpatialGDK
