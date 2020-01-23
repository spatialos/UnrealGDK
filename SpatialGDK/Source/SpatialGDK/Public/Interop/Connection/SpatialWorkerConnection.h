// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "Containers/Queue.h"
#include "HAL/Runnable.h"
#include "HAL/ThreadSafeBool.h"

#include "Interop/Connection/ConnectionConfig.h"
#include "Interop/Connection/OutgoingMessages.h"
#include "SpatialCommonTypes.h"
#include "SpatialGDKSettings.h"
#include "UObject/WeakObjectPtr.h"
#include "Utils/SpatialLatencyTracer.h"

#include <WorkerSDK/improbable/c_schema.h>
#include <WorkerSDK/improbable/c_worker.h>

#include "SpatialWorkerConnection.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogSpatialWorkerConnection, Log, All);

enum class ESpatialConnectionType
{
	Receptionist,
	LegacyLocator,
	Locator
};

typedef std::function<bool(const Worker_Alpha_LoginTokensResponse*)> LoginTokenCb;

UCLASS()
class SPATIALGDK_API USpatialWorkerConnection : public UObject, public FRunnable
{
	GENERATED_BODY()

public:
	virtual void FinishDestroy() override;
	void DestroyConnection();
	
	using LoginTokenRes_Callback = TFunction<bool(const Worker_Alpha_LoginTokensResponse*)>;
    
    /// Register a callback using this function.
    /// It will be triggered when receiving login tokens using the development authentication flow inside SpatialWorkerConnection.
    /// @param cb - callback function.
	void RegisterOnLoginTokensCb(const LoginTokenRes_Callback& Callback) {LoginTokenResCallback = Callback;};

	void Connect(bool bConnectAsClient, uint32 PlayInEditorID);

	FORCEINLINE bool IsConnected() { return bIsConnected; }

	// Worker Connection Interface
	TArray<Worker_OpList*> GetOpList();
	Worker_RequestId SendReserveEntityIdsRequest(uint32_t NumOfEntities);
	Worker_RequestId SendCreateEntityRequest(TArray<Worker_ComponentData>&& Components, const Worker_EntityId* EntityId);
	Worker_RequestId SendDeleteEntityRequest(Worker_EntityId EntityId);
	void SendAddComponent(Worker_EntityId EntityId, Worker_ComponentData* ComponentData, const TraceKey Key = USpatialLatencyTracer::InvalidTraceKey);
	void SendRemoveComponent(Worker_EntityId EntityId, Worker_ComponentId ComponentId);
	void SendComponentUpdate(Worker_EntityId EntityId, const Worker_ComponentUpdate* ComponentUpdate, const TraceKey Key = USpatialLatencyTracer::InvalidTraceKey);
	Worker_RequestId SendCommandRequest(Worker_EntityId EntityId, const Worker_CommandRequest* Request, uint32_t CommandId);
	void SendCommandResponse(Worker_RequestId RequestId, const Worker_CommandResponse* Response);
	void SendCommandFailure(Worker_RequestId RequestId, const FString& Message);
	void SendLogMessage(uint8_t Level, const FName& LoggerName, const TCHAR* Message);
	void SendComponentInterest(Worker_EntityId EntityId, TArray<Worker_InterestOverride>&& ComponentInterest);
	Worker_RequestId SendEntityQueryRequest(const Worker_EntityQuery* EntityQuery);
	void SendMetrics(const SpatialGDK::SpatialMetrics& Metrics);

	PhysicalWorkerName GetWorkerId() const;
	const TArray<FString>& GetWorkerAttributes() const;

	void SetConnectionType(ESpatialConnectionType InConnectionType);

	FReceptionistConfig ReceptionistConfig;
	FLocatorConfig LocatorConfig;

	DECLARE_MULTICAST_DELEGATE_OneParam(FOnEnqueueMessage, const SpatialGDK::FOutgoingMessage*);
	FOnEnqueueMessage OnEnqueueMessage;

	DECLARE_MULTICAST_DELEGATE_OneParam(FOnDequeueMessage, const SpatialGDK::FOutgoingMessage*);
	FOnDequeueMessage OnDequeueMessage;

	DECLARE_DELEGATE(OnConnectionToSpatialOSSucceededDelegate)
	OnConnectionToSpatialOSSucceededDelegate OnConnectedCallback;

	DECLARE_DELEGATE_TwoParams(OnConnectionToSpatialOSFailedDelegate, uint8_t, const FString&);
	OnConnectionToSpatialOSFailedDelegate OnFailedToConnectCallback;

private:
	void ConnectToReceptionist(uint32 PlayInEditorID);
	void ConnectToLocator();
	void FinishConnecting(Worker_ConnectionFuture* ConnectionFuture);

	void OnConnectionSuccess();
	void OnConnectionFailure();

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

	void StartDevelopmentAuth(FString DevAuthToken);
	static void OnPlayerIdentityToken(void* UserData, const Worker_Alpha_PlayerIdentityTokenResponse* PIToken);
	static void OnLoginTokens(void* UserData, const Worker_Alpha_LoginTokensResponse* LoginTokens);
	void ProcessLoginTokensResponse(const Worker_Alpha_LoginTokensResponse* LoginTokens);

	template <typename T, typename... ArgsType>
	void QueueOutgoingMessage(ArgsType&&... Args);

private:
	Worker_Connection* WorkerConnection;
	Worker_Locator* WorkerLocator;

	bool bIsConnected;
	bool bConnectAsClient = false;

	TArray<FString> CachedWorkerAttributes;

	FRunnableThread* OpsProcessingThread;
	FThreadSafeBool KeepRunning = true;
	float OpsUpdateInterval;

	TQueue<Worker_OpList*> OpListQueue;
	TQueue<TUniquePtr<SpatialGDK::FOutgoingMessage>> OutgoingMessagesQueue;

	// RequestIds per worker connection start at 0 and incrementally go up each command sent.
	Worker_RequestId NextRequestId = 0;

	ESpatialConnectionType ConnectionType = ESpatialConnectionType::Receptionist;
	
	LoginTokenRes_Callback    LoginTokenResCallback = nullptr;
};
