// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "Interop/Connection/SpatialOSWorkerInterface.h"
#include "SpatialCommonTypes.h"
#include "SpatialView/OpList/ExtractedOpList.h"
#include "SpatialView/OpList/OpList.h"
#include "SpatialView/ViewCoordinator.h"

#include "SpatialWorkerConnection.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogSpatialWorkerConnection, Log, All);

UCLASS()
class SPATIALGDK_API USpatialWorkerConnection : public UObject, public SpatialOSWorkerInterface
{
	GENERATED_BODY()

public:
	void SetConnection(Worker_Connection* WorkerConnectionIn);
	void FinishDestroy();
	void DestroyConnection();

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

	PhysicalWorkerName GetWorkerId() const;
	const TArray<FString>& GetWorkerAttributes() const;

	void ProcessOutgoingMessages();
	void MaybeFlush();
	void Flush();

	void SetStartupComplete();

	DECLARE_MULTICAST_DELEGATE_OneParam(FOnEnqueueMessage, const SpatialGDK::FOutgoingMessage*);
	FOnEnqueueMessage OnEnqueueMessage;

	DECLARE_MULTICAST_DELEGATE_OneParam(FOnDequeueMessage, const SpatialGDK::FOutgoingMessage*);
	FOnDequeueMessage OnDequeueMessage;

private:
	static bool IsStartupComponent(Worker_ComponentId Id);
	static void ExtractStartupOps(SpatialGDK::OpList& OpList, SpatialGDK::ExtractedOpListData& ExtractedOpList);
	bool StartupComplete = false;
	TUniquePtr<SpatialGDK::ViewCoordinator> Coordinator;
};
