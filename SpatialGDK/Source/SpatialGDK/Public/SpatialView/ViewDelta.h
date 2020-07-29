// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "SpatialView/EntityDelta.h"
#include "SpatialView/EntityView.h"
#include "SpatialView/OpList/OpList.h"

#include "Containers/Array.h"
#include <improbable/c_worker.h>

namespace SpatialGDK
{

/**
 * Lists of changes made to a view as a list of EntityDeltas and miscellaneous other messages.
 * EntityDeltas are sorted by entity ID.
 * Within an EntityDelta the component and authority changes are ordered by component ID.
 *
 * Rough outline of how it works.
 * Input a set of op lists. These should not have any unfinished critical sections in them.
 * Take all ops corresponding to entity components and sort them into entity order then component
 * order within that. Put all other ops in some other list to be read without pre-processing. For
 * ops related to components added, updated, and removed: For each entity-component look at the last
 * op received and check if the component is currently in the view. From this you can work out if
 * the net effect on component was added, removed or updated. If updated read whichever ops are
 * needed to work out what the total update was. For ops related to authority do the same but
 * checking the view to see the current authority state. For add and remove entity ops it's the same
 * again. If an entity is removed, skip reading the received ops and check the view to see what
 * components or authority to remove.
 */
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
		explicit ReceivedComponentChange(const Worker_AddComponentOp& Op);
		explicit ReceivedComponentChange(const Worker_ComponentUpdateOp& Op);
		explicit ReceivedComponentChange(const Worker_RemoveComponentOp& Op);

		Worker_EntityId EntityId;
		Worker_ComponentId ComponentId;
		enum { ADD, UPDATE, REMOVE } Type;
		union
		{
			Schema_ComponentData* ComponentAdded;
			Schema_ComponentUpdate* ComponentUpdate;
		};
	};

	struct ReceivedEntityChange
	{
		Worker_EntityId EntityId;
		bool Added;
	};

	struct DifferentEntity
	{
		Worker_EntityId EntityId;
		bool operator()(const ReceivedEntityChange& E) const;
		bool operator()(const ReceivedComponentChange& Op) const;
		bool operator()(const Worker_AuthorityChangeOp& Op) const;
	};

	struct DifferentEntityComponent
	{
		Worker_EntityId EntityId;
		Worker_ComponentId component_id;
		bool operator()(const ReceivedComponentChange& Op) const;
		bool operator()(const Worker_AuthorityChangeOp& Op) const;
	};

	struct EntityComponentComparison
	{
		bool operator()(const ReceivedComponentChange& Lhs, const ReceivedComponentChange& Rhs) const;
		bool operator()(const Worker_AuthorityChangeOp& Lhs, const Worker_AuthorityChangeOp& Rhs) const;
	};

	struct EntityComparison
	{
		bool operator()(const ReceivedEntityChange& Lhs, const ReceivedEntityChange& Rhs) const;
	};

	// Calculate and return the net component added in [`start`, `end`).
	// Also add the resulting component to `components`.
	// The accumulated component change in this range must component add.
	static ComponentChange CalculateAdd(ReceivedComponentChange* Start,
										ReceivedComponentChange* End,
										TArray<ComponentData>& Components);

	// Calculate and return the net complete update in [`start`, `end`).
	// Also set `component` to match.
	// The accumulated component change in this range must be an update or a complete-update or
	// `startingData` and `startingEvents` should be non null.
	static ComponentChange CalculateCompleteUpdate(ReceivedComponentChange* Start,ReceivedComponentChange* End,
		Schema_ComponentData* Data, Schema_ComponentUpdate* Events, ComponentData& Component);

	// Calculate and return the net complete update in [`start`, `end`).
	// Also apply the update to `component`.
	// The accumulated component change in this range must be an update or a complete-update.
	static ComponentChange CalculateUpdate(ReceivedComponentChange* Start, ReceivedComponentChange* End, ComponentData& Component);

	void ProcessOp(Worker_Op& Op);
	void PopulateEntityDeltas(EntityView& View);

	// Adds component changes to `entity_delta` and updates `components` accordingly.
	// `it` must point to the first element with a given entity ID.
	// Returns an iterator to the next entity in the component changes list.
	ReceivedComponentChange* ProcessEntityComponentChanges(ReceivedComponentChange* It, ReceivedComponentChange* End,
		TArray<ComponentData>& Components, EntityDelta& Delta);

	// Adds authority changes to `entity_delta` and updates `authority` accordingly.
	// `it` must point to the first element with a given entity ID.
	// Returns an iterator to the next entity in the authority changes list.
	Worker_AuthorityChangeOp* ProcessEntityAuthorityChanges(Worker_AuthorityChangeOp* It, Worker_AuthorityChangeOp* End,
		TArray<Worker_ComponentId>& EntityAuthority, EntityDelta& Delta);

	// Sets `added` and `removed` fields in the `entity_delta`.
	// `it` must point to the first element with a given entity ID.
	// `view_it` must point to the same entity in the view or end if it doesn't exist.
	// Returns an iterator to the next entity in the authority changes list.
	// After returning `view_it` will point to that entity in the view or end if it doesn't exist.
	ReceivedEntityChange* ProcessEntityExistenceChange(ReceivedEntityChange* It, ReceivedEntityChange* End,
		EntityDelta& Delta, EntityViewElement** ViewElement, EntityView& View);

	TArray<ReceivedEntityChange> EntityChanges;
	TArray<ReceivedComponentChange> ComponentChanges;
	TArray<Worker_AuthorityChangeOp> AuthorityChanges;

	uint8 ConnectionStatusCode = 0;
	FString ConnectionStatusMessage;

	TArray<EntityDelta> EntityDeltas;
	TArray<Worker_Op> WorkerMessages;

	TArray<AuthorityChange> AuthorityGainedForDelta;
	TArray<AuthorityChange> AuthorityLostForDelta;
	TArray<AuthorityChange> AuthorityLostTempForDelta;
	TArray<ComponentChange> ComponentsAddedForDelta;
	TArray<ComponentChange> ComponentsRemovedForDelta;
	TArray<ComponentChange> ComponentUpdatesForDelta;
	TArray<ComponentChange> ComponentsRefreshedForDelta;
	TArray<OpList> OpListStorage;
};

}  // namespace SpatialGDK
