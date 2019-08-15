// Copyright (c) Improbable Worlds Ltd, All Rights Reserved
#pragma once

#include "Interop/Connection/ConnectionConfig.h"
#include "Interop/Connection/OutgoingMessages.h"

#include <WorkerSDK/improbable/c_schema.h>
#include <WorkerSDK/improbable/c_worker.h>

#include <gdk/spatialos_worker.h>

#include "SpatialWorkerConnection.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogSpatialWorkerConnection, Log, All);

class USpatialNetDriver;
class USpatialGameInstance;
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
	void Init(USpatialGameInstance* InGameInstance);

	virtual void FinishDestroy() override;
	void DestroyConnection();

	void Connect(bool bConnectAsClient);

	bool IsConnected() const { return bIsConnected; }

	// Worker Connection Interface
	TArray<Worker_OpList*> GetOpList();
	const gdk::SpatialOsWorker& GetWorker() const;

	Worker_RequestId SendReserveEntityIdsRequest(uint32_t NumOfEntities);
	Worker_RequestId SendCreateEntityRequest(TArray<Worker_ComponentData>&& Components, const Worker_EntityId* EntityId);
	Worker_RequestId SendDeleteEntityRequest(Worker_EntityId EntityId);
	void SendAddComponent(Worker_EntityId EntityId, Worker_ComponentData* ComponentData);
	void SendRemoveComponent(Worker_EntityId EntityId, Worker_ComponentId ComponentId);
	void SendComponentUpdate(Worker_EntityId EntityId, const Worker_ComponentUpdate* ComponentUpdate);
	Worker_RequestId SendCommandRequest(Worker_EntityId EntityId, const Worker_CommandRequest* Request, uint32_t CommandId);
	void SendCommandResponse(Worker_RequestId RequestId, const Worker_CommandResponse* Response);
	void SendCommandFailure(Worker_RequestId RequestId, const FString& Message);
	void SendLogMessage(uint8_t Level, const FName& LoggerName, const TCHAR* Message);
	void SendComponentInterest(Worker_EntityId EntityId, TArray<Worker_InterestOverride>&& ComponentInterest);
	Worker_RequestId SendEntityQueryRequest(const Worker_EntityQuery* EntityQuery);
	void SendMetrics(const Worker_Metrics& Metrics);

	FString GetWorkerId() const;
	const TArray<FString>& GetWorkerAttributes() const;
	void FlushMessageToSend();
	void Advance();

	FReceptionistConfig ReceptionistConfig;
	FLocatorConfig LocatorConfig;

private:
	void ConnectToReceptionist(bool bConnectAsClient);
	void ConnectToLocator();
	void FinishConnecting(Worker_ConnectionFuture* ConnectionFuture);

	void OnConnectionSuccess(Worker_Connection* Connection);
	void OnPreConnectionFailure(const FString& Reason);
	void OnConnectionFailure(Worker_Connection* Connection);

	SpatialConnectionType GetConnectionType() const;

	void CacheWorkerAttributes();

	USpatialNetDriver* GetSpatialNetDriverChecked() const;

	void StartDevelopmentAuth(FString DevAuthToken);
	static void OnPlayerIdentityToken(void* UserData, const Worker_Alpha_PlayerIdentityTokenResponse* PIToken);
	static void OnLoginTokens(void* UserData, const Worker_Alpha_LoginTokensResponse* LoginTokens);

	TUniquePtr<gdk::SpatialOsWorker> Worker;

	TWeakObjectPtr<USpatialGameInstance> GameInstance;

	bool bIsConnected = false;

	TArray<FString> CachedWorkerAttributes;

	// RequestIds per worker connection start at 1 and incrementally go up each command sent.
	Worker_RequestId NextRequestId = 1;
};
