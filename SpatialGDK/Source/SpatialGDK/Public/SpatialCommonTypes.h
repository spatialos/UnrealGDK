// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "Containers/Array.h"
#include "Containers/Map.h"
#include "HAL/Platform.h"
#include "SpatialView/CommonTypes.h"

struct FUnrealObjectRef;
struct FReliableRPCForRetry;
struct FObjectReferences;
class USpatialActorChannel;

using VirtualWorkerId = uint32;
using PhysicalWorkerName = FString;
using ActorLockToken = int64;
using TraceKey = int32;
constexpr TraceKey InvalidTraceKey{ -1 };

using WorkerAttributeSet = TArray<FString>;
using WorkerRequirementSet = TArray<WorkerAttributeSet>;
using WriteAclMap = TMap<FComponentId, WorkerRequirementSet>;

using FChannelObjectPair = TPair<TWeakObjectPtr<USpatialActorChannel>, TWeakObjectPtr<UObject>>;
using FObjectReferencesMap = TMap<int32, FObjectReferences>;
using FReliableRPCMap = TMap<FRequestId, TSharedRef<FReliableRPCForRetry>>;

using FObjectToRepStateMap = TMap<FUnrealObjectRef, TSet<FChannelObjectPair>>;

template <typename T>
struct FTrackableWorkerType : public T
{
	FTrackableWorkerType() = default;

	FTrackableWorkerType(const T& Update)
		: T(Update)
	{
	}

	FTrackableWorkerType(T&& Update)
		: T(MoveTemp(Update))
	{
	}

#if TRACE_LIB_ACTIVE
	TraceKey Trace{ InvalidTraceKey };
#endif
};

// TODO: These can be removed once event tracing is enabled UNR-3981
using FWorkerComponentUpdate = FTrackableWorkerType<Worker_ComponentUpdate>;
using FWorkerComponentData = FTrackableWorkerType<Worker_ComponentData>;
