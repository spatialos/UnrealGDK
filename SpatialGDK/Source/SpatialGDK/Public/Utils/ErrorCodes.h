// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "Engine/EngineBaseTypes.h"
#include <WorkerSDK/improbable/c_schema.h>

typedef TOptional<ENetworkFailure::Type> FRemappedNetworkFailure;

namespace ENetworkFailure
{
	static inline FRemappedNetworkFailure FromWorkerStatusCode(uint8_t StatusCode)
	{
		// For full status code descriptions, see WorkerSDK\improbable\c_worker.h
		switch (StatusCode)
		{
			case Worker_ConnectionStatusCode::WORKER_CONNECTION_STATUS_CODE_TIMEOUT:
				return FRemappedNetworkFailure(ENetworkFailure::ConnectionTimeout);
			case Worker_ConnectionStatusCode::WORKER_CONNECTION_STATUS_CODE_INTERNAL_ERROR:
			case Worker_ConnectionStatusCode::WORKER_CONNECTION_STATUS_CODE_NETWORK_ERROR:
			case Worker_ConnectionStatusCode::WORKER_CONNECTION_STATUS_CODE_SERVER_SHUTDOWN:
				return FRemappedNetworkFailure(ENetworkFailure::ConnectionLost);
			default:
				return FRemappedNetworkFailure();
		}
	}
}

//
///** The remote call was successful, or we are successfully connected. */
//WORKER_CONNECTION_STATUS_CODE_SUCCESS = 1,
///**
// * Protocol violation, or some part of the system otherwise behaved in an unexpected way. Not
// * expected to occur in normal operation.
// */
//	WORKER_CONNECTION_STATUS_CODE_INTERNAL_ERROR = 2,
//	/**
//	 * An argument provided by the caller was determined to be invalid. This is a local failure; no
//	 * actual attempt was made to contact the host. Not retryable.
//	 */
//	WORKER_CONNECTION_STATUS_CODE_INVALID_ARGUMENT = 3,
//	/** Failed due to a networking issue or otherwise unreachable host. */
//	WORKER_CONNECTION_STATUS_CODE_NETWORK_ERROR = 4,
//	/** A timeout provided by the caller or enforced by the system was exceeded. Can be retried. */
//	WORKER_CONNECTION_STATUS_CODE_TIMEOUT = 5,
//	/** Attempt was cancelled by the caller. Currently shouldn't happen; reserved for future use. */
//	WORKER_CONNECTION_STATUS_CODE_CANCELLED = 6,
//	/**
//	 * Made contact with the host, but the request was explicitly rejected. Unlikely to be retryable.
//	 * Possible causes include: the request was made to the wrong host; the host considered the
//	 * request invalid for some othe reason.
//	 */
//	WORKER_CONNECTION_STATUS_CODE_REJECTED = 7,
//	/** The player identity token provided by the caller has expired. Generate a new one and retry. */
//	WORKER_CONNECTION_STATUS_CODE_PLAYER_IDENTITY_TOKEN_EXPIRED = 8,
//	/** The login token provided by the caller has expired. Generate a new one and retry. */
//	WORKER_CONNECTION_STATUS_CODE_LOGIN_TOKEN_EXPIRED = 9,
//	/**
//	 * Failed because the deployment associated with the provided login token was at capacity.
//	 * Retryable.
//	 */
//	WORKER_CONNECTION_STATUS_CODE_CAPACITY_EXCEEDED = 10,
//	/**
//	 * Failed due to rate-limiting of new connections to the deployment associated with the provided
//	 * login token. Retryable.
//	 */
//	WORKER_CONNECTION_STATUS_CODE_RATE_EXCEEDED = 11,
//	/**
//	 * After a successful connection attempt, the server later explicitly terminated the connection.
//	 * Possible causes include: the deployment was stopped; the worker was killed due to
//	 * unresponsiveness.
//	 */
//	WORKER_CONNECTION_STATUS_CODE_SERVER_SHUTDOWN = 12,
