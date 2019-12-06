// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "Containers/Queue.h"
#include "HAL/Runnable.h"
#include "HAL/ThreadSafeBool.h"

#include "Interop/Connection/ConnectionConfig.h"
#include "Interop/Connection/OutgoingMessages.h"
#include "SpatialGDKSettings.h"
#include "UObject/WeakObjectPtr.h"
#include "Utils/SpatialLatencyTracer.h"

#include <WorkerSDK/improbable/c_schema.h>
#include <WorkerSDK/improbable/c_worker.h>

#include "SpatialWorkerConnection.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogSpatialWorkerConnection, Log, All);

class UGlobalStateManager;
class USpatialGameInstance;
class USpatialStaticComponentView;
class UWorld;

enum class ESpatialConnectionType
{
	Receptionist,
	LegacyLocator,
	Locator
};

DECLARE_DELEGATE(NetDriverOnConnectedToSpatialOS)
DECLARE_DELEGATE_TwoParams(NetDriverOnConnectionFailure, uint8_t, const FString);


UCLASS()
class SPATIALGDK_API USpatialWorkerConnection : public UObject, public FRunnable
{
	GENERATED_BODY()

public:
	void Init(USpatialGameInstance* InGameInstance);

	virtual void FinishDestroy() override;
	void DestroyConnection();

	FORCEINLINE bool IsConnected() { return bIsConnected; }

	// Worker Connection Interface
	TArray<Worker_OpList*> GetOpList();
	Worker_RequestId SendReserveEntityIdsRequest(uint32_t NumOfEntities);
	Worker_RequestId SendCreateEntityRequest(TArray<Worker_ComponentData>&& Components, const Worker_EntityId* EntityId);
	Worker_RequestId SendDeleteEntityRequest(Worker_EntityId EntityId);
	void SendAddComponent(Worker_EntityId EntityId, Worker_ComponentData* ComponentData);
	void SendRemoveComponent(Worker_EntityId EntityId, Worker_ComponentId ComponentId);
	void SendComponentUpdate(Worker_EntityId EntityId, const Worker_ComponentUpdate* ComponentUpdate, const TraceKey Key = USpatialLatencyTracer::InvalidTraceKey);
	Worker_RequestId SendCommandRequest(Worker_EntityId EntityId, const Worker_CommandRequest* Request, uint32_t CommandId);
	void SendCommandResponse(Worker_RequestId RequestId, const Worker_CommandResponse* Response);
	void SendCommandFailure(Worker_RequestId RequestId, const FString& Message);
	void SendLogMessage(uint8_t Level, const FName& LoggerName, const TCHAR* Message);
	void SendComponentInterest(Worker_EntityId EntityId, TArray<Worker_InterestOverride>&& ComponentInterest);
	Worker_RequestId SendEntityQueryRequest(const Worker_EntityQuery* EntityQuery);
	void SendMetrics(const SpatialGDK::SpatialMetrics& Metrics);

	FString GetWorkerId() const;
	const TArray<FString>& GetWorkerAttributes() const;

	void SetConnectionType(ESpatialConnectionType InConnectionType);

	FReceptionistConfig ReceptionistConfig;
	FLocatorConfig LocatorConfig;

	DECLARE_MULTICAST_DELEGATE_OneParam(FOnEnqueueMessage, const SpatialGDK::FOutgoingMessage*);
	FOnEnqueueMessage OnEnqueueMessage;

	DECLARE_MULTICAST_DELEGATE_OneParam(FOnDequeueMessage, const SpatialGDK::FOutgoingMessage*);
	FOnDequeueMessage OnDequeueMessage;

	UPROPERTY()
	USpatialStaticComponentView* StaticComponentView;

	UPROPERTY()
	UGlobalStateManager* GlobalStateManager;

	// TODO(Alex): move these to a new class
	void Connect(bool bConnectAsClient, USpatialNetDriver* NetDriver);
	void ConnectToLocator();
	void ConnectToReceptionist(bool bConnectAsClient, uint32 PlayInEditorID);
	void FinishConnecting(Worker_ConnectionFuture* ConnectionFuture);
	void OnConnectionSuccess();
	void OnPreConnectionFailure(const FString& Reason);
	void OnConnectionFailure();

	void BindOnConnectedToSpatialOS(const NetDriverOnConnectedToSpatialOS& Function);
	void BindOnConnectionFailure(const NetDriverOnConnectionFailure& Function);
	NetDriverOnConnectedToSpatialOS NetDriverOnConnectedCallback;
	NetDriverOnConnectionFailure NetDriverOnFailureCallback;

private:

	ESpatialConnectionType GetConnectionType() const;

	void CacheWorkerAttributes();

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
	Worker_Locator* WorkerLocator;

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

	ESpatialConnectionType ConnectionType = ESpatialConnectionType::Receptionist;
};
