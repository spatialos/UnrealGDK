// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "SpatialView/ViewDelta/IntermediateReceivedViewChange.h"

#include "Algo/StableSort.h"

namespace SpatialGDK
{
void FIntermediateReceivedViewChange::SetFromOpList(TArray<OpList> OpLists, const EntityView& View,
													const FComponentSetData& ComponentSetData)
{
	Clear();
	OpListStorage = MoveTemp(OpLists);
	for (OpList& Ops : OpListStorage)
	{
		ExtractChangesFromOpList(Ops, View, ComponentSetData);
	}

	Algo::StableSort(ComponentChanges, EntityComponentComparison{});
	Algo::StableSort(AuthorityChanges, EntityComponentComparison{});
	Algo::StableSort(EntityChanges, EntityComparison{});
}

void FIntermediateReceivedViewChange::Clear()
{
	EntityChanges.Reset();
	ComponentChanges.Reset();
	AuthorityChanges.Reset();
	WorkerMessages.Reset();

	ConnectionStatusCode = 0;
	ConnectionStatusMessage.Empty();
}

void FIntermediateReceivedViewChange::ExtractChangesFromOpList(const OpList& Ops, const EntityView& View,
															   const FComponentSetData& ComponentSetData)
{
	for (uint32 i = 0; i < Ops.Count; ++i)
	{
		const Worker_Op& Op = Ops.Ops[i];
		switch (static_cast<Worker_OpType>(Op.op_type))
		{
		case WORKER_OP_TYPE_DISCONNECT:
			ConnectionStatusCode = Op.op.disconnect.connection_status_code;
			ConnectionStatusMessage = Op.op.disconnect.reason;
			break;
		case WORKER_OP_TYPE_CRITICAL_SECTION:
			// Ignore critical sections.
			break;
		case WORKER_OP_TYPE_ADD_ENTITY:
			EntityChanges.Push(ReceivedEntityChange{ Op.op.add_entity.entity_id, true });
			break;
		case WORKER_OP_TYPE_REMOVE_ENTITY:
			EntityChanges.Push(ReceivedEntityChange{ Op.op.remove_entity.entity_id, false });
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
		case WORKER_OP_TYPE_COMPONENT_SET_AUTHORITY_CHANGE:
			GenerateComponentChangesFromSetData(Op.op.component_set_authority_change, View, ComponentSetData);
			AuthorityChanges.Push(ReceivedAuthorityChange{ Op.op.component_set_authority_change.entity_id,
														   Op.op.component_set_authority_change.component_set_id,
														   static_cast<bool>(Op.op.component_set_authority_change.authority) });
			break;
		case WORKER_OP_TYPE_COMPONENT_UPDATE:
			ComponentChanges.Emplace(Op.op.component_update);
			break;
		default:
			break;
		}
	}
}

void FIntermediateReceivedViewChange::GenerateComponentChangesFromSetData(const Worker_ComponentSetAuthorityChangeOp& Op,
																		  const EntityView& View, const FComponentSetData& ComponentSetData)
{
	// Generate component changes to:
	// * Remove all components on the entity, that are in the component set.
	// * Add all components the with data in the op.
	// If one component is both removed and added then this is interpreted as component refresh in the view delta.
	// Otherwise the component will be added or removed as appropriate.

	const TSet<Worker_ComponentId>& Set = ComponentSetData.ComponentSets[Op.component_set_id];

	// If a component on the entity is in the set then generate a remove operation.
	if (const EntityViewElement* Entity = View.Find(Op.entity_id))
	{
		for (const ComponentData& Component : Entity->Components)
		{
			const Worker_ComponentId ComponentId = Component.GetComponentId();
			if (Set.Contains(ComponentId))
			{
				Worker_RemoveComponentOp RemoveOp = { Op.entity_id, ComponentId };
				ComponentChanges.Emplace(RemoveOp);
			}
		}
	}

	// If the component has data in the authority op then generate an add operation.
	for (uint32 i = 0; i < Op.canonical_component_set_data_count; ++i)
	{
		Worker_AddComponentOp AddOp = { Op.entity_id, Op.canonical_component_set_data[i] };
		ComponentChanges.Emplace(AddOp);
	}
}

} // namespace SpatialGDK
