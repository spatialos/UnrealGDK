// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "SpatialView/ConnectionHandler/AbstractConnectionHandler.h"
#include "SpatialView/OpList/OpList.h"

namespace SpatialGDK
{
class SpatialEventTracer;

class SpatialOSConnectionHandler : public AbstractConnectionHandler
{
public:
	explicit SpatialOSConnectionHandler(Worker_Connection* Connection, TSharedPtr<SpatialEventTracer> EventTracer);
	~SpatialOSConnectionHandler();

	virtual void Advance() override;
	virtual uint32 GetOpListCount() override;
	virtual OpList GetNextOpList() override;
	virtual void SendMessages(TUniquePtr<MessagesToSend> Messages) override;
	virtual const FString& GetWorkerId() const override;
	virtual Worker_EntityId GetWorkerSystemEntityId() const override;

private:
	struct WorkerConnectionDeleter
	{
		void operator()(Worker_Connection* ConnectionToDestroy) const noexcept;
	};
#
	TSharedPtr<SpatialEventTracer> EventTracer;
	TUniquePtr<Worker_Connection, WorkerConnectionDeleter> Connection;

	TMap<int64, int64> InternalToUserRequestId;
	FString WorkerId;
	Worker_EntityId WorkerSystemEntityId;
};

} // namespace SpatialGDK
