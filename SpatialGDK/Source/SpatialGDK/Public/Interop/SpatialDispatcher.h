// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"

#include "Schema/Component.h"
#include "Schema/StandardLibrary.h"
#include "Schema/UnrealMetadata.h"
#include "SpatialCommonTypes.h"
#include "SpatialConstants.h"
#include "Utils/OpCallbackTemplate.h"

#include <WorkerSDK/improbable/c_schema.h>
#include <WorkerSDK/improbable/c_worker.h>

#include "SpatialDispatcher.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogSpatialView, Log, All);

class USpatialNetDriver;
class USpatialReceiver;
class USpatialStaticComponentView;

UCLASS()
class SPATIALGDK_API USpatialDispatcher : public UObject
{
	GENERATED_BODY()

public:
	void Init(USpatialNetDriver* NetDriver);
	void ProcessOps(Worker_OpList* OpList);

	// AddOpCallback returns a callback ID which is incremented on each callback that is registered.
	// Callbacks can be deregistered through passing the corresponding callback ID to the RemoveOpCallback function.
	using UserOpCallback = const TFunction<void(Worker_ComponentId, const Worker_Op*)>;
	uint32_t AddOpCallback(Worker_ComponentId ComponentId, const UserOpCallback& Callback);
	void RemoveOpCallback(uint32_t Id);

private:
	bool IsExternalSchemaOp(Worker_Op* Op) const;
	void ProcessExternalSchemaOp(Worker_Op* Op);
	Worker_ComponentId GetComponentId(Worker_Op* Op) const;

	UPROPERTY()
	USpatialNetDriver* NetDriver;

	UPROPERTY()
	USpatialReceiver* Receiver;

	UPROPERTY()
	USpatialStaticComponentView* StaticComponentView;

	struct UserOpCallbackData {
		Worker_ComponentId ComponentId;
		UserOpCallback& Callback;
	};

	// This index is incremented and returned every time the AddOpCallback function is called.
	// These indexes enable you to deregister callbacks using the RemoveOpCallback function. 
	// RunUserCallbacks is called by the SpatialDispatcher and executes all user registered 
	// callbacks for the matching component ID and network operation type.
	uint32_t NextCallbackId;
	void RunUserCallbacks(Worker_ComponentId ComponentId, const Worker_Op* Op);
	TMap<Worker_ComponentId, TSet<uint32_t>> ComponentToCallbackIdMap;
	TMap<uint32_t, UserOpCallbackData> CallbackIdToDataMap;
};
