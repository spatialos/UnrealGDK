// Copyright (c) Improbable Worlds Ltd, All Rights Reserved
#pragma once

#include "Interop/Connection/ConnectionConfig.h"
#include "HAL/Runnable.h"
#include "Misc/Variant.h"

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

enum class ESpatialVariantTypes : int32
{
	ReserveEntityIdsRequest = EVariantTypes::Custom,
	CreateEntityRequest,
	DeleteEntityRequest,
	ComponentUpdate,
	CommandRequest,
	CommandResponse,
	CommandFailure,
	LogMessage,
	ComponentInterest,
	EntityQueryRequest,
	Metrics
};

struct FReserveEntityIdsRequest
{
	const uint32_t NumOfEntities;
};

struct FCreateEntityRequest
{
	const TArray<Worker_ComponentData> Components;
	const TOptional<Worker_EntityId> EntityId;
};

struct FDeleteEntityRequest
{
	const Worker_EntityId EntityId;
};

struct FComponentUpdate
{
	const Worker_EntityId EntityId;
	const Worker_ComponentUpdate ComponentUpdate;
};

struct FCommandRequest
{
	const Worker_EntityId EntityId;
	const Worker_CommandRequest Request;
	const uint32_t CommandId;
};

struct FCommandResponse
{
	const Worker_RequestId RequestId;
	const Worker_CommandResponse Response;
};

struct FCommandFailure
{
	const Worker_RequestId RequestId;
	const FString Message;
};

struct FLogMessage
{
	const uint8_t Level;
	const FString LoggerName;
	const FString Message;
};

struct FComponentInterest
{
	const Worker_EntityId EntityId;
	const TArray<Worker_InterestOverride> ComponentInterest;
};

struct FEntityQueryRequest
{
	const Worker_EntityQuery EntityQuery;
};

struct FMetrics
{
	const Worker_Metrics Metrics;
};

template<> struct TVariantTraits<FReserveEntityIdsRequest>
{
	static CONSTEXPR EVariantTypes GetType() { return ESpatialVariantTypes::ReserveEntityIdsRequest; }
};

template<> struct TVariantTraits<FCreateEntityRequest>
{
	static CONSTEXPR EVariantTypes GetType() { return ESpatialVariantTypes::CreateEntityRequest; }
};

template<> struct TVariantTraits<FDeleteEntityRequest>
{
	static CONSTEXPR EVariantTypes GetType() { return ESpatialVariantTypes::DeleteEntityRequest; }
};

template<> struct TVariantTraits<FComponentUpdate>
{
	static CONSTEXPR EVariantTypes GetType() { return ESpatialVariantTypes::ComponentUpdate; }
};

template<> struct TVariantTraits<FCommandRequest>
{
	static CONSTEXPR EVariantTypes GetType() { return ESpatialVariantTypes::CommandRequest; }
};

template<> struct TVariantTraits<FCommandResponse>
{
	static CONSTEXPR EVariantTypes GetType() { return ESpatialVariantTypes::CommandResponse; }
};

template<> struct TVariantTraits<FCommandFailure>
{
	static CONSTEXPR EVariantTypes GetType() { return ESpatialVariantTypes::CommandFailure; }
};

template<> struct TVariantTraits<FLogMessage>
{
	static CONSTEXPR EVariantTypes GetType() { return ESpatialVariantTypes::LogMessage; }
};

template<> struct TVariantTraits<FComponentInterest>
{
	static CONSTEXPR EVariantTypes GetType() { return ESpatialVariantTypes::ComponentInterest; }
};

template<> struct TVariantTraits<FEntityQueryRequest>
{
	static CONSTEXPR EVariantTypes GetType() { return ESpatialVariantTypes::EntityQueryRequest; }
};

template<> struct TVariantTraits<FMetrics>
{
	static CONSTEXPR EVariantTypes GetType() { return ESpatialVariantTypes::Metrics; }
};

UCLASS()
class SPATIALGDK_API USpatialWorkerConnection : public UObject, public FRunnable
{

	GENERATED_BODY()

public:
	virtual void FinishDestroy() override;
	void DestroyConnection();

	void Connect(bool bConnectAsClient);

	FORCEINLINE bool IsConnected() { return bIsConnected; }

	// Worker Connection Interface
	Worker_OpList* GetOpList();
	Worker_RequestId SendReserveEntityIdsRequest(uint32_t NumOfEntities);
	Worker_RequestId SendCreateEntityRequest(uint32_t ComponentCount, const Worker_ComponentData* Components, const Worker_EntityId* EntityId);
	Worker_RequestId SendDeleteEntityRequest(Worker_EntityId EntityId);
	void SendComponentUpdate(Worker_EntityId EntityId, const Worker_ComponentUpdate* ComponentUpdate);
	Worker_RequestId SendCommandRequest(Worker_EntityId EntityId, const Worker_CommandRequest* Request, uint32_t CommandId);
	void SendCommandResponse(Worker_RequestId RequestId, const Worker_CommandResponse* Response);
	void SendCommandFailure(Worker_RequestId RequestId, const char* Message = "");
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

	// Begin FRunnable Interface
	virtual bool Init();
	virtual uint32 Run();
	virtual void Exit();
	virtual void Stop();
	// End FRunnable Interface

	FRunnableThread* WorkerThread;

	TQueue<Worker_OpList*> OpListQueue;
	TQueue<FVariant> OutgoingMessagesQueue;
};
