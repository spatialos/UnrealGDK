/* Copyright (c) Improbable Worlds Ltd, All Rights Reserved */
#ifndef WORKER_SDK_CORE_INCLUDE_IMPROBABLE_WORKER_PROTOCOL_H
#define WORKER_SDK_CORE_INCLUDE_IMPROBABLE_WORKER_PROTOCOL_H
/* This C API is intended to be consumed by generated code and wrapper APIs in target languages (see
   the bundled C++ API in <improbable/worker.h> for example). It cannot sensibly be used on its
   own. */

#if defined(CORE_SDK_DLL) && defined(_MSC_VER)

#ifdef CORE_SDK_DLL_EXPORT
#define CORE_SDK_API __declspec(dllexport)
#else /* CORE_SDK_DLL_EXPORT */
#define CORE_SDK_API __declspec(dllimport)
#endif /* CORE_SDK_DLL_EXPORT */

#else /* defined(CORE_SDK_DLL) && defined(_MSC_VER) */
#define CORE_SDK_API
#endif /* defined(CORE_SDK_DLL) && defined(_MSC_VER) */

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <stddef.h>
#include <stdint.h>

typedef int64_t WorkerProtocol_EntityId;
typedef uint32_t WorkerProtocol_ComponentId;
typedef uint32_t WorkerProtocol_RequestId;

struct Pbio_Object;
struct WorkerProtocol_Connection;
struct WorkerProtocol_ConnectionFuture;
struct WorkerProtocol_Constraint;
struct WorkerProtocol_DeploymentListFuture;
struct WorkerProtocol_Dispatcher;
struct WorkerProtocol_Locator;
struct WorkerProtocol_OpList;
struct WorkerProtocol_SnasphotInputStream;
struct WorkerProtocol_SnapshotOutputStream;

typedef struct WorkerProtocol_Connection WorkerProtocol_Connection;
typedef struct WorkerProtocol_ConnectionFuture WorkerProtocol_ConnectionFuture;
typedef struct WorkerProtocol_Constraint WorkerProtocol_Constraint;
typedef struct WorkerProtocol_DeploymentListFuture WorkerProtocol_DeploymentListFuture;
typedef struct WorkerProtocol_Dispatcher WorkerProtocol_Dispatcher;
typedef struct WorkerProtocol_Locator WorkerProtocol_Locator;
typedef struct WorkerProtocol_OpList WorkerProtocol_OpList;
typedef struct WorkerProtocol_SnapshotInputStream WorkerProtocol_SnapshotInputStream;
typedef struct WorkerProtocol_SnapshotOutputStream WorkerProtocol_SnapshotOutputStream;

/**
 * Enum defining the severities of log messages that can be sent to SpatialOS and received from the
 * SDK.
 */
typedef enum WorkerProtocol_LogLevel {
  WORKER_PROTOCOL_LOG_LEVEL_DEBUG = 1,
  WORKER_PROTOCOL_LOG_LEVEL_INFO = 2,
  WORKER_PROTOCOL_LOG_LEVEL_WARN = 3,
  WORKER_PROTOCOL_LOG_LEVEL_ERROR = 4,
  WORKER_PROTOCOL_LOG_LEVEL_FATAL = 5
} WorkerProtocol_LogLevel;

/** Enum defining possible command status codes. */
typedef enum WorkerProtocol_StatusCode {
  WORKER_PROTOCOL_STATUS_CODE_SUCCESS = 1,
  WORKER_PROTOCOL_STATUS_CODE_TIMEOUT = 2,
  WORKER_PROTOCOL_STATUS_CODE_NOT_FOUND = 3,
  WORKER_PROTOCOL_STATUS_CODE_AUTHORITY_LOST = 4,
  WORKER_PROTOCOL_STATUS_CODE_PERMISSION_DENIED = 5,
  WORKER_PROTOCOL_STATUS_CODE_APPLICATION_ERROR = 6,
  WORKER_PROTOCOL_STATUS_CODE_INTERNAL_ERROR = 7,
} WorkerProtocol_StatusCode;

/** Enum defining the possible authority states for an entity component. */
typedef enum WorkerProtocol_Authority {
  WORKER_PROTOCOL_AUTHORITY_NOT_AUTHORITATIVE = 0,
  WORKER_PROTOCOL_AUTHORITY_AUTHORITATIVE = 1,
  WORKER_PROTOCOL_AUTHORITY_AUTHORITY_LOSS_IMMINENT = 2,
} WorkerProtocol_Authority;

/** Parameters for sending a log message to SpatialOS. */
typedef struct WorkerProtocol_LogMessage {
  /** The severity of the log message; defined in the WorkerProtocol_LogLevel enumeration. */
  uint8_t LogLevel;
  /** The name of the logger. */
  const char* LoggerName;
  /** The full log message. */
  const char* Message;
  /** The ID of the entity this message relates to, or NULL for none. */
  const WorkerProtocol_EntityId* EntityId;
} WorkerProtocol_LogMessage;

/** Parameters for a gauge metric. */
typedef struct WorkerProtocol_GaugeMetric {
  /* The name of the metric. */
  const char* Key;
  /* The current value of the metric. */
  double Value;
} WorkerProtocol_GaugeMetric;

/* Parameters for a histogram metric bucket. */
typedef struct WorkerProtocol_HistogramMetricBucket {
  /* The upper bound. */
  double UpperBound;
  /* The number of observations that were less than or equal to the upper bound. */
  uint32_t Samples;
} WorkerProtocol_HistogramMetricBucket;

/* Parameters for a histogram metric. */
typedef struct WorkerProtocol_HistogramMetric {
  /* The name of the metric. */
  const char* Key;
  /* The sum of all observations. */
  double Sum;
  /* The number of buckets. */
  uint32_t BucketCount;
  /* Array of buckets. */
  const WorkerProtocol_HistogramMetricBucket* Bucket;
} WorkerProtocol_HistogramMetric;

/** Parameters for sending metrics to SpatialOS. */
typedef struct WorkerProtocol_Metrics {
  /** The load value of this worker. If NULL, do not report load. */
  const double* Load;
  /** The number of gauge metrics. */
  uint32_t GaugeMetricCount;
  /** Array of gauge metrics. */
  const WorkerProtocol_GaugeMetric* GaugeMetric;
  /** The number of histogram metrics. */
  uint32_t HistogramMetricCount;
  /** Array of histogram metrics. */
  const WorkerProtocol_HistogramMetric* HistogramMetric;
} WorkerProtocol_Metrics;

/** Enumeration used to control dispatch of Vtable functions. */
typedef enum WorkerProtocol_ClientHandleType {
  WORKER_PROTOCOL_CLIENT_HANDLE_TYPE_UPDATE = 1,
  WORKER_PROTOCOL_CLIENT_HANDLE_TYPE_SNAPSHOT = 2,
  WORKER_PROTOCOL_CLIENT_HANDLE_TYPE_REQUEST = 3,
  WORKER_PROTOCOL_CLIENT_HANDLE_TYPE_RESPONSE = 4
} WorkerProtocol_ClientHandleType;

/** A type for passing user-controlled component objects into the C API. */
typedef void WorkerProtocol_ClientHandle;
/** Component vtable typedef. */
typedef void WorkerProtocol_ClientFree(WorkerProtocol_ComponentId component_id, uint8_t object_type,
                                       WorkerProtocol_ClientHandle* handle);
/** Component vtable typedef. */
typedef WorkerProtocol_ClientHandle*
WorkerProtocol_ClientCopy(WorkerProtocol_ComponentId component_id, uint8_t object_type,
                          const WorkerProtocol_ClientHandle* handle);
/** Component vtable typedef. */
typedef uint8_t WorkerProtocol_ClientDeserialize(WorkerProtocol_ComponentId component_id,
                                                 uint8_t object_type, Pbio_Object* source,
                                                 WorkerProtocol_ClientHandle** handle_out);
/** Component vtable typedef. */
typedef void WorkerProtocol_ClientSerialize(WorkerProtocol_ComponentId component_id,
                                            uint8_t object_type,
                                            const WorkerProtocol_ClientHandle* handle,
                                            Pbio_Object* target);

/**
 * A virtual function table for user-controlled component data. A vtable must be registered with
 * the connection for each component in WorkerProtocol_ConnectionParameters. This allows the API
 * to perform component update serialization and similar tasks in a target-language agnostic way.
 *
 * The API will not call virtual functions on the *same* client handle concurrently, but might call
 * virtual functions for *different* client handles concurrently. Therefore, the functions supplied
 * must be thread-safe except for possibly accessing resources associated with particular
 * WorkerProtocol_ClientHandle instances.
 */
typedef struct WorkerProtocol_ComponentVtable {
  /** Component ID that this vtable is for. */
  WorkerProtocol_ComponentId ComponentId;
  /**
   * Called to clean up and free the memory associated with a handle produced by Copy or
   * Deserialize.
   */
  WorkerProtocol_ClientFree* Free;
  /** Called to make a copy of the given handle. Must be compatible with Free. */
  WorkerProtocol_ClientCopy* Copy;
  /**
   * Called to produce a handle to deserialized object based on a language-independent PBIO
   * structure. Must be compatible with Free.
   */
  WorkerProtocol_ClientDeserialize* Deserialize;
  /** Called to produce a language-independent PBIO structure from a handle for serialization. */
  WorkerProtocol_ClientSerialize* Serialize;
} WorkerProtocol_ComponentVtable;

/**
 * An abstract representation of some data for a particular component. Might be an update, a
 * snapshot, or a command request or response. Manipulated via vtable functions.
 */
typedef struct WorkerProtocol_ComponentHandle {
  /** The ID of the component. */
  WorkerProtocol_ComponentId ComponentId;
  /** The user-controlled client handle type. */
  const WorkerProtocol_ClientHandle* Handle;
} WorkerProtocol_ComponentHandle;

/** Represents an entity with an ID and a component data snapshot. */
typedef struct WorkerProtocol_Entity {
  /** The ID of the entity. */
  WorkerProtocol_EntityId EntityId;
  /** Number of components for the entity. */
  uint32_t ComponentCount;
  /** Array of initial component data for the entity. */
  const WorkerProtocol_ComponentHandle* Component;
} WorkerProtocol_Entity;

typedef enum WorkerProtocol_ConstraintType {
  WORKER_PROTOCOL_CONSTRAINT_TYPE_ENTITY_ID = 1,
  WORKER_PROTOCOL_CONSTRAINT_TYPE_COMPONENT = 2,
  WORKER_PROTOCOL_CONSTRAINT_TYPE_SPHERE = 3,
  WORKER_PROTOCOL_CONSTRAINT_TYPE_AND = 4,
  WORKER_PROTOCOL_CONSTRAINT_TYPE_OR = 5,
  WORKER_PROTOCOL_CONSTRAINT_TYPE_NOT = 6,
} WorkerProtocol_ConstraintType;

typedef struct WorkerProtocol_EntityIdConstraint {
  WorkerProtocol_EntityId EntityId;
} WorkerProtocol_EntityIdConstraint;

typedef struct WorkerProtocol_ComponentConstraint {
  WorkerProtocol_ComponentId ComponentId;
} WorkerProtocol_ComponentConstraint;

typedef struct WorkerProtocol_SphereConstraint {
  double PositionX;
  double PositionY;
  double PositionZ;
  double Radius;
} WorkerProtocol_SphereConstraint;

typedef struct WorkerProtocol_AndConstraint {
  uint32_t ConstraintCount;
  WorkerProtocol_Constraint* Constraint;
} WorkerProtocol_AndConstraint;

typedef struct WorkerProtocol_OrConstraint {
  uint32_t ConstraintCount;
  WorkerProtocol_Constraint* Constraint;
} WorkerProtocol_OrConstraint;

typedef struct WorkerProtocol_NotConstraint {
  WorkerProtocol_Constraint* Constraint;
} WorkerProtocol_NotConstraint;

/** A single query constraint. */
typedef struct WorkerProtocol_Constraint {
  /** The type of constraint, defined using WorkerProtocol_ConstraintType. */
  uint8_t ConstraintType;
  /** Union with fields corresponding to each constraint type. */
  union {
    WorkerProtocol_EntityIdConstraint EntityIdConstraint;
    WorkerProtocol_ComponentConstraint ComponentConstraint;
    WorkerProtocol_SphereConstraint SphereConstraint;
    WorkerProtocol_AndConstraint AndConstraint;
    WorkerProtocol_OrConstraint OrConstraint;
    WorkerProtocol_NotConstraint NotConstraint;
  };
} WorkerProtocol_Constraint;

typedef enum WorkerProtocol_ResultType {
  WORKER_PROTOCOL_RESULT_TYPE_COUNT = 1,
  WORKER_PROTOCOL_RESULT_TYPE_SNAPSHOT = 2,
} WorkerProtocol_ResultType;

/** An entity query. */
typedef struct WorkerProtocol_EntityQuery {
  /** The constraint for this query. */
  WorkerProtocol_Constraint Constraint;
  /** Result type for this query, using WorkerProtocol_ResultType. */
  uint8_t ResultType;
  /** Number of component IDs in the array for a snapshot result type. */
  uint32_t SnapshotResultTypeComponentIdCount;
  /** Pointer to component ID data for a snapshot result type. NULL means all component IDs. */
  WorkerProtocol_ComponentId* SnapshotResultTypeComponentId;
} WorkerProtocol_EntityQuery;

/** An interest override for a particular (entity ID, component ID) pair. */
typedef struct WorkerProtocol_InterestOverride {
  /** The ID of the component for which interest is being overridden. */
  uint32_t ComponentId;
  /** Whether the worker is interested in this component. */
  uint8_t IsInterested;
} WorkerProtocol_InterestOverride;

/** Worker attributes that are part of a worker's runtime configuration. */
typedef struct WorkerProtocol_WorkerAttributes {
  /** Number of worker attributes. */
  uint32_t AttributeCount;
  /** Will be NULL if there are no attributes associated with the worker. */
  const char** Attribute;
} WorkerProtocol_WorkerAttributes;

/* The ops are placed in the same order everywhere. This order should be used everywhere there is
   some similar code for each op (including wrapper APIs, definitions, function calls, thunks, and
   so on).

   (SECTION 1) GLOBAL ops, which do not depend on any entity. */

/** Data for a log message from the SDK. */
typedef struct WorkerProtocol_DisconnectOp {
  /** The reason for the disconnect. */
  const char* Reason;
} WorkerProtocol_DisconnectOp;

/** Data for a FlagUpdate operation. */
typedef struct WorkerProtocol_FlagUpdateOp {
  /** The name of the updated worker flag. */
  const char* Name;
  /**
   * The new value of the updated worker flag.
   * A null value indicates that the flag has been deleted.
   */
  const char* Value;
} WorkerProtocol_FlagUpdateOp;

/** Data for a log message from the SDK. */
typedef struct WorkerProtocol_LogMessageOp {
  /** The severity of the log message; defined in the WorkerProtocol_LogLevel enumeration. */
  uint8_t LogLevel;
  /** The message. */
  const char* Message;
} WorkerProtocol_LogMessageOp;

/** Data for a set of built-in metrics reported by the SDK. */
typedef struct WorkerProtocol_MetricsOp {
  WorkerProtocol_Metrics Metrics;
} WorkerProtocol_MetricsOp;

/** Data for a critical section boundary (enter or leave) operation. */
typedef struct WorkerProtocol_CriticalSectionOp {
  /** Whether the protocol is entering a critical section (true) or leaving it (false). */
  uint8_t InCriticalSection;
} WorkerProtocol_CriticalSectionOp;

/* (SECTION 2) ENTITY-SPECIFIC ops, which do not depend on any component. */

/** Data for an AddEntity operation. */
typedef struct WorkerProtocol_AddEntityOp {
  /** The ID of the entity that was added to the worker's view of the simulation. */
  WorkerProtocol_EntityId EntityId;
} WorkerProtocol_AddEntityOp;

/** Data for a RemoveEntity operation. */
typedef struct WorkerProtocol_RemoveEntityOp {
  /** The ID of the entity that was removed from the worker's view of the simulation. */
  WorkerProtocol_EntityId EntityId;
} WorkerProtocol_RemoveEntityOp;

/** Data for a ReserveEntityIdResponse operation. */
typedef struct WorkerProtocol_ReserveEntityIdResponseOp {
  /** The ID of the reserve entity ID request for which there was a response. */
  WorkerProtocol_RequestId RequestId;
  /** Status code of the response, using WorkerProtocol_StatusCode. */
  uint8_t StatusCode;
  /** The error message. */
  const char* Message;
  /**
   * If successful, newly allocated entity id which is guaranteed to be unused in the current
   * deployment.
   */
  WorkerProtocol_EntityId EntityId;
} WorkerProtocol_ReserveEntityIdResponseOp;

/** Data for a ReserveEntityIdsResponse operation. */
typedef struct WorkerProtocol_ReserveEntityIdsResponseOp {
  /** The ID of the reserve entity ID request for which there was a response. */
  WorkerProtocol_RequestId RequestId;
  /** Status code of the response, using WorkerProtocol_StatusCode. */
  uint8_t StatusCode;
  /** The error message. */
  const char* Message;
  /**
   * If successful, an ID which is the first in a contiguous range of newly allocated entity
   * IDs which are guaranteed to be unused in the current deployment.
   */
  WorkerProtocol_EntityId FirstEntityId;
  /** If successful, the number of IDs reserved in the contiguous range, otherwise 0. */
  uint32_t NumberOfEntityIds;
} WorkerProtocol_ReserveEntityIdsResponseOp;

/** Data for a CreateEntity operation. */
typedef struct WorkerProtocol_CreateEntityResponseOp {
  /** The ID of the request for which there was a response. */
  WorkerProtocol_RequestId RequestId;
  /** Status code of the response, using WorkerProtocol_StatusCode. */
  uint8_t StatusCode;
  /** The error message. */
  const char* Message;
  /** If successful, the entity ID of the newly created entity. */
  WorkerProtocol_EntityId EntityId;
} WorkerProtocol_CreateEntityResponseOp;

/** Data for a DeleteEntity operation. */
typedef struct WorkerProtocol_DeleteEntityResponseOp {
  /** The ID of the delete entity request for which there was a command response. */
  WorkerProtocol_RequestId RequestId;
  /** The ID of the target entity of this request. */
  WorkerProtocol_EntityId EntityId;
  /** Status code of the response, using WorkerProtocol_StatusCode. */
  uint8_t StatusCode;
  /** The error message. */
  const char* Message;
} WorkerProtocol_DeleteEntityResponseOp;

/** A response indicating the result of an entity query request. */
typedef struct WorkerProtocol_EntityQueryResponseOp {
  /** The ID of the entity query request for which there was a response. */
  WorkerProtocol_RequestId RequestId;
  /** Status code of the response, using WorkerProtocol_StatusCode. */
  uint8_t StatusCode;
  /** The error message. */
  const char* Message;
  /**
   * Number of entities in the result set. Reused to indicate the result itself for CountResultType
   * queries.
   */
  uint32_t ResultCount;
  /**
   * Array of entities in the result set. Will be NULL if the query was a count query. Snapshot data
   * in the result is deserialized with the corresponding vtable Deserialize function and freed with
   * the vtable Free function when the OpList is destroyed.
   */
  const WorkerProtocol_Entity* Result;
} WorkerProtocol_EntityQueryResponseOp;

/* (SECTION 3) COMPONENT-SPECIFIC ops. */

/** Data for an AddComponent operation. */
typedef struct WorkerProtocol_AddComponentOp {
  /** The ID of the entity for which a component was added. */
  WorkerProtocol_EntityId EntityId;
  /**
   * The initial data for the new component. Deserialized with the corresponding vtable Deserialize
   * function and freed with the vtable Free function when the OpList is destroyed.
   */
  WorkerProtocol_ComponentHandle InitialComponent;
} WorkerProtocol_AddComponentOp;

/** Data for a RemoveComponent operation. */
typedef struct WorkerProtocol_RemoveComponentOp {
  /** The ID of the entity for which a component was removed. */
  WorkerProtocol_EntityId EntityId;
  /** The ID of the component that was removed. */
  WorkerProtocol_ComponentId ComponentId;
} WorkerProtocol_RemoveComponentOp;

/** Data for an AuthorityChange operation. */
typedef struct WorkerProtocol_AuthorityChangeOp {
  /** The ID of the entity for which there was an authority change. */
  WorkerProtocol_EntityId EntityId;
  /** The ID of the component over which the worker's authority has changed. */
  WorkerProtocol_ComponentId ComponentId;
  /** The authority state of the component, using the WorkerProtocol_Authority enumeration. */
  uint8_t Authority;
} WorkerProtocol_AuthorityChangeOp;

/** Data for a ComponentUpdate operation. */
typedef struct WorkerProtocol_ComponentUpdateOp {
  /** The ID of the entity for which there was a component update. */
  WorkerProtocol_EntityId EntityId;
  /**
   * The new component data for the updated entity. Deserialized with the corresponding vtable
   * Deserialize function and freed with the vtable Free function when the OpList is destroyed.
   */
  WorkerProtocol_ComponentHandle Update;
} WorkerProtocol_ComponentUpdateOp;

/** Data for a CommandRequest operation. */
typedef struct WorkerProtocol_CommandRequestOp {
  /** The incoming command request ID. */
  WorkerProtocol_RequestId RequestId;
  /** The ID of the entity for which there was a command request. */
  WorkerProtocol_EntityId EntityId;
  /** Upper bound on request timeout provided by the platform. */
  uint32_t TimeoutMillis;
  /** The ID of the worker that sent the request. */
  const char* CallerWorkerId;
  /** The attributes of the worker that sent the request. */
  WorkerProtocol_WorkerAttributes CallerAttributes;
  /**
   * The command request data. Deserialized with the corresponding vtable Deserialize function and
   * freed with the vtable Free function when the OpList is destroyed.
   */
  WorkerProtocol_ComponentHandle Request;
} WorkerProtocol_CommandRequestOp;

/** Data for a CommandResponse operation. */
typedef struct WorkerProtocol_CommandResponseOp {
  /** The ID of the command request for which there was a command response. */
  WorkerProtocol_RequestId RequestId;
  /** The ID of the entity originally targeted by the command request. */
  WorkerProtocol_EntityId EntityId;
  /** Status code of the response, using WorkerProtocol_StatusCode. */
  uint8_t StatusCode;
  /** The error message. */
  const char* Message;
  /**
   * The command response data. Deserialized with the corresponding vtable Deserialize function and
   * freed with the vtable Free function when the OpList is destroyed.
   */
  WorkerProtocol_ComponentHandle Response;
  /** The command ID given to WorkerProtocol_Connection_SendCommandRequest. */
  uint32_t CommandId;
} WorkerProtocol_CommandResponseOp;

/** Dispatcher callback typedefs, in the same order as above. */
typedef void WorkerProtocol_DisconnectCallback(void* user_data,
                                               const WorkerProtocol_DisconnectOp* op);
typedef void WorkerProtocol_FlagUpdateCallback(void* user_data,
                                               const WorkerProtocol_FlagUpdateOp* op);
typedef void WorkerProtocol_LogMessageCallback(void* user_data,
                                               const WorkerProtocol_LogMessageOp* op);
typedef void WorkerProtocol_MetricsCallback(void* user_data, const WorkerProtocol_MetricsOp* op);
typedef void WorkerProtocol_CriticalSectionCallback(void* user_data,
                                                    const WorkerProtocol_CriticalSectionOp* op);
typedef void WorkerProtocol_AddEntityCallback(void* user_data,
                                              const WorkerProtocol_AddEntityOp* op);
typedef void WorkerProtocol_RemoveEntityCallback(void* user_data,
                                                 const WorkerProtocol_RemoveEntityOp* op);
typedef void
WorkerProtocol_ReserveEntityIdResponseCallback(void* user_data,
                                               const WorkerProtocol_ReserveEntityIdResponseOp* op);
typedef void WorkerProtocol_ReserveEntityIdsResponseCallback(
    void* user_data, const WorkerProtocol_ReserveEntityIdsResponseOp* op);
typedef void
WorkerProtocol_CreateEntityResponseCallback(void* user_data,
                                            const WorkerProtocol_CreateEntityResponseOp* op);
typedef void
WorkerProtocol_DeleteEntityResponseCallback(void* user_data,
                                            const WorkerProtocol_DeleteEntityResponseOp* op);
typedef void
WorkerProtocol_EntityQueryResponseCallback(void* user_data,
                                           const WorkerProtocol_EntityQueryResponseOp* op);
typedef void WorkerProtocol_AddComponentCallback(void* user_data,
                                                 const WorkerProtocol_AddComponentOp* op);
typedef void WorkerProtocol_RemoveComponentCallback(void* user_data,
                                                    const WorkerProtocol_RemoveComponentOp* op);
typedef void WorkerProtocol_AuthorityChangeCallback(void* user_data,
                                                    const WorkerProtocol_AuthorityChangeOp* op);
typedef void WorkerProtocol_ComponentUpdateCallback(void* user_data,
                                                    const WorkerProtocol_ComponentUpdateOp* op);
typedef void WorkerProtocol_CommandRequestCallback(void* user_data,
                                                   const WorkerProtocol_CommandRequestOp* op);
typedef void WorkerProtocol_CommandResponseCallback(void* user_data,
                                                    const WorkerProtocol_CommandResponseOp* op);

/**
 * WorkerProtocol_Dispatcher API. This is the main way of processing operations from a
 * WorkerProtocol_Connection.
 */
CORE_SDK_API WorkerProtocol_Dispatcher* WorkerProtocol_Dispatcher_Create();
/**
 * Frees resources for a WorkerProtocol_Dispatcher created with WorkerProtocol_Dispatcher_Create.
 */
CORE_SDK_API void WorkerProtocol_Dispatcher_Destroy(WorkerProtocol_Dispatcher* dispatcher);
/**
 * Registers a callback to be invoked when the Connection has disconnected and can no longer be
 * used.
 */
CORE_SDK_API void WorkerProtocol_Dispatcher_RegisterDisconnectCallback(
    WorkerProtocol_Dispatcher* dispatcher, void* data, WorkerProtocol_DisconnectCallback* callback);
/** Registers a callback to be invoked when a worker flag is updated. */
CORE_SDK_API void WorkerProtocol_Dispatcher_RegisterFlagUpdateCallback(
    WorkerProtocol_Dispatcher* dispatcher, void* data, WorkerProtocol_FlagUpdateCallback* callback);
/**  Registers a callback to be invoked when the SDK logs a message.*/
CORE_SDK_API void WorkerProtocol_Dispatcher_RegisterLogMessageCallback(
    WorkerProtocol_Dispatcher* dispatcher, void* data, WorkerProtocol_LogMessageCallback* callback);
/**  Registers a callback to be invoked when the SDK reports its built-in local metrics.*/
CORE_SDK_API void
WorkerProtocol_Dispatcher_RegisterMetricsCallback(WorkerProtocol_Dispatcher* dispatcher, void* data,
                                                  WorkerProtocol_MetricsCallback* callback);
/**
 * Registers a callback to be invoked when the message stream enters or leaves a critical section.
 */
CORE_SDK_API void WorkerProtocol_Dispatcher_RegisterCriticalSectionCallback(
    WorkerProtocol_Dispatcher* dispatcher, void* data,
    WorkerProtocol_CriticalSectionCallback* callback);
/**
 * Sets the callback to be invoked when a new entity is added to the worker's view of the
 * simulation.
 */
CORE_SDK_API void WorkerProtocol_Dispatcher_RegisterAddEntityCallback(
    WorkerProtocol_Dispatcher* dispatcher, void* data, WorkerProtocol_AddEntityCallback* callback);
/**
 * Registers a callback to be invoked when an entity is removed from the worker's view of the
 * simulation.
 */
CORE_SDK_API void WorkerProtocol_Dispatcher_RegisterRemoveEntityCallback(
    WorkerProtocol_Dispatcher* dispatcher, void* data,
    WorkerProtocol_RemoveEntityCallback* callback);
/** Registers a callback to be invoked when an entity ID reservation response is received. */
CORE_SDK_API void WorkerProtocol_Dispatcher_RegisterReserveEntityIdResponseCallback(
    WorkerProtocol_Dispatcher* dispatcher, void* data,
    WorkerProtocol_ReserveEntityIdResponseCallback* callback);
/**
 * Registers a callback to be invoked when an entity ID reservation response is received
 * in response to a request to reserve mutiple entity IDs.
 */
CORE_SDK_API void WorkerProtocol_Dispatcher_RegisterReserveEntityIdsResponseCallback(
    WorkerProtocol_Dispatcher* dispatcher, void* data,
    WorkerProtocol_ReserveEntityIdsResponseCallback* callback);
/** Registers a callback to be invoked when an entity creation response is received. */
CORE_SDK_API void WorkerProtocol_Dispatcher_RegisterCreateEntityResponseCallback(
    WorkerProtocol_Dispatcher* dispatcher, void* data,
    WorkerProtocol_CreateEntityResponseCallback* callback);
/** Registers a callback to be invoked when an entity deletion response is received. */
CORE_SDK_API void WorkerProtocol_Dispatcher_RegisterDeleteEntityResponseCallback(
    WorkerProtocol_Dispatcher* dispatcher, void* data,
    WorkerProtocol_DeleteEntityResponseCallback* callback);
/** Registers a callback to be invoked when an entity query response is received. */
CORE_SDK_API void WorkerProtocol_Dispatcher_RegisterEntityQueryResponseCallback(
    WorkerProtocol_Dispatcher* dispatcher, void* data,
    WorkerProtocol_EntityQueryResponseCallback* callback);
/** Registers a callback to be invoked when a component is added to an entity. */
CORE_SDK_API void WorkerProtocol_Dispatcher_RegisterAddComponentCallback(
    WorkerProtocol_Dispatcher* dispatcher, void* data,
    WorkerProtocol_AddComponentCallback* callback);
/** Registers a callback to be invoked when a component is removed from an entity. */
CORE_SDK_API void WorkerProtocol_Dispatcher_RegisterRemoveComponentCallback(
    WorkerProtocol_Dispatcher* dispatcher, void* data,
    WorkerProtocol_RemoveComponentCallback* callback);
/**
 * Registers a callback to be invoked when the worker's authority over an entity's component is
 * changed.
 */
CORE_SDK_API void WorkerProtocol_Dispatcher_RegisterAuthorityChangeCallback(
    WorkerProtocol_Dispatcher* dispatcher, void* data,
    WorkerProtocol_AuthorityChangeCallback* callback);
/** Registers a callback to be invoked when an entity's component is updated. */
CORE_SDK_API void WorkerProtocol_Dispatcher_RegisterComponentUpdateCallback(
    WorkerProtocol_Dispatcher* dispatcher, void* data,
    WorkerProtocol_ComponentUpdateCallback* callback);
/** Registers a callback to be invoked when a command request is received. */
CORE_SDK_API void WorkerProtocol_Dispatcher_RegisterCommandRequestCallback(
    WorkerProtocol_Dispatcher* dispatcher, void* data,
    WorkerProtocol_CommandRequestCallback* callback);
/** Registers a callback to be invoked when a command response is received. */
CORE_SDK_API void WorkerProtocol_Dispatcher_RegisterCommandResponseCallback(
    WorkerProtocol_Dispatcher* dispatcher, void* data,
    WorkerProtocol_CommandResponseCallback* callback);

/**
 * Processes a WorkerProtocol_OpList returned by WorkerConnection_GetOpList, invoking the
 * appropriate registered callbacks for each operation in the list.
 */
CORE_SDK_API void WorkerProtocol_Dispatcher_Process(const WorkerProtocol_Dispatcher* dispatcher,
                                                    const WorkerProtocol_OpList* op_list);

/** Parameters for configuring a RakNet connection. Used by WorkerProtocol_NetworkParameters. */
typedef struct WorkerProtocol_RakNetParameters {
  /** Time (in milliseconds) that RakNet should use for its heartbeat protocol. */
  uint32_t HeartbeatTimeoutMillis;
} WorkerProtocol_RakNetParameters;

/** Parameters for configuring a TCP connection. Used by WorkerProtocol_NetworkParameters. */
typedef struct WorkerProtocol_TcpParameters {
  /** The number of multiplexed TCP connections to use. */
  uint8_t MultiplexLevel;
  /** Whether to enable TCP_NODELAY. */
  uint8_t NoDelay;
  /** Size in bytes of the TCP send buffer. */
  uint32_t SendBufferSize;
  /** Size in bytes of the TCP receive buffer. */
  uint32_t ReceiveBufferSize;
} WorkerProtocol_TcpParameters;

/** Flags used by the WorkerProtocol_NetworkParameters struct. */
typedef enum WorkerProtocol_NetworkFlags {
  /** Use this flag to connect over TCP. */
  WORKER_PROTOCOL_NETWORK_TCP = 1,
  /** Use this flag to connect over RakNet. */
  WORKER_PROTOCOL_NETWORK_RAKNET = 2,
  /**
   * Set this flag to connect to SpatialOS using the externally-visible IP address. This flag must
   * be set when connecting externally (i.e. from outside the cloud) to a cloud deployment.
   */
  WORKER_PROTOCOL_NETWORK_USE_EXTERNAL_IP = 4,
} WorkerProtocol_NetworkFlags;

/** Parameters for configuring the network connection. */
typedef struct WorkerProtocol_NetworkParameters {
  /** Flags controlling network connection behaviour, defined in WorkerProtocol_NetworkFlags. */
  uint8_t Flags;
  /** Parameters used if the WORKER_PROTOCOL_NETWORK_RAKNET flag is set. */
  WorkerProtocol_RakNetParameters RakNet;
  /** Parameters used if the WORKER_PROTOCOL_NETWORK_TCP flag is set. */
  WorkerProtocol_TcpParameters Tcp;
} WorkerProtocol_NetworkParameters;

/**
 * Tuning parameters for configuring internal details of the SDK. Used by
 * WorkerProtocol_ConnectionParameters.
 */
typedef struct WorkerProtocol_SdkParameters {
  /**
   * Number of messages that can be stored on the send queue. When the send queue is full, calls to
   * WorkerProtocol_Connection_Send functions can block.
   */
  uint32_t SendQueueCapacity;
  /**
   * Number of messages that can be stored on the receive queue. When the receive queue is full,
   * SpatialOS can apply QoS and drop messages to the worker.
   */
  uint32_t ReceiveQueueCapacity;
  /**
   * Number of messages logged by the SDK that can be stored in the log message queue. When the log
   * message queue is full, messages logged by the SDK can be dropped.
   */
  uint32_t LogMessageQueueCapacity;
  /**
   * The Connection tracks several internal metrics, such as send and receive queue statistics. This
   * parameter controls how frequently the Connection will return a MetricsOp reporting its built-in
   * metrics. If set to zero, this functionality is disabled.
   */
  uint32_t BuiltInMetricsReportPeriodMillis;

  /** Whether to enable protocol logging at startup. */
  uint8_t EnableProtocolLoggingAtStartup;
  /** Log file names are prefixed with this prefix, are numbered, and have the extension .log. */
  const char* LogPrefix;
  /**
   * Maximum number of log files to keep. Note that logs from any previous protocol logging
   * sessions will be overwritten.
   */
  uint32_t MaxLogFiles;
  /** Once the size of a log file reaches this size, a new log file is created. */
  uint32_t MaxLogFileSizeBytes;
} WorkerProtocol_SdkParameters;

/** Parameters for creating a WorkerProtocol_Connection and connecting to SpatialOS. */
typedef struct WorkerProtocol_ConnectionParameters {
  /** Worker type (platform). */
  const char* WorkerType;

  /** Network parameters. */
  WorkerProtocol_NetworkParameters Network;
  /** SDK behaviour parameters. */
  WorkerProtocol_SdkParameters Sdk;

  /** Number of component vtables. */
  uint32_t ComponentVtableCount;
  /** Component vtable for each component that the connection will deal with. */
  const WorkerProtocol_ComponentVtable* ComponentVtable;
} WorkerProtocol_ConnectionParameters;

/** Parameters for authenticating using a SpatialOS login token. */
typedef struct WorkerProtocol_LoginTokenCredentials {
  /** The token would typically be provided on the command-line by the SpatialOS launcher. */
  const char* Token;
} WorkerProtocol_LoginTokenCredentials;

/** Parameters for authenticating using Steam credentials. */
typedef struct WorkerProtocol_SteamCredentials {
  /**
   * Steam ticket for the steam app ID and publisher key corresponding to the project name specified
   * in the WorkerProtocol_LocatorParameters. Typically obtained from the steam APIs.
   */
  const char* Ticket;
  /**
   * Deployment tag to request access for. If non-empty, must match the following regex:
   * [A-Za-z0-9][A-Za-z0-9_]*
   */
  const char* DeploymentTag;
} WorkerProtocol_SteamCredentials;

/** Flags used by the WorkerProtocol_LocatorParameters struct. */
typedef enum WorkerProtocol_LocatorFlags {
  WORKER_PROTOCOL_LOCATOR_LOGIN_TOKEN_CREDENTIALS = 1,
  WORKER_PROTOCOL_LOCATOR_STEAM_CREDENTIALS = 2,
} WorkerProtocol_LocatorFlags;

/** Parameters for authenticating and logging in to a SpatialOS deployment. */
typedef struct WorkerProtocol_LocatorParameters {
  /** The name of the SpatialOS project. */
  const char* ProjectName;
  /** Flags controlling locator client behaviour, defined in WorkerProtocol_LocatorFlags. */
  uint8_t Flags;
  /** Parameters used if the WORKER_PROTOCOL_LOGIN_TOKEN_CREDENTIALS flag is set. */
  WorkerProtocol_LoginTokenCredentials LoginToken;
  /** Parameters used if the WORKER_PROTOCOL_STEAM_CREDENTIALS flag is set. */
  WorkerProtocol_SteamCredentials Steam;
} WorkerProtocol_LocatorParameters;

/** Details of a specific deployment obtained via WorkerProtocol_Locator_GetDeploymentListAsync. */
typedef struct WorkerProtocol_Deployment {
  /** Name of the deployment. */
  const char* DeploymentName;
  /** The name of the assembly used by this deployment. */
  const char* AssemblyName;
  /** Description of the deployment. */
  const char* Description;
  /** Number of users currently connected to the deployment. */
  uint32_t UsersConnected;
  /** Total user capacity of the deployment. */
  uint32_t UsersCapacity;
} WorkerProtocol_Deployment;

/** A deployment list obtained via WorkerProtocol_Locator_GetDeploymentListAsync. */
typedef struct WorkerProtocol_DeploymentList {
  /** Number of deployments. */
  uint32_t DeploymentCount;
  /** Array of deployments. */
  WorkerProtocol_Deployment* Deployment;
  /** Will be non-NULL if an error occurred. */
  const char* Error;
} WorkerProtocol_DeploymentList;

/**
 * A queue status update when connecting to a deployment via WorkerProtocol_Locator_ConnectAsync.
 */
typedef struct WorkerProtocol_QueueStatus {
  /** Position in the queue. Decreases as we advance to the front of the queue. */
  uint32_t PositionInQueue;
  /** Will be non-NULL if an error occurred. */
  const char* Error;
} WorkerProtocol_QueueStatus;

/** Command parameters. Used to modify the behaviour of a command request. */
typedef struct WorkerProtocol_CommandParameters {
  /**
   * Allow command requests to bypass the bridge when this worker is authoritative over the target
   * entity-component.
   */
  uint8_t AllowShortCircuit;
} WorkerProtocol_CommandParameters;

/** Locator callback typedef. */
typedef void
WorkerProtocol_DeploymentListCallback(void* user_data,
                                      const WorkerProtocol_DeploymentList* deployment_list);
/** Locator callback typedef. */
typedef uint8_t WorkerProtocol_QueueStatusCallback(void* user_data,
                                                   const WorkerProtocol_QueueStatus* queue_status);
/** Worker flags callback typedef. */
typedef void WorkerProtocol_GetFlagCallback(void* user_data, const char* value);

/**
 * Creates a client which can be used to connect to a SpatialOS deployment via a locator service.
 * This is the standard flow used to connect a local worker to a cloud deployment.
 *
 * The hostname would typically be either "locator.improbable.io" (for production) or
 * "locator-staging.improbable.io" (for staging).
 */
CORE_SDK_API WorkerProtocol_Locator*
WorkerProtocol_Locator_Create(const char* hostname, const WorkerProtocol_LocatorParameters* params);
/** Frees resources for a WorkerProtocol_Locator created with WorkerProtocol_Locator_Create. */
CORE_SDK_API void WorkerProtocol_Locator_Destroy(WorkerProtocol_Locator* locator);
/**
 * Queries the current list of deployments for the project given in the
 * WorkerProtocol_LocatorParameters.
 */
CORE_SDK_API WorkerProtocol_DeploymentListFuture*
WorkerProtocol_Locator_GetDeploymentListAsync(const WorkerProtocol_Locator* locator);
/**
 * Connects to a specific deployment. The deployment name should be obtained by calling
 * WorkerProtocol_Locator_GetDeploymentListAsync. The callback should return zero to cancel queuing,
 * or non-zero to continue queueing.
 *
 * Returns a WorkerProtocol_ConnectionFuture that can be used to obtain a WorkerProtocol_Connection
 * by using WorkerProtocol_ConnectionFuture_Get. Caller is responsible for destroying it when no
 * longer needed by using WorkerProtocol_ConnectionFuture_Destroy.
 */
CORE_SDK_API WorkerProtocol_ConnectionFuture*
WorkerProtocol_Locator_ConnectAsync(const WorkerProtocol_Locator* locator,
                                    const char* deployment_name,
                                    const WorkerProtocol_ConnectionParameters* params, void* data,
                                    WorkerProtocol_QueueStatusCallback* callback);

/**
 * Connect to a SpatialOS deployment via a receptionist. This is the flow used to connect a managed
 * worker running in the cloud alongside the deployment, and also to connect any local worker to a
 * (local or remote) deployment via a locally-running receptionist.
 *
 * The hostname and port would typically be provided by SpatialOS on the command-line, if this is a
 * managed worker on the cloud, or otherwise be predetermined (e.g. localhost:7777 for the default
 * receptionist of a locally-running deployment).
 *
 * Returns a WorkerProtocol_ConnectionFuture that can be used to obtain a WorkerProtocol_Connection
 * by using WorkerProtocol_ConnectionFuture_Get. Caller is responsible for destroying it when no
 * longer needed by using WorkerProtocol_ConnectionFuture_Destroy.
 */
CORE_SDK_API WorkerProtocol_ConnectionFuture*
WorkerProtocol_ConnectAsync(const char* hostname, uint16_t port, const char* worker_id,
                            const WorkerProtocol_ConnectionParameters* params);

/** Destroys a WorkerProtocol_DeploymentListFuture. Blocks until the future has completed. */
CORE_SDK_API void
WorkerProtocol_DeploymentListFuture_Destroy(WorkerProtocol_DeploymentListFuture* future);
/**
 * Gets the result of a WorkerProtocol_DeploymentListFuture, waiting for up to *timeout_millis to
 * become available (or forever if timeout_millis is NULL).
 *
 * It is an error to call this method again once it has succeeded (e.g. not timed out) once.
 */
CORE_SDK_API void
WorkerProtocol_DeploymentListFuture_Get(WorkerProtocol_DeploymentListFuture* future,
                                        const uint32_t* timeout_millis, void* data,
                                        WorkerProtocol_DeploymentListCallback* callback);

/** Destroys a WorkerProtocol_ConnectionFuture. Blocks until the future has completed. */
CORE_SDK_API void WorkerProtocol_ConnectionFuture_Destroy(WorkerProtocol_ConnectionFuture* future);
/**
 * Gets the result of a WorkerProtocol_ConnectionFuture, waiting for up to *timeout_millis to
 * become available (or forever if timeout_millis is NULL). It returns NULL in case of a timeout.
 *
 * It is an error to call this method again once it has succeeded (e.g. not timed out) once.
 */
CORE_SDK_API WorkerProtocol_Connection*
WorkerProtocol_ConnectionFuture_Get(WorkerProtocol_ConnectionFuture* future,
                                    const uint32_t* timeout_millis);

/**
 * Frees resources for a WorkerProtocol_Connection created with WorkerProtocol_ConnectAsync or
 * WorkerProtocol_Locator_ConnectAsync.
 */
CORE_SDK_API void WorkerProtocol_Connection_Destroy(WorkerProtocol_Connection* connection);
/** Sends a log message from the worker to SpatialOS. */
CORE_SDK_API void
WorkerProtocol_Connection_SendLogMessage(WorkerProtocol_Connection* connection,
                                         const WorkerProtocol_LogMessage* log_message);
/** Sends metrics data for the worker to SpatialOS. */
CORE_SDK_API void WorkerProtocol_Connection_SendMetrics(WorkerProtocol_Connection* connection,
                                                        const WorkerProtocol_Metrics* metrics);
/** Requests SpatialOS to reserve an entity ID. */
CORE_SDK_API WorkerProtocol_RequestId WorkerProtocol_Connection_SendReserveEntityIdRequest(
    WorkerProtocol_Connection* connection, const uint32_t* timeout_millis);
/** Requests SpatialOS to reserve multiple entity IDs. */
CORE_SDK_API WorkerProtocol_RequestId WorkerProtocol_Connection_SendReserveEntityIdsRequest(
    WorkerProtocol_Connection* connection, const uint32_t number_of_entity_ids,
    const uint32_t* timeout_millis);
/**
 * Requests SpatialOS to create an entity. The entity data is serialized immediately using the
 * corresponding vtable Serialize function; no copy is made or ownership transferred.
 */
CORE_SDK_API WorkerProtocol_RequestId WorkerProtocol_Connection_SendCreateEntityRequest(
    WorkerProtocol_Connection* connection, uint32_t component_count,
    const WorkerProtocol_ComponentHandle* component, const WorkerProtocol_EntityId* entity_id,
    const uint32_t* timeout_millis);
/** Requests SpatialOS to delete an entity. */
CORE_SDK_API WorkerProtocol_RequestId WorkerProtocol_Connection_SendDeleteEntityRequest(
    WorkerProtocol_Connection* connection, WorkerProtocol_EntityId entity_id,
    const uint32_t* timeout_millis);
/** Queries SpatialOS for entity data. */
CORE_SDK_API WorkerProtocol_RequestId WorkerProtocol_Connection_SendEntityQueryRequest(
    WorkerProtocol_Connection* connection, const WorkerProtocol_EntityQuery* entity_query,
    const uint32_t* timeout_millis);
/**
 * Sends a component update for the given entity to SpatialOS. Note that the sent component update
 * is added as an operation to the operation list and will be returned by a subsequent call to
 * WorkerProtocol_connection_GetOpList. The update data is copied with the corresponding vtable Copy
 * function and the copy is later freed with the vtable Free function.
 *
 * Note that the latter behaviour can be disabled by passing a nonzero value for
 * legacy_callback_semantics, but this is deprecated and for legacy compatibility only and will be
 * removed (see WIT-949).
 */
CORE_SDK_API void WorkerProtocol_Connection_SendComponentUpdate(
    WorkerProtocol_Connection* connection, WorkerProtocol_EntityId entity_id,
    const WorkerProtocol_ComponentHandle* component_update, uint8_t legacy_callback_semantics);
/**
 * Sends a command request targeting the given entity and component to SpatialOS. If timeout_millis
 * is null, the default will be used. The request data is copied with the corresponding vtable Copy
 * function and the copy is later freed with the vtable Free function.
 *
 * The command_id parameter has no effect other than being exposed in the
 * WorkerProtocol_CommandResponseOp so that callers can correctly handle command failures.
 *
 * The command parameters argument must not be NULL.
 */
CORE_SDK_API WorkerProtocol_RequestId WorkerProtocol_Connection_SendCommandRequest(
    WorkerProtocol_Connection* connection, WorkerProtocol_EntityId entity_id,
    const WorkerProtocol_ComponentHandle* request, uint32_t command_id,
    const uint32_t* timeout_millis, const WorkerProtocol_CommandParameters* command_parameters);
/**
 * Sends a command response for the given request ID to SpatialOS. The response data is copied with
 * the corresponding vtable Copy function and the copy is later freed with the vtable Free function.
 */
CORE_SDK_API void
WorkerProtocol_Connection_SendCommandResponse(WorkerProtocol_Connection* connection,
                                              WorkerProtocol_RequestId request_id,
                                              const WorkerProtocol_ComponentHandle* response);
/** Sends an explicit failure for the given command request ID to SpatialOS. */
CORE_SDK_API void
WorkerProtocol_Connection_SendCommandFailure(WorkerProtocol_Connection* connection,
                                             WorkerProtocol_RequestId request_id,
                                             const char* message);
/**
 * Sends a diff-based component interest update for the given entity to SpatialOS. By default, the
 * worker receives data for all entities according to the default component interest specified in
 * its bridge settings. This function allows interest override by (entity ID, component ID) pair to
 * force the data to either always be sent or never be sent. Note that this does not apply if the
 * worker is _authoritative_ over a particular (entity ID, component ID) pair, in which case the
 * data is always sent.
 */
CORE_SDK_API void WorkerProtocol_Connection_SendComponentInterest(
    WorkerProtocol_Connection* connection, WorkerProtocol_EntityId entity_id,
    const WorkerProtocol_InterestOverride* interest_override, uint32_t interest_override_count);
/**
 * Sends an acknowledgement of the receipt of an AuthorityLossImminent authority change for a
 * component. Sending the acknowledgement signifies that this worker is ready to lose authority
 * over the component.
 */
CORE_SDK_API void WorkerProtocol_Connection_SendAuthorityLossImminentAcknowledgement(
    WorkerProtocol_Connection* connection, WorkerProtocol_EntityId entity_id,
    WorkerProtocol_ComponentId component_id);
/**
 * Enables or disables protocol logging. Logging uses the parameters specified when the connection
 * was created. Enabling it when already enabled, or disabling it when already disabled, do nothing.
 *
 * Note that logs from any previous protocol logging sessions will be overwritten.
 */
CORE_SDK_API void
WorkerProtocol_Connection_SetProtocolLoggingEnabled(WorkerProtocol_Connection* connection,
                                                    uint8_t enabled);
/** Returns true if the connection has been successfully created and communication is ongoing. */
CORE_SDK_API uint8_t
WorkerProtocol_Connection_IsConnected(const WorkerProtocol_Connection* connection);
/**
 * Retrieves the ID of the worker as assigned by the runtime. The returned pointer points to data
 * that is owned by the SDK and will remain valid for the lifetime of the connection.
 */
CORE_SDK_API const char*
WorkerProtocol_Connection_GetWorkerId(const WorkerProtocol_Connection* connection);
/**
 * Retrieves the attributes associated with the worker at runtime. The result to data that is owned
 * by the SDK and will remain valid for the lifetime of the connection.
 */
CORE_SDK_API const WorkerProtocol_WorkerAttributes*
WorkerProtocol_Connection_GetWorkerAttributes(const WorkerProtocol_Connection* connection);
/**
 * Queries the worker flag with the given name. If the worker flag does not exist, the value will
 * be NULL.
 *
 * Worker flags are remotely configurable and may change during the runtime of the worker,
 * including addition and deletion.
 */
CORE_SDK_API void WorkerProtocol_Connection_GetFlag(const WorkerProtocol_Connection* connection,
                                                    const char* name, void* user_data,
                                                    WorkerProtocol_GetFlagCallback* callback);
/**
 * Retrieves the list of operations that have occurred since the last call to this function.
 *
 * If timeout_millis is non-zero, the function will block until there is at least one operation to
 * return, or the timeout has been exceeded. If the timeout is exceeded, an empty list will be
 * returned.
 *
 * If timeout_millis is zero the function is non-blocking.
 *
 * It is the caller's responsibility to destroy the returned WorkerProtocol_OpList with the
 * WorkerProtocol_OpList_Destroy function.
 */
CORE_SDK_API WorkerProtocol_OpList*
WorkerProtocol_Connection_GetOpList(WorkerProtocol_Connection* connection, uint32_t timeout_millis);
/** Frees resources for WorkerProtocol_OpList returned by WorkerProtocol_connection_GetOpList. */
CORE_SDK_API void WorkerProtocol_OpList_Destroy(WorkerProtocol_OpList* op_list);

typedef struct WorkerProtocol_SnapshotParameters {
  /** Number of component vtables. */
  uint32_t ComponentVtableCount;
  /** Component vtable for each component that the connection will deal with. */
  const WorkerProtocol_ComponentVtable* ComponentVtable;
} WorkerProtocol_SnapshotParameters;

typedef struct WorkerProtocol_Snapshot {
  /** Number of entities in the snapshot. */
  uint32_t EntityCount;
  /** Array of entity data in the snapshot. */
  const WorkerProtocol_Entity* Entity;
} WorkerProtocol_Snapshot;

/**
 * Opens a WorkerProtocol_SnapshotInputStream. The caller must manage the memory of the
 * returned WorkerProtocol_SnapshotInputStream* by calling WorkerProtocol_SnapshotInputStream to
 * write the EOF and release resources.
 *
 * If an error occurs, a pointer to a WorkerProtocol_SnapshotInputStream is still returned.
 * Calling WorkerProtocol_SnapshotInputStream_GetError with this pointer will return
 * an error message describing any error that occured. In the event of an error, the caller still
 * must release the memory of the WorkerProtocol_SnapshotInputStream by calling
 * WorkerProtocol_SnapshotInputStream.
 */
CORE_SDK_API WorkerProtocol_SnapshotInputStream*
WorkerProtocol_SnapshotInputStream_Create(const char* filename,
                                          const WorkerProtocol_SnapshotParameters* params);

/**
 * Deletes the SnapshotInputStream. Unlike SnapshotOutputStream,
 * we do not need two step destruction.
 */
CORE_SDK_API void
WorkerProtocol_SnapshotInputStream_Destroy(WorkerProtocol_SnapshotInputStream* input_stream);

/**
 * Returns zero (false) if the WorkerProtocol_SnapshotInputStream has reached the EOF
 * of the Snapshot.
 */
CORE_SDK_API uint8_t
WorkerProtocol_SnapshotInputStream_HasNext(WorkerProtocol_SnapshotInputStream* input_stream);

/**
 * Reads next WorkerProtocol_Entity* entity from input_stream.
 * WorkerProtocol_SnapshotInputStream_ReadEntity manages the memory for the returned
 * entity internally. The next call to WorkerProtocol_SnapshotInputStream_ReadEntity overwrites this
 * value.
 */
CORE_SDK_API const WorkerProtocol_Entity*
WorkerProtocol_SnapshotInputStream_ReadEntity(WorkerProtocol_SnapshotInputStream* input_stream);

/**
 * Must be called after any operation on WorkerProtocol_SnapshotInputStream to get the error
 * message associated with previous operation. If error is null, no error occured.
 *
 * Returns a read only const char* representation of the error message.
 */
CORE_SDK_API const char*
WorkerProtocol_SnapshotInputStream_GetError(WorkerProtocol_SnapshotInputStream* input_stream);

/**
 * Opens WorkerProtocol_SnapshotOutputStream stream. The caller must manage the memory of the
 * returned WorkerProtocol_SnapshotOutputStream* by calling
 * WorkerProtocol_SnapshotOutputStream_Destroy to write the EOF and release resources.
 *
 * If an error occurs, a pointer to a WorkerProtocol_SnapshotOutputStream is still returned.
 * Calling WorkerProtocol_SnapshotOutputStream_GetError with this pointer will return
 * an error message describing any error that occured. In the event of an error, the caller still
 * must release the memory of the WorkerProtocol_SnapshotOutputStream by calling
 * WorkerProtocol_SnapshotOutputStream_Destroy.
 */
CORE_SDK_API WorkerProtocol_SnapshotOutputStream*
WorkerProtocol_SnapshotOutputStream_Create(const char* filename,
                                           const WorkerProtocol_SnapshotParameters* params);

/** Closes the snapshot output stream and releases its resources. */
CORE_SDK_API void
WorkerProtocol_SnapshotOutputStream_Destroy(WorkerProtocol_SnapshotOutputStream* output_stream);

/**
 * Writes next entity_id, entity pair from input. Must call
 * WorkerProtocol_SnapshotOutputStream_GetError
 * to get any error that occured during operation.
 * Returns non-zero (true) if the write was successful.
 */
CORE_SDK_API uint8_t WorkerProtocol_SnapshotOutputStream_WriteEntity(
    WorkerProtocol_SnapshotOutputStream* output_stream, const WorkerProtocol_Entity* entity);

/**
 * Must be called after any operation on WorkerProtocol_SnapshotOutputStream to get the error
 * message associated with previous operation. If error is null, no error occured.
 *
 * Returns a read only const char* representation of the error message.
 */
CORE_SDK_API const char*
WorkerProtocol_SnapshotOutputStream_GetError(WorkerProtocol_SnapshotOutputStream* output_stream);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* WORKER_SDK_CORE_INCLUDE_IMPROBABLE_WORKER_PROTOCOL_H */
