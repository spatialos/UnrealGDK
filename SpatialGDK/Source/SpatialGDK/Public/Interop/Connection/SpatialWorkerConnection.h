// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "Containers/Queue.h"
#include "HAL/Runnable.h"
#include "HAL/ThreadSafeBool.h"

#include "Interop/Connection/SpatialOSWorkerInterface.h"
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
	Locator,
	DevAuthFlow
};

UCLASS()
class SPATIALGDK_API USpatialWorkerConnection : public UObject, public FRunnable, public SpatialOSWorkerInterface
{
	GENERATED_BODY()

public:
	virtual void FinishDestroy() override;
	void DestroyConnection();
	
	using LoginTokenResponseCallback = TFunction<bool(const Worker_Alpha_LoginTokensResponse*)>;
    
    /// Register a callback using this function.
    /// It will be triggered when receiving login tokens using the development authentication flow inside SpatialWorkerConnection.
    /// @param Callback - callback function.
	void RegisterOnLoginTokensCallback(const LoginTokenResponseCallback& Callback) {LoginTokenResCallback = Callback;}

	void Connect(bool bConnectAsClient, uint32 PlayInEditorID);

	FORCEINLINE bool IsConnected() { return bIsConnected; }

	// Worker Connection Interface
	virtual TArray<Worker_OpList*> GetOpList() override;
	virtual Worker_RequestId SendReserveEntityIdsRequest(uint32_t NumOfEntities) override;
	virtual Worker_RequestId SendCreateEntityRequest(TArray<FWorkerComponentData>&& Components, const Worker_EntityId* EntityId) override;
	virtual Worker_RequestId SendDeleteEntityRequest(Worker_EntityId EntityId) override;
	virtual void SendAddComponent(Worker_EntityId EntityId, FWorkerComponentData* ComponentData) override;
	virtual void SendRemoveComponent(Worker_EntityId EntityId, Worker_ComponentId ComponentId) override;
	virtual void SendComponentUpdate(Worker_EntityId EntityId, const FWorkerComponentUpdate* ComponentUpdate) override;
	virtual Worker_RequestId SendCommandRequest(Worker_EntityId EntityId, const Worker_CommandRequest* Request, uint32_t CommandId) override;
	virtual void SendCommandResponse(Worker_RequestId RequestId, const Worker_CommandResponse* Response) override;
	virtual void SendCommandFailure(Worker_RequestId RequestId, const FString& Message) override;
	virtual void SendLogMessage(uint8_t Level, const FName& LoggerName, const TCHAR* Message) override;
	virtual void SendComponentInterest(Worker_EntityId EntityId, TArray<Worker_InterestOverride>&& ComponentInterest) override;
	virtual Worker_RequestId SendEntityQueryRequest(const Worker_EntityQuery* EntityQuery) override;
	virtual void SendMetrics(const SpatialGDK::SpatialMetrics& Metrics) override;

	PhysicalWorkerName GetWorkerId() const;
	const TArray<FString>& GetWorkerAttributes() const;

	void SetConnectionType(ESpatialConnectionType InConnectionType);

	// TODO: UNR-2753
	FReceptionistConfig ReceptionistConfig;
	FLocatorConfig LocatorConfig;
	FDevAuthConfig DevAuthConfig;

	DECLARE_MULTICAST_DELEGATE_OneParam(FOnEnqueueMessage, const SpatialGDK::FOutgoingMessage*);
	FOnEnqueueMessage OnEnqueueMessage;

	DECLARE_MULTICAST_DELEGATE_OneParam(FOnDequeueMessage, const SpatialGDK::FOutgoingMessage*);
	FOnDequeueMessage OnDequeueMessage;

	DECLARE_DELEGATE(OnConnectionToSpatialOSSucceededDelegate)
	OnConnectionToSpatialOSSucceededDelegate OnConnectedCallback;

	DECLARE_DELEGATE_TwoParams(OnConnectionToSpatialOSFailedDelegate, uint8_t, const FString&);
	OnConnectionToSpatialOSFailedDelegate OnFailedToConnectCallback;

	bool TrySetupConnectionConfigFromCommandLine(const FString& SpatialWorkerType);
	void SetupConnectionConfigFromURL(const FURL& URL, const FString& SpatialWorkerType);

	void QueueLatestOpList();
	void ProcessOutgoingMessages();

private:
	void ConnectToReceptionist(uint32 PlayInEditorID);
	void ConnectToLocator(FLocatorConfig* InLocatorConfig);
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

	void StartDevelopmentAuth(const FString& DevAuthToken);
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
	LoginTokenResponseCallback LoginTokenResCallback;
};
