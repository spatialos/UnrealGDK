// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "Interop/Connection/SpatialTraceEvent.h"
#include "SpatialCommonTypes.h"
#include "WorkerSDK/improbable/c_worker.h"

#define GDK_EVENT_NAMESPACE "unreal_gdk."

namespace SpatialGDK
{
class FSpatialTraceEventBuilder
{
public:
	explicit FSpatialTraceEventBuilder(FString InType);
	explicit FSpatialTraceEventBuilder(FString InType, FString InMessage);

	// --- Builder Functions ---

	FSpatialTraceEventBuilder AddObject(FString Key, const UObject* Object);
	FSpatialTraceEventBuilder AddFunction(FString Key, const UFunction* Function);
	FSpatialTraceEventBuilder AddEntityId(FString Key, const FEntityId EntityId);
	FSpatialTraceEventBuilder AddComponentId(FString Key, const FComponentId ComponentId);
	FSpatialTraceEventBuilder AddFieldId(FString Key, const uint32 FieldId);
	FSpatialTraceEventBuilder AddNewWorkerId(FString Key, const uint32 NewWorkerId);
	FSpatialTraceEventBuilder AddCommand(FString Key, const FString& Command);
	FSpatialTraceEventBuilder AddRequestId(FString Key, const int64 RequestId);
	FSpatialTraceEventBuilder AddAuthority(FString Key, const Worker_Authority Role);
	FSpatialTraceEventBuilder AddKeyValue(FString Key, FString Value);
	FSpatialTraceEvent GetEvent() &&;

	// --- Static Functions ---

	static FSpatialTraceEvent ProcessRPC(const UObject* Object, UFunction* Function);
	static FSpatialTraceEvent SendRPC(const UObject* Object, UFunction* Function);
	static FSpatialTraceEvent QueueRPC();
	static FSpatialTraceEvent RetryRPC();
	static FSpatialTraceEvent PropertyUpdate(const UObject* Object, const FEntityId EntityId, const FComponentId ComponentId,
											 const FString& PropertyName);
	static FSpatialTraceEvent MergeComponent(const FEntityId EntityId, const FComponentId ComponentId);
	static FSpatialTraceEvent SendCommandRequest(const FString& Command, const int64 RequestId);
	static FSpatialTraceEvent ReceiveCommandRequest(const FString& Command, const int64 RequestId);
	static FSpatialTraceEvent ReceiveCommandRequest(const FString& Command, const UObject* Actor, const UObject* TargetObject,
													const UFunction* Function, const int32 TraceId, const int64 RequestId);
	static FSpatialTraceEvent SendCommandResponse(const int64 RequestId, const bool bSuccess);
	static FSpatialTraceEvent ReceiveCommandResponse(const FString& Command, const int64 RequestId);
	static FSpatialTraceEvent ReceiveCommandResponse(const UObject* Actor, const int64 RequestId, const bool bSuccess);
	static FSpatialTraceEvent ReceiveCommandResponse(const UObject* Actor, const UObject* TargetObject, const UFunction* Function,
													 int64 RequestId, const bool bSuccess);
	static FSpatialTraceEvent SendRemoveEntity(const UObject* Object, const FEntityId EntityId);
	static FSpatialTraceEvent ReceiveRemoveEntity(const FEntityId EntityId);
	static FSpatialTraceEvent SendCreateEntity(const UObject* Object, const FEntityId EntityId);
	static FSpatialTraceEvent ReceiveCreateEntity(const FEntityId EntityId);
	static FSpatialTraceEvent ReceiveCreateEntitySuccess(const UObject* Object, const FEntityId EntityId);
	static FSpatialTraceEvent SendRetireEntity(const UObject* Object, const FEntityId EntityId);
	static FSpatialTraceEvent AuthorityIntentUpdate(VirtualWorkerId WorkerId, const UObject* Object);
	static FSpatialTraceEvent AuthorityChange(const FEntityId EntityId, const FComponentId ComponentId, const Worker_Authority Authority);
	static FSpatialTraceEvent ComponentUpdate(const UObject* Object, const UObject* TargetObject, const FEntityId EntityId,
											  const FComponentId ComponentId);
	static FSpatialTraceEvent GenericMessage(FString Message);
	static FString AuthorityToString(Worker_Authority Authority);
	static FString BoolToString(bool bInput);

private:
	FSpatialTraceEvent SpatialTraceEvent;
};
} // namespace SpatialGDK
