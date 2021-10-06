// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "Engine/EngineBaseTypes.h"
#include <WorkerSDK/improbable/c_schema.h>

namespace ENetworkFailure
{
static inline ENetworkFailure::Type FromDisconnectOpStatusCode(uint8_t StatusCode)
{
	// For full status code descriptions, see WorkerSDK\improbable\c_worker.h
	switch (StatusCode)
	{
	case Worker_ConnectionStatusCode::WORKER_CONNECTION_STATUS_CODE_TIMEOUT:
		return ENetworkFailure::ConnectionTimeout;

	case Worker_ConnectionStatusCode::WORKER_CONNECTION_STATUS_CODE_INTERNAL_ERROR:
	case Worker_ConnectionStatusCode::WORKER_CONNECTION_STATUS_CODE_NETWORK_ERROR:
	case Worker_ConnectionStatusCode::WORKER_CONNECTION_STATUS_CODE_SERVER_SHUTDOWN:
		return ENetworkFailure::ConnectionLost;

	case Worker_ConnectionStatusCode::WORKER_CONNECTION_STATUS_CODE_INVALID_ARGUMENT:
	case Worker_ConnectionStatusCode::WORKER_CONNECTION_STATUS_CODE_CANCELLED:
	case Worker_ConnectionStatusCode::WORKER_CONNECTION_STATUS_CODE_REJECTED:
	case Worker_ConnectionStatusCode::WORKER_CONNECTION_STATUS_CODE_PLAYER_IDENTITY_TOKEN_EXPIRED:
	case Worker_ConnectionStatusCode::WORKER_CONNECTION_STATUS_CODE_LOGIN_TOKEN_EXPIRED:
	case Worker_ConnectionStatusCode::WORKER_CONNECTION_STATUS_CODE_CAPACITY_EXCEEDED:
	case Worker_ConnectionStatusCode::WORKER_CONNECTION_STATUS_CODE_RATE_EXCEEDED:
		return ENetworkFailure::PendingConnectionFailure;

	default:
		// Execution of this code path should be considered an error as all worker status codes map to an ENetworkFailure
		// The only exception to this is WORKER_CONNECTION_STATUS_CODE_SUCCESS which does not indicate an error and will never
		// be received in a WORKER_OP_TYPE_DISCONNECT
		checkNoEntry();
		return ENetworkFailure::FailureReceived;
	}
}
} // namespace ENetworkFailure
