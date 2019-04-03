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
	using UserCallback = TFunction<void(T)>;
	using CallbackId = uint32;
	CallbackId AddOpCallback(Worker_ComponentId ComponentId, const UserCallback<Worker_AddComponentOp>& Callback);
	CallbackId AddOpCallback(Worker_ComponentId ComponentId, const UserCallback<Worker_RemoveComponentOp>& Callback);
	CallbackId AddOpCallback(Worker_ComponentId ComponentId, const UserCallback<Worker_AuthorityChangeOp>& Callback);
	CallbackId AddOpCallback(Worker_ComponentId ComponentId, const UserCallback<Worker_ComponentUpdateOp>& Callback);
	CallbackId AddOpCallback(Worker_ComponentId ComponentId, const UserCallback<Worker_CommandRequestOp>& Callback);
	CallbackId AddOpCallback(Worker_ComponentId ComponentId, const UserCallback<Worker_CommandResponseOp>& Callback);
	void RemoveOpCallback(CallbackId Id);

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

	struct UserOpCallbackData
	{
		Worker_ComponentId ComponentId;
		Worker_OpType OpType;
		UserCallback<const Worker_Op*> Callback;
	};

	// This index is incremented and returned every time the AddOpCallback function is called.
	// These indexes enable you to deregister callbacks using the RemoveOpCallback function. 
	// RunUserCallbacks is called by the SpatialDispatcher and executes all user registered 
	// callbacks for the matching component ID and network operation type.
	CallbackId NextCallbackId;
	CallbackId AddGenericOpCallback(Worker_ComponentId ComponentId, Worker_OpType OpType, UserCallback<const Worker_Op*> Callback);
	void RunUserCallbacks(Worker_ComponentId ComponentId, const Worker_Op* Op);
	TMap<Worker_ComponentId, TMap<Worker_OpType, TArray<CallbackId>>> ComponentOpTypeToCallbackIdMap;
	TMap<CallbackId, UserOpCallbackData> CallbackIdToDataMap;
};
