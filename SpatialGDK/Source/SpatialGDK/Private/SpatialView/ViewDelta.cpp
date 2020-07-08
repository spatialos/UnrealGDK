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
	op_list_storage = MoveTemp(OpLists);

	PopulateEntityDeltas(View);
}

void ViewDelta::Clear()
{
  entity_changes.Empty();
  component_changes.Empty();
  authority_changes.Empty();

  connection_status_code = 0;

  entity_deltas.Empty();
  worker_messages.Empty();
  authority_change_storage.Empty();
  component_change_storage.Empty();
  op_list_storage.Empty();
}

const TArray<EntityDelta>& ViewDelta::GetEntityDeltas() const
{
	return entity_deltas;
}

const TArray<Worker_Op>& ViewDelta::GetWorkerMessages() const
{
	return worker_messages;
}

bool ViewDelta::HasDisconnected() const
{
	return connection_status_code != 0;
}

Worker_ConnectionStatusCode ViewDelta::GetConnectionStatus() const
{
	check(HasDisconnected());
	return static_cast<Worker_ConnectionStatusCode>(connection_status_code);
}

FString ViewDelta::GetDisconnectReason() const
{
	check(HasDisconnected());
	return connection_status_message;
}

ViewDelta::ReceivedComponentChange::ReceivedComponentChange(const Worker_AddComponentOp& op)
	: entity_id(op.entity_id) , component_id(op.data.component_id) , type(kAdd) , component_added(op.data.schema_type)
{
}

ViewDelta::ReceivedComponentChange::ReceivedComponentChange(const Worker_ComponentUpdateOp& op)
	: entity_id(op.entity_id) , component_id(op.update.component_id) , type(kUpdate) , component_update(op.update.schema_type)
{
}

ViewDelta::ReceivedComponentChange::ReceivedComponentChange(const Worker_RemoveComponentOp& op)
	: entity_id(op.entity_id), component_id(op.component_id), type(kRemove)
{
}

bool ViewDelta::DifferentEntity::operator()(const ReceivedEntityChange& e) const
{
	return e.entity_id != entity_id;
}

bool ViewDelta::DifferentEntity::operator()(const ReceivedComponentChange& op) const
{
	return op.entity_id != entity_id;
}

bool ViewDelta::DifferentEntity::operator()(const Worker_AuthorityChangeOp& op) const
{
	return op.entity_id != entity_id;
}

bool ViewDelta::DifferentEntityComponent::operator()(const ReceivedComponentChange& op) const
{
	return op.component_id != component_id || op.entity_id != entity_id;
}

bool ViewDelta::DifferentEntityComponent::operator()(const Worker_AuthorityChangeOp& op) const
{
	return op.component_id != component_id || op.entity_id != entity_id;
}

bool ViewDelta::EntityComponentComparison::operator()(const ReceivedComponentChange& lhs, const ReceivedComponentChange& rhs) const
{
	if (lhs.entity_id != rhs.entity_id)
	{
		return lhs.entity_id < rhs.entity_id;
	}
	return lhs.component_id < rhs.component_id;
}

bool ViewDelta::EntityComponentComparison::operator()(const Worker_AuthorityChangeOp& lhs, const Worker_AuthorityChangeOp& rhs) const
{
	if (lhs.entity_id != rhs.entity_id)
	{
		return lhs.entity_id < rhs.entity_id;
	}
	return lhs.component_id < rhs.component_id;
}

bool ViewDelta::EntityComparison::operator()(const ReceivedEntityChange& lhs, const ReceivedEntityChange& rhs) const
{
	return lhs.entity_id < rhs.entity_id;
}

ComponentChange ViewDelta::CalculateAdd(ReceivedComponentChange* start, ReceivedComponentChange* end, TArray<ComponentData>& components)
{
	// There must be at least one component add; anything before it can be ignored.
	ReceivedComponentChange* it = std::find_if(start, end, [](const ReceivedComponentChange& op)
	{
		return op.type == ReceivedComponentChange::kAdd;
	});

	Schema_ComponentData* data = it->component_added;
	++it;

	while (it != end)
	{
		switch (it->type)
		{
		case ReceivedComponentChange::kAdd:
			data = it->component_added;
			break;
		case ReceivedComponentChange::kUpdate:
			Schema_ApplyComponentUpdateToData(it->component_update, data);
			break;
		case ReceivedComponentChange::kRemove:
			break;
		}
		++it;
	}
	components.Emplace(ComponentData::CreateCopy(data, start->component_id));
	// We don't want to reference the component in the view as is isn't stable.
	return ComponentChange(start->component_id, data);
}

ComponentChange ViewDelta::CalculateCompleteUpdate(ReceivedComponentChange* start, ReceivedComponentChange* end,
	Schema_ComponentData* starting_data, Schema_ComponentUpdate* starting_events, ComponentData& component)
{
	Schema_ComponentData* data = starting_data;
	Schema_ComponentUpdate* events = starting_events;

	for (auto it = start; it != end; ++it)
	{
		switch (it->type)
		{
		case ReceivedComponentChange::kAdd:
			data = it->component_added;
			break;
		case ReceivedComponentChange::kUpdate:
			if (data)
			{
				Schema_ApplyComponentUpdateToData(it->component_update, data);
			}
			if (events)
			{
				Schema_MergeComponentUpdateIntoUpdate(it->component_update, events);
			}
			else
			{
				events = it->component_update;
			}
			break;
		case ReceivedComponentChange::kRemove:
			break;
		}
	}

	component = ComponentData::CreateCopy(data, start->component_id);
	Schema_Object* events_obj = events ? Schema_GetComponentUpdateEvents(events) : nullptr;
	// Use the data from the op list as pointers from the view aren't stable.
	return ComponentChange(start->component_id, data, events_obj);
}

ComponentChange ViewDelta::CalculateUpdate(ReceivedComponentChange* start, ReceivedComponentChange* end, ComponentData& component)
{
	// For an update we don't know if we are calculating a complete-update or a regular update.
	// So the first message processed might be an add or an update.
	auto it = std::find_if(start, end, [](const ReceivedComponentChange& op)
	{
		return op.type != ReceivedComponentChange::kRemove;
	});

	// If the first message is an add then calculate a complete-update.
	if (it->type == ReceivedComponentChange::kAdd)
	{
		return CalculateCompleteUpdate(it + 1, end, it->component_added, nullptr, component);
	}

	Schema_ComponentUpdate* update = it->component_update;
	++it;
	while (it != end)
	{
		switch (it->type)
		{
		case ReceivedComponentChange::kAdd:
			return CalculateCompleteUpdate(it + 1, end, it->component_added, update, component);
		case ReceivedComponentChange::kUpdate:
			Schema_MergeComponentUpdateIntoUpdate(it->component_update, update);
			break;
		case ReceivedComponentChange::kRemove:
			return CalculateCompleteUpdate(it + 1, end, nullptr, nullptr, component);
		}
		++it;
	}

	Schema_ApplyComponentUpdateToData(update, component.GetUnderlying());
	component = component.DeepCopy();
	return ComponentChange(start->component_id, update);
}

void ViewDelta::ProcessOp(Worker_Op& op)
{
	switch (static_cast<Worker_OpType>(op.op_type))
	{
	case WORKER_OP_TYPE_DISCONNECT:
		connection_status_code = op.op.disconnect.connection_status_code;
		connection_status_message = op.op.disconnect.reason;
		break;
	case WORKER_OP_TYPE_LOG_MESSAGE:
		// Log messages deprecated.
		break;
	case WORKER_OP_TYPE_CRITICAL_SECTION:
		// Ignore critical sections.
		break;
	case WORKER_OP_TYPE_ADD_ENTITY:
		entity_changes.Push(ReceivedEntityChange{op.op.add_entity.entity_id, true});
		break;
	case WORKER_OP_TYPE_REMOVE_ENTITY:
		entity_changes.Push(ReceivedEntityChange{op.op.remove_entity.entity_id, false});
		break;
	case WORKER_OP_TYPE_METRICS:
	case WORKER_OP_TYPE_FLAG_UPDATE:
	case WORKER_OP_TYPE_RESERVE_ENTITY_IDS_RESPONSE:
	case WORKER_OP_TYPE_CREATE_ENTITY_RESPONSE:
	case WORKER_OP_TYPE_DELETE_ENTITY_RESPONSE:
	case WORKER_OP_TYPE_ENTITY_QUERY_RESPONSE:
	case WORKER_OP_TYPE_COMMAND_REQUEST:
	case WORKER_OP_TYPE_COMMAND_RESPONSE:
		worker_messages.Push(op);
		break;
	case WORKER_OP_TYPE_ADD_COMPONENT:
		component_changes.Emplace(op.op.add_component);
		break;
	case WORKER_OP_TYPE_REMOVE_COMPONENT:
		component_changes.Emplace(op.op.remove_component);
		break;
	case WORKER_OP_TYPE_AUTHORITY_CHANGE:
		if (op.op.authority_change.authority != WORKER_AUTHORITY_AUTHORITY_LOSS_IMMINENT)
		{
			authority_changes.Emplace(op.op.authority_change);
		}
		break;
	case WORKER_OP_TYPE_COMPONENT_UPDATE:
		component_changes.Emplace(op.op.component_update);
		break;
	default:
		// Ignore other ops.
		break;
	}
}

void ViewDelta::PopulateEntityDeltas(EntityView& view)
{
	// Make sure there is enough space in the view delta storage.
	// This allows us to rely on stable pointers as we add new elements.
	component_change_storage.Reserve(component_changes.Num());
	authority_change_storage.Reserve(authority_changes.Num());

	Algo::StableSort(component_changes, EntityComponentComparison{});
	Algo::StableSort(authority_changes, EntityComponentComparison{});
	Algo::StableSort(entity_changes, EntityComparison{});

	// todo is this actually better than just adding more conditions?
	// Add sentinel elements to the ends of the arrays with entity_id -1.
	// Prevents the need for bounds checks on the iterators.
	component_changes.Emplace(Worker_RemoveComponentOp{-1, 0});
	authority_changes.Emplace(Worker_AuthorityChangeOp{-1, 0, 0});
	entity_changes.Emplace(ReceivedEntityChange{-1, false});

	auto component_it = component_changes.GetData();
	auto authority_it = authority_changes.GetData();
	auto entity_it = entity_changes.GetData();

	ReceivedComponentChange* ComponentChangesEnd = component_it + component_changes.Num();
	Worker_AuthorityChangeOp* AuthorityChangesEnd = authority_it + authority_changes.Num();
	ReceivedEntityChange* EntityChangesEnd = entity_it + entity_changes.Num();

	// At the beginning of each loop each iterator should point to the first element for an entity.
	// Each loop we want to work with a single entity ID.
	// We check the entities each iterator, that can be dereferenced, is pointing to and pick
	// the smallest one. Stop when all lists have been read.
	for (;;)
	{
		// Get the next entity Id. We want to pick the smallest entity referenced by the iterators.
		// If the unsigned final value is greater than INT64_MAX (entity_id < 0) then stop.
		uint64 min_entity_id = static_cast<std::uint64_t>(component_it->entity_id);
		min_entity_id = FMath::Min(min_entity_id, static_cast<std::uint64_t>(authority_it->entity_id));
		min_entity_id = FMath::Min(min_entity_id, static_cast<std::uint64_t>(entity_it->entity_id));

		// If no list has elements left to read then stop.
		if (min_entity_id > static_cast<uint64>(std::numeric_limits<Worker_EntityId>::max()))
		{
			break;
		}

		const Worker_EntityId current_entity_id = static_cast<Worker_EntityId>(min_entity_id);

		EntityDelta entity_delta = {};
		entity_delta.entity_id = current_entity_id;

		EntityViewElement* view_it = view.Find(current_entity_id);

		if (entity_it->entity_id == current_entity_id)
		{
			entity_it = ProcessEntityExistenceChange(entity_it, EntityChangesEnd, entity_delta, &view_it, view);
			// If the entity isn't present we don't need to process component and authority changes.
			if (view_it == nullptr)
			{
				component_it = std::find_if(component_it, ComponentChangesEnd, DifferentEntity{current_entity_id});
				authority_it = std::find_if(authority_it, AuthorityChangesEnd, DifferentEntity{current_entity_id});

				// Only add the entity delta if previously existed in the view.
				if (entity_delta.removed)
				{
					entity_deltas.Push(entity_delta);
				}
				continue;
			}
		}

		if (component_it->entity_id == current_entity_id)
		{
			component_it = ProcessEntityComponentChanges(component_it, ComponentChangesEnd, view_it->Components, entity_delta);
		}

		if (authority_it->entity_id == current_entity_id)
		{
			authority_it = ProcessEntityAuthorityChanges(authority_it, AuthorityChangesEnd, view_it->Authority, entity_delta);
		}

		entity_deltas.Push(entity_delta);
	}
}

ViewDelta::ReceivedComponentChange* ViewDelta::ProcessEntityComponentChanges(ReceivedComponentChange* it,
	ReceivedComponentChange* End, TArray<ComponentData>& components, EntityDelta& entity_delta)
{
	int32 count = 0;
	const Worker_EntityId entity_id = it->entity_id;
	// At the end of each loop it should point to the first element for an entity-component.
	// Stop and return when the component is for a different entity.
	// There will always be at least one iteration of the loop.
	for (;;)
	{
		ReceivedComponentChange* next_component_it = std::find_if(it, End, DifferentEntityComponent{entity_id, it->component_id});

		ComponentData* component_it = components.FindByPredicate(ComponentIdEquality{it->component_id});
		const bool component_exists = component_it != nullptr;
		// The element one before nextComponentIt must be the last element for this component.
		switch ((next_component_it - 1)->type)
		{
		case ReceivedComponentChange::kAdd:
			if (component_exists) {
				component_change_storage.Emplace(CalculateCompleteUpdate(it, next_component_it, nullptr, nullptr, *component_it));
			}
			else
			{
				component_change_storage.Emplace(CalculateAdd(it, next_component_it, components));
			}
			++count;
			break;
		case ReceivedComponentChange::kUpdate:
			if (component_exists)
			{
				component_change_storage.Emplace(CalculateUpdate(it, next_component_it, *component_it));
			}
			else
			{
				component_change_storage.Emplace(CalculateAdd(it, next_component_it, components));
			}
			++count;
			break;
		case ReceivedComponentChange::kRemove:
			if (component_exists)
			{
				component_change_storage.Emplace(it->component_id);
				components.RemoveAtSwap(component_it - components.GetData());
				++count;
			}
			break;
		}

		if (next_component_it->entity_id != entity_id) {
			entity_delta.component_changes = {
				component_change_storage.GetData() + component_change_storage.Num() - count,
				count
			};
			return next_component_it;
		}

		it = next_component_it;
	}
}

Worker_AuthorityChangeOp* ViewDelta::ProcessEntityAuthorityChanges(Worker_AuthorityChangeOp* it,
	Worker_AuthorityChangeOp* End, TArray<Worker_ComponentId>& authority, EntityDelta& entity_delta)
{
	int32 count = 0;
	const Worker_EntityId entity_id = it->entity_id;
	// After each loop the iterator points to the first op relating to the next entity-component.
	// Stop and return when that component is for a different entity.
	// There will always be at least one iteration of the loop.
	for (;;)
	{
		// Find the last element for this entity-component.
		const Worker_ComponentId component_id = it->component_id;
		it = std::find_if(it, End, DifferentEntityComponent{entity_id, component_id}) - 1;
		const int32 AuthorityIndex = authority.Find(component_id);
		const bool has_authority = AuthorityIndex != INDEX_NONE;

		if (it->authority == WORKER_AUTHORITY_AUTHORITATIVE)
		{
			if (has_authority)
			{
				authority_change_storage.Emplace(component_id, AuthorityChange::kAuthorityLostTemporarily);
			}
			else
			{
				authority.Push(component_id);
				authority_change_storage.Emplace(component_id, AuthorityChange::kAuthorityGained);
			}
			++count;
		}
		else if (has_authority)
		{
			authority_change_storage.Emplace(component_id, AuthorityChange::kAuthorityLost);
			authority.RemoveAtSwap(AuthorityIndex);
			++count;
		}

		// Move to the next entity-component.
		++it;

		if (it->entity_id != entity_id)
		{
			entity_delta.authority_changes = {
				authority_change_storage.GetData() + authority_change_storage.Num() - count,
				count
			};
			return it;
		}
	}
}

ViewDelta::ReceivedEntityChange* ViewDelta::ProcessEntityExistenceChange(ReceivedEntityChange* entity_it,
	ReceivedEntityChange* End, EntityDelta& entity_delta, EntityViewElement** view_it, EntityView& view)
{
	// Find the last element relating to the same entity.
	const Worker_EntityId entity_id = entity_it->entity_id;
	entity_it = std::find_if(entity_it, End, DifferentEntity{entity_id}) - 1;

	const bool already_in_view = *view_it != nullptr;
	const bool entity_added = entity_it->added;

	// If the entity's presence has not changed then return.
	if (entity_added == already_in_view)
	{
		return entity_it + 1;
	}

	if (entity_added)
	{
		entity_delta.added = true;
		*view_it = &view.Emplace(entity_id, EntityViewElement{});
	}
	else
	{
		entity_delta.removed = true;

		// Remove components.
		const auto& components = (*view_it)->Components;
		for (const auto& component : components)
		{
			component_change_storage.Emplace(component.GetComponentId());
		}
		entity_delta.component_changes = {
			component_change_storage.GetData() + component_change_storage.Num() - components.Num(),
			components.Num()
		};

		// Remove authority.
		const auto& authority = (*view_it)->Authority;
		for (const auto& id : authority)
		{
			authority_change_storage.Emplace(id, AuthorityChange::kAuthorityLost);
		}
		entity_delta.authority_changes = {
			authority_change_storage.GetData() + authority_change_storage.Num() - authority.Num(),
			authority.Num()
		};

		// Remove from view.
		view.Remove(entity_id);
		*view_it = nullptr;
	}

	return entity_it + 1;
}

}  // namespace SpatialGDK
