// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "SpatialView/OpList/ViewDeltaLegacyOpList.h"

#include "Algo/StableSort.h"
#include "Containers/StringConv.h"

namespace
{
Worker_EntityId GetEntityIdFromOp(const Worker_Op& Op)
{
	switch (static_cast<Worker_OpType>(Op.op_type))
	{
	case WORKER_OP_TYPE_ADD_ENTITY:
		return Op.op.add_entity.entity_id;
	case WORKER_OP_TYPE_REMOVE_ENTITY:
		return Op.op.remove_entity.entity_id;
	case WORKER_OP_TYPE_ADD_COMPONENT:
		return Op.op.add_component.entity_id;
	case WORKER_OP_TYPE_REMOVE_COMPONENT:
		return Op.op.remove_component.entity_id;
	case WORKER_OP_TYPE_AUTHORITY_CHANGE:
		return Op.op.authority_change.entity_id;
	case WORKER_OP_TYPE_COMPONENT_UPDATE:
		return Op.op.component_update.entity_id;
	default:
		checkNoEntry();
	}
	return 0;
}

} // anonymous namespace

namespace SpatialGDK
{
OpList GetOpListFromViewDelta(ViewDelta Delta)
{
	// The order of ops should be:
	// Disconnect (we do not need to add further ops if disconnected).
	// Add entities
	// Add components
	// Authority lost (from lost and lost temporarily)
	// Component updates (complete updates and regular updates)
	// Remove components
	// Authority gained (from gained and lost temporarily)
	// Entities Removed (can be reordered with authority gained)
	//
	// We can then order this by entity ID and surround the ops for each entity in a critical section.
	//
	// Worker messages can be placed anywhere.

	auto OpData = MakeUnique<ViewDeltaLegacyOpListData>();

	if (Delta.HasDisconnected())
	{
		Worker_Op Op = {};
		Op.op_type = WORKER_OP_TYPE_DISCONNECT;
		Op.op.disconnect.connection_status_code = Delta.GetConnectionStatus();

		// Convert an FString to a char* that we can store.
		const TCHAR* Reason = *Delta.GetDisconnectReason();
		int32 SourceLength = TCString<TCHAR>::Strlen(Reason);
		// Includes the null terminator.
		int32 BufferSize = FTCHARToUTF8_Convert::ConvertedLength(Reason, SourceLength) + 1;
		OpData->DisconnectReason = MakeUnique<char[]>(BufferSize);
		FTCHARToUTF8_Convert::Convert(OpData->DisconnectReason.Get(), BufferSize, Reason, SourceLength + 1);

		Op.op.disconnect.reason = OpData->DisconnectReason.Get();
		OpData->Ops.Push(Op);

		return { OpData->Ops.GetData(), static_cast<uint32>(OpData->Ops.Num()), MoveTemp(OpData) };
	}

	TArray<Worker_Op> Ops;

	for (const Worker_EntityId& Id : Delta.GetEntitiesAdded())
	{
		Worker_Op Op = {};
		Op.op_type = WORKER_OP_TYPE_ADD_ENTITY;
		Op.op.add_entity.entity_id = Id;
		Ops.Push(Op);
	}

	for (const EntityComponentData& Data : Delta.GetComponentsAdded())
	{
		Worker_Op Op = {};
		Op.op_type = WORKER_OP_TYPE_ADD_COMPONENT;
		Op.op.add_component.entity_id = Data.EntityId;
		Op.op.add_component.data = Worker_ComponentData{ nullptr, Data.Data.GetComponentId(), Data.Data.GetUnderlying(), nullptr };
		Ops.Push(Op);
	}

	for (const EntityComponentId& Id : Delta.GetAuthorityLost())
	{
		Worker_Op Op = {};
		Op.op_type = WORKER_OP_TYPE_AUTHORITY_CHANGE;
		Op.op.authority_change.entity_id = Id.EntityId;
		Op.op.authority_change.component_id = Id.ComponentId;
		Op.op.authority_change.authority = WORKER_AUTHORITY_NOT_AUTHORITATIVE;
		Ops.Push(Op);
	}

	for (const EntityComponentId& Id : Delta.GetAuthorityLostTemporarily())
	{
		Worker_Op Op = {};
		Op.op_type = WORKER_OP_TYPE_AUTHORITY_CHANGE;
		Op.op.authority_change.entity_id = Id.EntityId;
		Op.op.authority_change.component_id = Id.ComponentId;
		Op.op.authority_change.authority = WORKER_AUTHORITY_NOT_AUTHORITATIVE;
		Ops.Push(Op);
	}

	for (const EntityComponentCompleteUpdate& Update : Delta.GetCompleteUpdates())
	{
		// We deliberately ignore the events update here to avoid breaking code that expects each update to contain data.
		Worker_Op AddOp = {};
		AddOp.op_type = WORKER_OP_TYPE_ADD_COMPONENT;
		AddOp.op.add_component.entity_id = Update.EntityId;
		AddOp.op.add_component.data =
			Worker_ComponentData{ nullptr, Update.CompleteUpdate.GetComponentId(), Update.CompleteUpdate.GetUnderlying(), nullptr };
		Ops.Push(AddOp);
	}

	for (const EntityComponentUpdate& Update : Delta.GetUpdates())
	{
		Worker_Op Op = {};
		Op.op_type = WORKER_OP_TYPE_COMPONENT_UPDATE;
		Op.op.component_update.entity_id = Update.EntityId;
		Op.op.component_update.update =
			Worker_ComponentUpdate{ nullptr, Update.Update.GetComponentId(), Update.Update.GetUnderlying(), nullptr };
		Ops.Push(Op);
	}

	for (const EntityComponentId& Id : Delta.GetComponentsRemoved())
	{
		Worker_Op Op = {};
		Op.op_type = WORKER_OP_TYPE_REMOVE_COMPONENT;
		Op.op.remove_component.entity_id = Id.EntityId;
		Op.op.remove_component.component_id = Id.ComponentId;
		Ops.Push(Op);
	}

	for (const EntityComponentId& Id : Delta.GetAuthorityLostTemporarily())
	{
		Worker_Op Op = {};
		Op.op_type = WORKER_OP_TYPE_AUTHORITY_CHANGE;
		Op.op.authority_change.entity_id = Id.EntityId;
		Op.op.authority_change.component_id = Id.ComponentId;
		Op.op.authority_change.authority = WORKER_AUTHORITY_AUTHORITATIVE;
		Ops.Push(Op);
	}

	for (const EntityComponentId& Id : Delta.GetAuthorityGained())
	{
		Worker_Op Op = {};
		Op.op_type = WORKER_OP_TYPE_AUTHORITY_CHANGE;
		Op.op.authority_change.entity_id = Id.EntityId;
		Op.op.authority_change.component_id = Id.ComponentId;
		Op.op.authority_change.authority = WORKER_AUTHORITY_AUTHORITATIVE;
		Ops.Push(Op);
	}

	for (const Worker_EntityId& Id : Delta.GetEntitiesRemoved())
	{
		Worker_Op Op = {};
		Op.op_type = WORKER_OP_TYPE_REMOVE_ENTITY;
		Op.op.remove_entity.entity_id = Id;
		Ops.Push(Op);
	}

	// Sort the entity ops by entity ID and surround each set of entity ops with a critical section.
	Algo::StableSort(Ops, [](const Worker_Op& Lhs, const Worker_Op& Rhs) {
		return GetEntityIdFromOp(Lhs) < GetEntityIdFromOp(Rhs);
	});

	OpData->Ops.Reserve(Ops.Num());

	Worker_EntityId PreviousEntityId = 0;
	for (const Worker_Op& Op : Ops)
	{
		const Worker_EntityId CurrentEntityId = GetEntityIdFromOp(Op);

		if (CurrentEntityId > PreviousEntityId)
		{
			if (PreviousEntityId != 0)
			{
				Worker_Op EndCriticalSection = {};
				EndCriticalSection.op_type = WORKER_OP_TYPE_CRITICAL_SECTION;
				EndCriticalSection.op.critical_section.in_critical_section = 0;
				OpData->Ops.Add(EndCriticalSection);
			}

			Worker_Op StartCriticalSection = {};
			StartCriticalSection.op_type = WORKER_OP_TYPE_CRITICAL_SECTION;
			StartCriticalSection.op.critical_section.in_critical_section = 1;
			OpData->Ops.Add(StartCriticalSection);

			PreviousEntityId = CurrentEntityId;
		}
		OpData->Ops.Add(Op);
	}

	if (PreviousEntityId > 0)
	{
		Worker_Op EndCriticalSection = {};
		EndCriticalSection.op_type = WORKER_OP_TYPE_CRITICAL_SECTION;
		EndCriticalSection.op.critical_section.in_critical_section = 0;
		OpData->Ops.Add(EndCriticalSection);
	}

	// Worker messages do not have ordering constraints so can just go at the end.
	OpData->Ops.Append(Delta.GetWorkerMessages());

	OpData->Delta = MoveTemp(Delta);
	return { OpData->Ops.GetData(), static_cast<uint32>(OpData->Ops.Num()), MoveTemp(OpData) };
}

} // namespace SpatialGDK
