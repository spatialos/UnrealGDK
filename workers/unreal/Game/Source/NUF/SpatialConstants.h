#pragma once

// Needed for UnrealObjectRef.
#include <unreal/core_types.h>

namespace SpatialConstants
{
	const int SPAWNER_ENTITY_ID = 1;
	const int LEVEL_DATA_ENTITY_ID = 2;
	const float FIRST_COMMAND_RETRY_WAIT_SECONDS = 0.2f;
	const uint32 MAX_NUMBER_COMMAND_ATTEMPTS = 5u;

	const improbable::unreal::UnrealObjectRef NULL_OBJECT_REF = {0, 0};
	const improbable::unreal::UnrealObjectRef UNRESOLVED_OBJECT_REF = {0, 1};

	inline float GetCommandRetryWaitTimeSeconds(uint32 NumAttempts)
	{
		// Double the time to wait on each failure.
		uint32 WaitTimeExponentialFactor = 1u << (NumAttempts - 1);
		return FIRST_COMMAND_RETRY_WAIT_SECONDS * WaitTimeExponentialFactor;
	}
}
