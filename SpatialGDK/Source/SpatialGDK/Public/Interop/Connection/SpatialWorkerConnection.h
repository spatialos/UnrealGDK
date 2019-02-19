// Copyright (c) Improbable Worlds Ltd, All Rights Reserved
#pragma once

#include "Interop/Connection/ConnectionConfig.h"

#include <WorkerSDK/improbable/c_schema.h>
#include <WorkerSDK/improbable/c_worker.h>

#include "SpatialWorkerConnection.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogSpatialWorkerConnection, Log, All);

class UWorld;

enum class SpatialConnectionType
{
	Receptionist,
	LegacyLocator,
	Locator
};

UCLASS()
class SPATIALGDK_API USpatialWorkerConnection : public UObject
{

	GENERATED_BODY()

public:
	virtual void FinishDestroy() override;
	void DestroyConnection();

	void Connect(bool bConnectAsClient);

	FORCEINLINE bool IsConnected() { return bIsConnected; }

	// Worker Connection Interface
	Worker_OpList* GetOpList();
	Worker_RequestId SendReserveEntityIdRequest();
	Worker_RequestId SendReserveEntityIdsRequest(uint32_t NumOfEntities);
	Worker_RequestId SendCreateEntityRequest(uint32_t ComponentCount, const Worker_ComponentData* Components, const Worker_EntityId* EntityId);
	Worker_RequestId SendDeleteEntityRequest(Worker_EntityId EntityId);
	void SendComponentUpdate(Worker_EntityId EntityId, const Worker_ComponentUpdate* ComponentUpdate);
	Worker_RequestId SendCommandRequest(Worker_EntityId EntityId, const Worker_CommandRequest* Request, uint32_t CommandId);
	void SendCommandResponse(Worker_RequestId RequestId, const Worker_CommandResponse* Response);
	void SendLogMessage(const uint8_t Level, const char* LoggerName, const char* Message);
	void SendComponentInterest(Worker_EntityId EntityId, const TArray<Worker_InterestOverride>& ComponentInterest);
	Worker_RequestId SendEntityQueryRequest(const Worker_EntityQuery* EntityQuery);
	void SendMetrics(const Worker_Metrics* Metrics);
	FString GetWorkerId() const;
	const TArray<FString>& GetWorkerAttributes() const;

	FReceptionistConfig ReceptionistConfig;
	FLegacyLocatorConfig LegacyLocatorConfig;
	FLocatorConfig LocatorConfig;

private:
	void ConnectToReceptionist(bool bConnectAsClient);
	void ConnectToLegacyLocator();
	void ConnectToLocator();

	void OnConnectionSuccess();
	void OnPreConnectionFailure(const FString& Reason);
	void OnConnectionFailure();

	Worker_ConnectionParameters CreateConnectionParameters(FConnectionConfig& Config);
	SpatialConnectionType GetConnectionType() const;

	void CacheWorkerAttributes();

	class USpatialNetDriver* GetSpatialNetDriverChecked() const;

	Worker_Connection* WorkerConnection;
	Worker_Locator* WorkerLegacyLocator;
	Worker_Alpha_Locator* WorkerLocator;

	bool bIsConnected;

	TArray<FString> CachedWorkerAttributes;
};
