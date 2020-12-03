// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "SpatialView/OpList/OpList.h"

#include "CoreMinimal.h"

#include <WorkerSDK/improbable/c_worker.h>

namespace SpatialGDK
{
Worker_ComponentId GetComponentId(const Worker_Op& Op);

template <typename T>
constexpr uint8 GetWorkerOpType();

template <>
constexpr uint8 GetWorkerOpType<Worker_DisconnectOp>()
{
	return WORKER_OP_TYPE_DISCONNECT;
}

template <>
constexpr uint8 GetWorkerOpType<Worker_FlagUpdateOp>()
{
	return WORKER_OP_TYPE_FLAG_UPDATE;
}

template <>
constexpr uint8 GetWorkerOpType<Worker_MetricsOp>()
{
	return WORKER_OP_TYPE_METRICS;
}

template <>
constexpr uint8 GetWorkerOpType<Worker_CriticalSectionOp>()
{
	return WORKER_OP_TYPE_CRITICAL_SECTION;
}

template <>
constexpr uint8 GetWorkerOpType<Worker_AddEntityOp>()
{
	return WORKER_OP_TYPE_ADD_ENTITY;
}

template <>
constexpr uint8 GetWorkerOpType<Worker_RemoveEntityOp>()
{
	return WORKER_OP_TYPE_REMOVE_ENTITY;
}

template <>
constexpr uint8 GetWorkerOpType<Worker_ReserveEntityIdsResponseOp>()
{
	return WORKER_OP_TYPE_RESERVE_ENTITY_IDS_RESPONSE;
}

template <>
constexpr uint8 GetWorkerOpType<Worker_CreateEntityResponseOp>()
{
	return WORKER_OP_TYPE_CREATE_ENTITY_RESPONSE;
}

template <>
constexpr uint8 GetWorkerOpType<Worker_DeleteEntityResponseOp>()
{
	return WORKER_OP_TYPE_DELETE_ENTITY_RESPONSE;
}

template <>
constexpr uint8 GetWorkerOpType<Worker_EntityQueryResponseOp>()
{
	return WORKER_OP_TYPE_ENTITY_QUERY_RESPONSE;
}

template <>
constexpr uint8 GetWorkerOpType<Worker_AddComponentOp>()
{
	return WORKER_OP_TYPE_ADD_COMPONENT;
}

template <>
constexpr uint8 GetWorkerOpType<Worker_RemoveComponentOp>()
{
	return WORKER_OP_TYPE_REMOVE_COMPONENT;
}

template <>
constexpr uint8 GetWorkerOpType<Worker_ComponentUpdateOp>()
{
	return WORKER_OP_TYPE_COMPONENT_UPDATE;
}

template <>
constexpr uint8 GetWorkerOpType<Worker_CommandRequestOp>()
{
	return WORKER_OP_TYPE_COMMAND_REQUEST;
}

template <>
constexpr uint8 GetWorkerOpType<Worker_CommandResponseOp>()
{
	return WORKER_OP_TYPE_COMMAND_RESPONSE;
}

template <>
constexpr uint8 GetWorkerOpType<Worker_ComponentSetAuthorityChangeOp>()
{
	return WORKER_OP_TYPE_COMPONENT_SET_AUTHORITY_CHANGE;
}

template <typename T>
Worker_Op CreateWorkerOp(const T& OpData)
{
	Worker_Op Op = {};
	Op.op_type = GetWorkerOpType<T>();
	reinterpret_cast<T&>(Op.op) = OpData;

	return Op;
}

} // namespace SpatialGDK
