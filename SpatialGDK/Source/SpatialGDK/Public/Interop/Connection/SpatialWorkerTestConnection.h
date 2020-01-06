// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "SpatialWorkerConnectionInterface.h"

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

DECLARE_LOG_CATEGORY_EXTERN(LogSpatialWorkerTestConnection, Log, All);

class SPATIALGDK_API USpatialWorkerTestConnection : public USpatialWorkerConnectionInterface
{
public:
	// TODO(Alex): is it called properly?
	~USpatialWorkerTestConnection();
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

	TArray<FString> CachedWorkerAttributes;
	ESpatialConnectionType ConnectionType = ESpatialConnectionType::Receptionist;
};
