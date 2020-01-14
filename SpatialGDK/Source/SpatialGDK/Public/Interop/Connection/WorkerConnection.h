// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "SpatialWorkerConnectionInterface.h"

#include "SpatialCommonTypes.h"

#include "Interop/Connection/ConnectionConfig.h"

#include "Utils/SpatialLatencyTracer.h"

#include "WorkerOpListSerializer.h"

#include "HAL/Runnable.h"
#include "CoreMinimal.h"

#include "WorkerConnection.generated.h"

namespace SpatialGDK
{
	struct SpatialMetrics;
}
class FRunnableThread;

UCLASS()
class SPATIALGDK_API UWorkerConnectionCallbacks : public UObject
{
	GENERATED_BODY()

public:
	// TODO(Alex): fix these!
	DECLARE_MULTICAST_DELEGATE_OneParam(FOnEnqueueMessage, const SpatialGDK::FOutgoingMessage*);
	FOnEnqueueMessage OnEnqueueMessage;

	DECLARE_MULTICAST_DELEGATE_OneParam(FOnDequeueMessage, const SpatialGDK::FOutgoingMessage*);
	FOnDequeueMessage OnDequeueMessage;

	DECLARE_DELEGATE(OnConnectionToSpatialOSSucceededDelegate)
	OnConnectionToSpatialOSSucceededDelegate OnConnectedCallback;

	DECLARE_DELEGATE_TwoParams(OnConnectionToSpatialOSFailedDelegate, uint8_t, const FString&);
	OnConnectionToSpatialOSFailedDelegate OnFailedToConnectCallback;
};

// TODO(Alex): use MoveTemp for function arguments

UCLASS()
class SPATIALGDK_API UWorkerConnection : public UObject, public FRunnable
{
	GENERATED_BODY()

public:
	UWorkerConnection(const FObjectInitializer & ObjectInitializer = FObjectInitializer::Get());
	void ProperInit(bool bInitAsClient);
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
	void SendAddComponent(Worker_EntityId EntityId, Worker_ComponentData* ComponentData);
	void SendRemoveComponent(Worker_EntityId EntityId, Worker_ComponentId ComponentId);
	void SendCommandResponse(Worker_RequestId RequestId, const Worker_CommandResponse* Response);
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
	UWorkerConnectionCallbacks* WorkerConnectionCallbacks;

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

	TUniquePtr<USpatialWorkerConnectionInterface> WorkerConnectionImpl;

	// TODO(Alex): 
	static UE4_OpLists ClientSerializedOpLists;
	static UE4_OpLists ServerSerializedOpLists;
	//UE4_OpLists SerializedOpLists;
};
