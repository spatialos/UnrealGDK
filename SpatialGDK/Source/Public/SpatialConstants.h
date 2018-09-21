// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "UnrealObjectRefStub.h"

#include <improbable/c_worker.h>

using Worker_EntityId_Key = int64;

namespace SpatialConstants
{
	enum EntityIds
	{
		INVALID_ENTITY_ID = 0,
		SPAWNER_ENTITY_ID = 1,
		GLOBAL_STATE_MANAGER = 2,
		PLACEHOLDER_ENTITY_ID_FIRST = 3,
		PLACEHOLDER_ENTITY_ID_LAST = PLACEHOLDER_ENTITY_ID_FIRST + 35, // 36 placeholder entities.
	};

	const Worker_ComponentId ENTITY_ACL_COMPONENT_ID			= 50;
	const Worker_ComponentId METADATA_COMPONENT_ID				= 53;
	const Worker_ComponentId POSITION_COMPONENT_ID				= 54;
	const Worker_ComponentId PERSISTENCE_COMPONENT_ID			= 55;

	const Worker_ComponentId ROTATION_COMPONENT_ID				= 100001;
	const Worker_ComponentId PLAYER_SPAWNER_COMPONENT_ID		= 100002;
	const Worker_ComponentId UNREAL_METADATA_COMPONENT_ID		= 100004;
	const Worker_ComponentId GLOBAL_STATE_MANAGER_COMPONENT_ID	= 100007;
	const Worker_ComponentId STARTING_GENERATED_COMPONENT_ID	= 100010;

	const float FIRST_COMMAND_RETRY_WAIT_SECONDS = 0.2f;
	const float REPLICATED_STABLY_NAMED_ACTORS_DELETION_TIMEOUT_SECONDS = 5.0f;
	const uint32 MAX_NUMBER_COMMAND_ATTEMPTS = 5u;

	const FUnrealObjectRef NULL_OBJECT_REF(0, 0);
	const FUnrealObjectRef UNRESOLVED_OBJECT_REF(0, 1);

	static const FString ClientWorkerType = TEXT("UnrealClient");
	static const FString ServerWorkerType = TEXT("UnrealWorker");

	inline float GetCommandRetryWaitTimeSeconds(uint32 NumAttempts)
	{
		// Double the time to wait on each failure.
		uint32 WaitTimeExponentialFactor = 1u << (NumAttempts - 1);
		return FIRST_COMMAND_RETRY_WAIT_SECONDS * WaitTimeExponentialFactor;
	}

	const FString LOCAL_HOST = TEXT("127.0.0.1");
	const uint16 DEFAULT_PORT = 7777;
}
