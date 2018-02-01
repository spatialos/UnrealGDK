#pragma once

namespace SpatialConstants
{
	const int SPAWNER_ENTITY_ID = 1;
	const int PACKAGE_MAP_ENTITY_ID = 2;
	const float FIRST_COMMAND_RETRY_WAIT_SECONDS = 0.2f;
	const uint32 MAX_NUMBER_COMMAND_ATTEMPTS = 5u;

	inline float GetCommandRetryWaitTimeSeconds(uint32 NumFailures)
	{
		// Double the time to wait on each failure.
		uint32 WaitTimeExponentialFactor = 1u << NumFailures;
		return FIRST_COMMAND_RETRY_WAIT_SECONDS * WaitTimeExponentialFactor;
	}
}
