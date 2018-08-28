// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreTypes/UnrealObjectRef.h"

namespace SpatialConstants
{
	enum EntityIds
	{
		SPAWNER_ENTITY_ID = 1,
		GLOBAL_STATE_MANAGER = 2,
		PLACEHOLDER_ENTITY_ID_FIRST = 3,
		PLACEHOLDER_ENTITY_ID_LAST = PLACEHOLDER_ENTITY_ID_FIRST + 35, // 36 placeholder entities.

		PLAYER_SPAWNER_COMPONENT_ID = 100002,
	};

	const float FIRST_COMMAND_RETRY_WAIT_SECONDS = 0.2f;
	const float REPLICATED_STABLY_NAMED_ACTORS_DELETION_TIMEOUT_SECONDS = 5.0f;
	const uint32 MAX_NUMBER_COMMAND_ATTEMPTS = 5u;

	const UnrealObjectRef NULL_OBJECT_REF(0, 0);
	const UnrealObjectRef UNRESOLVED_OBJECT_REF(0, 1);

	inline float GetCommandRetryWaitTimeSeconds(uint32 NumAttempts)
	{
		// Double the time to wait on each failure.
		uint32 WaitTimeExponentialFactor = 1u << (NumAttempts - 1);
		return FIRST_COMMAND_RETRY_WAIT_SECONDS * WaitTimeExponentialFactor;
	}
}
