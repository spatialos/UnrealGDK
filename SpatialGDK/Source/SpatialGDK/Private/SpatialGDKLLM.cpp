// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "SpatialGDKLLM.h"
#include "HAL/LowLevelMemStats.h"

struct FLLMTagInfoSpatial
{
	const TCHAR* Name;
	FName StatName;		   // shows in the LLMFULL stat group
	FName SummaryStatName; // shows in the LLM summary stat group
};

DECLARE_LLM_MEMORY_STAT(TEXT("Spatial Receiver"), STAT_ReceiverLLM, STATGROUP_LLMPlatform);
DECLARE_LLM_MEMORY_STAT(TEXT("Spatial Sender"), STAT_SenderLLM, STATGROUP_LLMPlatform);
DECLARE_LLM_MEMORY_STAT(TEXT("Spatial NetDriver"), STAT_NetDriverLLM, STATGROUP_LLMPlatform);
DECLARE_LLM_MEMORY_STAT(TEXT("Spatial RPCService"), STAT_RPCServiceLLM, STATGROUP_LLMPlatform);

// *** order must match ELLMTagSpatialGDK enum ***
const FLLMTagInfoSpatial ELLMTagNamesSpatial[] = {
	// csv name						// stat name						// summary stat name
	{ TEXT("Spatial Receiver"), GET_STATFNAME(STAT_ReceiverLLM), GET_STATFNAME(STAT_EngineSummaryLLM) },
	{ TEXT("Spatial Sender"), GET_STATFNAME(STAT_SenderLLM), GET_STATFNAME(STAT_EngineSummaryLLM) },
	{ TEXT("Spatial NetDriver"), GET_STATFNAME(STAT_NetDriverLLM), GET_STATFNAME(STAT_EngineSummaryLLM) },
	{ TEXT("Spatial RPCService"), GET_STATFNAME(STAT_RPCServiceLLM), GET_STATFNAME(STAT_EngineSummaryLLM) },
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
