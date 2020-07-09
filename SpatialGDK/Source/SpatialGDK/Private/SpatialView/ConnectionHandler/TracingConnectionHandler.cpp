// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "SpatialView/ConnectionHandler/TracingConnectionHandler.h"

namespace SpatialGDK
{

TracingConnectionHandler::TracingConnectionHandler(TUniquePtr<AbstractConnectionHandler> InnerConnectionHandler)
	: InnerConnectionHandler(MoveTemp(InnerConnectionHandler))
{
}

void TracingConnectionHandler::Advance()
{
	InnerConnectionHandler->Advance();
}

uint32 TracingConnectionHandler::GetOpListCount()
{
	return InnerConnectionHandler->GetOpListCount();
}

OpList TracingConnectionHandler::GetNextOpList()
{
	return InnerConnectionHandler->GetNextOpList();
}

void TracingConnectionHandler::SendMessages(TUniquePtr<MessagesToSend> Messages)
{
	for (const OutgoingComponentMessage& Message : Messages->ComponentMessages)
	{
		switch (Message.GetType()) {
		case OutgoingComponentMessage::NONE:
			checkNoEntry();
			break;
		case OutgoingComponentMessage::ADD:
			if (TracedAdds.Num() != 0
				&& Message.ComponentId == TracedAdds[0].ComponentId
				&& Message.EntityId == TracedAdds[0].EntityId)
			{
				AddCallback(TracedAdds[0].TraceId);
				TracedAdds.RemoveAt(0);
			}
			break;
		case OutgoingComponentMessage::UPDATE:
			if (TracedUpdates.Num() != 0
				&& Message.ComponentId == TracedUpdates[0].ComponentId
				&& Message.EntityId == TracedUpdates[0].EntityId)
			{
				UpdateCallback(TracedUpdates[0].TraceId);
				TracedUpdates.RemoveAt(0);
			}
			break;
		case OutgoingComponentMessage::REMOVE:
			break;
		default:
			checkNoEntry();
			break;
		}
	}

	for (const CreateEntityRequest& Request : Messages->CreateEntityRequests)
	{
		TArray<int32>* Ids = TracedEntityCreations.Find(Request.EntityId.Get(0));

		if (Ids != nullptr)
		{
			EntityCreationCallback(*Ids);
		}
		TracedEntityCreations.Remove(Request.EntityId.Get(0));
	}

	InnerConnectionHandler->SendMessages(MoveTemp(Messages));
}

const FString& TracingConnectionHandler::GetWorkerId() const
{
	return InnerConnectionHandler->GetWorkerId();
}

const TArray<FString>& TracingConnectionHandler::GetWorkerAttributes() const
{
	return InnerConnectionHandler->GetWorkerAttributes();
}

void TracingConnectionHandler::SetAddComponentTrace(Worker_EntityId EntityId, Worker_ComponentId ComponentId, int32 TraceId)
{
	TracedAdds.Emplace(TracedComponent{EntityId, ComponentId, TraceId});
}

void TracingConnectionHandler::SetComponentUpdateTrace(Worker_EntityId EntityId, Worker_ComponentId ComponentId, int32 TraceId)
{
	TracedUpdates.Emplace(TracedComponent{EntityId, ComponentId, TraceId});
}

void TracingConnectionHandler::SetEntityCreationTrace(Worker_EntityId EntityId, Worker_ComponentId ComponentId, int32 TraceId)
{
	TracedEntityCreations.FindOrAdd(EntityId).Emplace(TraceId);
}

} // namespace SpatialGDK
