// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "WorkerView.h"
#include "MessagesToSend.h"

namespace SpatialGDK
{

WorkerView::WorkerView()
: LocalChanges(MakeUnique<MessagesToSend>())
{
}

const ViewDelta* WorkerView::GenerateViewDelta()
{
	Delta.Clear();
	for (const auto& OpList : QueuedOps)
	{
		const uint32 OpCount = OpList->GetCount();
		for (uint32 i = 0; i < OpCount; ++i)
		{
			ProcessOp((*OpList)[i]);
		}
	}

	return &Delta;
}

void WorkerView::EnqueueOpList(TUniquePtr<AbstractOpList> OpList)
{
	QueuedOps.Push(MoveTemp(OpList));
}

TUniquePtr<MessagesToSend> WorkerView::FlushLocalChanges()
{
	TUniquePtr<MessagesToSend> OutgoingMessages = MoveTemp(LocalChanges);
	LocalChanges = MakeUnique<MessagesToSend>();
	return OutgoingMessages;
}

void WorkerView::SendCreateEntityRequest(CreateEntityRequest Request)
{
	LocalChanges->CreateEntityRequests.Push(MoveTemp(Request));
}

void WorkerView::ProcessOp(const Worker_Op& Op)
{
	switch (static_cast<Worker_OpType>(Op.op_type))
	{
	case WORKER_OP_TYPE_DISCONNECT:
		break;
	case WORKER_OP_TYPE_FLAG_UPDATE:
		break;
	case WORKER_OP_TYPE_LOG_MESSAGE:
		break;
	case WORKER_OP_TYPE_METRICS:
		break;
	case WORKER_OP_TYPE_CRITICAL_SECTION:
		break;
	case WORKER_OP_TYPE_ADD_ENTITY:
		break;
	case WORKER_OP_TYPE_REMOVE_ENTITY:
		break;
	case WORKER_OP_TYPE_RESERVE_ENTITY_IDS_RESPONSE:
		break;
	case WORKER_OP_TYPE_CREATE_ENTITY_RESPONSE:
		HandleCreateEntityResponse(Op.op.create_entity_response);
		break;
	case WORKER_OP_TYPE_DELETE_ENTITY_RESPONSE:
		break;
	case WORKER_OP_TYPE_ENTITY_QUERY_RESPONSE:
		break;
	case WORKER_OP_TYPE_ADD_COMPONENT:
		break;
	case WORKER_OP_TYPE_REMOVE_COMPONENT:
		break;
	case WORKER_OP_TYPE_AUTHORITY_CHANGE:
		break;
	case WORKER_OP_TYPE_COMPONENT_UPDATE:
		break;
	case WORKER_OP_TYPE_COMMAND_REQUEST:
		break;
	case WORKER_OP_TYPE_COMMAND_RESPONSE:
		break;
	}
}

void WorkerView::HandleCreateEntityResponse(const Worker_CreateEntityResponseOp& Response)
{
	Delta.AddCreateEntityResponse(CreateEntityResponse{
		Response.request_id,
		static_cast<Worker_StatusCode>(Response.status_code),
		FString{Response.message},
		Response.entity_id
	});
}

}  // namespace SpatialGDK
