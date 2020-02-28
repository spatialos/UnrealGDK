// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "ViewDelta.h"
#include "OpList/ViewDeltaLegacyOpList.h"
#include "Containers/StringConv.h"

namespace SpatialGDK
{

void ViewDelta::AddCreateEntityResponse(CreateEntityResponse Response)
{
	CreateEntityResponses.Push(MoveTemp(Response));
}

const TArray<CreateEntityResponse>& ViewDelta::GetCreateEntityResponses() const
{
	return CreateEntityResponses;
}

TUniquePtr<AbstractOpList> ViewDelta::GenerateLegacyOpList() const
{
	TArray<Worker_Op> OpList;
	OpList.Reserve(CreateEntityResponses.Num());

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
}

}  // namespace SpatialGDK
