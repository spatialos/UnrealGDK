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
	explicit SpatialOSConnectionHandler(Worker_Connection* Connection, TUniquePtr<SpatialEventTracer> EventTracer);
	~SpatialOSConnectionHandler();

	virtual void Advance() override;
	virtual uint32 GetOpListCount() override;
	virtual OpList GetNextOpList() override;
	virtual void SendMessages(TUniquePtr<MessagesToSend> Messages) override;
	virtual const FString& GetWorkerId() const override;
	virtual const TArray<FString>& GetWorkerAttributes() const override;

private:

	struct WorkerConnectionDeleter
	{
		void operator()(Worker_Connection* Connection) const noexcept;
	};
#
	TUniquePtr<SpatialEventTracer> EventTracer;
	TUniquePtr<Worker_Connection, WorkerConnectionDeleter> Connection;

	TMap<int64, int64> InternalToUserRequestId;
	FString WorkerId;
	TArray<FString> WorkerAttributes;
};

} // namespace SpatialGDK
