// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "LoadBalancing/ActorSetSystem.h"

namespace SpatialGDK
{
void FActorSetSystem::Update(TLBDataStorage<ActorSetMember>& Data, TSet<Worker_EntityId_Key>& DeletedEntities)
{
	for (Worker_EntityId Modified : Data.GetModifiedEntities())
	{
		const ActorSetMember* Membership = Data.GetObjects().Find(Modified);
		if (!ensureAlways(Membership != nullptr))
		{
			continue;
		}

		Worker_EntityId NewSetLeader = Membership->ActorSetId;
		if (DeletedEntities.Contains(NewSetLeader) || NewSetLeader == Modified)
		{
			NewSetLeader = SpatialConstants::INVALID_ENTITY_ID;
		}

		// The assumption is that we are given valid set leaders (which are not themselves in another set).
		TSet<Worker_EntityId_Key>* NewSet =
			NewSetLeader != SpatialConstants::INVALID_ENTITY_ID ? &ActorSets.FindOrAdd(NewSetLeader) : nullptr;

		Worker_EntityId_Key* PreviousSetLeader = ActorSetMembership.Find(Modified);

		if (NewSetLeader == SpatialConstants::INVALID_ENTITY_ID && PreviousSetLeader == nullptr
			|| (PreviousSetLeader != nullptr && *PreviousSetLeader == NewSetLeader))
		{
			// no change
			continue;
		}

		TSet<Worker_EntityId_Key>* PreviousSet = PreviousSetLeader != nullptr ? ActorSets.Find(*PreviousSetLeader) : nullptr;

		if (PreviousSet != nullptr)
		{
			PreviousSet->Remove(Modified);
			if (PreviousSet->Num() == 0)
			{
				ActorSets.Remove(*PreviousSetLeader);
			}
		}

		if (NewSetLeader == SpatialConstants::INVALID_ENTITY_ID)
		{
			EntitiesToEvaluate.Add(Modified);
			ActorSetMembership.Remove(Modified);
		}
		else
		{
			ActorSetMembership.Add(Modified, NewSetLeader);
			ActorSets.Remove(Modified);
		}

		if (NewSet && !DeletedEntities.Contains(NewSetLeader))
		{
			NewSet->Add(Modified);
			EntitiesToEvaluate.Add(NewSetLeader);
			EntitiesToAttach.Add(Modified);
		}
	}

	for (Worker_EntityId Deleted : DeletedEntities)
	{
		TSet<Worker_EntityId_Key> SetToClear;
		if (ActorSets.RemoveAndCopyValue(Deleted, SetToClear))
		{
			EntitiesToEvaluate.Append(SetToClear);
		}
		EntitiesToEvaluate.Remove(Deleted);
		EntitiesToAttach.Remove(Deleted);

		Worker_EntityId_Key CurrentSetLeader;
		if (ActorSetMembership.RemoveAndCopyValue(Deleted, CurrentSetLeader))
		{
			if (auto* CurrentSet = ActorSets.Find(CurrentSetLeader))
			{
				CurrentSet->Remove(Deleted);
				if (CurrentSet->Num() == 0)
				{
					ActorSets.Remove(CurrentSetLeader);
				}
			}
		}
	}
}

} // namespace SpatialGDK
