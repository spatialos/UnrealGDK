// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "Interop/Connection/SpatialWorkerConnection.h"
#include "SpatialCommonTypes.h"
#include "SpatialView/OpList/OpList.h"
#include "SpatialView/ViewCoordinator.h"

#include "SpatialViewWorkerConnection.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogSpatialViewWorkerConnection, Log, All);

UCLASS()
class SPATIALGDK_API USpatialViewWorkerConnection : public USpatialWorkerConnection
{
	GENERATED_BODY()

public:
	virtual void SetConnection(Worker_Connection* WorkerConnectionIn) override;
	virtual void FinishDestroy() override;
	virtual void DestroyConnection() override;

	// Worker Connection Interface
	virtual TArray<SpatialGDK::OpList> GetOpList() override;
	virtual Worker_RequestId SendReserveEntityIdsRequest(uint32_t NumOfEntities) override;
	virtual Worker_RequestId SendCreateEntityRequest(TArray<FWorkerComponentData> Components, const Worker_EntityId* EntityId) override;
	virtual Worker_RequestId SendDeleteEntityRequest(Worker_EntityId EntityId) override;
	virtual void SendAddComponent(Worker_EntityId EntityId, FWorkerComponentData* ComponentData) override;
	virtual void SendRemoveComponent(Worker_EntityId EntityId, Worker_ComponentId ComponentId) override;
	virtual void SendComponentUpdate(Worker_EntityId EntityId, FWorkerComponentUpdate* ComponentUpdate) override;
	virtual Worker_RequestId SendCommandRequest(Worker_EntityId EntityId, Worker_CommandRequest* Request, uint32_t CommandId) override;
	virtual void SendCommandResponse(Worker_RequestId RequestId, Worker_CommandResponse* Response) override;
	virtual void SendCommandFailure(Worker_RequestId RequestId, const FString& Message) override;
	virtual void SendLogMessage(uint8_t Level, const FName& LoggerName, const TCHAR* Message) override;
	virtual void SendComponentInterest(Worker_EntityId EntityId, TArray<Worker_InterestOverride>&& ComponentInterest) override;
	virtual Worker_RequestId SendEntityQueryRequest(const Worker_EntityQuery* EntityQuery) override;
	virtual void SendMetrics(SpatialGDK::SpatialMetrics Metrics) override;

	virtual PhysicalWorkerName GetWorkerId() const override;
	virtual const TArray<FString>& GetWorkerAttributes() const override;

	virtual void ProcessOutgoingMessages() override;
	virtual void MaybeFlush() override;
	virtual void Flush() override;

private:
	TUniquePtr<SpatialGDK::ViewCoordinator> Coordinator;
};
