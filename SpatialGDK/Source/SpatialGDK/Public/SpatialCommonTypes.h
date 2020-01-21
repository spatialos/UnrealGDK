// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"

#include <WorkerSDK/improbable/c_worker.h>

// IMPORTANT: This is required for Linux builds to succeed - don't remove!
// Worker_EntityId from the Worker SDK resolves to a long on Linux.
// These are not a type of key supported by TMap.
using Worker_EntityId_Key = int64;
using Worker_RequestId_Key = int64;

using VirtualWorkerId = uint32;
using PhysicalWorkerName = FString;
using ActorLockToken = int64;
using TraceKey = int32;
constexpr TraceKey InvalidTraceKey = -1;

using WorkerAttributeSet = TArray<FString>;
using WorkerRequirementSet = TArray<WorkerAttributeSet>;
using WriteAclMap = TMap<Worker_ComponentId, WorkerRequirementSet>;

using FChannelObjectPair = TPair<TWeakObjectPtr<class USpatialActorChannel>, TWeakObjectPtr<UObject>>;
using FObjectReferencesMap = TMap<int32, struct FObjectReferences>;
using FReliableRPCMap = TMap<Worker_RequestId_Key, TSharedRef<struct FReliableRPCForRetry>>;

using FObjectToRepStateMap = TMap <struct FUnrealObjectRef, TSet<FChannelObjectPair> >;

struct FWorkerComponentData
{
	FWorkerComponentData() = default;
	FWorkerComponentData(const Worker_ComponentData& InData, TraceKey InTrace = InvalidTraceKey)
		: Data(InData)
#if TRACE_LIB_ACTIVE
		, Trace(InTrace)
#endif
	{}

	Worker_ComponentData Data{};
#if TRACE_LIB_ACTIVE
	TraceKey Trace{ InvalidTraceKey };
#endif
};
struct FWorkerComponentUpdate
{
	FWorkerComponentUpdate() = default;
	FWorkerComponentUpdate(const Worker_ComponentUpdate& InUpdate, TraceKey InTrace = InvalidTraceKey)
		: Update(InUpdate)
#if TRACE_LIB_ACTIVE
		, Trace(InTrace)
#endif
	{}

	Worker_ComponentUpdate Update{};
#if TRACE_LIB_ACTIVE
	TraceKey Trace{ InvalidTraceKey };
#endif
};
