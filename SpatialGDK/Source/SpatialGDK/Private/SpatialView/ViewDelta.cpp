// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "SpatialView/ViewDelta.h"
#include "SpatialView/OpList/ViewDeltaLegacyOpList.h"
#include "Containers/StringConv.h"

namespace SpatialGDK
{

void ViewDelta::AddCreateEntityResponse(CreateEntityResponse Response)
{
	CreateEntityResponses.Push(MoveTemp(Response));
}

void ViewDelta::SetAuthority(Worker_EntityId EntityId, Worker_ComponentId ComponentId, Worker_Authority Authority)
{
	AuthorityChanges.SetAuthority(EntityId, ComponentId, Authority);
}

const TArray<CreateEntityResponse>& ViewDelta::GetCreateEntityResponses() const
{
	return CreateEntityResponses;
}

const TArray<EntityComponentId>& ViewDelta::GetAuthorityGained() const
{
	return AuthorityChanges.GetAuthorityGained();
}

const TArray<EntityComponentId>& ViewDelta::GetAuthorityLost() const
{
	return AuthorityChanges.GetAuthorityLost();
}

const TArray<EntityComponentId>& ViewDelta::GetAuthorityLostTemporarily() const
{
	return AuthorityChanges.GetAuthorityLostTemporarily();
}

TUniquePtr<AbstractOpList> ViewDelta::GenerateLegacyOpList() const
{
	// Todo - refactor individual op creation to an oplist type.
	TArray<Worker_Op> OpList;
	OpList.Reserve(CreateEntityResponses.Num());

	// todo Entity added ops get created here.

	// todo Component Added ops get created here.

	for (const EntityComponentId& Id : AuthorityChanges.GetAuthorityLost())
	{
		Worker_Op Op = {};
		Op.op_type = WORKER_OP_TYPE_AUTHORITY_CHANGE;
		Op.op.authority_change.entity_id = Id.EntityId;
		Op.op.authority_change.component_id = Id.ComponentId;
		Op.op.authority_change.authority = WORKER_AUTHORITY_NOT_AUTHORITATIVE;
		OpList.Push(Op);
	}

	for (const EntityComponentId& Id : AuthorityChanges.GetAuthorityLostTemporarily())
	{
		Worker_Op Op = {};
		Op.op_type = WORKER_OP_TYPE_AUTHORITY_CHANGE;
		Op.op.authority_change.entity_id = Id.EntityId;
		Op.op.authority_change.component_id = Id.ComponentId;
		Op.op.authority_change.authority = WORKER_AUTHORITY_NOT_AUTHORITATIVE;
		OpList.Push(Op);
	}

	// todo Component update and remove ops get created here.

	// todo Entity removed ops get created here or below.

	for (const EntityComponentId& Id : AuthorityChanges.GetAuthorityLostTemporarily())
	{
		Worker_Op Op = {};
		Op.op_type = WORKER_OP_TYPE_AUTHORITY_CHANGE;
		Op.op.authority_change.entity_id = Id.EntityId;
		Op.op.authority_change.component_id = Id.ComponentId;
		Op.op.authority_change.authority = WORKER_AUTHORITY_AUTHORITATIVE;
		OpList.Push(Op);
	}

	for (const EntityComponentId& Id : AuthorityChanges.GetAuthorityGained())
	{
		Worker_Op Op = {};
		Op.op_type = WORKER_OP_TYPE_AUTHORITY_CHANGE;
		Op.op.authority_change.entity_id = Id.EntityId;
		Op.op.authority_change.component_id = Id.ComponentId;
		Op.op.authority_change.authority = WORKER_AUTHORITY_AUTHORITATIVE;
		OpList.Push(Op);
	}

	// todo Command requests ops are created here.

	// The following ops do not have ordering constraints.

	for (const CreateEntityResponse& Response : CreateEntityResponses)
	{
		Worker_Op Op = {};
		Op.op_type = WORKER_OP_TYPE_CREATE_ENTITY_RESPONSE;
		Op.op.create_entity_response.request_id = Response.RequestId;
		Op.op.create_entity_response.status_code = Response.StatusCode;
		Op.op.create_entity_response.message = TCHAR_TO_UTF8(Response.Message.GetCharArray().GetData());
		Op.op.create_entity_response.entity_id = Response.EntityId;
		OpList.Push(Op);
	}

	return MakeUnique<ViewDeltaLegacyOpList>(MoveTemp(OpList));
}

void ViewDelta::Clear()
{
	CreateEntityResponses.Empty();
	AuthorityChanges.Clear();
}

}  // namespace SpatialGDK
