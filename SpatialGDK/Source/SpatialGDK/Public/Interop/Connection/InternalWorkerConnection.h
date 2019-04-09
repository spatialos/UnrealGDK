// Copyright (c) Improbable Worlds Ltd, All Rights Reserved
#pragma once

#include "HAL/Runnable.h"

#include <WorkerSDK/improbable/c_schema.h>
#include <WorkerSDK/improbable/c_worker.h>

#include "InternalWorkerConnection.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogInternalWorkerConnection, Log, All);

UCLASS()
class SPATIALGDK_API UInternalWorkerConnection : public UObject, public FRunnable
{
	GENERATED_BODY()

public:
	void ConnectToSpatialOS();

	// Begin FRunnable Interface
	virtual bool Init();
	virtual uint32 Run();
	virtual void Stop();
	// End FRunnable Interface

	// These functions on the GameThread
	TArray<Worker_OpList*> GetOpList();

	template <typename T>
	void QueueOutgoingMessage(const T& Message);

private:
	// These functions on the WorkerThread
	void QueueLatestOpList();
	void ProcessOutgoingMessages();


private:
	UPROPERTY()
	USpatialWorkerConnection* SpatialWorkerConnection;

	Worker_Connection* WorkerConnection;

	FRunnableThread* WorkerThread;
	FThreadSafeBool KeepRunning;

	TQueue<Worker_OpList*> OpListQueue;
	TQueue<FOutgoingMessageWrapper> OutgoingMessagesQueue;
};
