// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "Utils/OpUtils.h"
#include "SpatialConstants.h"

namespace SpatialGDK
{
Worker_ComponentId GetComponentId(const Worker_Op& Op)
{
	switch (Op.op_type)
	{
	case WORKER_OP_TYPE_ADD_COMPONENT:
		return Op.op.add_component.data.component_id;
	case WORKER_OP_TYPE_REMOVE_COMPONENT:
		return Op.op.remove_component.component_id;
	case WORKER_OP_TYPE_COMPONENT_UPDATE:
		return Op.op.component_update.update.component_id;
	case WORKER_OP_TYPE_COMPONENT_SET_AUTHORITY_CHANGE:
		return Op.op.component_set_authority_change.component_set_id;
	case WORKER_OP_TYPE_COMMAND_REQUEST:
		return Op.op.command_request.request.component_id;
	case WORKER_OP_TYPE_COMMAND_RESPONSE:
		return Op.op.command_response.response.component_id;
	default:
		return SpatialConstants::INVALID_COMPONENT_ID;
	}
}

} // namespace SpatialGDK
