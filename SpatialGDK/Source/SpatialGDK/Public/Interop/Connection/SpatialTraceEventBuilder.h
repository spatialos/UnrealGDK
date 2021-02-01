// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "Interop/Connection/SpatialTraceEvent.h"
#include "Interop/Connection/SpatialTraceUniqueId.h"
#include "SpatialCommonTypes.h"
#include "WorkerSDK/improbable/c_worker.h"

#define GDK_EVENT_NAMESPACE "unreal_gdk."

namespace SpatialGDK
{
class SPATIALGDK_API FSpatialTraceEventBuilder
{
public:
	FSpatialTraceEventBuilder(FName InType);
	FSpatialTraceEventBuilder(FName InType, FString InMessage);

	FSpatialTraceEventBuilder AddObject(FString Key, const UObject* Object);
	FSpatialTraceEventBuilder AddFunction(FString Key, const UFunction* Function);
	FSpatialTraceEventBuilder AddEntityId(FString Key, const Worker_EntityId EntityId);
	FSpatialTraceEventBuilder AddComponentId(FString Key, const Worker_ComponentId ComponentId);
	FSpatialTraceEventBuilder AddFieldId(FString Key, const uint32 FieldId);
	FSpatialTraceEventBuilder AddNewWorkerId(FString Key, const uint32 NewWorkerId);
	FSpatialTraceEventBuilder AddCommand(FString Key, const FString& Command);
	FSpatialTraceEventBuilder AddRequestId(FString Key, const int64 RequestId);
	FSpatialTraceEventBuilder AddAuthority(FString Key, const Worker_Authority Role);
	FSpatialTraceEventBuilder AddKeyValue(FString Key, FString Value);
	FSpatialTraceEvent GetEvent() &&;

	static FSpatialTraceEvent CreateProcessRPC(const UObject* Object, UFunction* Function, const EventTraceUniqueId& LinearTraceId);
	static FSpatialTraceEvent CreatePushRPC(const UObject* Object, UFunction* Function);
	static FSpatialTraceEvent CreateSendRPC(const EventTraceUniqueId& LinearTraceId);

	static FSpatialTraceEvent CreateQueueRPC();
	static FSpatialTraceEvent CreateRetryRPC();
	static FSpatialTraceEvent CreatePropertyChanged(const UObject* Object, const Worker_EntityId EntityId, const FString& PropertyName,
													EventTraceUniqueId LinearTraceId);
	static FSpatialTraceEvent CreateSendPropertyUpdate(const UObject* Object, const Worker_EntityId EntityId,
													   const Worker_ComponentId ComponentId);
	static FSpatialTraceEvent CreateReceivePropertyUpdate(const UObject* Object, const Worker_EntityId EntityId,
														  const Worker_ComponentId ComponentId, const FString& PropertyName,
														  EventTraceUniqueId LinearTraceId);
	static FSpatialTraceEvent CreateMergeSendRPCs(const Worker_EntityId EntityId, const Worker_ComponentId ComponentId);
	static FSpatialTraceEvent CreateMergeComponentUpdate(const Worker_EntityId EntityId, const Worker_ComponentId ComponentId);
	static FSpatialTraceEvent CreateObjectPropertyComponentUpdate(const UObject* Object);
	static FSpatialTraceEvent CreateSendCommandRequest(const FString& Command, const int64 RequestId);
	static FSpatialTraceEvent CreateReceiveCommandRequest(const FString& Command, const int64 RequestId);
	static FSpatialTraceEvent CreateReceiveCommandRequest(const FString& Command, const UObject* Actor, const UObject* TargetObject,
														  const UFunction* Function, const int32 TraceId, const int64 RequestId);
	static FSpatialTraceEvent CreateSendCommandResponse(const int64 RequestId, const bool bSuccess);
	static FSpatialTraceEvent CreateReceiveCommandResponse(const FString& Command, const int64 RequestId);
	static FSpatialTraceEvent CreateReceiveCommandResponse(const UObject* Actor, const int64 RequestId, const bool bSuccess);
	static FSpatialTraceEvent CreateReceiveCommandResponse(const UObject* Actor, const UObject* TargetObject, const UFunction* Function,
														   int64 RequestId, const bool bSuccess);
	static FSpatialTraceEvent CreateSendRemoveEntity(const UObject* Object, const Worker_EntityId EntityId);
	static FSpatialTraceEvent CreateReceiveRemoveEntity(const Worker_EntityId EntityId);
	static FSpatialTraceEvent CreateSendCreateEntity(const UObject* Object, const Worker_EntityId EntityId);
	static FSpatialTraceEvent CreateReceiveCreateEntity(const Worker_EntityId EntityId);
	static FSpatialTraceEvent CreateReceiveCreateEntitySuccess(const UObject* Object, const Worker_EntityId EntityId);
	static FSpatialTraceEvent CreateSendRetireEntity(const UObject* Object, const Worker_EntityId EntityId);
	static FSpatialTraceEvent CreateAuthorityIntentUpdate(VirtualWorkerId WorkerId, const UObject* Object);
	static FSpatialTraceEvent CreateAuthorityChange(const Worker_EntityId EntityId, const Worker_ComponentId ComponentId,
													const Worker_Authority Authority);
	static FSpatialTraceEvent CreateComponentUpdate(const UObject* Object, const UObject* TargetObject, const Worker_EntityId EntityId,
													const Worker_ComponentId ComponentId);
	static FSpatialTraceEvent CreateGenericMessage(FString Message);

private:
	static FString AuthorityToString(Worker_Authority Authority);
	static FString BoolToString(bool bInput);

	FSpatialTraceEvent SpatialTraceEvent;
};
} // namespace SpatialGDK
