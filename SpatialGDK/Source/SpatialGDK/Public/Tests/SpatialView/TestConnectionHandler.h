// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "SpatialView/ConnectionHandler/AbstractConnectionHandler.h"
#include "SpatialView/WorkerView.h"

namespace SpatialGDK
{
using FSendMessageCallback = TUniqueFunction<void(TUniquePtr<MessagesToSend>)>;

class FTestConnectionHandler : public AbstractConnectionHandler
{
public:
	using FSendMessageCallback = TUniqueFunction<void(TUniquePtr<MessagesToSend>)>;

	explicit FTestConnectionHandler(FString InWorkerId = {}, FSpatialEntityId InWorkerSystemEntityId = SpatialConstants::INVALID_ENTITY_ID)
		: WorkerId(InWorkerId)
		, WorkerEntityId(InWorkerSystemEntityId)
	{
	}

	virtual void Advance() override {}

	virtual uint32 GetOpListCount() override { return PendingOpLists.Num(); }

	virtual OpList GetNextOpList() override
	{
		if (PendingOpLists.Num() == 0)
		{
			return {};
		}
		OpList Temp = MoveTemp(PendingOpLists[0]);
		PendingOpLists.RemoveAt(0);
		return Temp;
	}

	virtual void SendMessages(TUniquePtr<MessagesToSend> Messages) override
	{
		if (SendMessageCallback)
		{
			SendMessageCallback(MoveTemp(Messages));
		}
	}

	virtual const FString& GetWorkerId() const override { return WorkerId; }

	virtual FSpatialEntityId GetWorkerSystemEntityId() const override { return WorkerEntityId; }

	void SetSendMessageCallback(FSendMessageCallback InSendMessageCallback) { SendMessageCallback = MoveTemp(InSendMessageCallback); }
	void AddOpList(OpList Ops) { PendingOpLists.Add(MoveTemp(Ops)); }

private:
	FString WorkerId;
	FSpatialEntityId WorkerEntityId;
	FSendMessageCallback SendMessageCallback;
	TArray<OpList> PendingOpLists;
};
} // namespace SpatialGDK
