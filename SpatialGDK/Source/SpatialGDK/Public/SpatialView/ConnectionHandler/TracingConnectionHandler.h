
// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "SpatialView/ConnectionHandler/AbstractConnectionHandler.h"
#include "SpatialView/OpList/OpList.h"

#include "Templates/UniquePtr.h"

namespace SpatialGDK
{

class TracingConnectionHandler : public AbstractConnectionHandler
{
public:
	explicit TracingConnectionHandler(TUniquePtr<AbstractConnectionHandler> InnerConnectionHandler);

	virtual void Advance() override;
	virtual uint32 GetOpListCount() override;
	virtual OpList GetNextOpList() override;
	virtual void SendMessages(TUniquePtr<MessagesToSend> Messages) override;
	virtual const FString& GetWorkerId() const override;
	virtual const TArray<FString>& GetWorkerAttributes() const override;

	void SetAddComponentTrace(Worker_EntityId EntityId, Worker_ComponentId ComponentId, int32 TraceId);
	void SetComponentUpdateTrace(Worker_EntityId EntityId, Worker_ComponentId ComponentId, int32 TraceId);
	void SetEntityCreationTrace(Worker_EntityId EntityId, Worker_ComponentId ComponentId, int32 TraceId);

	TFunction<void(int32)> AddCallback;
	TFunction<void(int32)> UpdateCallback;
	TFunction<void(TArray<int32>)> EntityCreationCallback;

private:
	struct TracedComponent
	{
		Worker_EntityId EntityId;
		Worker_ComponentId ComponentId;
		int32 TraceId;
	};

	TArray<TracedComponent> TracedUpdates;
	TArray<TracedComponent> TracedAdds;
	TMap<int64, TArray<int32>> TracedEntityCreations;

	TUniquePtr<AbstractConnectionHandler> InnerConnectionHandler;
};

}  // namespace SpatialGDK
