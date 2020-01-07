// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "SpatialWorkerConnectionInterface.h"

#include "Containers/Queue.h"
#include "HAL/Runnable.h"
#include "HAL/ThreadSafeBool.h"

#include "Interop/Connection/ConnectionConfig.h"
#include "Interop/Connection/OutgoingMessages.h"
#include "SpatialCommonTypes.h"
#include "SpatialGDKSettings.h"
#include "UObject/WeakObjectPtr.h"
#include "Utils/SpatialLatencyTracer.h"

#include <WorkerSDK/improbable/c_schema.h>
#include <WorkerSDK/improbable/c_worker.h>

struct UE4_Op
{
};

struct UE4_DisconnectOp : public UE4_Op
{
	UE4_DisconnectOp(const Worker_DisconnectOp& Op)
		: connection_status_code(Op.connection_status_code)
		, reason(Op.reason)
	{
	}

	/** A value from the UE4_ConnectionStatusCode enumeration. */
	uint8_t connection_status_code;
	/** A string giving detailed information on the reason for disconnecting. */
	const FString reason;
};

struct UE4_FlagUpdateOp : public UE4_Op
{
	UE4_FlagUpdateOp(const Worker_FlagUpdateOp& Op)
		: name(Op.name)
		, value(Op.value)
	{
	}

	/** The name of the updated worker flag. */
	const FString name;
	/**
	 * The new value of the updated worker flag.
	 * A null value indicates that the flag has been deleted.
	 */
	const FString value;
};

struct UE4_LogMessageOp : public UE4_Op
{
	UE4_LogMessageOp(const Worker_LogMessageOp& Op)
		: level(Op.level)
		, message(Op.message)
	{
	}

	/** The severity of the log message; defined in the Worker_LogLevel enumeration. */
	uint8_t level;
	/** The message. */
	const FString message;
};

struct UE4_MetricsOp : public UE4_Op
{
	UE4_MetricsOp(const Worker_MetricsOp& Op)
		: metrics(Op.metrics)
	{
	}

	// TODO(Alex): 
	Worker_Metrics metrics;
};

struct UE4_CriticalSectionOp : public UE4_Op
{
	UE4_CriticalSectionOp(const Worker_CriticalSectionOp& Op)
		: in_critical_section(Op.in_critical_section)
	{
	}

	/** Whether the protocol is entering a critical section (true) or leaving it (false). */
	uint8_t in_critical_section;
};

struct UE4_AddEntityOp : public UE4_Op
{
	UE4_AddEntityOp(const Worker_AddEntityOp& Op)
		: entity_id(Op.entity_id)
	{
	}

	/** The ID of the entity that was added to the worker's view of the simulation. */
	Worker_EntityId entity_id;
};

struct UE4_RemoveEntityOp : public UE4_Op
{
	UE4_RemoveEntityOp(const Worker_RemoveEntityOp& Op)
		: entity_id(Op.entity_id)
	{
	}

	/** The ID of the entity that was removed from the worker's view of the simulation. */
	Worker_EntityId entity_id;
};

struct UE4_ReserveEntityIdsResponseOp : public UE4_Op
{
	UE4_ReserveEntityIdsResponseOp(const Worker_ReserveEntityIdsResponseOp& Op)
		: request_id(Op.request_id)
		, status_code(Op.status_code)
		, message(Op.message)
		, first_entity_id(Op.first_entity_id)
		, number_of_entity_ids(Op.number_of_entity_ids)
	{
	}

	/** The ID of the reserve entity ID request for which there was a response. */
	Worker_RequestId request_id;
	/** Status code of the response, using Worker_StatusCode. */
	uint8_t status_code;
	/** The error message. */
	const FString message;
	/**
	 * If successful, an ID which is the first in a contiguous range of newly allocated entity
	 * IDs which are guaranteed to be unused in the current deployment.
	 */
	Worker_EntityId first_entity_id;
	/** If successful, the number of IDs reserved in the contiguous range, otherwise 0. */
	uint32_t number_of_entity_ids;
};

struct UE4_CreateEntityResponseOp : public UE4_Op
{
	UE4_CreateEntityResponseOp(const Worker_CreateEntityResponseOp& Op)
		: request_id(Op.request_id)
		, status_code(Op.status_code)
		, message(Op.message)
		, entity_id(Op.entity_id)
	{
	}

	/** The ID of the request for which there was a response. */
	Worker_RequestId request_id;
	/** Status code of the response, using Worker_StatusCode. */
	uint8_t status_code;
	/** The error message. */
	const FString message;
	/** If successful, the entity ID of the newly created entity. */
	Worker_EntityId entity_id;
};

struct UE4_DeleteEntityResponseOp : public UE4_Op
{
	UE4_DeleteEntityResponseOp(const Worker_DeleteEntityResponseOp& Op)
		: request_id(Op.request_id)
		, entity_id(Op.entity_id)
		, status_code(Op.status_code)
		, message(Op.message)
	{
	}

	/** The ID of the delete entity request for which there was a command response. */
	Worker_RequestId request_id;
	/** The ID of the target entity of this request. */
	Worker_EntityId entity_id;
	/** Status code of the response, using Worker_StatusCode. */
	uint8_t status_code;
	/** The error message. */
	const FString message;
};

struct UE4_EntityQueryResponseOp : public UE4_Op
{
	UE4_EntityQueryResponseOp(const Worker_EntityQueryResponseOp& Op)
		: request_id(Op.request_id)
		, status_code(Op.status_code)
		, message(Op.message)
		, result_count(Op.result_count)
		// TODO(Alex): 
		, results(nullptr)
	{
	}

	/** The ID of the entity query request for which there was a response. */
	Worker_RequestId request_id;
	/** Status code of the response, using Worker_StatusCode. */
	uint8_t status_code;
	/** The error message. */
	const FString message;
	/**
	 * Number of entities in the result set. Reused to indicate the result itself for CountResultType
	 * queries.
	 */
	uint32_t result_count;
	/**
	 * Array of entities in the result set. Will be NULL if the query was a count query. Snapshot data
	 * in the result is deserialized with the corresponding vtable deserialize function and freed with
	 * the vtable free function when the OpList is destroyed.
	 */
	const Worker_Entity* results;
};

struct UE4_AddComponentOp : public UE4_Op
{
	UE4_AddComponentOp(const Worker_AddComponentOp& Op)
		: entity_id(Op.entity_id)
		, data(Op.data)
	{
	}

	/** The ID of the entity for which a component was added. */
	Worker_EntityId entity_id;
	/**
	 * The initial data for the new component. Deserialized with the corresponding vtable deserialize
	 * function and freed with the vtable free function when the OpList is destroyed.
	 */
	// TODO(Alex): 
	Worker_ComponentData data;
};

struct UE4_RemoveComponentOp : public UE4_Op
{
	UE4_RemoveComponentOp(const Worker_RemoveComponentOp& Op)
		: entity_id(Op.entity_id)
		, component_id(Op.component_id)
	{
	}

	/** The ID of the entity for which a component was removed. */
	Worker_EntityId entity_id;
	/** The ID of the component that was removed. */
	Worker_ComponentId component_id;
};

struct UE4_AuthorityChangeOp : public UE4_Op
{
	UE4_AuthorityChangeOp(const Worker_AuthorityChangeOp& Op)
		: entity_id(Op.entity_id)
		, component_id(Op.component_id)
		, authority(Op.authority)
	{
	}

	/** The ID of the entity for which there was an authority change. */
	Worker_EntityId entity_id;
	/** The ID of the component over which the worker's authority has changed. */
	Worker_ComponentId component_id;
	/** The authority state of the component, using the Worker_Authority enumeration. */
	uint8_t authority;
};

struct UE4_ComponentUpdateOp : public UE4_Op
{
	UE4_ComponentUpdateOp(const Worker_ComponentUpdateOp& Op)
		: entity_id(Op.entity_id)
		, update(Op.update)
	{
	}

	/** The ID of the entity for which there was a component update. */
	Worker_EntityId entity_id;
	/**
	 * The new component data for the updated entity. Deserialized with the corresponding vtable
	 * deserialize function and freed with the vtable free function when the OpList is destroyed.
	 */
	// TODO(Alex): 
	Worker_ComponentUpdate update;
};

struct UE4_CommandRequestOp : public UE4_Op
{
	UE4_CommandRequestOp(const Worker_CommandRequestOp& Op)
		: request_id(Op.request_id)
		, entity_id(Op.entity_id)
		, timeout_millis(Op.timeout_millis)
		, caller_worker_id(Op.caller_worker_id)
		, caller_attribute_set(Op.caller_attribute_set)
		, request(Op.request)
	{
	}

	/** The incoming command request ID. */
	Worker_RequestId request_id;
	/** The ID of the entity for which there was a command request. */
	Worker_EntityId entity_id;
	/** Upper bound on request timeout provided by the platform. */
	uint32_t timeout_millis;
	/** The ID of the worker that sent the request. */
	const FString caller_worker_id;
	/** The attributes of the worker that sent the request. */
	// TODO(Alex): 
	Worker_WorkerAttributes caller_attribute_set;
	/**
	 * The command request data. Deserialized with the corresponding vtable deserialize function and
	 * freed with the vtable free function when the OpList is destroyed.
	 */
	// TODO(Alex): 
	Worker_CommandRequest request;
};

struct UE4_CommandResponseOp : public UE4_Op
{
	UE4_CommandResponseOp(const Worker_CommandResponseOp& Op)
		: request_id(Op.request_id)
		, entity_id(Op.entity_id)
		, status_code(Op.status_code)
		, message(Op.message)
		, response(Op.response)
	{
	}

	/** The ID of the command request for which there was a command response. */
	Worker_RequestId request_id;
	/** The ID of the entity originally targeted by the command request. */
	Worker_EntityId entity_id;
	/** Status code of the response, using Worker_StatusCode. */
	uint8_t status_code;
	/** The error message. */
	const FString message;
	/**
	 * The command response data. Deserialized with the corresponding vtable deserialize function and
	 * freed with the vtable free function when the OpList is destroyed.
	 */
	// TODO(Alex): safe without deep copy?
	Worker_CommandResponse response;
};

struct UE4_OpLists
{
	struct UE4_OpAndType
	{
		UE4_Op* Op;
		uint8_t op_type;
	};

	using UE4_OpList = TArray<UE4_OpAndType>;
	TArray<UE4_OpList> OpLists;

	void SerializedOpList(const Worker_OpList* OpList);
	void DumpSavedOpLists();
	TArray<Worker_OpList*> LoadSavedOpLists();
};
