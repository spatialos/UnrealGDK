// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "SpatialView/ViewCoordinator.h"
#include "SpatialView/OpList/ViewDeltaLegacyOpList.h"

#include <cstring>

namespace SpatialGDK
{
ViewCoordinator::ViewCoordinator(TUniquePtr<AbstractConnectionHandler> ConnectionHandler)
	: ConnectionHandler(MoveTemp(ConnectionHandler))
	, NextRequestId(1)
	, FenceQueryRequestId(0)
{
}

ViewCoordinator::~ViewCoordinator()
{
	FlushMessagesToSend();
}

void ViewCoordinator::Advance()
{
	ConnectionHandler->Advance();
	const uint32 OpListCount = ConnectionHandler->GetOpListCount();
	for (uint32 i = 0; i < OpListCount; ++i)
	{
		View.EnqueueOpList(ConnectionHandler->GetNextOpList());
	}
	View.AdvanceViewDelta();

	if (PendingFences.Num() > 0)
	{
		CheckFenceResponse();
	}
	if (PendingFences.Num() > 0)
	{
		CheckFenceComponentAuthority();
	}
}

const ViewDelta& ViewCoordinator::GetViewDelta()
{
	return View.GetViewDelta();
}

const EntityView& ViewCoordinator::GetView()
{
	return View.GetView();
}

void ViewCoordinator::FlushMessagesToSend()
{
	if (PendingFences.Num() == 0)
	{
		ConnectionHandler->SendMessages(View.FlushLocalChanges());
	}
}

void ViewCoordinator::SendAddComponent(Worker_EntityId EntityId, ComponentData Data)
{
	View.SendAddComponent(EntityId, MoveTemp(Data));
}

void ViewCoordinator::SendComponentUpdate(Worker_EntityId EntityId, ComponentUpdate Update)
{
	View.SendComponentUpdate(EntityId, MoveTemp(Update));
}

void ViewCoordinator::SendRemoveComponent(Worker_EntityId EntityId, Worker_ComponentId ComponentId)
{
	View.SendRemoveComponent(EntityId, ComponentId);
}

Worker_RequestId ViewCoordinator::SendReserveEntityIdsRequest(uint32 NumberOfEntityIds, TOptional<uint32> TimeoutMillis)
{
	View.SendReserveEntityIdsRequest({ NextRequestId, NumberOfEntityIds, TimeoutMillis });
	return NextRequestId++;
}

Worker_RequestId ViewCoordinator::SendCreateEntityRequest(TArray<ComponentData> EntityComponents, TOptional<Worker_EntityId> EntityId,
														  TOptional<uint32> TimeoutMillis)
{
	View.SendCreateEntityRequest({ NextRequestId, MoveTemp(EntityComponents), EntityId, TimeoutMillis });
	return NextRequestId++;
}

Worker_RequestId ViewCoordinator::SendDeleteEntityRequest(Worker_EntityId EntityId, TOptional<uint32> TimeoutMillis)
{
	View.SendDeleteEntityRequest({ NextRequestId, EntityId, TimeoutMillis });
	return NextRequestId++;
}

Worker_RequestId ViewCoordinator::SendEntityQueryRequest(EntityQuery Query, TOptional<uint32> TimeoutMillis)
{
	View.SendEntityQueryRequest({ NextRequestId, MoveTemp(Query), TimeoutMillis });
	return NextRequestId++;
}

Worker_RequestId ViewCoordinator::SendEntityCommandRequest(Worker_EntityId EntityId, CommandRequest Request,
														   TOptional<uint32> TimeoutMillis)
{
	View.SendEntityCommandRequest({ EntityId, NextRequestId, MoveTemp(Request), TimeoutMillis });
	return NextRequestId++;
}

void ViewCoordinator::SendEntityCommandResponse(Worker_RequestId RequestId, CommandResponse Response)
{
	View.SendEntityCommandResponse({ RequestId, MoveTemp(Response) });
}

void ViewCoordinator::SendEntityCommandFailure(Worker_RequestId RequestId, FString Message)
{
	View.SendEntityCommandFailure({ RequestId, MoveTemp(Message) });
}

void ViewCoordinator::SendMetrics(SpatialMetrics Metrics)
{
	View.SendMetrics(MoveTemp(Metrics));
}

void ViewCoordinator::SendLogMessage(Worker_LogLevel Level, const FName& LoggerName, FString Message)
{
	View.SendLogMessage({ Level, LoggerName, MoveTemp(Message) });
}

void ViewCoordinator::SendFencedUpdate(Worker_EntityId EntityId, ComponentUpdate Update)
{
	const Worker_ComponentId ComponentId = Update.GetComponentId();
	View.SendComponentUpdate(EntityId, MoveTemp(Update));

	// Get the component data from the view.
	const EntityViewElement& EntityData = View.GetView().FindChecked(EntityId);
	const ComponentData* Data = EntityData.Components.FindByPredicate([ComponentId](const ComponentData& Element) {
		return Element.GetComponentId() == ComponentId;
	});
	if (Data == nullptr)
	{
		// todo - remove this once we stop the gdk doing some questionable stuff
		return;
	}

	// Get the serialised form of the component for later comparison.
	const uint32 BufferLength = Schema_GetWriteBufferLength(Data->GetFields());
	TUniquePtr<uint8[]> Buffer = MakeUnique<uint8[]>(BufferLength);
	Schema_SerializeToBuffer(Data->GetFields(), Buffer.Get(), BufferLength);

	if (PendingFences.Num() == 0)
	{
		// Send an entity query awaiting the result.
		SendFenceQuery(EntityId, ComponentId);
		ConnectionHandler->SendMessages(View.FlushLocalChanges());
	}
	else
	{
		QueuedMessagesToSend.Emplace(View.FlushLocalChanges());
	}

	PendingFences.Push({ EntityId, ComponentId, MoveTemp(Buffer), BufferLength });
}

void ViewCoordinator::SendFenceQuery(Worker_EntityId EntityId, Worker_ComponentId ComponentId)
{
	Worker_EntityQuery Query;
	Query.constraint.constraint_type = WORKER_CONSTRAINT_TYPE_ENTITY_ID;
	Query.constraint.constraint.entity_id_constraint.entity_id = EntityId;
	Query.result_type = WORKER_RESULT_TYPE_SNAPSHOT;
	Query.snapshot_result_type_component_id_count = 1;
	Query.snapshot_result_type_component_ids = &ComponentId;
	FenceQueryRequestId = SendEntityQueryRequest(EntityQuery(Query));
}

void ViewCoordinator::CheckFenceResponse()
{
	for (const Worker_Op& Op : View.GetViewDelta().GetWorkerMessages())
	{
		if (Op.op_type != WORKER_OP_TYPE_ENTITY_QUERY_RESPONSE)
		{
			continue;
		}
		if (Op.op.entity_query_response.request_id != FenceQueryRequestId)
		{
			continue;
		}

		if (FenceQuerySuccess(Op.op.entity_query_response))
		{
			CompleteCurrentFence();
		}
		else if (PendingFences.Num() > 0)
		{
			SendFenceQuery(PendingFences[0].EntityId, PendingFences[0].ComponentId);
			return;
		}
	}
}

bool ViewCoordinator::FenceQuerySuccess(const Worker_EntityQueryResponseOp& Op)
{
	// todo ideally the status code check would be part of an auto retry system. Not going to put things like back off here.
	if (Op.status_code != WORKER_STATUS_CODE_SUCCESS || Op.result_count != 1 || Op.results[0].component_count != 1)
	{
		return false;
	}

	const PendingFenceData& FenceData = PendingFences[0];
	if (FenceData.ComponentId != Op.results[0].components[0].component_id)
	{
		// todo Check if this can still happen once you've written the authority stuff.
		return false;
	}
	Schema_Object* Data = Schema_GetComponentDataFields(Op.results[0].components[0].schema_type);
	const uint32 BufferLength = Schema_GetWriteBufferLength(Data);
	if (BufferLength != FenceData.DataLength)
	{
		return false;
	}
	const TUniquePtr<uint8[]> Buffer = MakeUnique<uint8[]>(BufferLength);
	Schema_SerializeToBuffer(Data, Buffer.Get(), BufferLength);

	return std::memcmp(Buffer.Get(), FenceData.Data.Get(), BufferLength) == 0;
}

void ViewCoordinator::CheckFenceComponentAuthority()
{
	// Could check the delta or the view.
	const PendingFenceData& FenceData = PendingFences[0];
	const EntityViewElement* Entity = View.GetView().Find(FenceData.EntityId);
	if (Entity == nullptr || !Entity->Authority.Contains(FenceData.ComponentId))
	{
		CompleteCurrentFence();
	}
}

void ViewCoordinator::CompleteCurrentFence()
{
	if (QueuedMessagesToSend.Num() > 0)
	{
		ConnectionHandler->SendMessages(MoveTemp(QueuedMessagesToSend[0]));
		QueuedMessagesToSend.RemoveAt(0);
	}
	else
	{
		ConnectionHandler->SendMessages(View.FlushLocalChanges());
	}
	PendingFences.RemoveAt(0);

	if (PendingFences.Num() > 0)
	{
		SendFenceQuery(PendingFences[0].EntityId, PendingFences[0].ComponentId);
	}
}

const FString& ViewCoordinator::GetWorkerId() const
{
	return ConnectionHandler->GetWorkerId();
}

const TArray<FString>& ViewCoordinator::GetWorkerAttributes() const
{
	return ConnectionHandler->GetWorkerAttributes();
}

} // namespace SpatialGDK
