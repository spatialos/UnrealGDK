// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "SpatialGDKLLM.h"
#include "HAL/LowLevelMemStats.h"

struct FLLMTagInfoSpatial
{
	const TCHAR* Name;
	FName StatName;		   // shows in the LLMFULL stat group
	FName SummaryStatName; // shows in the LLM summary stat group
};

DECLARE_LLM_MEMORY_STAT(TEXT("Spatial Receiver"), STAT_SpatialReceiverLLM, STATGROUP_LLMPlatform);
DECLARE_LLM_MEMORY_STAT(TEXT("Spatial Sender"), STAT_SpatialSenderLLM, STATGROUP_LLMPlatform);
DECLARE_LLM_MEMORY_STAT(TEXT("Spatial NetDriver"), STAT_SpatialNetDriverLLM, STATGROUP_LLMPlatform);
DECLARE_LLM_MEMORY_STAT(TEXT("Spatial RPCService"), STAT_SpatialRPCServiceLLM, STATGROUP_LLMPlatform);
DECLARE_LLM_MEMORY_STAT(TEXT("Spatial DebugCtx"), STAT_SpatialDebugCtxLLM, STATGROUP_LLMPlatform);
DECLARE_LLM_MEMORY_STAT(TEXT("Spatial ClientConnectionManager"), STAT_SpatialClientConnectionManagerLLM, STATGROUP_LLMPlatform);
DECLARE_LLM_MEMORY_STAT(TEXT("Spatial ActorSystem"), STAT_SpatialActorSystemLLM, STATGROUP_LLMPlatform);
DECLARE_LLM_MEMORY_STAT(TEXT("Spatial DebuggerSystem"), STAT_SpatialDebuggerSystemLLM, STATGROUP_LLMPlatform);
DECLARE_LLM_MEMORY_STAT(TEXT("Spatial Dispatcher"), STAT_SpatialDispatcherLLM, STATGROUP_LLMPlatform);
DECLARE_LLM_MEMORY_STAT(TEXT("Spatial CrossServerRPCHandler"), STAT_SpatialCrossServerRPCHandlerLLM, STATGROUP_LLMPlatform);
DECLARE_LLM_MEMORY_STAT(TEXT("Spatial WellKnownEntitySystem"), STAT_SpatialWellKnownEntitySystemLLM, STATGROUP_LLMPlatform);
DECLARE_LLM_MEMORY_STAT(TEXT("Spatial RoutingSystem"), STAT_SpatialRoutingSystemLLM, STATGROUP_LLMPlatform);
DECLARE_LLM_MEMORY_STAT(TEXT("Spatial SpatialMetrics"), STAT_SpatialSpatialMetricsLLM, STATGROUP_LLMPlatform);
DECLARE_LLM_MEMORY_STAT(TEXT("Spatial AsyncPackageLoadFilter"), STAT_SpatialAsyncPackageLoadFilterLLM, STATGROUP_LLMPlatform);

// *** order must match ELLMTagSpatialGDK enum ***
const FLLMTagInfoSpatial ELLMTagNamesSpatial[] = {
	// csv name						// stat name						// summary stat name
	{ TEXT("Spatial Receiver"), GET_STATFNAME(STAT_SpatialReceiverLLM), GET_STATFNAME(STAT_EngineSummaryLLM) },
	{ TEXT("Spatial Sender"), GET_STATFNAME(STAT_SpatialSenderLLM), GET_STATFNAME(STAT_EngineSummaryLLM) },
	{ TEXT("Spatial NetDriver"), GET_STATFNAME(STAT_SpatialNetDriverLLM), GET_STATFNAME(STAT_EngineSummaryLLM) },
	{ TEXT("Spatial RPCService"), GET_STATFNAME(STAT_SpatialRPCServiceLLM), GET_STATFNAME(STAT_EngineSummaryLLM) },
	{ TEXT("Spatial DebugCtx"), GET_STATFNAME(STAT_SpatialDebugCtxLLM), GET_STATFNAME(STAT_EngineSummaryLLM) },
	{ TEXT("Spatial ClientConnectionManager"), GET_STATFNAME(STAT_SpatialClientConnectionManagerLLM),
	  GET_STATFNAME(STAT_EngineSummaryLLM) },
	{ TEXT("Spatial ActorSystem"), GET_STATFNAME(STAT_SpatialActorSystemLLM), GET_STATFNAME(STAT_EngineSummaryLLM) },
	{ TEXT("Spatial DebuggerSystem"), GET_STATFNAME(STAT_SpatialDebuggerSystemLLM), GET_STATFNAME(STAT_EngineSummaryLLM) },
	{ TEXT("Spatial Dispatcher"), GET_STATFNAME(STAT_SpatialDispatcherLLM), GET_STATFNAME(STAT_EngineSummaryLLM) },
	{ TEXT("Spatial CrossServerRPCHandler"), GET_STATFNAME(STAT_SpatialCrossServerRPCHandlerLLM), GET_STATFNAME(STAT_EngineSummaryLLM) },
	{ TEXT("Spatial WellKnownEntitySystem"), GET_STATFNAME(STAT_SpatialWellKnownEntitySystemLLM), GET_STATFNAME(STAT_EngineSummaryLLM) },
	{ TEXT("Spatial RoutingSystem"), GET_STATFNAME(STAT_SpatialRoutingSystemLLM), GET_STATFNAME(STAT_EngineSummaryLLM) },
	{ TEXT("Spatial SpatialMetrics"), GET_STATFNAME(STAT_SpatialSpatialMetricsLLM), GET_STATFNAME(STAT_EngineSummaryLLM) },
	{ TEXT("Spatial AsyncPackageLoadFilter"), GET_STATFNAME(STAT_SpatialAsyncPackageLoadFilterLLM), GET_STATFNAME(STAT_EngineSummaryLLM) },
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
