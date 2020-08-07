// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "SpatialView/OpList/EntityComponentOpList.h"

namespace SpatialGDK
{
EntityComponentOpListBuilder& EntityComponentOpListBuilder::AddComponent(Worker_EntityId EntityId, ComponentData Data)
{
	Worker_Op Op = {};
	Op.op_type = WORKER_OP_TYPE_ADD_COMPONENT;
	Op.op.add_component.entity_id = EntityId;
	Op.op.add_component.data = Data.GetWorkerComponentData();
	OpListData->DataStorage.Emplace(MoveTemp(Data));

	OpListData->Ops.Add(Op);
	return *this;
}

EntityComponentOpListBuilder& EntityComponentOpListBuilder::UpdateComponent(Worker_EntityId EntityId, ComponentUpdate Update)
{
	Worker_Op Op = {};
	Op.op_type = WORKER_OP_TYPE_COMPONENT_UPDATE;
	Op.op.component_update.entity_id = EntityId;
	Op.op.component_update.update = Update.GetWorkerComponentUpdate();
	OpListData->UpdateStorage.Emplace(MoveTemp(Update));

	OpListData->Ops.Add(Op);
	return *this;
}

EntityComponentOpListBuilder& EntityComponentOpListBuilder::RemoveComponent(Worker_EntityId EntityId, Worker_ComponentId ComponentId)
{
	Worker_Op Op = {};
	Op.op_type = WORKER_OP_TYPE_REMOVE_COMPONENT;
	Op.op.remove_component.entity_id = EntityId;
	Op.op.remove_component.component_id = ComponentId;

	OpListData->Ops.Add(Op);
	return *this;
}

EntityComponentOpListBuilder& EntityComponentOpListBuilder::SetAuthority(Worker_EntityId EntityId, Worker_ComponentId ComponentId,
																		 Worker_Authority Authority)
{
	Worker_Op Op = {};
	Op.op_type = WORKER_OP_TYPE_AUTHORITY_CHANGE;
	Op.op.authority_change.entity_id = EntityId;
	Op.op.authority_change.component_id = ComponentId;
	Op.op.authority_change.authority = Authority;

	OpListData->Ops.Add(Op);
	return *this;
}

OpList EntityComponentOpListBuilder::CreateOpList() &&
{
	return { OpListData->Ops.GetData(), static_cast<uint32>(OpListData->Ops.Num()), MoveTemp(OpListData) };
}

} // namespace SpatialGDK
