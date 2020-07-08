// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "SpatialView/EntityDelta.h"
#include "SpatialView/EntityView.h"
#include "SpatialView/OpList/OpList.h"

#include "Containers/Array.h"
#include <improbable/c_worker.h>

namespace SpatialGDK
{

class ViewDelta
{
public:
	void SetFromOpList(TArray<OpList> OpLists, EntityView& View);
	void Clear();

	const TArray<EntityDelta>& GetEntityDeltas() const;
	const TArray<Worker_Op>& GetWorkerMessages() const;

	bool HasDisconnected() const;
	Worker_ConnectionStatusCode GetConnectionStatus() const;
	FString GetDisconnectReason() const;

private:
	struct ReceivedComponentChange
	{
		explicit ReceivedComponentChange(const Worker_AddComponentOp& op);
		explicit ReceivedComponentChange(const Worker_ComponentUpdateOp& op);
		explicit ReceivedComponentChange(const Worker_RemoveComponentOp& op);

		Worker_EntityId entity_id;
		Worker_ComponentId component_id;
		enum { kAdd, kUpdate, kRemove } type;
		union
		{
			Schema_ComponentData* component_added;
			Schema_ComponentUpdate* component_update;
		};
	};

	struct ReceivedEntityChange
	{
		Worker_EntityId entity_id;
		bool added;
	};

	struct DifferentEntity
	{
		Worker_EntityId entity_id;
		bool operator()(const ReceivedEntityChange& e) const;
		bool operator()(const ReceivedComponentChange& op) const;
		bool operator()(const Worker_AuthorityChangeOp& op) const;
	};

	struct DifferentEntityComponent
	{
		Worker_EntityId entity_id;
		Worker_ComponentId component_id;
		bool operator()(const ReceivedComponentChange& op) const;
		bool operator()(const Worker_AuthorityChangeOp& op) const;
	};

	struct EntityComponentComparison
	{
		bool operator()(const ReceivedComponentChange& lhs, const ReceivedComponentChange& rhs) const;
		bool operator()(const Worker_AuthorityChangeOp& lhs, const Worker_AuthorityChangeOp& rhs) const;
	};

	struct EntityComparison
	{
		bool operator()(const ReceivedEntityChange& lhs, const ReceivedEntityChange& rhs) const;
	};

	using EntityChanges = TArray<ReceivedEntityChange>;
	using ComponentChanges = TArray<ReceivedComponentChange>;
	using AuthorityChanges = TArray<Worker_AuthorityChangeOp>;

	// Calculate and return the net component added in [`start`, `end`).
	// Also add the resulting component to `components`.
	// The accumulated component change in this range must component add.
	static ComponentChange CalculateAdd(ReceivedComponentChange* start,
										ReceivedComponentChange* end,
										TArray<ComponentData>& components);

	// Calculate and return the net complete update in [`start`, `end`).
	// Also set `component` to match.
	// The accumulated component change in this range must be an update or a complete-update or
	// `startingData` and `startingEvents` should be non null.
	static ComponentChange CalculateCompleteUpdate(ReceivedComponentChange* start,ReceivedComponentChange* end,
		Schema_ComponentData* starting_data, Schema_ComponentUpdate* starting_events, ComponentData& component);

	// Calculate and return the net complete update in [`start`, `end`).
	// Also apply the update to `component`.
	// The accumulated component change in this range must be an update or a complete-update.
	static ComponentChange CalculateUpdate(ReceivedComponentChange* start, ReceivedComponentChange* end, ComponentData& component);

	void ProcessOp(Worker_Op& op);
	void PopulateEntityDeltas(EntityView& view);

	// Adds component changes to `entity_delta` and updates `components` accordingly.
	// `it` must point to the first element with a given entity ID.
	// Returns an iterator to the next entity in the component changes list.
	ReceivedComponentChange* ProcessEntityComponentChanges(ReceivedComponentChange* it, ReceivedComponentChange* End,
		TArray<ComponentData>& components, EntityDelta& entity_delta);

	// Adds authority changes to `entity_delta` and updates `authority` accordingly.
	// `it` must point to the first element with a given entity ID.
	// Returns an iterator to the next entity in the authority changes list.
	Worker_AuthorityChangeOp* ProcessEntityAuthorityChanges(Worker_AuthorityChangeOp* it, Worker_AuthorityChangeOp* End,
		TArray<Worker_ComponentId>& authority, EntityDelta& entity_delta);

	// Sets `added` and `removed` fields in the `entity_delta`.
	// `it` must point to the first element with a given entity ID.
	// `view_it` must point to the same entity in the view or end if it doesn't exist.
	// Returns an iterator to the next entity in the authority changes list.
	// After returning `view_it` will point to that entity in the view or end if it doesn't exist.
	ReceivedEntityChange* ProcessEntityExistenceChange(ReceivedEntityChange* entity_it, ReceivedEntityChange* End,
		EntityDelta& entity_delta, EntityViewElement** view_it, EntityView& view);

	EntityChanges entity_changes;
	ComponentChanges component_changes;
	AuthorityChanges authority_changes;

	uint8 connection_status_code = 0;
	FString connection_status_message;

	TArray<EntityDelta> entity_deltas;
	TArray<Worker_Op> worker_messages;

	TArray<AuthorityChange> authority_change_storage;
	TArray<ComponentChange> component_change_storage;
	TArray<OpList> op_list_storage;
};

}  // namespace SpatialGDK
