// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "SpatialGDKLLM.h"
#include "HAL/LowLevelMemStats.h"

struct FLLMTagInfoSpatial
{
	const TCHAR* Name;
	FName StatName;		   // shows in the LLMFULL stat group
	FName SummaryStatName; // shows in the LLM summary stat group
};

DECLARE_LLM_MEMORY_STAT(TEXT("SpatialReceiver"), STAT_SpatialReceiverLLM, STATGROUP_LLMPlatform);
DECLARE_LLM_MEMORY_STAT(TEXT("SpatialSender"), STAT_SpatialSenderLLM, STATGROUP_LLMPlatform);
DECLARE_LLM_MEMORY_STAT(TEXT("SpatialNetDriver"), STAT_SpatialNetDriverLLM, STATGROUP_LLMPlatform);
DECLARE_LLM_MEMORY_STAT(TEXT("SpatialRPCService"), STAT_SpatialRPCServiceLLM, STATGROUP_LLMPlatform);
DECLARE_LLM_MEMORY_STAT(TEXT("SpatialDebugCtx"), STAT_SpatialDebugCtxLLM, STATGROUP_LLMPlatform);
DECLARE_LLM_MEMORY_STAT(TEXT("SpatialClientConnectionManager"), STAT_SpatialClientConnectionManagerLLM, STATGROUP_LLMPlatform);
DECLARE_LLM_MEMORY_STAT(TEXT("SpatialActorSystem"), STAT_SpatialActorSystemLLM, STATGROUP_LLMPlatform);
DECLARE_LLM_MEMORY_STAT(TEXT("SpatialDebuggerSystem"), STAT_SpatialDebuggerSystemLLM, STATGROUP_LLMPlatform);
DECLARE_LLM_MEMORY_STAT(TEXT("SpatialDispatcher"), STAT_SpatialDispatcherLLM, STATGROUP_LLMPlatform);
DECLARE_LLM_MEMORY_STAT(TEXT("SpatialCrossServerRPCHandler"), STAT_SpatialCrossServerRPCHandlerLLM, STATGROUP_LLMPlatform);
DECLARE_LLM_MEMORY_STAT(TEXT("SpatialWellKnownEntitySystem"), STAT_SpatialWellKnownEntitySystemLLM, STATGROUP_LLMPlatform);
DECLARE_LLM_MEMORY_STAT(TEXT("SpatialRoutingSystem"), STAT_SpatialRoutingSystemLLM, STATGROUP_LLMPlatform);
DECLARE_LLM_MEMORY_STAT(TEXT("SpatialSpatialMetrics"), STAT_SpatialSpatialMetricsLLM, STATGROUP_LLMPlatform);
DECLARE_LLM_MEMORY_STAT(TEXT("SpatialAsyncPackageLoadFilter"), STAT_SpatialAsyncPackageLoadFilterLLM, STATGROUP_LLMPlatform);

// *** order must match ELLMTagSpatialGDK enum ***
const FLLMTagInfoSpatial ELLMTagNamesSpatial[] = {
	// csv name						// stat name						// summary stat name
	{ TEXT("SpatialReceiver"), GET_STATFNAME(STAT_SpatialReceiverLLM), GET_STATFNAME(STAT_EngineSummaryLLM) },
	{ TEXT("SpatialSender"), GET_STATFNAME(STAT_SpatialSenderLLM), GET_STATFNAME(STAT_EngineSummaryLLM) },
	{ TEXT("SpatialNetDriver"), GET_STATFNAME(STAT_SpatialNetDriverLLM), GET_STATFNAME(STAT_EngineSummaryLLM) },
	{ TEXT("SpatialRPCService"), GET_STATFNAME(STAT_SpatialRPCServiceLLM), GET_STATFNAME(STAT_EngineSummaryLLM) },
	{ TEXT("SpatialDebugCtx"), GET_STATFNAME(STAT_SpatialDebugCtxLLM), GET_STATFNAME(STAT_EngineSummaryLLM) },
	{ TEXT("SpatialClientConnectionManager"), GET_STATFNAME(STAT_SpatialClientConnectionManagerLLM),
	  GET_STATFNAME(STAT_EngineSummaryLLM) },
	{ TEXT("SpatialActorSystem"), GET_STATFNAME(STAT_SpatialActorSystemLLM), GET_STATFNAME(STAT_EngineSummaryLLM) },
	{ TEXT("SpatialDebuggerSystem"), GET_STATFNAME(STAT_SpatialDebuggerSystemLLM), GET_STATFNAME(STAT_EngineSummaryLLM) },
	{ TEXT("SpatialDispatcher"), GET_STATFNAME(STAT_SpatialDispatcherLLM), GET_STATFNAME(STAT_EngineSummaryLLM) },
	{ TEXT("SpatialCrossServerRPCHandler"), GET_STATFNAME(STAT_SpatialCrossServerRPCHandlerLLM), GET_STATFNAME(STAT_EngineSummaryLLM) },
	{ TEXT("SpatialWellKnownEntitySystem"), GET_STATFNAME(STAT_SpatialWellKnownEntitySystemLLM), GET_STATFNAME(STAT_EngineSummaryLLM) },
	{ TEXT("SpatialRoutingSystem"), GET_STATFNAME(STAT_SpatialRoutingSystemLLM), GET_STATFNAME(STAT_EngineSummaryLLM) },
	{ TEXT("SpatialSpatialMetrics"), GET_STATFNAME(STAT_SpatialSpatialMetricsLLM), GET_STATFNAME(STAT_EngineSummaryLLM) },
	{ TEXT("SpatialAsyncPackageLoadFilter"), GET_STATFNAME(STAT_SpatialAsyncPackageLoadFilterLLM), GET_STATFNAME(STAT_EngineSummaryLLM) },
};

void SpatialGDKLLM::Initialise()
{
	int32 TagCount = sizeof(ELLMTagNamesSpatial) / sizeof(FLLMTagInfoSpatial);

	for (int32 Index = 0; Index < TagCount; ++Index)
	{
		int32 Tag = (int32)ELLMTagSpatialGDK::Start + Index;
		const FLLMTagInfoSpatial& TagInfo = ELLMTagNamesSpatial[Index];

		FLowLevelMemTracker::Get().RegisterPlatformTag(Tag, TagInfo.Name, TagInfo.StatName, TagInfo.SummaryStatName);
	}
}
