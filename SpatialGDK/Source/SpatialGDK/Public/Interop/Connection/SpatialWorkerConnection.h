// Copyright (c) Improbable Worlds Ltd, All Rights Reserved
#pragma once

#include "Containers/Queue.h"
#include "HAL/Runnable.h"
#include "HAL/ThreadSafeBool.h"

#include "Interop/Connection/ConnectionConfig.h"
#include "Interop/Connection/OutgoingMessages.h"
#include "UObject/WeakObjectPtr.h"

#include <WorkerSDK/improbable/c_schema.h>
#include <WorkerSDK/improbable/c_worker.h>

#include "SpatialWorkerConnection.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogSpatialWorkerConnection, Log, All);

class USpatialGameInstance;
class UWorld;

enum class SpatialConnectionType
{
	Receptionist,
	LegacyLocator,
	Locator
};

UCLASS()
class SPATIALGDK_API USpatialWorkerConnection : public UObject, public FRunnable
{
	GENERATED_BODY()

public:
	void Init(USpatialGameInstance* InGameInstance);

	virtual void FinishDestroy() override;
	void DestroyConnection();

	void Connect(bool bConnectAsClient);

	FORCEINLINE bool IsConnected() { return bIsConnected; }

	// Worker Connection Interface
	TArray<Worker_OpList*> GetOpList();
	Worker_RequestId SendReserveEntityIdsRequest(uint32_t NumOfEntities);
	Worker_RequestId SendCreateEntityRequest(TArray<Worker_ComponentData>&& Components, const Worker_EntityId* EntityId);
	Worker_RequestId SendDeleteEntityRequest(Worker_EntityId EntityId);
	void SendComponentUpdate(Worker_EntityId EntityId, const Worker_ComponentUpdate* ComponentUpdate);
	Worker_RequestId SendCommandRequest(Worker_EntityId EntityId, const Worker_CommandRequest* Request, uint32_t CommandId);
	void SendCommandResponse(Worker_RequestId RequestId, const Worker_CommandResponse* Response);
	void SendCommandFailure(Worker_RequestId RequestId, const FString& Message);
	void SendLogMessage(uint8_t Level, const FName& LoggerName, const TCHAR* Message);
	void SendComponentInterest(Worker_EntityId EntityId, TArray<Worker_InterestOverride>&& ComponentInterest);
	Worker_RequestId SendEntityQueryRequest(const Worker_EntityQuery* EntityQuery);
	void SendMetrics(const SpatialGDK::SpatialMetrics& Metrics);

	FString GetWorkerId() const;
	const TArray<FString>& GetWorkerAttributes() const;

	FReceptionistConfig ReceptionistConfig;
	FLocatorConfig LocatorConfig;

private:
	void ConnectToReceptionist(bool bConnectAsClient);
	void ConnectToLocator();
	void FinishConnecting(Worker_ConnectionFuture* ConnectionFuture);

	void OnConnectionSuccess();
	void OnPreConnectionFailure(const FString& Reason);
	void OnConnectionFailure();

	SpatialConnectionType GetConnectionType() const;

	void CacheWorkerAttributes();

	class USpatialNetDriver* GetSpatialNetDriverChecked() const;

	// Begin FRunnable Interface
	virtual bool Init() override;
	virtual uint32 Run() override;
	virtual void Stop() override;
	// End FRunnable Interface

	void InitializeOpsProcessingThread();
	void QueueLatestOpList();
	void ProcessOutgoingMessages();

	template <typename T, typename... ArgsType>
	void QueueOutgoingMessage(ArgsType&&... Args);

private:
	Worker_Connection* WorkerConnection;
	Worker_Alpha_Locator* WorkerLocator;

	TWeakObjectPtr<USpatialGameInstance> GameInstance;

	bool bIsConnected;

	TArray<FString> CachedWorkerAttributes;

	FRunnableThread* OpsProcessingThread;
	FThreadSafeBool KeepRunning = true;
	float OpsUpdateInterval;

	TQueue<Worker_OpList*> OpListQueue;
	TQueue<TUniquePtr<SpatialGDK::FOutgoingMessage>> OutgoingMessagesQueue;

	// RequestIds per worker connection start at 0 and incrementally go up each command sent.
	Worker_RequestId NextRequestId = 0;
};
