// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "SpatialView/ComponentSetData.h"
#include "SpatialView/EntityView.h"
#include "SpatialView/OpList/OpList.h"
#include "SpatialView/ViewDelta/ReceivedViewChangeTypes.h"

namespace SpatialGDK
{
/**
 * A structured representation of information contained in a set of op lists.
 * Ops are divided into 4 buckets with the exception of disconnect and critical_section ops.
 *
 * We parse disconnect ops differently as there can only be one and it invalidates everything else received.
 * We ignore critical_section ops as a partial critical_section is considered an invalid view-change.
 * It is assumed all op lists provided represent a valid view-change.
 *
 * For the rest the buckets are:
 *
 *	Entity-changes are ops pertaining to an entity being added or removed from view.
 *		This includes add_entity and remove_entity.
 *
 *	Component-changes are ops that change the state of an entity in view.
 *		This includes add_component, remove_component, and component_update.
 *
 *	Authority-changes are ops that denote a change of component-set authority.
 *		This is only component_set_authority_change.
 *
 *	Worker-messages are everything else.
 *
 * For each bucket there is a single array of changes.
 * To populate these, the Ops are traversed in the order they appear and a representation of them is appended to the relevant array.
 *
 * Entity-change, component-change, and authority-change are then sorted by entity ID.
 * Elements with the same entity ID are sorted by component/component-set ID if applicable.
 * Elements with the same entity ID and component/component-set ID are kept in the same order as they appeared in the oplist.
 * After being sorted a sentinel element is added to each bucket with ID = -1.
 * This is because it makes traversing the different buckets together to find the net change for a specific entity easier.
 */
struct FIntermediateReceivedViewChange
{
	// Resets all state to represent whatever is stored in `OpLists`. This has no preconditions.
	void SetFromOpList(TArray<OpList> OpLists, const EntityView& View, const FComponentSetData& ComponentSetData);

	TArray<FReceivedEntityChange> EntityChanges;
	TArray<FReceivedComponentChange> ComponentChanges;
	TArray<FReceivedAuthorityChange> AuthorityChanges;
	TArray<Worker_Op> WorkerMessages;

	uint8 ConnectionStatusCode = 0;
	FString ConnectionStatusMessage;

private:
	void Clear();
	void ExtractChangesFromOpList(const OpList& Ops, const EntityView& View, const FComponentSetData& ComponentSetData);
	void GenerateComponentChangesFromSetData(const Worker_ComponentSetAuthorityChangeOp& Op, const EntityView& View,
											 const FComponentSetData& ComponentSetData);

	TArray<OpList> OpListStorage;
};

} // namespace SpatialGDK
