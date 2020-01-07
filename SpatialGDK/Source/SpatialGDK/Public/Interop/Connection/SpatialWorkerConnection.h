// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "SpatialWorkerConnectionInterface.h"

#include "WorkerOpListSerializer.h"

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

DECLARE_LOG_CATEGORY_EXTERN(LogSpatialWorkerConnection, Log, All);

class SPATIALGDK_API USpatialWorkerConnection : public USpatialWorkerConnectionInterface
{
public:
	// TODO(Alex): is it called properly?
	~USpatialWorkerConnection();
	virtual void DestroyConnection() override;

	virtual Worker_Connection* Connect(uint32 PlayInEditorID, bool bConnectAsClient) override;
	virtual void SetConnection(Worker_Connection* InConnection) override;

	// Worker Connection Interface
	virtual TArray<Worker_OpList*> GetOpList() override;
	virtual Worker_RequestId SendReserveEntityIdsRequest(uint32_t NumOfEntities) override;
	virtual Worker_RequestId SendCreateEntityRequest(TArray<Worker_ComponentData>&& Components, const Worker_EntityId* EntityId) override;
	virtual Worker_RequestId SendDeleteEntityRequest(Worker_EntityId EntityId) override;
	virtual void SendAddComponent(Worker_EntityId EntityId, Worker_ComponentData* ComponentData) override;
	virtual void SendRemoveComponent(Worker_EntityId EntityId, Worker_ComponentId ComponentId) override;
	virtual void SendComponentUpdate(Worker_EntityId EntityId, const Worker_ComponentUpdate* ComponentUpdate, const TraceKey Key = USpatialLatencyTracer::InvalidTraceKey) override;
	virtual Worker_RequestId SendCommandRequest(Worker_EntityId EntityId, const Worker_CommandRequest* Request, uint32_t CommandId) override;
	virtual void SendCommandResponse(Worker_RequestId RequestId, const Worker_CommandResponse* Response) override;
	virtual void SendCommandFailure(Worker_RequestId RequestId, const FString& Message) override;
	virtual void SendLogMessage(uint8_t Level, const FName& LoggerName, const TCHAR* Message) override;
	virtual void SendComponentInterest(Worker_EntityId EntityId, TArray<Worker_InterestOverride>&& ComponentInterest) override;
	virtual Worker_RequestId SendEntityQueryRequest(const Worker_EntityQuery* EntityQuery) override;
	virtual void SendMetrics(const SpatialGDK::SpatialMetrics& Metrics) override;

	virtual PhysicalWorkerName GetWorkerId() const override;
	virtual const TArray<FString>& GetWorkerAttributes() const override;

	virtual void SetConnectionType(ESpatialConnectionType InConnectionType) override;

	virtual void GetErrorCodeAndMessage(uint8_t& OutConnectionStatusCode, FString& OutErrorMessage) const override;

private:
	// TODO(Alex): hide it back
public:
	virtual Worker_ConnectionFuture* ConnectToReceptionist(uint32 PlayInEditorID, bool bConnectAsClient) override;
	virtual Worker_ConnectionFuture* ConnectToLocator(bool bConnectAsClient) override;
private:

	virtual ESpatialConnectionType GetConnectionType() const override;

	// TODO(Alex): hide it back
public:
	virtual void CacheWorkerAttributes() override;
private:

private:

	// TODO(Alex): hide it back
public:
	virtual void QueueLatestOpList() override;
	virtual void ProcessOutgoingMessages() override;
private:

	// TODO(Alex): hide it back
public:
	virtual void StartDevelopmentAuth(FString DevAuthToken, bool bInConnectToLocatorAsClient) override;
private:
	static void OnPlayerIdentityToken(void* UserData, const Worker_Alpha_PlayerIdentityTokenResponse* PIToken);
	static void OnLoginTokens(void* UserData, const Worker_Alpha_LoginTokensResponse* LoginTokens);

	template <typename T, typename... ArgsType>
	void QueueOutgoingMessage(ArgsType&&... Args);

private:
	// TODO(Alex): hide back?
public:
	Worker_Connection* WorkerConnection;
private:
	Worker_Locator* WorkerLocator;

	// TODO(Alex): not nice to have this variable
	bool bConnectToLocatorAsClient = false;

	TArray<FString> CachedWorkerAttributes;


	TQueue<Worker_OpList*> OpListQueue;
	TQueue<TUniquePtr<SpatialGDK::FOutgoingMessage>> OutgoingMessagesQueue;

	// RequestIds per worker connection start at 0 and incrementally go up each command sent.
	Worker_RequestId NextRequestId = 0;

	ESpatialConnectionType ConnectionType = ESpatialConnectionType::Receptionist;

	FReceptionistConfig ReceptionistConfig;
	FLocatorConfig LocatorConfig;

	UE4_OpLists SerializedOpLists;
};
