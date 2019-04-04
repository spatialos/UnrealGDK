// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"

#include "Schema/Component.h"
#include "Schema/StandardLibrary.h"
#include "Schema/UnrealMetadata.h"
#include "SpatialCommonTypes.h"
#include "SpatialConstants.h"

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
	// ComponentId must be in the range 1000 - 2000.
	// Callbacks can be deregistered through passing the corresponding callback ID to the RemoveOpCallback function.
	template<typename T>
	using TCallback = TFunction<void(T)>;
	using FCallbackId = uint32;
	FCallbackId AddOpCallback(Worker_ComponentId ComponentId, const TCallback<const Worker_AddComponentOp&>& Callback);
	FCallbackId AddOpCallback(Worker_ComponentId ComponentId, const TCallback<const Worker_RemoveComponentOp&>& Callback);
	FCallbackId AddOpCallback(Worker_ComponentId ComponentId, const TCallback<const Worker_AuthorityChangeOp&>& Callback);
	FCallbackId AddOpCallback(Worker_ComponentId ComponentId, const TCallback<const Worker_ComponentUpdateOp&>& Callback);
	FCallbackId AddOpCallback(Worker_ComponentId ComponentId, const TCallback<const Worker_CommandRequestOp&>& Callback);
	FCallbackId AddOpCallback(Worker_ComponentId ComponentId, const TCallback<const Worker_CommandResponseOp&>& Callback);
	void RemoveOpCallback(FCallbackId Id);

private:
	struct UserOpCallbackData
	{
		FCallbackId Id;
		TCallback<const Worker_Op*> Callback;
	};

	struct CallbackIdData
	{
		Worker_ComponentId ComponentId;
		Worker_OpType OpType;
	};

	bool IsExternalSchemaOp(Worker_Op* Op) const;
	void ProcessExternalSchemaOp(Worker_Op* Op);
	Worker_ComponentId GetComponentId(Worker_Op* Op) const;
	FCallbackId AddGenericOpCallback(Worker_ComponentId ComponentId, Worker_OpType OpType, const TCallback<const Worker_Op*>& Callback);
	void RunCallbacks(Worker_ComponentId ComponentId, const Worker_Op* Op);

	UPROPERTY()
	USpatialNetDriver* NetDriver;

	UPROPERTY()
	USpatialReceiver* Receiver;

	UPROPERTY()
	USpatialStaticComponentView* StaticComponentView;

	// This index is incremented and returned every time an AddOpCallback function is called.
	// CallbackIds enable you to deregister callbacks using the RemoveOpCallback function. 
	// RunCallbacks is called by the SpatialDispatcher and executes all user registered 
	// callbacks for the matching component ID and network operation type.
	FCallbackId NextCallbackId;
	TMap<Worker_ComponentId, TMap<Worker_OpType, TArray<UserOpCallbackData>>> ComponentOpTypeToCallbackIdMap;
	TMap<FCallbackId, CallbackIdData> CallbackIdToDataMap;
};
