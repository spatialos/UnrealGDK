// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

// Needed for improbable::Coordinates
#include <improbable/standard_library.h>

// Needed for UnrealObjectRef.
#include <improbable/unreal/gdk/core_types.h>

namespace SpatialConstants
{
	enum EntityIds
	{
		SPAWNER_ENTITY_ID = 1,
		GLOBAL_STATE_MANAGER = 2,
		PLACEHOLDER_ENTITY_ID_FIRST = 3,
		PLACEHOLDER_ENTITY_ID_LAST = PLACEHOLDER_ENTITY_ID_FIRST + 35, // 36 placeholder entities.
	};

	const float FIRST_COMMAND_RETRY_WAIT_SECONDS = 0.2f;
	const float REPLICATED_STABLY_NAMED_ACTORS_DELETION_TIMEOUT_SECONDS = 15.0f;
	const uint32 MAX_NUMBER_COMMAND_ATTEMPTS = 5u;

	const improbable::unreal::UnrealObjectRef NULL_OBJECT_REF = { 0, 0, {}, {} };
	const improbable::unreal::UnrealObjectRef UNRESOLVED_OBJECT_REF = { 0, 1, {}, {} };

	inline float GetCommandRetryWaitTimeSeconds(uint32 NumAttempts)
	{
		// Double the time to wait on each failure.
		uint32 WaitTimeExponentialFactor = 1u << (NumAttempts - 1);
		return FIRST_COMMAND_RETRY_WAIT_SECONDS * WaitTimeExponentialFactor;
	}

	FORCEINLINE improbable::Coordinates LocationToSpatialOSCoordinates(const FVector& Location)
	{
		return {Location.Y * 0.01f, Location.Z * 0.01f, Location.X * 0.01f};
	}

	FORCEINLINE FVector SpatialOSCoordinatesToLocation(const improbable::Coordinates& Coords)
	{
		return {float(Coords.z() * 100.0), float(Coords.x() * 100.0), float(Coords.y() * 100.0f)};
	}
}
