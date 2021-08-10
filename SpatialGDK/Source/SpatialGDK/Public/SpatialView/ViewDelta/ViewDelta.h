// Copyright (c) Improbable Worlds Ltd, All Rights Reserved
#pragma once
#include "Containers/Array.h"

#include "SpatialView/ComponentSetData.h"
#include "SpatialView/EntityDelta.h"
#include "SpatialView/EntityView.h"
#include "SpatialView/ViewDelta/IntermediateReceivedViewChange.h"

namespace SpatialGDK
{
struct FSubViewDelta
{
	TArray<EntityDelta> EntityDeltas;
	const TArray<Worker_Op>* WorkerMessages;
};

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
	void SetFromOpList(TArray<OpList> OpLists, EntityView& View, const FComponentSetData& ComponentSetData);
	// Produces a projection of a given main view delta to a sub view delta. The passed SubViewDelta is populated with
	// the projection. The given arrays represent the state of the sub view and dictates the projection.
	// Entity ID arrays are assumed to be sorted for view delta projection.
	void Project(FSubViewDelta& SubDelta, const TArray<Worker_EntityId>& CompleteEntities,
				 const TArray<Worker_EntityId>& NewlyCompleteEntities, const TArray<Worker_EntityId>& NewlyIncompleteEntities,
				 const TArray<Worker_EntityId>& TemporarilyIncompleteEntities) const;
	void Clear();
	const TArray<EntityDelta>& GetEntityDeltas() const;
	const TArray<Worker_Op>& GetWorkerMessages() const;
	bool HasConnectionStatusChanged() const;
	Worker_ConnectionStatusCode GetConnectionStatusChange() const;
	FString GetConnectionStatusChangeMessage() const;

private:
	// Calculate and return the net component added in [`Start`, `End`).
	// Also add the resulting component to `Components`.
	// The accumulated component change in this range must be a component add.
	static ComponentChange CalculateAdd(FReceivedComponentChange* Start, FReceivedComponentChange* End, TArray<ComponentData>& Components);
	// Calculate and return the net complete update in [`Start`, `End`).
	// Also set `Component` to match.
	// The accumulated component change in this range must be a complete-update or
	// `Data` and `Events` should be non null.
	static ComponentChange CalculateCompleteUpdate(FReceivedComponentChange* Start, FReceivedComponentChange* End,
												   Schema_ComponentData* Data, Schema_ComponentUpdate* Events, ComponentData& Component);
	// Calculate and return the net update in [`Start`, `End`).
	// Also apply the update to `Component`.
	// The accumulated component change in this range must be an update or a complete-update.
	static ComponentChange CalculateUpdate(FReceivedComponentChange* Start, FReceivedComponentChange* End, ComponentData& Component);

	// The sentinel entity ID has the property that when converted to a uint64 it will be greater than INT64_MAX.
	// If we convert all entity IDs to uint64s before comparing them we can then be assured that the sentinel value
	// will be greater than all valid IDs.
	static const Worker_EntityId SENTINEL_ENTITY_ID = -1;

	void PopulateEntityDeltas(EntityView& View);

	// Adds component changes to `Delta` and updates `Components` accordingly.
	// `It` must point to the first element with a given entity ID.
	// Returns a pointer to the next entity in the component changes list.
	FReceivedComponentChange* ProcessEntityComponentChanges(FReceivedComponentChange* It, FReceivedComponentChange* End,
															TArray<ComponentData>& Components, EntityDelta& Delta);
	// Adds authority changes to `Delta` and updates `EntityAuthority` accordingly.
	// `It` must point to the first element with a given entity ID.
	// Returns a pointer to the next entity in the authority changes list.
	FReceivedAuthorityChange* ProcessEntityAuthorityChanges(FReceivedAuthorityChange* It, FReceivedAuthorityChange* End,
															TArray<Worker_ComponentSetId>& EntityAuthority, EntityDelta& Delta);
	// Sets `bAdded` and `bRemoved` fields in the `Delta`.
	// `It` must point to the first element with a given entity ID.
	// `ViewElement` must point to the same entity in the view or end if it doesn't exist.
	// Returns a pointer to the next entity in the authority changes list.
	// After returning `*ViewElement` will point to that entity in the view or nullptr if it doesn't exist.
	FReceivedEntityChange* ProcessEntityExistenceChange(FReceivedEntityChange* It, FReceivedEntityChange* End, EntityDelta& Delta,
														bool bAlreadyInView, EntityView& View);

	TArray<EntityDelta> EntityDeltas;
	TArray<AuthorityChange> AuthorityGainedForDelta;
	TArray<AuthorityChange> AuthorityLostForDelta;
	TArray<AuthorityChange> AuthorityLostTempForDelta;
	TArray<ComponentChange> ComponentsAddedForDelta;
	TArray<ComponentChange> ComponentsRemovedForDelta;
	TArray<ComponentChange> ComponentUpdatesForDelta;
	TArray<ComponentChange> ComponentsRefreshedForDelta;

	FIntermediateReceivedViewChange ViewChange;
};
} // namespace SpatialGDK
