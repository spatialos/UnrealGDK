// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "SpatialView/ConnectionHandler/AbstractConnectionHandler.h"
#include "SpatialView/OpList/ExtractedOpList.h"
#include "SpatialView/OpList/OpList.h"

#include "Containers/Array.h"
#include "Templates/UniquePtr.h"

namespace SpatialGDK
{
// A connection handler that can present selected ops early, holding back all others.
class InitialOpListConnectionHandler : public AbstractConnectionHandler
{
public:
	explicit InitialOpListConnectionHandler(TUniquePtr<AbstractConnectionHandler> InnerHandler,
											TFunction<bool(OpList&, ExtractedOpListData&)> OpExtractor);

	virtual void Advance() override;
	virtual uint32 GetOpListCount() override;
	virtual OpList GetNextOpList() override;
	virtual void SendMessages(TUniquePtr<MessagesToSend> Messages) override;
	virtual const FString& GetWorkerId() const override;
	virtual Worker_EntityId GetWorkerSystemEntityId() const override;

private:
	enum
	{
		EXTRACTING_OPS,
		FLUSHING_QUEUED_OP_LISTS,
		PASS_THROUGH
	} State;

	OpList QueueAndExtractOps();

	TUniquePtr<AbstractConnectionHandler> InnerHandler;
	TFunction<bool(OpList&, ExtractedOpListData&)> OpExtractor;
	TArray<OpList> QueuedOpLists;
};

} // namespace SpatialGDK
