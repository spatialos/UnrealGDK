// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once
#include <improbable/c_worker.h>

#include "Schema/RPCPayload.h"
#include "SpatialView/CommandRequest.h"
#include "SpatialView/CommandResponse.h"

namespace SpatialGDK
{
// Writes to a command on a component with the form:
// component RingBufferComponent {
//   id = ComponentId;
//   command EmptyResponseType rpc_command(Rpc) = CommandIndex;
// }
class FCommandRpcWriter
{
public:
	FCommandRpcWriter(Worker_ComponentId ComponentId, Worker_CommandIndex CommandIndex)
		: ComponentId(ComponentId)
		, CommandIndex(CommandIndex)
	{
	}

	CommandRequest CreateRequest(RPCPayload Rpc) const
	{
		CommandRequest Request(ComponentId, CommandIndex);
		return Request;
	}

	CommandResponse CreateResponse() const
	{
		CommandResponse Response(ComponentId, CommandIndex);
		return Response;
	}

private:
	Worker_ComponentId ComponentId;
	Worker_CommandIndex CommandIndex;
};

} // namespace SpatialGDK
