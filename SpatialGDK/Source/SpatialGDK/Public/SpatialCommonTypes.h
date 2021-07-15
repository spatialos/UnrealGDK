// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "Containers/Array.h"
#include "Containers/Map.h"
#include "HAL/Platform.h"
#include "Misc/AssertionMacros.h"
#include <WorkerSDK/improbable/c_worker.h>

// IMPORTANT: This is required for Linux builds to succeed - don't remove!
// Worker_EntityId from the Worker SDK resolves to a long on Linux.
// These are not a type of key supported by TMap.
using Worker_EntityId_Key = int64;
using Worker_RequestId_Key = int64;

using Worker_PartitionId = Worker_EntityId_Key;
using Trace_SpanIdType = uint8_t;

using VirtualWorkerId = uint32;
using PhysicalWorkerName = FString;
using ActorLockToken = int64;
using TraceKey = int32;
constexpr TraceKey InvalidTraceKey{ -1 };

using FChannelObjectPair = TPair<TWeakObjectPtr<class USpatialActorChannel>, TWeakObjectPtr<UObject>>;
using FObjectReferencesMap = TMap<int32, struct FObjectReferences>;
using FReliableRPCMap = TMap<Worker_RequestId_Key, TSharedRef<struct FReliableRPCForRetry>>;

using FObjectToRepStateMap = TMap<struct FUnrealObjectRef, TSet<FChannelObjectPair>>;

using AuthorityDelegationMap = TMap<Worker_ComponentSetId, Worker_PartitionId>;

template <typename ElementType, bool bInAllowDuplicateKeys /*= false*/>
struct TWeakObjectPtrKeyFuncs : DefaultKeyFuncs<ElementType, bInAllowDuplicateKeys>
{
	using typename DefaultKeyFuncs<ElementType, bInAllowDuplicateKeys>::KeyInitType;
	using typename DefaultKeyFuncs<ElementType, bInAllowDuplicateKeys>::ElementInitType;
	/**
	 * @return True if the keys match.
	 */
	static FORCEINLINE bool Matches(KeyInitType A, KeyInitType B) { return A.HasSameIndexAndSerialNumber(B); }
};

// TODO: These can be removed once event tracing is enabled UNR-3981
using FWorkerComponentUpdate = Worker_ComponentUpdate;
using FWorkerComponentData = Worker_ComponentData;

/* A little helper to ensure no re-entrancy of modifications of data structures. */
struct FUsageLock
{
	bool bIsSet = false;

	struct Scope
	{
		bool& bIsSetRef;
		Scope(bool& bInIsSetRef)
			: bIsSetRef(bInIsSetRef)
		{
			ensureMsgf(!bIsSetRef, TEXT("Unexpected re-entrancy occured in the Spatial GDK."));
			bIsSetRef = true;
		}
		~Scope() { bIsSetRef = false; }
	};
};

// A macro to prevent re-entrant calls
#if DO_CHECK
#define __GDK_ENSURE_NO_MODIFICATIONS(x, y) x##y
#define _GDK_ENSURE_NO_MODIFICATIONS(x, y) __GDK_ENSURE_NO_MODIFICATIONS(x, y)
#define GDK_ENSURE_NO_MODIFICATIONS(FLAG) FUsageLock::Scope _GDK_ENSURE_NO_MODIFICATIONS(ScopedUsageCheck, __LINE__)(FLAG.bIsSet);
#else
#define GDK_ENSURE_NO_MODIFICATIONS(FLAG)
#endif
