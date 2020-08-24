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
constexpr TraceKey InvalidTraceKey{ -1 };

using WorkerAttributeSet = TArray<FString>;
using WorkerRequirementSet = TArray<WorkerAttributeSet>;
using WriteAclMap = TMap<Worker_ComponentId, WorkerRequirementSet>;

using FChannelObjectPair = TPair<TWeakObjectPtr<class USpatialActorChannel>, TWeakObjectPtr<UObject>>;
using FObjectReferencesMap = TMap<int32, struct FObjectReferences>;
using FReliableRPCMap = TMap<Worker_RequestId_Key, TSharedRef<struct FReliableRPCForRetry>>;

using FObjectToRepStateMap = TMap<struct FUnrealObjectRef, TSet<FChannelObjectPair>>;

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

using FWorkerComponentUpdate = FTrackableWorkerType<Worker_ComponentUpdate>;
using FWorkerComponentData = FTrackableWorkerType<Worker_ComponentData>;
