// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "SpatialCommonTypes.h"

// TODO(Alex): remove!
#include "SpatialWorkerConnection.h"

#include "Utils/SpatialLatencyTracer.h"

#include "HAL/Runnable.h"
#include "CoreMinimal.h"

#include "WorkerConnection.generated.h"

UCLASS()
class SPATIALGDK_API UWorkerConnection : public UObject, public FRunnable
{
	GENERATED_BODY()

public:
	void DestroyConnection();
	PhysicalWorkerName GetWorkerId() const;

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

	void Connect(bool bConnectAsClient, uint32 PlayInEditorID);
	bool IsConnected() const;

	TArray<Worker_OpList*> GetOpList();

	DECLARE_DELEGATE(OnConnectionToSpatialOSSucceededDelegate)
	OnConnectionToSpatialOSSucceededDelegate OnConnectedCallback;

	DECLARE_DELEGATE_TwoParams(OnConnectionToSpatialOSFailedDelegate, uint8_t, const FString&);
	OnConnectionToSpatialOSFailedDelegate OnFailedToConnectCallback;

	FReceptionistConfig ReceptionistConfig;
	FLocatorConfig LocatorConfig;

private:
	// Begin FRunnable Interface
	virtual bool Init() override;
	virtual uint32 Run() override;
	virtual void Stop() override;
	// End FRunnable Interface

private:
	// TODO(Alex): remove it
	TArray<FString> WorkerAttributes;
};
