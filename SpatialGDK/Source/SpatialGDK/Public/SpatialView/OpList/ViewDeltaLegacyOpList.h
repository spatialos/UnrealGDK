// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "SpatialView/OpList/OpList.h"
#include "Containers/Array.h"
#include "Containers/UnrealString.h"
#include <improbable/c_worker.h>
#include <cstring>

namespace SpatialGDK
{

struct ViewDeltaLegacyOpListData: public OpListData
{
	TArray<Worker_Op> Ops;
	// Used to store UTF8 disconnect string.
	TUniquePtr<char[]> DisconnectReason;
};

inline OpList GetOpListFromViewDelta(const ViewDelta& Delta)
{
	// The order of op creation should be:
	// Disconnect (we do not need to add further ops if disconnected).
	// Add entities
	// Add components
	// Authority lost (from lost and lost temporarily)
	// Component updates (complete updates and regular updates)
	// Remove components
	// Authority gained (from gained and lost temporarily)
	// Entities Removed (can be reordered with authority gained)
	//
	// Worker messages can be placed anywhere.
	TUniquePtr<ViewDeltaLegacyOpListData> OpData = MakeUnique<ViewDeltaLegacyOpListData>();
	TArray<Worker_Op>& Ops = OpData->Ops;

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
		Ops.Push(Op);

		return {Ops.GetData(), static_cast<uint32>(Ops.Num()), MoveTemp(OpData)};
	}

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
		Op.op.add_component.data = Worker_ComponentData{nullptr, Data.Data.GetComponentId(),
			Data.Data.GetUnderlying(), nullptr};
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
		Worker_Op UpdateOp = {};
		UpdateOp.op_type = WORKER_OP_TYPE_COMPONENT_UPDATE;
		UpdateOp.op.component_update.entity_id = Update.EntityId;
		UpdateOp.op.component_update.update = Worker_ComponentUpdate{nullptr, Update.Events.GetComponentId(),
			Update.Events.GetUnderlying(), nullptr};
		Ops.Push(UpdateOp);

		Worker_Op AddOp = {};
		AddOp.op_type = WORKER_OP_TYPE_ADD_COMPONENT;
		AddOp.op.add_component.entity_id = Update.EntityId;
		AddOp.op.add_component.data = Worker_ComponentData{nullptr, Update.CompleteUpdate.GetComponentId(),
			Update.CompleteUpdate.GetUnderlying(), nullptr};
		Ops.Push(AddOp);
	}

	for (const EntityComponentUpdate& Update : Delta.GetUpdates())
	{
		Worker_Op Op = {};
		Op.op_type = WORKER_OP_TYPE_COMPONENT_UPDATE;
		Op.op.component_update.entity_id = Update.EntityId;
		Op.op.component_update.update = Worker_ComponentUpdate{nullptr, Update.Update.GetComponentId(),
            Update.Update.GetUnderlying(), nullptr};
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
		Op.op_type = WORKER_OP_TYPE_ADD_ENTITY;
		Op.op.remove_entity.entity_id = Id;
		Ops.Push(Op);
	}

	// Worker messages do not have ordering constraints.
	Ops.Append(Delta.GetWorkerMessages());

	return {Ops.GetData(), static_cast<uint32>(Ops.Num()), MoveTemp(OpData)};
}

}  // namespace SpatialGDK
