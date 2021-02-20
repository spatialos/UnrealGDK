// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "HAL/LowLevelMemTracker.h"

enum class ELLMTagSpatialGDK : LLM_TAG_TYPE
{
	Start = (LLM_TAG_TYPE)ELLMTag::PlatformTagStart + 10,
	Receiver = Start,
	Sender,
	NetDriver,
	RPCService,
	DebugCtx,
	ClientConnectionManager,
	ActorSystem,
	DebuggerSystem,
	Dispatcher,
	CrossServerRPCHandler,
	WellKnownEntitySystem,
	RoutingSystem,
	SpatialMetrics,
	AsyncPackageLoadFilter,


	Count
};

namespace SpatialGDKLLM
{
void Initialise();
}

#define LLM_PLATFORM_SCOPE_SPATIAL(Tag) LLM_PLATFORM_SCOPE((ELLMTag)Tag)
