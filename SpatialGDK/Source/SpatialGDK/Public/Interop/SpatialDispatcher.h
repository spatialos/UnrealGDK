// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"

#include "Schema/UnrealMetadata.h"

#include <gdk/worker_sdk.h>

#include "SpatialDispatcher.generated.h"

namespace gdk {
	class SpatialOsWorker;
}   // namespace gdk

DECLARE_LOG_CATEGORY_EXTERN(LogSpatialView, Log, All);

class USpatialMetrics;
class USpatialReceiver;
class USpatialStaticComponentView;

UCLASS()
class SPATIALGDK_API USpatialDispatcher : public UObject
{
	GENERATED_BODY()

public:
	using FCallbackId = uint32;

	void Init(USpatialReceiver* InReceiver, USpatialStaticComponentView* InStaticComponentView, USpatialMetrics* InSpatialMetrics);
	void ProcessOps(const gdk::SpatialOsWorker& Worker);

	// Each callback method returns a callback ID which is incremented for each registration.
	// ComponentId must be in the range 1000 - 2000.
	// Callbacks can be deregistered through passing the corresponding callback ID to the RemoveOpCallback function.
	FCallbackId OnAddComponent(Worker_ComponentId ComponentId, const TFunction<void(const Worker_AddComponentOp&)>& Callback);
	FCallbackId OnRemoveComponent(Worker_ComponentId ComponentId, const TFunction<void(const Worker_RemoveComponentOp&)>& Callback);
	FCallbackId OnAuthorityChange(Worker_ComponentId ComponentId, const TFunction<void(const Worker_AuthorityChangeOp&)>& Callback);
	FCallbackId OnComponentUpdate(Worker_ComponentId ComponentId, const TFunction<void(const Worker_ComponentUpdateOp&)>& Callback);
	FCallbackId OnCommandRequest(Worker_ComponentId ComponentId, const TFunction<void(const Worker_CommandRequestOp&)>& Callback);
	FCallbackId OnCommandResponse(Worker_ComponentId ComponentId, const TFunction<void(const Worker_CommandResponseOp&)>& Callback);
	bool RemoveOpCallback(FCallbackId Id);

private:
	struct UserOpCallbackData
	{
		FCallbackId Id;
		TFunction<void(const Worker_Op*)> Callback;
	};

	struct CallbackIdData
	{
		Worker_ComponentId ComponentId;
		Worker_OpType OpType;
	};

	using OpTypeToCallbacksMap = TMap<Worker_OpType, TArray<UserOpCallbackData>>;

	void ProcessWorkerMessages(const gdk::SpatialOsWorker& Worker);
	void ProcessNewEntities(const gdk::SpatialOsWorker& Worker);
	void ProcessEntityComponentMessages(const gdk::SpatialOsWorker& Worker, gdk::ComponentId RangeId);
	void ProcessGdkCommands(const gdk::SpatialOsWorker& Worker, gdk::ComponentId RangeId);
	void ProcessUserMessages(const gdk::SpatialOsWorker& Worker, gdk::ComponentId RangeId);
	void ProcessWorldCommandResponses(const gdk::SpatialOsWorker& Worker);

	void ProcessExternalSchemaOp(Worker_Op* Op);
	FCallbackId AddGenericOpCallback(Worker_ComponentId ComponentId, Worker_OpType OpType, const TFunction<void(const Worker_Op*)>& Callback);
	void RunCallbacks(Worker_ComponentId ComponentId, const Worker_Op* Op);

	UPROPERTY()
	USpatialReceiver* Receiver;

	UPROPERTY()
	USpatialStaticComponentView* StaticComponentView;

	UPROPERTY()
	USpatialMetrics* SpatialMetrics;

	// This index is incremented and returned every time an AddOpCallback function is called.
	// CallbackIds enable you to deregister callbacks using the RemoveOpCallback function. 
	// RunCallbacks is called by the SpatialDispatcher and executes all user registered 
	// callbacks for the matching component ID and network operation type.
	FCallbackId NextCallbackId;
	TMap<Worker_ComponentId, OpTypeToCallbacksMap> ComponentOpTypeToCallbacksMap;
	TMap<FCallbackId, CallbackIdData> CallbackIdToDataMap;
};
