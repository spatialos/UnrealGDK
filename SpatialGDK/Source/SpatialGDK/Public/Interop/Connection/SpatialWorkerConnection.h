// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

// TODO(Alex): move include to .cpp, doesn't work? :(
#include "RealWorkerConnection.h"

#include "SpatialWorkerConnectionCallbacks.h"

#include "SpatialCommonTypes.h"

#include "Interop/Connection/ConnectionConfig.h"

#include "Utils/SpatialLatencyTracer.h"

#include "HAL/Runnable.h"
#include "CoreMinimal.h"

#include "SpatialWorkerConnection.generated.h"

namespace SpatialGDK
{
	struct SpatialMetrics;
}
class RealWorkerConnection;
class FRunnableThread;

// TODO(Alex): use MoveTemp for function arguments
// TODO(Alex): inline trivial functions

UCLASS()
class SPATIALGDK_API USpatialWorkerConnection : public UObject, public FRunnable
{
	GENERATED_BODY()

public:
	USpatialWorkerConnection(const FObjectInitializer & ObjectInitializer = FObjectInitializer::Get());
	virtual void FinishDestroy() override;

	void DestroyConnection();
	PhysicalWorkerName GetWorkerId() const;
	void CacheWorkerAttributes();

	bool IsConnected() const;

	Worker_RequestId SendCommandRequest(Worker_EntityId EntityId, const Worker_CommandRequest* Request, uint32_t CommandId);
	void SendComponentUpdate(Worker_EntityId EntityId, const Worker_ComponentUpdate* ComponentUpdate, const TraceKey Key = USpatialLatencyTracer::InvalidTraceKey);
	Worker_RequestId SendEntityQueryRequest(const Worker_EntityQuery* EntityQuery);
	Worker_RequestId SendReserveEntityIdsRequest(uint32_t NumOfEntities);
	Worker_RequestId SendDeleteEntityRequest(Worker_EntityId EntityId);

	Worker_RequestId SendCreateEntityRequest(TArray<Worker_ComponentData>&& Components, const Worker_EntityId* EntityId);
	void SendAddComponent(Worker_EntityId EntityId, Worker_ComponentData* ComponentData, const TraceKey Key = USpatialLatencyTracer::InvalidTraceKey);

	void SendRemoveComponent(Worker_EntityId EntityId, Worker_ComponentId ComponentId);
	void SendCommandResponse(Worker_RequestId RequestId, const Worker_CommandResponse* Response);
	void SendCommandFailure(Worker_RequestId RequestId, const FString& Message);
	void SendComponentInterest(Worker_EntityId EntityId, TArray<Worker_InterestOverride>&& ComponentInterest);

	void SendMetrics(const SpatialGDK::SpatialMetrics& Metrics);
	void SendLogMessage(uint8_t Level, const FName& LoggerName, const TCHAR* Message);

	const TArray<FString>& GetWorkerAttributes() const;
	void SetConnectionType(ESpatialConnectionType InConnectionType);

	void Connect(bool bInitAsClient, uint32 PlayInEditorID);

	TArray<Worker_OpList*> GetOpList();

	FReceptionistConfig ReceptionistConfig;
	FLocatorConfig LocatorConfig;

	// TODO(Alex): Use TUniquePtr
	UPROPERTY()
	USpatialWorkerConnectionCallbacks* Callbacks;

private:
	void InitializeOpsProcessingThread();
	void OnConnectionSuccess();
	void OnConnectionFailure();

	// Begin FRunnable Interface
	virtual bool Init() override;
	virtual uint32 Run() override;
	virtual void Stop() override;
	// End FRunnable Interface

private:
	bool bIsConnected;
	float OpsUpdateInterval;
	FThreadSafeBool KeepRunning = true;
	FRunnableThread* OpsProcessingThread;

	bool bConnectAsClient = false;

	TUniquePtr<RealWorkerConnection> WorkerConnectionImpl;
};
