// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "SpatialView/ConnectionHandler/InitialOpListConnectionHandler.h"

namespace SpatialGDK
{
InitialOpListConnectionHandler::InitialOpListConnectionHandler(TUniquePtr<AbstractConnectionHandler> InnerHandler,
															   TFunction<bool(OpList&, ExtractedOpListData&)> OpExtractor)
	: State(EXTRACTING_OPS)
	, InnerHandler(MoveTemp(InnerHandler))
	, OpExtractor(MoveTemp(OpExtractor))
{
}

void InitialOpListConnectionHandler::Advance()
{
	InnerHandler->Advance();
}

uint32 InitialOpListConnectionHandler::GetOpListCount()
{
	switch (State)
	{
	case EXTRACTING_OPS:
		return 1;
	case FLUSHING_QUEUED_OP_LISTS:
		return QueuedOpLists.Num();
	case PASS_THROUGH:
		return InnerHandler->GetOpListCount();
	default:
		checkNoEntry();
		return 0;
	}
}

OpList InitialOpListConnectionHandler::GetNextOpList()
{
	switch (State)
	{
	case EXTRACTING_OPS:
		return QueueAndExtractOps();
	case FLUSHING_QUEUED_OP_LISTS:
	{
		OpList Temp = MoveTemp(QueuedOpLists[0]);
		QueuedOpLists.RemoveAt(0);
		if (QueuedOpLists.Num() == 0)
		{
			State = PASS_THROUGH;
		}
		return Temp;
	}
	case PASS_THROUGH:
		return InnerHandler->GetNextOpList();
	default:
		checkNoEntry();
		return {};
	}
}

void InitialOpListConnectionHandler::SendMessages(TUniquePtr<MessagesToSend> Messages)
{
	InnerHandler->SendMessages(MoveTemp(Messages));
}

const FString& InitialOpListConnectionHandler::GetWorkerId() const
{
	return InnerHandler->GetWorkerId();
}

Worker_EntityId InitialOpListConnectionHandler::GetWorkerSystemEntityId() const
{
	return InnerHandler->GetWorkerSystemEntityId();
}

OpList InitialOpListConnectionHandler::QueueAndExtractOps()
{
	TUniquePtr<ExtractedOpListData> ExtractedOpList = MakeUnique<ExtractedOpListData>();

	// Extract from an empty op list to ensure forward progress.
	OpList EmptyOpList = {};
	if (OpExtractor(EmptyOpList, *ExtractedOpList))
	{
		State = FLUSHING_QUEUED_OP_LISTS;
		return OpList{ ExtractedOpList->ExtractedOps.GetData(), static_cast<uint32>(ExtractedOpList->ExtractedOps.Num()),
					   MoveTemp(ExtractedOpList) };
	}

	// Extract and queue ops from the inner connection handler.
	const uint32 OpListsAvailable = InnerHandler->GetOpListCount();
	for (uint32 i = 0; i < OpListsAvailable; ++i)
	{
		QueuedOpLists.Push(InnerHandler->GetNextOpList());
		if (OpExtractor(QueuedOpLists.Last(), *ExtractedOpList))
		{
			State = FLUSHING_QUEUED_OP_LISTS;
			break;
		}
	}

	return OpList{ ExtractedOpList->ExtractedOps.GetData(), static_cast<uint32>(ExtractedOpList->ExtractedOps.Num()),
				   MoveTemp(ExtractedOpList) };
}
} // namespace SpatialGDK
