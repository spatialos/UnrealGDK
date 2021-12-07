// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "improbable/c_worker.h"

#define WORKER_SDK_VERSION_FULL_STR "15.4.0-b1125-60bf4dd-WORKER-SNAPSHOT"

constexpr bool StringsEqual(char const* A, char const* B)
{
	return *A == *B && (*A == '\0' || StringsEqual(A + 1, B + 1));
}

// Check if the current version of the Worker SDK is compatible with the current version of UnrealGDK WORKER_SDK_VERSION_FULL_STR
// is incremented here when breaking changes are made that make previous versions of the SDK incompatible
static_assert(StringsEqual(WORKER_API_VERSION_FULL_STR, WORKER_SDK_VERSION_FULL_STR),
			  "Worker SDK version is incompatible with the UnrealGDK version. Check both the Worker SDK and GDK are up to date");
