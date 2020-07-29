// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "SpatialView/ViewDelta.h"
#include "SpatialView/EntityComponentTypes.h"

#include "Algo/StableSort.h"
#include <algorithm>

namespace SpatialGDK
{

void ViewDelta::SetFromOpList(TArray<OpList> OpLists, EntityView& View)
{
	Clear();

	for (OpList& Ops : OpLists)
	{
		const uint32 Count = Ops.Count;
		Worker_Op* OpData = Ops.Ops;
		for (uint32 i = 0; i < Count; ++i)
		{
			ProcessOp(OpData[i]);
		}
	}
	OpListStorage = MoveTemp(OpLists);

	PopulateEntityDeltas(View);
}

void ViewDelta::Clear()
{
  EntityChanges.Empty();
  ComponentChanges.Empty();
  AuthorityChanges.Empty();

  ConnectionStatusCode = 0;

  EntityDeltas.Empty();
  WorkerMessages.Empty();
  AuthorityGainedForDelta.Empty();
  AuthorityLostForDelta.Empty();
  AuthorityLostTempForDelta.Empty();
  ComponentsAddedForDelta.Empty();
  ComponentsRemovedForDelta.Empty();
  ComponentUpdatesForDelta.Empty();
  ComponentsRefreshedForDelta.Empty();
  OpListStorage.Empty();
}

const TArray<EntityDelta>& ViewDelta::GetEntityDeltas() const
{
	return EntityDeltas;
}

const TArray<Worker_Op>& ViewDelta::GetWorkerMessages() const
{
	return WorkerMessages;
}

bool ViewDelta::HasDisconnected() const
{
	return ConnectionStatusCode != 0;
}

Worker_ConnectionStatusCode ViewDelta::GetConnectionStatus() const
{
	check(HasDisconnected());
	return static_cast<Worker_ConnectionStatusCode>(ConnectionStatusCode);
}

FString ViewDelta::GetDisconnectReason() const
{
	check(HasDisconnected());
	return ConnectionStatusMessage;
}

ViewDelta::ReceivedComponentChange::ReceivedComponentChange(const Worker_AddComponentOp& Op)
	: EntityId(Op.entity_id) , ComponentId(Op.data.component_id) , Type(ADD) , ComponentAdded(Op.data.schema_type)
{
}

ViewDelta::ReceivedComponentChange::ReceivedComponentChange(const Worker_ComponentUpdateOp& Op)
	: EntityId(Op.entity_id) , ComponentId(Op.update.component_id) , Type(UPDATE) , ComponentUpdate(Op.update.schema_type)
{
}

ViewDelta::ReceivedComponentChange::ReceivedComponentChange(const Worker_RemoveComponentOp& Op)
	: EntityId(Op.entity_id), ComponentId(Op.component_id), Type(REMOVE)
{
}

bool ViewDelta::DifferentEntity::operator()(const ReceivedEntityChange& E) const
{
	return E.EntityId != EntityId;
}

bool ViewDelta::DifferentEntity::operator()(const ReceivedComponentChange& Op) const
{
	return Op.EntityId != EntityId;
}

bool ViewDelta::DifferentEntity::operator()(const Worker_AuthorityChangeOp& Op) const
{
	return Op.entity_id != EntityId;
}

bool ViewDelta::DifferentEntityComponent::operator()(const ReceivedComponentChange& Op) const
{
	return Op.ComponentId != component_id || Op.EntityId != EntityId;
}

bool ViewDelta::DifferentEntityComponent::operator()(const Worker_AuthorityChangeOp& Op) const
{
	return Op.component_id != component_id || Op.entity_id != EntityId;
}

bool ViewDelta::EntityComponentComparison::operator()(const ReceivedComponentChange& Lhs, const ReceivedComponentChange& Rhs) const
{
	if (Lhs.EntityId != Rhs.EntityId)
	{
		return Lhs.EntityId < Rhs.EntityId;
	}
	return Lhs.ComponentId < Rhs.ComponentId;
}

bool ViewDelta::EntityComponentComparison::operator()(const Worker_AuthorityChangeOp& Lhs, const Worker_AuthorityChangeOp& Rhs) const
{
	if (Lhs.entity_id != Rhs.entity_id)
	{
		return Lhs.entity_id < Rhs.entity_id;
	}
	return Lhs.component_id < Rhs.component_id;
}

bool ViewDelta::EntityComparison::operator()(const ReceivedEntityChange& Lhs, const ReceivedEntityChange& Rhs) const
{
	return Lhs.EntityId < Rhs.EntityId;
}

ComponentChange ViewDelta::CalculateAdd(ReceivedComponentChange* Start, ReceivedComponentChange* End, TArray<ComponentData>& Components)
{
	// There must be at least one component add; anything before it can be ignored.
	ReceivedComponentChange* It = std::find_if(Start, End, [](const ReceivedComponentChange& Op)
	{
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

ComponentChange ViewDelta::CalculateCompleteUpdate(ReceivedComponentChange* Start, ReceivedComponentChange* End,
	Schema_ComponentData* Data, Schema_ComponentUpdate* Events, ComponentData& Component)
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
	Schema_Object* events_obj = Events ? Schema_GetComponentUpdateEvents(Events) : nullptr;
	// Use the data from the op list as pointers from the view aren't stable.
	return ComponentChange(Start->ComponentId, Data, events_obj);
}

ComponentChange ViewDelta::CalculateUpdate(ReceivedComponentChange* Start, ReceivedComponentChange* End, ComponentData& Component)
{
	// For an update we don't know if we are calculating a complete-update or a regular update.
	// So the first message processed might be an add or an update.
	auto It = std::find_if(Start, End, [](const ReceivedComponentChange& Op)
	{
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

void ViewDelta::ProcessOp(Worker_Op& Op)
{
	switch (static_cast<Worker_OpType>(Op.op_type))
	{
	case WORKER_OP_TYPE_DISCONNECT:
		ConnectionStatusCode = Op.op.disconnect.connection_status_code;
		ConnectionStatusMessage = Op.op.disconnect.reason;
		break;
	case WORKER_OP_TYPE_LOG_MESSAGE:
		// Log messages deprecated.
		break;
	case WORKER_OP_TYPE_CRITICAL_SECTION:
		// Ignore critical sections.
		break;
	case WORKER_OP_TYPE_ADD_ENTITY:
		EntityChanges.Push(ReceivedEntityChange{Op.op.add_entity.entity_id, true});
		break;
	case WORKER_OP_TYPE_REMOVE_ENTITY:
		EntityChanges.Push(ReceivedEntityChange{Op.op.remove_entity.entity_id, false});
		break;
	case WORKER_OP_TYPE_METRICS:
	case WORKER_OP_TYPE_FLAG_UPDATE:
	case WORKER_OP_TYPE_RESERVE_ENTITY_IDS_RESPONSE:
	case WORKER_OP_TYPE_CREATE_ENTITY_RESPONSE:
	case WORKER_OP_TYPE_DELETE_ENTITY_RESPONSE:
	case WORKER_OP_TYPE_ENTITY_QUERY_RESPONSE:
	case WORKER_OP_TYPE_COMMAND_REQUEST:
	case WORKER_OP_TYPE_COMMAND_RESPONSE:
		WorkerMessages.Push(Op);
		break;
	case WORKER_OP_TYPE_ADD_COMPONENT:
		ComponentChanges.Emplace(Op.op.add_component);
		break;
	case WORKER_OP_TYPE_REMOVE_COMPONENT:
		ComponentChanges.Emplace(Op.op.remove_component);
		break;
	case WORKER_OP_TYPE_AUTHORITY_CHANGE:
		if (Op.op.authority_change.authority != WORKER_AUTHORITY_AUTHORITY_LOSS_IMMINENT)
		{
			AuthorityChanges.Emplace(Op.op.authority_change);
		}
		break;
	case WORKER_OP_TYPE_COMPONENT_UPDATE:
		ComponentChanges.Emplace(Op.op.component_update);
		break;
	default:
		// Ignore other ops.
		break;
	}
}

void ViewDelta::PopulateEntityDeltas(EntityView& View)
{
	// Make sure there is enough space in the view delta storage.
	// This allows us to rely on stable pointers as we add new elements.
	ComponentsAddedForDelta.Reserve(ComponentChanges.Num());
	ComponentsRemovedForDelta.Reserve(ComponentChanges.Num());
	ComponentUpdatesForDelta.Reserve(ComponentChanges.Num());
	ComponentsRefreshedForDelta.Reserve(ComponentChanges.Num());
	AuthorityGainedForDelta.Reserve(AuthorityChanges.Num());
	AuthorityLostForDelta.Reserve(AuthorityChanges.Num());
	AuthorityLostTempForDelta.Reserve(AuthorityChanges.Num());

	Algo::StableSort(ComponentChanges, EntityComponentComparison{});
	Algo::StableSort(AuthorityChanges, EntityComponentComparison{});
	Algo::StableSort(EntityChanges, EntityComparison{});

	// todo is this actually better than just adding more conditions?
	// Add sentinel elements to the ends of the arrays with entity_id -1.
	// Prevents the need for bounds checks on the iterators.
	ComponentChanges.Emplace(Worker_RemoveComponentOp{-1, 0});
	AuthorityChanges.Emplace(Worker_AuthorityChangeOp{-1, 0, 0});
	EntityChanges.Emplace(ReceivedEntityChange{-1, false});

	auto ComponentIt = ComponentChanges.GetData();
	auto AuthorityIt = AuthorityChanges.GetData();
	auto EntityIt = EntityChanges.GetData();

	ReceivedComponentChange* ComponentChangesEnd = ComponentIt + ComponentChanges.Num();
	Worker_AuthorityChangeOp* AuthorityChangesEnd = AuthorityIt + AuthorityChanges.Num();
	ReceivedEntityChange* EntityChangesEnd = EntityIt + EntityChanges.Num();

	// At the beginning of each loop each iterator should point to the first element for an entity.
	// Each loop we want to work with a single entity ID.
	// We check the entities each iterator, that can be dereferenced, is pointing to and pick
	// the smallest one. Stop when all lists have been read.
	for (;;)
	{
		// Get the next entity Id. We want to pick the smallest entity referenced by the iterators.
		// If the unsigned final value is greater than INT64_MAX (entity_id < 0) then stop.
		uint64 MinEntityId = static_cast<std::uint64_t>(ComponentIt->EntityId);
		MinEntityId = FMath::Min(MinEntityId, static_cast<std::uint64_t>(AuthorityIt->entity_id));
		MinEntityId = FMath::Min(MinEntityId, static_cast<std::uint64_t>(EntityIt->EntityId));

		// If no list has elements left to read then stop.
		if (MinEntityId > static_cast<uint64>(std::numeric_limits<Worker_EntityId>::max()))
		{
			break;
		}

		const Worker_EntityId CurrentEntityId = static_cast<Worker_EntityId>(MinEntityId);

		EntityDelta Delta = {};
		Delta.EntityId = CurrentEntityId;

		EntityViewElement* ViewElement = View.Find(CurrentEntityId);

		if (EntityIt->EntityId == CurrentEntityId)
		{
			EntityIt = ProcessEntityExistenceChange(EntityIt, EntityChangesEnd, Delta, &ViewElement, View);
			// If the entity isn't present we don't need to process component and authority changes.
			if (ViewElement == nullptr)
			{
				ComponentIt = std::find_if(ComponentIt, ComponentChangesEnd, DifferentEntity{CurrentEntityId});
				AuthorityIt = std::find_if(AuthorityIt, AuthorityChangesEnd, DifferentEntity{CurrentEntityId});

				// Only add the entity delta if previously existed in the view.
				if (Delta.bRemoved)
				{
					EntityDeltas.Push(Delta);
				}
				continue;
			}
		}

		if (ComponentIt->EntityId == CurrentEntityId)
		{
			ComponentIt = ProcessEntityComponentChanges(ComponentIt, ComponentChangesEnd, ViewElement->Components, Delta);
		}

		if (AuthorityIt->entity_id == CurrentEntityId)
		{
			AuthorityIt = ProcessEntityAuthorityChanges(AuthorityIt, AuthorityChangesEnd, ViewElement->Authority, Delta);
		}

		EntityDeltas.Push(Delta);
	}
}

ViewDelta::ReceivedComponentChange* ViewDelta::ProcessEntityComponentChanges(ReceivedComponentChange* It,
	ReceivedComponentChange* End, TArray<ComponentData>& Components, EntityDelta& Delta)
{
	int32 AddCount = 0;
	int32 UpdateCount = 0;
	int32 RemoveCount = 0;
	int32 RefreshCount = 0;

	const Worker_EntityId EntityId = It->EntityId;
	// At the end of each loop it should point to the first element for an entity-component.
	// Stop and return when the component is for a different entity.
	// There will always be at least one iteration of the loop.
	for (;;)
	{
		ReceivedComponentChange* NextComponentIt = std::find_if(It, End, DifferentEntityComponent{EntityId, It->ComponentId});

		ComponentData* Component = Components.FindByPredicate(ComponentIdEquality{It->ComponentId});
		const bool bComponentExists = Component != nullptr;

		// The element one before nextComponentIt must be the last element for this component.
		switch ((NextComponentIt - 1)->Type)
		{
		case ReceivedComponentChange::ADD:
			if (bComponentExists) {
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

		if (NextComponentIt->EntityId != EntityId) {
			Delta.ComponentsAdded = {
				ComponentsAddedForDelta.GetData() + ComponentsAddedForDelta.Num() - AddCount,
				AddCount
			};
			Delta.ComponentsRemoved = {
				ComponentsRemovedForDelta.GetData() + ComponentsRemovedForDelta.Num() - RemoveCount,
				RemoveCount
			};
			Delta.ComponentUpdates = {
				ComponentUpdatesForDelta.GetData() + ComponentUpdatesForDelta.Num() - UpdateCount,
				UpdateCount
			};
			Delta.ComponentsRefreshed = {
				ComponentsRefreshedForDelta.GetData() + ComponentsRefreshedForDelta.Num() - RefreshCount,
				RefreshCount
			};
			return NextComponentIt;
		}

		It = NextComponentIt;
	}
}

Worker_AuthorityChangeOp* ViewDelta::ProcessEntityAuthorityChanges(Worker_AuthorityChangeOp* It,
	Worker_AuthorityChangeOp* End, TArray<Worker_ComponentId>& EntityAuthority, EntityDelta& Delta)
{
	int32 GainCount = 0;
	int32 LossCount = 0;
	int32 LossTempCount = 0;

	const Worker_EntityId EntityId = It->entity_id;
	// After each loop the iterator points to the first op relating to the next entity-component.
	// Stop and return when that component is for a different entity.
	// There will always be at least one iteration of the loop.
	for (;;)
	{
		// Find the last element for this entity-component.
		const Worker_ComponentId ComponentId = It->component_id;
		It = std::find_if(It, End, DifferentEntityComponent{EntityId, ComponentId}) - 1;
		const int32 AuthorityIndex = EntityAuthority.Find(ComponentId);
		const bool bHasAuthority = AuthorityIndex != INDEX_NONE;

		if (It->authority == WORKER_AUTHORITY_AUTHORITATIVE)
		{
			if (bHasAuthority)
			{
				AuthorityLostTempForDelta.Emplace(ComponentId, AuthorityChange::AUTHORITY_LOST_TEMPORARILY);
				++LossTempCount;
			}
			else
			{
				EntityAuthority.Push(ComponentId);
				AuthorityGainedForDelta.Emplace(ComponentId, AuthorityChange::AUTHORITY_GAINED);
				++GainCount;
			}
		}
		else if (bHasAuthority)
		{
			AuthorityLostForDelta.Emplace(ComponentId, AuthorityChange::AUTHORITY_LOST);
			EntityAuthority.RemoveAtSwap(AuthorityIndex);
			++LossCount;
		}

		// Move to the next entity-component.
		++It;

		if (It->entity_id != EntityId)
		{
			Delta.AuthorityGained = {
				AuthorityGainedForDelta.GetData() + AuthorityGainedForDelta.Num() - GainCount,
				GainCount
			};
			Delta.AuthorityLost = {
				AuthorityLostForDelta.GetData() + AuthorityLostForDelta.Num() - LossCount,
				LossCount
			};
			Delta.AuthorityLostTemporarily = {
				AuthorityLostTempForDelta.GetData() + AuthorityLostTempForDelta.Num() - LossTempCount,
				LossTempCount
			};
			return It;
		}
	}
}

ViewDelta::ReceivedEntityChange* ViewDelta::ProcessEntityExistenceChange(ReceivedEntityChange* It,
	ReceivedEntityChange* End, EntityDelta& Delta, EntityViewElement** ViewElement, EntityView& View)
{
	// Find the last element relating to the same entity.
	const Worker_EntityId EntityId = It->EntityId;
	It = std::find_if(It, End, DifferentEntity{EntityId}) - 1;

	const bool bAlreadyInView = *ViewElement != nullptr;
	const bool bEntityAdded = It->Added;

	// If the entity's presence has not changed then return.
	if (bEntityAdded == bAlreadyInView)
	{
		return It + 1;
	}

	if (bEntityAdded)
	{
		Delta.bAdded = true;
		*ViewElement = &View.Emplace(EntityId, EntityViewElement{});
	}
	else
	{
		Delta.bRemoved = true;

		// Remove components.
		const auto& Components = (*ViewElement)->Components;
		for (const auto& Component : Components)
		{
			ComponentsRemovedForDelta.Emplace(Component.GetComponentId());
		}
		Delta.ComponentsRemoved = {
			ComponentsRemovedForDelta.GetData() + ComponentsRemovedForDelta.Num() - Components.Num(),
			Components.Num()
		};

		// Remove authority.
		const auto& Authority = (*ViewElement)->Authority;
		for (const auto& Id : Authority)
		{
			AuthorityLostForDelta.Emplace(Id, AuthorityChange::AUTHORITY_LOST);
		}
		Delta.AuthorityLost = {
			AuthorityLostForDelta.GetData() + AuthorityLostForDelta.Num() - Authority.Num(),
			Authority.Num()
		};

		// Remove from view.
		View.Remove(EntityId);
		*ViewElement = nullptr;
	}

	return It + 1;
}

}  // namespace SpatialGDK
