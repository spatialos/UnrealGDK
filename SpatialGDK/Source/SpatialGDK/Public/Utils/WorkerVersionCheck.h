// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "improbable/c_worker.h"

#define WORKER_SDK_VERSION "14.8.0"

constexpr bool StringsEqual(char const* A, char const* B)
{
	return *A == *B && (*A == '\0' || StringsEqual(A + 1, B + 1));
}

// Check if the Worker SDK is compatible with the current version of Unreal Engine
// WORKER_SDK_VERSION is incremented in engine when breaking changes
// are made that make previous versions of the SDK incompatible
static_assert(StringsEqual(WORKER_API_VERSION_STR, WORKER_SDK_VERSION),
			  "Worker SDK Version is incompatible with the Engine Version. Check both the Worker SDK and Engine are up to date");
