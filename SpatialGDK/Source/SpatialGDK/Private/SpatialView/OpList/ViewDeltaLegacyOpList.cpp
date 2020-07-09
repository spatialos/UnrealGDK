// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "SpatialView/OpList/ViewDeltaLegacyOpList.h"

#include "Containers/StringConv.h"

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

		return {OpData->Ops.GetData(), static_cast<uint32>(OpData->Ops.Num()), MoveTemp(OpData)};
	}

	TArray<Worker_Op>& Ops = OpData->Ops;

	for (const EntityDelta& Entity : Delta.GetEntityDeltas())
	{
		Worker_Op StartCriticalSection = {};
		StartCriticalSection.op_type = WORKER_OP_TYPE_CRITICAL_SECTION;
		StartCriticalSection.op.critical_section.in_critical_section = 1;
		Ops.Add(StartCriticalSection);

		if (Entity.bAdded)
		{
			Worker_Op Op = {};
			Op.op_type = WORKER_OP_TYPE_ADD_ENTITY;
			Op.op.add_entity.entity_id = Entity.EntityId;
			Ops.Push(Op);
		}

		for (const ComponentChange& Change : Entity.ComponentChanges)
		{
			if (Change.Type == ComponentChange::ADD)
			{
				Worker_Op Op = {};
				Op.op_type = WORKER_OP_TYPE_ADD_COMPONENT;
				Op.op.add_component.entity_id = Entity.EntityId;
				Op.op.add_component.data = Worker_ComponentData{ nullptr, Change.ComponentId, Change.Data, nullptr };
				Ops.Push(Op);
			}
		}

		for (const AuthorityChange& Change : Entity.AuthorityChanges)
		{
			if (Change.Type == AuthorityChange::AUTHORITY_LOST || Change.Type == AuthorityChange::AUTHORITY_LOST_TEMPORARILY)
			{
				Worker_Op Op = {};
				Op.op_type = WORKER_OP_TYPE_AUTHORITY_CHANGE;
				Op.op.authority_change.entity_id = Entity.EntityId;
				Op.op.authority_change.component_id = Change.ComponentId;
				Op.op.authority_change.authority = WORKER_AUTHORITY_NOT_AUTHORITATIVE;
				Ops.Push(Op);
			}
		}

		for (const ComponentChange& Change : Entity.ComponentChanges)
		{
			if (Change.Type == ComponentChange::COMPLETE_UPDATE)
			{
				// We deliberately ignore the events update here to avoid breaking code that expects each update to contain data.
				Worker_Op AddOp = {};
				AddOp.op_type = WORKER_OP_TYPE_ADD_COMPONENT;
				AddOp.op.add_component.entity_id = Entity.EntityId;
				AddOp.op.add_component.data = Worker_ComponentData{ nullptr, Change.ComponentId, Change.CompleteUpdate.Data, nullptr };
				Ops.Push(AddOp);
			}

			if (Change.Type == ComponentChange::UPDATE)
			{
				Worker_Op Op = {};
				Op.op_type = WORKER_OP_TYPE_COMPONENT_UPDATE;
				Op.op.component_update.entity_id = Entity.EntityId;
				Op.op.component_update.update = Worker_ComponentUpdate{ nullptr, Change.ComponentId, Change.Update, nullptr };
				Ops.Push(Op);
			}

			if (Change.Type == ComponentChange::REMOVE)
			{
				Worker_Op Op = {};
				Op.op_type = WORKER_OP_TYPE_REMOVE_COMPONENT;
				Op.op.remove_component.entity_id = Entity.EntityId;
				Op.op.remove_component.component_id = Change.ComponentId;
				Ops.Push(Op);
			}
		}

		for (const AuthorityChange& Change : Entity.AuthorityChanges)
		{
			if (Change.Type == AuthorityChange::AUTHORITY_GAINED || Change.Type == AuthorityChange::AUTHORITY_LOST_TEMPORARILY)
			{
				Worker_Op Op = {};
				Op.op_type = WORKER_OP_TYPE_AUTHORITY_CHANGE;
				Op.op.authority_change.entity_id = Entity.EntityId;
				Op.op.authority_change.component_id = Change.ComponentId;
				Op.op.authority_change.authority = WORKER_AUTHORITY_AUTHORITATIVE;
				Ops.Push(Op);
			}
		}

		if (Entity.bRemoved)
		{
			Worker_Op Op = {};
			Op.op_type = WORKER_OP_TYPE_REMOVE_ENTITY;
			Op.op.remove_entity.entity_id = Entity.EntityId;
			Ops.Push(Op);
		}

		Worker_Op EndCriticalSection = {};
		EndCriticalSection.op_type = WORKER_OP_TYPE_CRITICAL_SECTION;
		EndCriticalSection.op.critical_section.in_critical_section = 0;
		Ops.Add(EndCriticalSection);
	}

	// Worker messages do not have ordering constraints so can just go at the end.
	Ops.Append(Delta.GetWorkerMessages());

	OpData->Delta = MoveTemp(Delta);
	return {OpData->Ops.GetData(), static_cast<uint32>(OpData->Ops.Num()), MoveTemp(OpData)};
}

} // namespace SpatialGDK
