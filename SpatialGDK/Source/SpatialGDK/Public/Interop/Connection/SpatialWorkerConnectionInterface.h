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

class SPATIALGDK_API USpatialWorkerConnectionInterface
{
public:
	virtual ~USpatialWorkerConnectionInterface() = default;
	virtual void DestroyConnection() = 0;

	virtual Worker_Connection* Connect(uint32 PlayInEditorID, bool bConnectAsClient) = 0;
	virtual void SetConnection(Worker_Connection* InConnection) = 0;

	// Worker Connection Interface
	virtual TArray<Worker_OpList*> GetOpList() = 0;
	virtual Worker_RequestId SendReserveEntityIdsRequest(uint32_t NumOfEntities) = 0;
	virtual Worker_RequestId SendCreateEntityRequest(TArray<Worker_ComponentData>&& Components, const Worker_EntityId* EntityId) = 0;
	virtual Worker_RequestId SendDeleteEntityRequest(Worker_EntityId EntityId) = 0;
	virtual void SendAddComponent(Worker_EntityId EntityId, Worker_ComponentData* ComponentData) = 0;
	virtual void SendRemoveComponent(Worker_EntityId EntityId, Worker_ComponentId ComponentId) = 0;
	virtual void SendComponentUpdate(Worker_EntityId EntityId, const Worker_ComponentUpdate* ComponentUpdate, const TraceKey Key = USpatialLatencyTracer::InvalidTraceKey) = 0;
	virtual Worker_RequestId SendCommandRequest(Worker_EntityId EntityId, const Worker_CommandRequest* Request, uint32_t CommandId) = 0;
	virtual void SendCommandResponse(Worker_RequestId RequestId, const Worker_CommandResponse* Response) = 0;
	virtual void SendCommandFailure(Worker_RequestId RequestId, const FString& Message) = 0;
	virtual void SendLogMessage(uint8_t Level, const FName& LoggerName, const TCHAR* Message) = 0;
	virtual void SendComponentInterest(Worker_EntityId EntityId, TArray<Worker_InterestOverride>&& ComponentInterest) = 0;
	virtual Worker_RequestId SendEntityQueryRequest(const Worker_EntityQuery* EntityQuery) = 0;
	virtual void SendMetrics(const SpatialGDK::SpatialMetrics& Metrics) = 0;

	virtual PhysicalWorkerName GetWorkerId() const = 0;
	virtual const TArray<FString>& GetWorkerAttributes() const = 0;

	virtual void SetConnectionType(ESpatialConnectionType InConnectionType) = 0;

	virtual void GetErrorCodeAndMessage(uint8_t& OutConnectionStatusCode, FString& OutErrorMessage) const = 0;

private:
	virtual Worker_ConnectionFuture* ConnectToReceptionist(uint32 PlayInEditorID, bool bConnectAsClient) = 0;
	virtual Worker_ConnectionFuture* ConnectToLocator(bool bConnectAsClient) = 0;
	virtual ESpatialConnectionType GetConnectionType() const = 0;

	// TODO(Alex): hide it back
public:
	virtual void CacheWorkerAttributes() = 0;
private:

	// TODO(Alex): hide it back
public:
	virtual void QueueLatestOpList() = 0;
	virtual void ProcessOutgoingMessages() = 0;
private:

	// TODO(Alex): hide it back
public:
	virtual void StartDevelopmentAuth(FString DevAuthToken, bool bInConnectToLocatorAsClient) = 0;
private:
	static void OnPlayerIdentityToken(void* UserData, const Worker_Alpha_PlayerIdentityTokenResponse* PIToken);
	static void OnLoginTokens(void* UserData, const Worker_Alpha_LoginTokensResponse* LoginTokens);
};
