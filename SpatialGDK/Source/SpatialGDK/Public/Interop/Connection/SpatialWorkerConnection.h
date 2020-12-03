// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "Interop/Connection/SpatialOSWorkerInterface.h"

#include "SpatialCommonTypes.h"
#include "SpatialConstants.h"
#include "SpatialView/EntityView.h"
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
	void SetConnection(Worker_Connection* WorkerConnectionIn, TSharedPtr<SpatialGDK::SpatialEventTracer> EventTracer,
					   SpatialGDK::FComponentSetData ComponentSetData);
	void DestroyConnection();

	// UObject interface.
	virtual void FinishDestroy() override;

	// Worker Connection Interface
	virtual const TArray<SpatialGDK::EntityDelta>& GetEntityDeltas() override;
	virtual const TArray<Worker_Op>& GetWorkerMessages() override;

	virtual Worker_RequestId SendReserveEntityIdsRequest(uint32_t NumOfEntities, const SpatialGDK::FRetryData& RetryData) override;
	virtual Worker_RequestId SendCreateEntityRequest(TArray<FWorkerComponentData> Components, const Worker_EntityId* EntityId,
													 const SpatialGDK::FRetryData& RetryData,
													 const FSpatialGDKSpanId& SpanId = {}) override;
	virtual Worker_RequestId SendDeleteEntityRequest(Worker_EntityId EntityId, const SpatialGDK::FRetryData& RetryData,
													 const FSpatialGDKSpanId& SpanId = {}) override;
	virtual void SendAddComponent(Worker_EntityId EntityId, FWorkerComponentData* ComponentData,
								  const FSpatialGDKSpanId& SpanId = {}) override;
	virtual void SendRemoveComponent(Worker_EntityId EntityId, Worker_ComponentId ComponentId,
									 const FSpatialGDKSpanId& SpanId = {}) override;
	virtual void SendComponentUpdate(Worker_EntityId EntityId, FWorkerComponentUpdate* ComponentUpdate,
									 const FSpatialGDKSpanId& SpanId = {}) override;
	virtual Worker_RequestId SendCommandRequest(Worker_EntityId EntityId, Worker_CommandRequest* Request,
												const SpatialGDK::FRetryData& RetryData, const FSpatialGDKSpanId& SpanId) override;
	virtual void SendCommandResponse(Worker_RequestId RequestId, Worker_CommandResponse* Response,
									 const FSpatialGDKSpanId& SpanId = {}) override;
	virtual void SendCommandFailure(Worker_RequestId RequestId, const FString& Message, const FSpatialGDKSpanId& SpanId = {}) override;
	virtual void SendLogMessage(uint8_t Level, const FName& LoggerName, const TCHAR* Message) override;
	virtual Worker_RequestId SendEntityQueryRequest(const Worker_EntityQuery* EntityQuery,
													const SpatialGDK::FRetryData& RetryData) override;
	virtual void SendMetrics(SpatialGDK::SpatialMetrics Metrics) override;

	void Advance(float DeltaTimeS);
	bool HasDisconnected() const;
	Worker_ConnectionStatusCode GetConnectionStatus() const;
	FString GetDisconnectReason() const;

	const SpatialGDK::EntityView& GetView() const;
	SpatialGDK::ViewCoordinator& GetCoordinator() const;

	PhysicalWorkerName GetWorkerId() const;
	Worker_EntityId GetWorkerSystemEntityId() const;

	SpatialGDK::CallbackId RegisterComponentAddedCallback(Worker_ComponentId ComponentId, SpatialGDK::FComponentValueCallback Callback);
	SpatialGDK::CallbackId RegisterComponentRemovedCallback(Worker_ComponentId ComponentId, SpatialGDK::FComponentValueCallback Callback);
	SpatialGDK::CallbackId RegisterComponentValueCallback(Worker_ComponentId ComponentId, SpatialGDK::FComponentValueCallback Callback);
	SpatialGDK::CallbackId RegisterAuthorityGainedCallback(Worker_ComponentId ComponentId, SpatialGDK::FEntityCallback Callback);
	SpatialGDK::CallbackId RegisterAuthorityLostCallback(Worker_ComponentId ComponentId, SpatialGDK::FEntityCallback Callback);
	SpatialGDK::CallbackId RegisterAuthorityLostTempCallback(Worker_ComponentId ComponentId, SpatialGDK::FEntityCallback Callback);
	void RemoveCallback(SpatialGDK::CallbackId Id);

	void Flush();

	void SetStartupComplete();

	DECLARE_MULTICAST_DELEGATE_OneParam(FOnEnqueueMessage, const SpatialGDK::FOutgoingMessage*);
	FOnEnqueueMessage OnEnqueueMessage;

	DECLARE_MULTICAST_DELEGATE_OneParam(FOnDequeueMessage, const SpatialGDK::FOutgoingMessage*);
	FOnDequeueMessage OnDequeueMessage;

	SpatialGDK::SpatialEventTracer* GetEventTracer() const { return EventTracer; }

private:
	static bool IsStartupComponent(Worker_ComponentId Id);
	static void ExtractStartupOps(SpatialGDK::OpList& OpList, SpatialGDK::ExtractedOpListData& ExtractedOpList);
	bool StartupComplete = false;
	SpatialGDK::SpatialEventTracer* EventTracer;
	TUniquePtr<SpatialGDK::ViewCoordinator> Coordinator;
};
