// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "SpatialView/ViewDelta.h"

#include "SpatialView/EntityComponentTypes.h"
#include "SpatialView/ViewDelta/IntermediateReceivedViewChange.h"

#include <algorithm>

namespace SpatialGDK
{
void ViewDelta::SetFromOpList(TArray<OpList> OpLists, EntityView& View, const FComponentSetData& ComponentSetData)
{
	Clear();
	ViewChange.SetFromOpList(MoveTemp(OpLists), View, ComponentSetData);
	PopulateEntityDeltas(View);
}

void ViewDelta::Project(FSubViewDelta& SubDelta, const TArray<Worker_EntityId>& CompleteEntities,
						const TArray<Worker_EntityId>& NewlyCompleteEntities, const TArray<Worker_EntityId>& NewlyIncompleteEntities,
						const TArray<Worker_EntityId>& TemporarilyIncompleteEntities) const
{
	SubDelta.EntityDeltas.Empty();

	// No projection is applied to worker messages, as they are not entity specific.
	SubDelta.WorkerMessages = &ViewChange.WorkerMessages;

	// All arrays here are sorted by entity ID.
	auto DeltaIt = EntityDeltas.CreateConstIterator();
	auto CompleteIt = CompleteEntities.CreateConstIterator();
	auto NewlyCompleteIt = NewlyCompleteEntities.CreateConstIterator();
	auto NewlyIncompleteIt = NewlyIncompleteEntities.CreateConstIterator();
	auto TemporarilyIncompleteIt = TemporarilyIncompleteEntities.CreateConstIterator();

	for (;;)
	{
		const Worker_EntityId DeltaId = DeltaIt ? DeltaIt->EntityId : SENTINEL_ENTITY_ID;
		const Worker_EntityId CompleteId = CompleteIt ? *CompleteIt : SENTINEL_ENTITY_ID;
		const Worker_EntityId NewlyCompleteId = NewlyCompleteIt ? *NewlyCompleteIt : SENTINEL_ENTITY_ID;
		const Worker_EntityId NewlyIncompleteId = NewlyIncompleteIt ? *NewlyIncompleteIt : SENTINEL_ENTITY_ID;
		const Worker_EntityId TemporarilyIncompleteId = TemporarilyIncompleteIt ? *TemporarilyIncompleteIt : SENTINEL_ENTITY_ID;
		const uint64 MinEntityId = FMath::Min3(FMath::Min(static_cast<uint64>(DeltaId), static_cast<uint64>(CompleteId)),
											   FMath::Min(static_cast<uint64>(NewlyCompleteId), static_cast<uint64>(NewlyIncompleteId)),
											   static_cast<uint64>(TemporarilyIncompleteId));
		const Worker_EntityId CurrentEntityId = static_cast<Worker_EntityId>(MinEntityId);
		// If no list has elements left to read then stop.
		if (CurrentEntityId == SENTINEL_ENTITY_ID)
		{
			break;
		}

		// Find the intersection between complete entities and the entity IDs in the view delta, add them to this
		// delta.
		if (CompleteId == CurrentEntityId && DeltaId == CurrentEntityId)
		{
			EntityDelta CompleteDelta = *DeltaIt;
			if (TemporarilyIncompleteId == CurrentEntityId)
			{
				// This is a delta for a complete entity which was also temporarily removed. Change its type to
				// reflect that.
				CompleteDelta.Type = EntityDelta::TEMPORARILY_REMOVED;
				++TemporarilyIncompleteIt;
			}
			SubDelta.EntityDeltas.Emplace(CompleteDelta);
		}
		// Temporarily incomplete entities which aren't present in the projecting view delta are represented as marker
		// temporarily removed entities with no state.
		else if (TemporarilyIncompleteId == CurrentEntityId)
		{
			SubDelta.EntityDeltas.Emplace(EntityDelta{ CurrentEntityId, EntityDelta::TEMPORARILY_REMOVED });
			++TemporarilyIncompleteIt;
		}
		// Newly complete entities are represented as marker add entities with no state.
		else if (NewlyCompleteId == CurrentEntityId)
		{
			SubDelta.EntityDeltas.Emplace(EntityDelta{ CurrentEntityId, EntityDelta::ADD });
			++NewlyCompleteIt;
		}
		// Newly incomplete entities are represented as marker remove entities with no state.
		else if (NewlyIncompleteId == CurrentEntityId)
		{
			SubDelta.EntityDeltas.Emplace(EntityDelta{ CurrentEntityId, EntityDelta::REMOVE });
			++NewlyIncompleteIt;
		}

		// Logic for incrementing complete and delta iterators. If either iterator is done, null the other,
		// as there can no longer be any intersection.
		if (CompleteId == CurrentEntityId)
		{
			++CompleteIt;
			if (!CompleteIt)
			{
				DeltaIt.SetToEnd();
			}
		}
		if (DeltaId == CurrentEntityId)
		{
			++DeltaIt;
			if (!DeltaIt)
			{
				CompleteIt.SetToEnd();
			}
		}
	}
}

void ViewDelta::Clear()
{
	EntityDeltas.Reset();
	AuthorityGainedForDelta.Reset();
	AuthorityLostForDelta.Reset();
	AuthorityLostTempForDelta.Reset();
	ComponentsAddedForDelta.Reset();
	ComponentsRemovedForDelta.Reset();
	ComponentUpdatesForDelta.Reset();
	ComponentsRefreshedForDelta.Reset();
}

const TArray<EntityDelta>& ViewDelta::GetEntityDeltas() const
{
	return EntityDeltas;
}

const TArray<Worker_Op>& ViewDelta::GetWorkerMessages() const
{
	return ViewChange.WorkerMessages;
}

bool ViewDelta::HasConnectionStatusChanged() const
{
	return ViewChange.ConnectionStatusCode != 0;
}

Worker_ConnectionStatusCode ViewDelta::GetConnectionStatusChange() const
{
	check(HasConnectionStatusChanged());
	return static_cast<enum Worker_ConnectionStatusCode>(ViewChange.ConnectionStatusCode);
}

FString ViewDelta::GetConnectionStatusChangeMessage() const
{
	check(HasConnectionStatusChanged());
	return ViewChange.ConnectionStatusMessage;
}

ComponentChange ViewDelta::CalculateAdd(ReceivedComponentChange* Start, ReceivedComponentChange* End, TArray<ComponentData>& Components)
{
	// There must be at least one component add; anything before it can be ignored.
	const ReceivedComponentChange* It = std::find_if(Start, End, [](const ReceivedComponentChange& Op) {
		return Op.Type == ReceivedComponentChange::ADD;
	});

	Schema_ComponentData* Data = It->ComponentAdded;
	++It;

	while (It != End)
	{
		switch (It->Type)
		{
		case ReceivedComponentChange::ADD:
			Data = It->ComponentAdded;
			break;
		case ReceivedComponentChange::UPDATE:
			Schema_ApplyComponentUpdateToData(It->ComponentUpdate, Data);
			break;
		case ReceivedComponentChange::REMOVE:
			break;
		}
		++It;
	}
	Components.Emplace(ComponentData::CreateCopy(Data, Start->ComponentId));
	// We don't want to reference the component in the view as is isn't stable.
	return ComponentChange(Start->ComponentId, Data);
}

ComponentChange ViewDelta::CalculateCompleteUpdate(ReceivedComponentChange* Start, ReceivedComponentChange* End, Schema_ComponentData* Data,
												   Schema_ComponentUpdate* Events, ComponentData& Component)
{
	for (auto It = Start; It != End; ++It)
	{
		switch (It->Type)
		{
		case ReceivedComponentChange::ADD:
			Data = It->ComponentAdded;
			break;
		case ReceivedComponentChange::UPDATE:
			if (Data)
			{
				Schema_ApplyComponentUpdateToData(It->ComponentUpdate, Data);
			}
			if (Events)
			{
				Schema_MergeComponentUpdateIntoUpdate(It->ComponentUpdate, Events);
			}
			else
			{
				Events = It->ComponentUpdate;
			}
			break;
		case ReceivedComponentChange::REMOVE:
			break;
		}
	}

	Component = ComponentData::CreateCopy(Data, Start->ComponentId);
	Schema_Object* EventsObj = Events ? Schema_GetComponentUpdateEvents(Events) : nullptr;
	// Use the data from the op list as pointers from the view aren't stable.
	return ComponentChange(Start->ComponentId, Data, EventsObj);
}

ComponentChange ViewDelta::CalculateUpdate(ReceivedComponentChange* Start, ReceivedComponentChange* End, ComponentData& Component)
{
	// For an update we don't know if we are calculating a complete-update or a regular update.
	// So the first message processed might be an add or an update.
	auto It = std::find_if(Start, End, [](const ReceivedComponentChange& Op) {
		return Op.Type != ReceivedComponentChange::REMOVE;
	});

	// If the first message is an add then calculate a complete-update.
	if (It->Type == ReceivedComponentChange::ADD)
	{
		return CalculateCompleteUpdate(It + 1, End, It->ComponentAdded, nullptr, Component);
	}

	Schema_ComponentUpdate* Update = It->ComponentUpdate;
	++It;
	while (It != End)
	{
		switch (It->Type)
		{
		case ReceivedComponentChange::ADD:
			return CalculateCompleteUpdate(It + 1, End, It->ComponentAdded, Update, Component);
		case ReceivedComponentChange::UPDATE:
			Schema_MergeComponentUpdateIntoUpdate(It->ComponentUpdate, Update);
			break;
		case ReceivedComponentChange::REMOVE:
			return CalculateCompleteUpdate(It + 1, End, nullptr, nullptr, Component);
		}
		++It;
	}

	Schema_ApplyComponentUpdateToData(Update, Component.GetUnderlying());
	Component = Component.DeepCopy();
	return ComponentChange(Start->ComponentId, Update);
}

void ViewDelta::PopulateEntityDeltas(EntityView& View)
{
	TArray<ReceivedComponentChange>& ComponentChanges = ViewChange.ComponentChanges;
	TArray<ReceivedAuthorityChange>& AuthorityChanges = ViewChange.AuthorityChanges;
	TArray<ReceivedEntityChange>& EntityChanges = ViewChange.EntityChanges;

	// Make sure there is enough space in the view delta storage.
	// This allows us to rely on stable pointers as we add new elements.
	ComponentsAddedForDelta.Reserve(ComponentChanges.Num());
	ComponentsRemovedForDelta.Reserve(ComponentChanges.Num());
	ComponentUpdatesForDelta.Reserve(ComponentChanges.Num());
	ComponentsRefreshedForDelta.Reserve(ComponentChanges.Num());
	AuthorityGainedForDelta.Reserve(AuthorityChanges.Num());
	AuthorityLostForDelta.Reserve(AuthorityChanges.Num());
	AuthorityLostTempForDelta.Reserve(AuthorityChanges.Num());

	// Add sentinel elements to the ends of the arrays.
	// Prevents the need for bounds checks on iterators over the change lists.
	ViewChange.ComponentChanges.Emplace(Worker_RemoveComponentOp{ SENTINEL_ENTITY_ID, 0 });
	AuthorityChanges.Push(ReceivedAuthorityChange{ SENTINEL_ENTITY_ID, 0, false });
	EntityChanges.Push(ReceivedEntityChange{ SENTINEL_ENTITY_ID, false });

	auto ComponentIt = ComponentChanges.GetData();
	auto AuthorityIt = AuthorityChanges.GetData();
	auto EntityIt = EntityChanges.GetData();

	ReceivedComponentChange* ComponentChangesEnd = ComponentIt + ComponentChanges.Num();
	ReceivedAuthorityChange* AuthorityChangesEnd = AuthorityIt + AuthorityChanges.Num();
	ReceivedEntityChange* EntityChangesEnd = EntityIt + EntityChanges.Num();

	// At the beginning of each loop each iterator should point to the first element for an entity.
	// Each loop we want to work with a single entity ID.
	// We check the entities each iterator is pointing to and pick the smallest one.
	// If that is the sentinel ID then stop.
	for (;;)
	{
		// Get the next entity ID. We want to pick the smallest entity referenced by the iterators.
		// Convert to uint64 to ensure the sentinel value is larger than all valid IDs.
		const uint64 MinEntityId = FMath::Min3(static_cast<uint64>(ComponentIt->EntityId), static_cast<uint64>(AuthorityIt->EntityId),
											   static_cast<uint64>(EntityIt->EntityId));

		// If no list has elements left to read then stop.
		if (static_cast<Worker_EntityId>(MinEntityId) == SENTINEL_ENTITY_ID)
		{
			break;
		}

		const Worker_EntityId CurrentEntityId = static_cast<Worker_EntityId>(MinEntityId);

		EntityDelta Delta = {};
		Delta.EntityId = CurrentEntityId;

		EntityViewElement* ViewElement = View.Find(CurrentEntityId);
		const bool bAlreadyExisted = ViewElement != nullptr;

		if (ViewElement == nullptr)
		{
			ViewElement = &View.Add(CurrentEntityId);
		}

		if (ComponentIt->EntityId == CurrentEntityId)
		{
			ComponentIt = ProcessEntityComponentChanges(ComponentIt, ComponentChangesEnd, ViewElement->Components, Delta);
		}

		if (AuthorityIt->EntityId == CurrentEntityId)
		{
			AuthorityIt = ProcessEntityAuthorityChanges(AuthorityIt, AuthorityChangesEnd, ViewElement->Authority, Delta);
		}

		if (EntityIt->EntityId == CurrentEntityId)
		{
			EntityIt = ProcessEntityExistenceChange(EntityIt, EntityChangesEnd, Delta, bAlreadyExisted, View);
			// Did the entity flicker into view for less than a tick.
			if (Delta.Type == EntityDelta::UPDATE && !bAlreadyExisted)
			{
				View.Remove(CurrentEntityId);
				continue;
			}
		}

		EntityDeltas.Push(Delta);
	}
}

ReceivedComponentChange* ViewDelta::ProcessEntityComponentChanges(ReceivedComponentChange* It, ReceivedComponentChange* End,
																  TArray<ComponentData>& Components, EntityDelta& Delta)
{
	int32 AddCount = 0;
	int32 UpdateCount = 0;
	int32 RemoveCount = 0;
	int32 RefreshCount = 0;

	const Worker_EntityId EntityId = It->EntityId;
	// At the end of each loop `It` should point to the first element for an entity-component.
	// Stop and return when the component is for a different entity.
	// There will always be at least one iteration of the loop.
	for (;;)
	{
		ReceivedComponentChange* NextComponentIt = std::find_if(It, End, DifferentEntityComponent{ EntityId, It->ComponentId });

		ComponentData* Component = Components.FindByPredicate(ComponentIdEquality{ It->ComponentId });
		const bool bComponentExists = Component != nullptr;

		// The element one before NextComponentIt must be the last element for this component.
		switch ((NextComponentIt - 1)->Type)
		{
		case ReceivedComponentChange::ADD:
			if (bComponentExists)
			{
				ComponentsRefreshedForDelta.Emplace(CalculateCompleteUpdate(It, NextComponentIt, nullptr, nullptr, *Component));
				++RefreshCount;
			}
			else
			{
				ComponentsAddedForDelta.Emplace(CalculateAdd(It, NextComponentIt, Components));
				++AddCount;
			}
			break;
		case ReceivedComponentChange::UPDATE:
			if (bComponentExists)
			{
				ComponentChange Update = CalculateUpdate(It, NextComponentIt, *Component);
				if (Update.Type == ComponentChange::COMPLETE_UPDATE)
				{
					ComponentsRefreshedForDelta.Emplace(Update);
					++RefreshCount;
				}
				else
				{
					ComponentUpdatesForDelta.Emplace(Update);
					++UpdateCount;
				}
			}
			else
			{
				ComponentsAddedForDelta.Emplace(CalculateAdd(It, NextComponentIt, Components));
				++AddCount;
			}
			break;
		case ReceivedComponentChange::REMOVE:
			if (bComponentExists)
			{
				ComponentsRemovedForDelta.Emplace(It->ComponentId);
				Components.RemoveAtSwap(Component - Components.GetData());
				++RemoveCount;
			}
			break;
		}

		if (NextComponentIt->EntityId != EntityId)
		{
			Delta.ComponentsAdded = { ComponentsAddedForDelta.GetData() + ComponentsAddedForDelta.Num() - AddCount, AddCount };
			Delta.ComponentsRemoved = { ComponentsRemovedForDelta.GetData() + ComponentsRemovedForDelta.Num() - RemoveCount, RemoveCount };
			Delta.ComponentUpdates = { ComponentUpdatesForDelta.GetData() + ComponentUpdatesForDelta.Num() - UpdateCount, UpdateCount };
			Delta.ComponentsRefreshed = { ComponentsRefreshedForDelta.GetData() + ComponentsRefreshedForDelta.Num() - RefreshCount,
										  RefreshCount };
			return NextComponentIt;
		}

		It = NextComponentIt;
	}
}

ReceivedAuthorityChange* ViewDelta::ProcessEntityAuthorityChanges(ReceivedAuthorityChange* It, ReceivedAuthorityChange* End,
																  TArray<Worker_ComponentSetId>& EntityAuthority, EntityDelta& Delta)
{
	int32 GainCount = 0;
	int32 LossCount = 0;
	int32 LossTempCount = 0;

	const Worker_EntityId EntityId = It->EntityId;
	// After each loop the iterator points to the first op relating to the next entity-component.
	// Stop and return when that component is for a different entity.
	// There will always be at least one iteration of the loop.
	for (;;)
	{
		// Find the last element for this entity-component.
		const Worker_ComponentSetId ComponentSetId = It->ComponentSetId;
		It = std::find_if(It, End, DifferentEntityComponentSet{ EntityId, ComponentSetId }) - 1;
		const int32 AuthorityIndex = EntityAuthority.Find(ComponentSetId);
		const bool bHasAuthority = AuthorityIndex != INDEX_NONE;

		if (It->bNewAuth)
		{
			if (bHasAuthority)
			{
				AuthorityLostTempForDelta.Emplace(ComponentSetId, AuthorityChange::AUTHORITY_LOST_TEMPORARILY);
				++LossTempCount;
			}
			else
			{
				EntityAuthority.Push(ComponentSetId);
				AuthorityGainedForDelta.Emplace(ComponentSetId, AuthorityChange::AUTHORITY_GAINED);
				++GainCount;
			}
		}
		else if (bHasAuthority)
		{
			AuthorityLostForDelta.Emplace(ComponentSetId, AuthorityChange::AUTHORITY_LOST);
			EntityAuthority.RemoveAtSwap(AuthorityIndex);
			++LossCount;
		}

		// Move to the next entity-component.
		++It;

		if (It->EntityId != EntityId)
		{
			Delta.AuthorityGained = { AuthorityGainedForDelta.GetData() + AuthorityGainedForDelta.Num() - GainCount, GainCount };
			Delta.AuthorityLost = { AuthorityLostForDelta.GetData() + AuthorityLostForDelta.Num() - LossCount, LossCount };
			Delta.AuthorityLostTemporarily = { AuthorityLostTempForDelta.GetData() + AuthorityLostTempForDelta.Num() - LossTempCount,
											   LossTempCount };
			return It;
		}
	}
}

ReceivedEntityChange* ViewDelta::ProcessEntityExistenceChange(ReceivedEntityChange* It, ReceivedEntityChange* End, EntityDelta& Delta,
															  bool bAlreadyInView, EntityView& View)
{
	// Find the last element relating to the same entity.
	const Worker_EntityId EntityId = It->EntityId;
	It = std::find_if(It, End, DifferentEntity{ EntityId }) - 1;

	const bool bEntityAdded = It->bAdded;

	// If the entity's presence has not changed then it's an update.
	if (bEntityAdded == bAlreadyInView)
	{
		Delta.Type = EntityDelta::UPDATE;
		return It + 1;
	}

	if (bEntityAdded)
	{
		Delta.Type = EntityDelta::ADD;
	}
	else
	{
		Delta.Type = EntityDelta::REMOVE;
		View.Remove(EntityId);
	}

	return It + 1;
}

} // namespace SpatialGDK
