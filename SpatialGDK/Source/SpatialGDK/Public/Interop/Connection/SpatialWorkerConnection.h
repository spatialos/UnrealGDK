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

DECLARE_LOG_CATEGORY_EXTERN(LogSpatialWorkerConnection, Log, All);

class SPATIALGDK_API USpatialWorkerConnection
{
public:
	// TODO(Alex): is it called properly?
	~USpatialWorkerConnection();
	void DestroyConnection();

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

	PhysicalWorkerName GetWorkerId() const;
	const TArray<FString>& GetWorkerAttributes() const;

	void SetConnectionType(ESpatialConnectionType InConnectionType);

	void GetErrorCodeAndMessage(uint8_t& OutConnectionStatusCode, FString& OutErrorMessage) const;

	FReceptionistConfig ReceptionistConfig;
	FLocatorConfig LocatorConfig;

private:
	// TODO(Alex): hide it back
public:
	Worker_ConnectionFuture* ConnectToReceptionist(uint32 PlayInEditorID, bool bConnectAsClient);
	Worker_ConnectionFuture* ConnectToLocator(bool bConnectAsClient);
private:

	// TODO(Alex): hide it back
public:
	ESpatialConnectionType GetConnectionType() const;
	void CacheWorkerAttributes();
private:

private:

	// TODO(Alex): hide it back
public:
	void QueueLatestOpList();
	void ProcessOutgoingMessages();
private:

	// TODO(Alex): hide it back
public:
	void StartDevelopmentAuth(FString DevAuthToken, bool bInConnectToLocatorAsClient);
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
};
