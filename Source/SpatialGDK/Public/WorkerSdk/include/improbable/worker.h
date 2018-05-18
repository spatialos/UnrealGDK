// Copyright (c) Improbable Worlds Ltd, All Rights Reserved
#ifndef WORKER_SDK_CPP_INCLUDE_IMPROBABLE_WORKER_H
#define WORKER_SDK_CPP_INCLUDE_IMPROBABLE_WORKER_H
#include <improbable/collections.h>
#include <improbable/defaults.h>
#include <improbable/detail/worker_detail.h>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <memory>
#include <string>
#include <unordered_map>

namespace worker {
using EntityId = std::int64_t;
using ComponentId = std::uint32_t;
/** A type-erased registry of components. */
struct ComponentRegistry {
  virtual ~ComponentRegistry() {}
  /** Internal implementation detail; users should not need to call this. */
  virtual detail::ComponentInfo& GetInternalComponentInfo() const = 0;
};

/** A compile-time static list of components. */
template <typename... Args>
struct Components : ComponentRegistry, detail::AllComponentMetaclasses<Args...> {
  Components() {}
  ~Components() override {}
  /** Internal implementation detail; users should not need to call this. */
  detail::ComponentInfo& GetInternalComponentInfo() const override;
};

enum class NetworkConnectionType { kTcp = 0, kRaknet = 1 };
enum class LocatorCredentialsType { kLoginToken = 0, kSteam = 1 };
enum class LogLevel { kDebug = 1, kInfo = 2, kWarn = 3, kError = 4, kFatal = 5 };
enum class StatusCode {
  /** The request was successfully executed and returned a response. */
  kSuccess = 1,
  /**
   * The request timed out before a response was received. It can be retried, but carefully - this
   * usually means the deployment is overloaded, so some sort of backoff should be used to avoid
   * making the problem worse. This can also be caused by the target worker's handling code failing
   * to respond to the command at all, perhaps due to a bug in its implementation.
   */
  kTimeout = 2,
  /**
   * The target entity did not exist, or did not have the target component. This probably means
   * the entity either hasn't been created yet or has already been deleted. It might make sense to
   * retry the request if there is reason to believe the entity hasn't yet been created but will be
   * soon.
   */
  kNotFound = 3,
  /**
   * The request could not be executed by a worker, either because it lost authority while handling
   * the request, or because no worker was authoritative at all. Assuming the deployment isn't
   * irrecoverably broken (e.g. due to misconfigured loadbalancing or crash-looping workers) this
   * is a transient failure and can be retried immediately.
   */
  kAuthorityLost = 4,
  /**
   * The worker did not have the required permissions to make the request. Permissions do not change
   * at runtime, so it doesn't make sense to retry the request.
   */
  kPermissionDenied = 5,
  /**
   * The command was delivered successfully, but the handler rejected it. Either the command was
   * delivered to a worker that explicitly rejected it by calling Connection::SendCommandFailure, or
   * the request data was rejected as invalid by SpatialOS itself. In the latter case, in
   * particular,
   * Connection::SendCreateEntityRequest will return kApplicationError if an entity ID reservation
   * has expired, and Connection::SendEntityQueryResult will return kApplicationError if the result
   * set is incomplete.
   */
  kApplicationError = 6,
  /** Some other error occurred. This likely indicates a bug in SpatialOS and should be reported. */
  kInternalError = 7,
};
enum class Authority { kNotAuthoritative = 0, kAuthoritative = 1, kAuthorityLossImminent = 2 };

/** Type parameter for entity ID reservation request IDs. */
struct ReserveEntityIdRequest {
  ReserveEntityIdRequest() = delete;
};
/** Type parameter for multiple entity ID reservation request IDs. */
struct ReserveEntityIdsRequest {
  ReserveEntityIdsRequest() = delete;
};
/** Type parameter for entity creation request IDs. */
struct CreateEntityRequest {
  CreateEntityRequest() = delete;
};
/** Type parameter for entity deletion request IDs. */
struct DeleteEntityRequest {
  DeleteEntityRequest() = delete;
};
/** Type parameter for entity query request IDs. */
struct EntityQueryRequest {
  EntityQueryRequest() = delete;
};
/** Type parameter for outgoing entity command request IDs. */
template <typename>
struct OutgoingCommandRequest {
  OutgoingCommandRequest() = delete;
};
/** Type parameter for incoming entity command request IDs. */
template <typename>
struct IncomingCommandRequest {
  IncomingCommandRequest() = delete;
};

/**
 * Represents an ID for a request. The type parameter should be one of the marker types defined
 * above.
 */
template <typename>
struct RequestId {
  RequestId();
  explicit RequestId(std::uint32_t id);

  bool operator==(const RequestId&) const;
  bool operator!=(const RequestId&) const;

  /**
   * The underlying raw ID of the request. Only use this if you know what you
   * are doing; prefer to use the RequestId object instead.
   */
  std::uint32_t Id;
};

/** Parameters for configuring a RakNet connection. Used by NetworkParameters. */
struct RakNetNetworkParameters {
  /** Time (in milliseconds) that RakNet should use for its heartbeat protocol. */
  std::uint32_t HeartbeatTimeoutMillis = defaults::kRakNetHeartbeatTimeoutMillis;
};

/** Parameters for configuring a TCP connection. Used by NetworkParameters. */
struct TcpNetworkParameters {
  /** Number of multiplexed TCP connections. */
  std::uint8_t MultiplexLevel = defaults::kTcpMultiplexLevel;
  /** Size in bytes of the TCP send buffer. */
  std::uint32_t SendBufferSize = defaults::kTcpSendBufferSize;
  /** Size in bytes of the TCP receive buffer. */
  std::uint32_t ReceiveBufferSize = defaults::kTcpReceiveBufferSize;
  /** Whether to enable TCP_NODELAY. */
  bool NoDelay = defaults::kTcpNoDelay;
};

/** Parameters for configuring the network connection. */
struct NetworkParameters {
  /**
   * Set this flag to connect to SpatialOS using the externally-visible IP address. This flag must
   * be set when connecting externally (i.e. from outside the cloud) to a cloud deployment.
   */
  bool UseExternalIp = defaults::kUseExternalIp;
  /** Type of network connection to use when connecting to SpatialOS. */
  NetworkConnectionType ConnectionType = NetworkConnectionType::kTcp;
  /** Connection parameters specific to RakNet connections. */
  RakNetNetworkParameters RakNet;
  /** Connection parameters specific to TCP connections. */
  TcpNetworkParameters Tcp;
};

/**
 * Parameters for configuring protocol logging. If enabled, logs all protocol messages sent and
 * received.
 */
struct ProtocolLoggingParameters {
  /** Log file names are prefixed with this prefix, are numbered, and have the extension .log. */
  std::string LogPrefix = defaults::kLogPrefix;
  /**
   * Maximum number of log files to keep. Note that logs from any previous protocol logging
   * sessions will be overwritten.
   */
  std::uint32_t MaxLogFiles = defaults::kMaxLogFiles;
  /** Once the size of a log file reaches this size, a new log file is created. */
  std::uint32_t MaxLogFileSizeBytes = defaults::kMaxLogFileSizeBytes;
};

/** Parameters for creating a Connection and connecting to SpatialOS. */
struct ConnectionParameters {
  /** Worker type (platform). */
  std::string WorkerType;

  /** Parameters controlling the network connection to SpatialOS. */
  NetworkParameters Network;

  /**
   * Number of messages that can be stored on the send queue. When the send queue is full, calls to
   * Connection::Send functions can block.
   */
  std::uint32_t SendQueueCapacity = defaults::kSendQueueCapacity;
  /**
   * Number of messages that can be stored on the receive queue. When the receive queue is full,
   * SpatialOS can apply QoS and drop messages to the worker.
   */
  std::uint32_t ReceiveQueueCapacity = defaults::kReceiveQueueCapacity;
  /**
   * Number of messages logged by the SDK that can be stored in the log message queue. When the log
   * message queue is full, messages logged by the SDK can be dropped.
   */
  std::uint32_t LogMessageQueueCapacity = defaults::kLogMessageQueueCapacity;
  /**
   * The Connection tracks several internal metrics, such as send and receive queue statistics. This
   * parameter controls how frequently the Connection will return a MetricsOp reporting its built-in
   * metrics. If set to zero, this functionality is disabled.
   */
  std::uint32_t BuiltInMetricsReportPeriodMillis = defaults::kBuiltInMetricsReportPeriodMillis;

  /** Parameters for configuring protocol logging. */
  ProtocolLoggingParameters ProtocolLogging;
  /** Whether to enable protocol logging at startup. */
  bool EnableProtocolLoggingAtStartup = false;
};

/** Overrides the default interest settings for a particular entity and component. */
struct InterestOverride {
  /** Controls whether checkout is explicitly enabled or disabled. */
  bool IsInterested;
};

/** Parameters for authenticating using a SpatialOS login token. */
struct LoginTokenCredentials {
  /** The token would typically be provided on the command-line by the SpatialOS launcher. */
  std::string Token;
};

/** Parameters for authenticating using Steam credentials. */
struct SteamCredentials {
  /**
   * Steam ticket for the steam app ID and publisher key corresponding to the project name specified
   * in the LocatorParameters. Typically obtained from the steam APIs.
   */
  std::string Ticket;
  /**
   * Deployment tag to request access for. If non-empty, must match the following regex:
   * [A-Za-z0-9][A-Za-z0-9_]*
   */
  std::string DeploymentTag;
};

/** Parameters for authenticating and logging in to a SpatialOS deployment via the locator. */
struct LocatorParameters {
  /** The name of the SpatialOS project. */
  std::string ProjectName;
  /** Type of credentials to use when authenticating via the Locator. */
  LocatorCredentialsType CredentialsType;
  /** Parameters used if the CredentialsType is LOGIN_TOKEN. */
  LoginTokenCredentials LoginToken;
  /** Parameters used if the CredentialsType is STEAM. */
  SteamCredentials Steam;
};

/** Details for a specific deployment obtained via Locator::GetDeploymentList. */
struct Deployment {
  /** The name of the deployment. */
  std::string DeploymentName;
  /** The name of the assembly used by this deployment. */
  std::string AssemblyName;
  /** Description of this deployment. */
  std::string Description;
  /** Number of users currently connected to the deployment. */
  std::uint32_t UsersConnected;
  /** Total user capacity of the deployment. */
  std::uint32_t UsersCapacity;
};

/** A deployment list obtained via Locator::GetDeploymentList. */
struct DeploymentList {
  /** List of accessible deployments for the given project. */
  List<Deployment> Deployments;
  /** Will be non-empty if an error occurred. */
  Option<std::string> Error;
};

/** A queue status update when connecting to a deployment via Locator::Connect. */
struct QueueStatus {
  /** Position in the queue. Decreases as we advance to the front of the queue. */
  std::uint32_t PositionInQueue;
  /** Will be non-empty if an error occurred. */
  Option<std::string> Error;
};

/** Command parameters. Used to modify the behaviour of a command request. */
struct CommandParameters {
  /**
   * Allow command requests to bypass the bridge when this worker is authoritative over the target
   * entity-component.
   */
  bool AllowShortCircuit;
};

/**
 * A class representing the standard future concept. It can be used for both synchronous and
 * asynchronous interaction.
 */
template <typename T>
class Future {
public:
  // Noncopyable, but movable.
  Future(Future&&) = default;
  Future(const Future&) = delete;
  Future& operator=(Future&&) = default;
  Future& operator=(const Future&) = delete;

  /**
   * Waits for the result of the future to become available. If timeout_millis is empty, blocks
   * until the result is available. If timeout_millis is non-empty, blocks for at most that many
   * milliseconds.
   *
   * Returns true if the result is now available (in which case Get() can be called without
   * blocking); or false if the result is not available.
   */
  bool Wait(const Option<std::uint32_t>& timeout_millis);

  /**
   * Waits until the result of the future is available, and returns it. This method can only be
   * called once; calling Get() a second time is a fatal error.
   */
  T Get();

private:
  friend class Connection;
  friend class Locator;
  Future(const std::function<Option<T>(const Option<std::uint32_t>&)> get,
         const std::function<void()>& destroy);
  std::unique_ptr<detail::FutureImpl<T>> impl;
};

/**
 * A client which can be used to connect to a SpatialOS deployment via a locator service.
 * This is the standard flow used to connect a local worker to a cloud deployment. This object
 * should not be used concurrently by multiple threads.
 */
class Locator {
public:
  /**
   * Creates a client for the locator service.
   *
   * The hostname would typically be either "locator.improbable.io" (for production) or
   * "locator-staging.improbable.io" (for staging).
   */
  Locator(const std::string& hostname, const LocatorParameters& params);

  // Noncopyable, but movable.
  Locator(const Locator&) = delete;
  Locator(Locator&&) = default;
  Locator& operator=(const Locator&) = delete;
  Locator& operator=(Locator&&) = default;

  using QueueStatusCallback = std::function<bool(const QueueStatus&)>;

  /** Queries the current list of deployments for the project given in the LocatorParameters. */
  Future<DeploymentList> GetDeploymentListAsync();

  /**
   * Connects to a specific deployment. The deployment name should be obtained by calling
   * GetDeploymentList. The callback should return false to cancel queuing, or true to continue
   * queueing.
   */
  Future<Connection> ConnectAsync(const ComponentRegistry& registry,
                                  const std::string& deployment_name,
                                  const ConnectionParameters& params,
                                  const QueueStatusCallback& callback);

private:
  std::unique_ptr<detail::internal::WorkerProtocol_Locator,
                  void (*)(detail::internal::WorkerProtocol_Locator*)>
      locator;
};

/**
 * Worker Connection API. This is the main way of communicating with SpatialOS, sending and
 * receiving data. This object should not be used concurrently by multiple threads.
 */
class Connection {
public:
  /**
   * Connects to a SpatialOS deployment via a receptionist. This is the flow used to connect a
   * managed worker running in the cloud alongside the deployment, and also to connect any local
   * worker to a (local or remote) deployment via a locally-running receptionist.
   *
   * The hostname and port would typically be provided by SpatialOS on the command-line, if this is
   * a managed worker on the cloud, or otherwise be predetermined (e.g. localhost:7777 for the
   * default receptionist of a locally-running deployment).
   */
  static Future<Connection> ConnectAsync(const ComponentRegistry& registry,
                                         const std::string& hostname, std::uint16_t port,
                                         const std::string& worker_id,
                                         const ConnectionParameters& params);

  // Noncopyable, but movable.
  Connection(const Connection&) = delete;
  Connection(Connection&&) = default;
  Connection& operator=(const Connection&) = delete;
  Connection& operator=(Connection&&) = default;

  /**
   * Returns true if the Connection object was created correctly and has successfully connected
   * to SpatialOS.
   */
  bool IsConnected() const;

  /**
   * Returns the ID that was assigned to this worker at runtime.
   */
  std::string GetWorkerId() const;

  /**
   * Returns the attributes associated with this worker at runtime.
   */
  worker::List<std::string> GetWorkerAttributes() const;

  /**
   * Returns the value of the worker flag with the given name. If no worker flag exists with the
   * given name, an empty option is returned.
   *
   * Worker flags are remotely configurable and may change during the runtime of the worker,
   * including addition and deletion.
   */
  Option<std::string> GetWorkerFlag(const std::string& flag_name) const;

  /**
   * Retrieves the list of operations that have occurred since the last call to this function.
   *
   * If timeout_millis is non-zero, the function will block until there is at least one operation to
   * return, or the timeout has been exceeded. If the timeout is exceeded, an empty list will be
   * returned.
   *
   * If timeout_millis is zero the function is non-blocking.
   */
  OpList GetOpList(std::uint32_t timeout_millis);

  /** Sends a log message for the worker to SpatialOS. */
  void SendLogMessage(LogLevel level, const std::string& logger_name, const std::string& message,
                      const Option<EntityId>& entity_id = {});

  /**
   * Sends a set of metrics for the worker to SpatialOS. Typically this function should be called
   * periodically (e.g. once every few seconds) to report the worker's status.
   *
   * Typically, the caller should merge built-in SDK metrics reported by the last MetricsOp into
   * the Metrics parameter before calling SendMetrics.
   *
   * Since histogram metrics are diff-based, calling this function clears each histogram in the
   * Metrics parameter.
   */
  void SendMetrics(Metrics& metrics);

  /**
   * Requests SpatialOS to reserve an entity ID. Returns a request ID, which can be used to identify
   * a response to the request via the Dispatcher::OnReserveEntityIdResponse callback.
   *
   * If timeout_millis is not specified, the default timeout will be used.
   *
   * This function is DEPRECATED; use SendReserveEntityIdsRequest instead.
   */
  RequestId<ReserveEntityIdRequest>
  SendReserveEntityIdRequest(const Option<std::uint32_t>& timeout_millis);

  /**
   * Requests SpatialOS to reserve multiple entity IDs. Returns a request ID, which can be used to
   * identify a response to the request via the Dispatcher::OnReserveEntityIdResponse callback.
   *
   * If timeout_millis is not specified, the default timeout will be used.
   */
  RequestId<ReserveEntityIdsRequest>
  SendReserveEntityIdsRequest(std::uint32_t number_of_entity_ids,
                              const Option<std::uint32_t>& timeout_millis);

  /**
   * Requests SpatialOS to create an entity. The provided entity data is copied and can subsequently
   * be reused by the caller. Returns a request ID, which can be used to identify a response to the
   * request via the Dispatcher::OnCreateEntityResponse callback.
   *
   * If an entity ID is provided, it must have been reserved using
   * Connection::SendReserveEntityIdRequest.
   *
   * If timeout_millis is not specified, the default timeout will be used.
   */
  RequestId<CreateEntityRequest>
  SendCreateEntityRequest(const Entity& entity, const Option<EntityId>& entity_id,
                          const Option<std::uint32_t>& timeout_millis);

  /**
   * Requests SpatialOS to delete an entity. Returns a request ID, which can be used to identify a
   * response to the request via the Dispatcher::OnDeleteEntityResponse callback.
   *
   * If timeout_millis is not specified, the default timeout will be used.
   */
  RequestId<DeleteEntityRequest>
  SendDeleteEntityRequest(EntityId entity_id, const Option<std::uint32_t>& timeout_millis);

  /**
   * Queries SpatialOS for remote entity data. Returns a request ID, which can be used to identify a
   * response to the request via the Dispatcher::OnEntityQueryResponse callback.
   *
   * If timeout_millis is not specified, the default timeout will be used.
   */
  RequestId<EntityQueryRequest> SendEntityQueryRequest(const query::EntityQuery& entity_query,
                                                       const Option<std::uint32_t>& timeout_millis);

  /**
   * Sends a component interest update for the given entity to SpatialOS. By default, the worker
   * receives data for all entities according to the default component interests specified in its
   * bridge settings. This function overrides the default to explicitly add or remove interest for
   * particular components.
   *
   * Interest for components not present in the interest_overrides map is unaffected. Note also that
   * components over which the worker is authoritative are always received, regardless of interest
   * settings.
   */
  void SendComponentInterest(EntityId entity_id,
                             const Map<ComponentId, InterestOverride>& interest_overrides);

  /**
   * Sends an acknowledgement of the receipt of an AuthorityLossImminent authority change for a
   * component. Sending the acknowledgement signifies that this worker is ready to lose authority
   * over the component.
   */
  void SendAuthorityLossImminentAcknowledgement(EntityId entity_id, ComponentId component_id);

  /**
   * Sends an update for an entity's component to SpatialOS. The provided update object is copied
   * and may be subsequently reused by the caller. The template parameter T should be a generated
   * component API metaclass.
   *
   * Note that the sent component update is added as an operation to the operation list and will be
   * returned by a subsequent call to GetOpList().
   */
  template <typename T>
  void SendComponentUpdate(EntityId entity_id, const typename T::Update& update);

  /**
   * Sends an update for an entity's component to SpatialOS. The provided update object is
   * moved-from. The template parameter T should be a generated component API metaclass.
   *
   * Note that the sent component update is added as an operation to the operation list and will be
   * returned by a subsequent call to GetOpList().
   */
  template <typename T>
  void SendComponentUpdate(EntityId entity_id, typename T::Update&& update);

  /**
   * Sends a command request to a component on a specific target entity. The provided request object
   * is copied and may be subsequently reused by the caller. Returns a request ID, which can be used
   * to identify a response to the command via the Dispatcher::OnCommandResponse callback. The
   * template parameter T should be a generated command API metaclass.
   *
   * If timeout_millis is not specified, the default timeout will be used.
   */
  template <typename T>
  RequestId<OutgoingCommandRequest<T>>
  SendCommandRequest(EntityId entity_id, const typename T::Request& request,
                     const Option<std::uint32_t>& timeout_millis,
                     const CommandParameters& parameters = {false});

  /**
   * Sends a command request to a component on a specific target entity. The provided request object
   * is moved-from. Returns a request ID, which can be used to identify a response to the command
   * via the Dispatcher::OnCommandResponse callback. The template parameter T should be a generated
   * command API metaclass.
   *
   * If timeout_millis is not specified, the default timeout will be used.
   */
  template <typename T>
  RequestId<OutgoingCommandRequest<T>>
  SendCommandRequest(EntityId entity_id, typename T::Request&& request,
                     const Option<std::uint32_t>& timeout_millis,
                     const CommandParameters& parameters = {false});

  /**
   * Sends a response to an incoming command request for a component on an entity over which this
   * worker has authority. The provided response object is copied and may be subsequently reused by
   * the caller. The request ID should match an incoming command request via the
   * Dispatcher::OnCommandRequest callback. The template parameter T should be a generated command
   * API metaclass.
   */
  template <typename T>
  void SendCommandResponse(RequestId<IncomingCommandRequest<T>> request_id,
                           const typename T::Response& response);

  /**
   * Sends a response to an incoming command request for a component on an entity over which this
   * worker has authority. The provided response object is moved-from. The request ID should match
   * an incoming command request via the Dispatcher::OnCommandRequest callback. The template
   * parameter T should be a generated command API metaclass.
   */
  template <typename T>
  void SendCommandResponse(RequestId<IncomingCommandRequest<T>> request_id,
                           typename T::Response&& response);

  /**
   * Explicitly fails an incoming command request for a component on an entity over which this
   * worker has authority. The request ID should match an incoming command request via the
   * Dispatcher::OnCommandRequest callback. The calling worker will receive a
   * command response with status code StatusCode::kApplicationError.
   */
  template <typename T>
  void SendCommandFailure(RequestId<IncomingCommandRequest<T>> request_id,
                          const std::string& message);

  /**
   * Enables or disables protocol logging. Logging uses the parameters specified when the Connection
   * was created. Enabling it when already enabled, or disabling it when already disabled, do
   * nothing.
   *
   * Note that logs from any previous protocol logging sessions will be overwritten.
   */
  void SetProtocolLoggingEnabled(bool enabled);

private:
  friend class Locator;
  explicit Connection(const detail::ComponentInfo& component_info,
                      detail::internal::WorkerProtocol_Connection* connection);
  const detail::ComponentInfo& component_info;
  std::unique_ptr<detail::internal::WorkerProtocol_Connection,
                  void (*)(detail::internal::WorkerProtocol_Connection*)>
      connection;
};

/**
 * An opaque list of operations retrieved from Connection::GetOpList(). It is usually passed to
 * Dispatcher::Process(), which dispatches the operations to the appropriate callbacks.
 */
class OpList {
public:
  // Noncopyable, but movable.
  OpList(const OpList&) = delete;
  OpList(OpList&&) = default;
  OpList& operator=(const OpList&) = delete;
  OpList& operator=(OpList&&) = default;

private:
  friend class Connection;
  friend class Dispatcher;
  explicit OpList(detail::internal::WorkerProtocol_OpList* op_list);
  std::unique_ptr<detail::internal::WorkerProtocol_OpList,
                  void (*)(detail::internal::WorkerProtocol_OpList*)>
      op_list;
};

/**
 * A histogram metric tracks observations of a given value by bucket. This corresponds to a
 * Prometheus histogram metric. This object should not be used concurrently by multiple threads.
 */
class HistogramMetric {
public:
  /** A histogram bucket. */
  struct Bucket {
    /** The upper bound. */
    double UpperBound;
    /** The number of observations that were less than or equal to the upper bound. */
    std::uint32_t Samples;
  };

  /**
   * Creates a histogram with the given bucket boundaries. Each bucket boundary is an upper bound;
   * the bucket tracks all observations with a value less than or equal to the bound. A final bucket
   * with a boundary of +INF is added automatically.
   */
  HistogramMetric(const List<double>& bucket_upper_bounds);
  /** Creates a histogram with a single bucket. */
  HistogramMetric();

  /** Clears all recorded oservations. Automatically called by Connection::SendMetrics. */
  void ClearObservations();
  /** Records a sample and adds it to the corresponding buckets. */
  void RecordObservation(double value);

  /** Returns the buckets for inspection. */
  const List<Bucket>& Buckets() const;
  /** Returns the sum of all observations since the last call to ClearObservations. */
  double Sum() const;

private:
  List<Bucket> buckets;
  double sum;
};

/**
 * A set of metrics sent up from a worker to SpatialOS.
 *
 * Keys for the contained metrics should match the following regex: [a-zA-Z_][a-zA-Z0-9_]*
 */
struct Metrics {
  /** Copies all metrics from another Metrics object into this one, overwriting existing values. */
  void Merge(const Metrics& metrics);

  /**
   * The load value of this worker. A value of 0 indicates that the worker is completely unloaded;
   * a value greater than 1 indicates that the worker is overloaded. The load value directs
   * SpatialOS's load-balancing strategy for managed workers (spinning them up, spinning them down,
   * and assigning work between them).
   */
  Option<double> Load;

  /** Gauge metrics for the worker. */
  Map<std::string, double> GaugeMetrics;
  /** Histogram metrics for the worker. */
  Map<std::string, HistogramMetric> HistogramMetrics;
};

/**
 * Stores the complete data for an entity's components.
 *
 * Note that an Entity object is simply a local data structure, and changes made here are not
 * automatically reflected across the SpatialOS simulation. To synchronize component state with
 * SpatialOS, use Connection::SendComponentUpdate.
 *
 * This object should not be modified concurrently by multiple threads.
 */
class Entity {
public:
  Entity() {}
  Entity(const Entity& entity);
  Entity(Entity&&) = default;
  Entity& operator=(const Entity& entity);
  Entity& operator=(Entity&&) = default;

  /**
   * Retrieves data for the given component. Returns an empty option if the entity does not have the
   * given component. The template parameter T should be a generated component API metaclass.
   */
  template <typename T>
  Option<typename T::Data&> Get();

  /**
   * Retrieves data for the given component. Returns an empty option if the entity does not have the
   * given component. The template parameter T should be a generated component API metaclass.
   */
  template <typename T>
  Option<const typename T::Data&> Get() const;

  /**
   * Creates the given component by copying the provided initial data. Has no effect if the entity
   * already has the given component. The template parameter T should be a generated component API
   * metaclass.
   */
  template <typename T>
  void Add(const typename T::Data& data);

  /**
   * Creates the given component by moving-from the provided initial data. Has no effect if the
   * entity already has the given component. The template parameter T should be a generated
   * component API metaclass.
   */
  template <typename T>
  void Add(typename T::Data&& data);

  /**
   * Applies an update to the given component. Has no effect if the entity does not have the given
   * component. The template parameter T should be a generated component API metaclass.
   */
  template <typename T>
  void Update(const typename T::Update& update);

  /**
   * Removes a component. The template parameter T should be a generated component API metaclass.
   */
  template <typename T>
  void Remove();

  /** Returns a list of the component IDs of the components present in this entity. */
  List<ComponentId> GetComponentIds() const;

private:
  Map<ComponentId, std::unique_ptr<detail::ComponentStorageBase>> components;
};

namespace query {
/** Constrains a query to match only entities with a particular ID. */
struct EntityIdConstraint {
  worker::EntityId EntityId;
};

/** Constraints a query to match only entities that have a specific component. */
struct ComponentConstraint {
  worker::ComponentId ComponentId;
};

/** Constrains a query to match only entities whose position lies within a given sphere. */
struct SphereConstraint {
  double X;
  double Y;
  double Z;
  double Radius;
};

struct AndConstraint;
struct OrConstraint;
struct NotConstraint;

using Constraint = Variant<EntityIdConstraint, SphereConstraint, ComponentConstraint, AndConstraint,
                           OrConstraint, NotConstraint>;

/** Constrains a query by the conjuction of one or more constraints. */
struct AndConstraint : List<Constraint> {
  using List<Constraint>::List;
};

/** Constrains a query by the disjuction of one or more constraints. */
struct OrConstraint : List<Constraint> {
  using List<Constraint>::List;
};

struct NotConstraint {
  query::Constraint Constraint;
};

/** Indicates that a query should return the number of entities it matched. */
struct CountResultType {};

/** Indicates that a query should return a component data snapshot for each matched entity. */
struct SnapshotResultType {
  /** If nonempty, filters the components returned in the snapshot for each entity. */
  Option<List<ComponentId>> ComponentIds;
};

using ResultType = Variant<CountResultType, SnapshotResultType>;

/** Represents a global query for entity data across the simulation. */
struct EntityQuery {
  query::Constraint Constraint;
  query::ResultType ResultType;
};
}  // ::query

/**
 * Data for an operation that indicates the Connection has disconnected and can no longer be
 * used.
 */
struct DisconnectOp {
  std::string Reason;
};

/** Data for an operation that indicates a worker flag has been updated. */
struct FlagUpdateOp {
  std::string Name;
  /** Either contains the new flag value, or empty if the flag has been deleted. */
  Option<std::string> Value;
};

/** Data for an operation that provides a log message from the SDK. */
struct LogMessageOp {
  LogLevel Level;
  std::string Message;
};

/** Data for an operation that provides a report on built-in metrics from the SDK. */
struct MetricsOp {
  worker::Metrics Metrics;
};

/**
 * Data for an operation that indicates the message stream received by the worker is entering or
 * leaving a critical section.
 *
 * Nested critical sections are currently not supported by the protocol.
 */
struct CriticalSectionOp {
  /** Whether the protocol is entering a critical section (true) or leaving it (false). */
  bool InCriticalSection;
};

/**
 * Data for an operation that indicates an entity has been added to the worker's view of the
 * simulation.
 */
struct AddEntityOp {
  /**
   * The ID of the entity that was added to the worker's view of the simulation.
   */
  worker::EntityId EntityId;
};

/**
 * Data for an operation that indicates an entity has been removed from the worker's view of the
 * simulation.
 */
struct RemoveEntityOp {
  worker::EntityId EntityId;
};

/** A response indicating the result of an entity ID reservation request. */
struct ReserveEntityIdResponseOp {
  /**
   * The outgoing request ID for which there was a response. Matches a previous call to
   * Connection::SendReserveEntityIdRequest.
   */
  worker::RequestId<ReserveEntityIdRequest> RequestId;
  /** Status code of the response. */
  worker::StatusCode StatusCode;
  /** A description of the status. Will contain the reason for failure if unsuccessful. */
  std::string Message;
  /**
   * If successful, a newly allocated entity ID, which is guaranteed to be unused in the current
   * deployment.
   */
  Option<worker::EntityId> EntityId;
};

/** A response indicating the result of a multiple entity ID reservation request. */
struct ReserveEntityIdsResponseOp {
  /**
   * The outgoing request ID for which there was a response. Matches a previous call to
   * Connection::SendReserveEntityIdsRequest.
   */
  worker::RequestId<ReserveEntityIdsRequest> RequestId;
  /** Status code of the response. */
  worker::StatusCode StatusCode;
  /** A description of the status. Will contain the reason for failure if unsuccessful. */
  std::string Message;
  /**
   * If successful, an ID which is the first in a contiguous range of newly allocated entity
   * IDs which are guaranteed to be unused in the current deployment.
   */
  Option<worker::EntityId> FirstEntityId;
  /** If successful, the number of IDs reserved in the contiguous range, otherwise 0. */
  std::size_t NumberOfEntityIds;
};

/** A response indicating the result of an entity creation request. */
struct CreateEntityResponseOp {
  /**
   * The outgoing request ID for which there was a response. Matches a previous call to
   * Connection::SendCreateEntityRequest.
   */
  worker::RequestId<CreateEntityRequest> RequestId;
  /**
   * Status code of the response. If the status code is StatusCode::kApplicationError, the entity
   * ID reservation has expired and must be retried.
   */
  worker::StatusCode StatusCode;
  /** A description of the status. Will contain the reason for failure if unsuccessful. */
  std::string Message;
  /** If successful, the entity ID of the newly created entity. */
  Option<worker::EntityId> EntityId;
};

/** A response indicating the result of an entity deletion request. */
struct DeleteEntityResponseOp {
  /**
   * The outgoing request ID for which there was a response. Matches a previous call to
   * Connection::SendDeleteEntityRequest.
   */
  worker::RequestId<DeleteEntityRequest> RequestId;
  /** The ID of the target entity of this request. */
  worker::EntityId EntityId;
  /** Status code of the response. */
  worker::StatusCode StatusCode;
  /** A description of the status. Will contain the reason for failure if unsuccessful. */
  std::string Message;
};

/** A response indicating the result of an entity query request. */
struct EntityQueryResponseOp {
  /**
   * The outgoing request ID for which there was a response. Matches a previous call to
   * Connection::SendEntityQueryRequest.
   */
  worker::RequestId<EntityQueryRequest> RequestId;
  /** Status code of the response. */
  worker::StatusCode StatusCode;
  /** A description of the status. Will contain the reason for failure if unsuccessful. */
  std::string Message;
  /**
   * The number of entities that matched the query.
   *
   * Note that a best-effort attempt is made to count the entities when the status code is
   * StatusCode::kApplicationError. In this case, the count can still be non-zero, but should be
   * considered a lower bound (i.e. there might be entities matching the query that were not
   * counted).
   */
  std::size_t ResultCount;
  /**
   * The result of the query. Not used for CountResultType queries.
   *
   * Note that a best-effort attempt is made to get results when the status code is
   * StatusCode::kApplicationError. In this case, the result can still be non-empty, but should be
   * considered incomplete (i.e. there might be entities matching the query that were not returned).
   */
  Map<EntityId, Entity> Result;
};

/**
 * Data for an operation that indicates a component has been added to an existing entity in the
 * worker's view of the simulation. The template parameter T should be a generated component
 * metaclass.
 */
template <typename T>
struct AddComponentOp {
  worker::EntityId EntityId;
  const typename T::Data& Data;
};

/**
 * Data for an operation that indicates a component has been removed from an existing entity in the
 * worker's view of the simulation.
 */
struct RemoveComponentOp {
  worker::EntityId EntityId;
};

/**
 * Data for an operation that indicates the worker's authority over a component for an entity has
 * been changed.
 */
struct AuthorityChangeOp {
  worker::EntityId EntityId;
  worker::Authority Authority;
};

/**
 * Data for an operation that indicates the component for an entity has been updated. The template
 * parameter T should be a generated component metaclass.
 */
template <typename T>
struct ComponentUpdateOp {
  worker::EntityId EntityId;
  const typename T::Update& Update;
};

/**
 * Data for an operation that indicates a command request has been received for a component on an
 * entity over which this worker has authority. The template parameter T should be a generated
 * component API metaclass. The worker should respond to the command by calling
 * Connection::SendCommandResponse<T> with the given request ID.
 */
template <typename T>
struct CommandRequestOp {
  /**
   * The incoming request ID, which should be passed to Connection::SendCommandResponse in order to
   * respond to this request.
   */
  worker::RequestId<IncomingCommandRequest<T>> RequestId;
  /** The ID of the target entity of this request. */
  worker::EntityId EntityId;
  /**
   * An upper bound on the timeout of this request. Any response sent after the timeout has expired
   * will be ignored by the SDK.
   */
  std::uint32_t TimeoutMillis;
  /** The ID of the worker that initiated this request. */
  std::string CallerWorkerId;
  /** The set of attributes of the worker that initiated this request. */
  List<std::string> CallerAttributeSet;
  /** The request data. */
  const typename T::Request& Request;
};

/**
 * Data for an operation that indicates a command response has been received for a request
 * previously issued by this worker. The template parameter T should be a generated component API
 * metaclass. The request ID will match a previous call to Connection::SendCommandRequest<T>.
 */
template <typename T>
struct CommandResponseOp {
  /**
   * The outgoing request ID, which matches a the request ID returned by a previous call to
   * Connection::SendCommandRequest.
   */
  worker::RequestId<OutgoingCommandRequest<T>> RequestId;
  /** The target entity ID of the original request. */
  worker::EntityId EntityId;
  /** The status code of the command response. */
  worker::StatusCode StatusCode;
  /** A description of the status. Will contain the reason for failure if unsuccessful. */
  std::string Message;
  /** The command response data. Present exactly when the status code is StatusCode::kSuccess. */
  Option<const typename T::Response&> Response;
};

/**
 * A Dispatcher processes OpLists retrieved from the Connection and invokes appropriate callbacks.
 * This object should not be modified concurrently by multiple threads.
 */
class Dispatcher {
public:
  template <typename T>
  using Callback = std::function<void(const T&)>;
  using CallbackKey = std::uint64_t;

  Dispatcher(const ComponentRegistry& registry);

  // Not copyable or movable.
  Dispatcher(const Dispatcher&) = delete;
  Dispatcher(Dispatcher&&) = delete;
  Dispatcher& operator=(const Dispatcher&) = delete;
  Dispatcher& operator=(Dispatcher&&) = delete;

  /**
   * Registers a callback to be invoked when the Connection has disconnected and can no longer be
   * used.
   */
  CallbackKey OnDisconnect(const Callback<DisconnectOp>& callback);

  /** Registers a callback to be invoked whhen a worker flag is updated. */
  CallbackKey OnFlagUpdate(const Callback<FlagUpdateOp>& callback);

  /** Registers a callback to be invoked when the SDK logs a message. */
  CallbackKey OnLogMessage(const Callback<LogMessageOp>& callback);

  /** Registers a callback to be invoked when the SDK reports built-in metrics. */
  CallbackKey OnMetrics(const Callback<MetricsOp>& callback);

  /**
   * Registers a callback to be invoked when the message stream enters or leaves a critical section.
   */
  CallbackKey OnCriticalSection(const Callback<CriticalSectionOp>& callback);

  /**
   * Registers a callback to be invoked when an entity is added to the worker's view of the
   * simulation.
   */
  CallbackKey OnAddEntity(const Callback<AddEntityOp>& callback);

  /**
   * Registers a callback to be invoked when an entity is removed from the worker's view of the
   * simulation.
   */
  CallbackKey OnRemoveEntity(const Callback<RemoveEntityOp>& callback);

  /**
   * Registers a callback to be invoked when an entity ID reservation response is received.
   *
   * This function is DEPRECATED; use OnReserveEntityIdsResponse in conjunction with
   * Connection::SendReserveEntityIdsRequest instead.
   */
  CallbackKey OnReserveEntityIdResponse(const Callback<ReserveEntityIdResponseOp>& callback);

  /**
   * Registers a callback to be invoked when a multiple entity ID reservation response is received.
   */
  CallbackKey OnReserveEntityIdsResponse(const Callback<ReserveEntityIdsResponseOp>& callback);

  /** Registers a callback to be invoked when an entity creation response is received. */
  CallbackKey OnCreateEntityResponse(const Callback<CreateEntityResponseOp>& callback);

  /** Registers a callback to be invoked when an entity delete response is received. */
  CallbackKey OnDeleteEntityResponse(const Callback<DeleteEntityResponseOp>& callback);

  /** Registers a callback to be invoked when an entity query response is recieved. */
  CallbackKey OnEntityQueryResponse(const Callback<EntityQueryResponseOp>& callback);

  /**
   * Registers a callback to be invoked when a particular component (indicated by the template
   * parameter T, which should be a generated component API metaclass) is added to an existing
   * entity in the worker's view fo the sumulation.
   */
  template <typename T>
  CallbackKey OnAddComponent(const Callback<AddComponentOp<T>>& callback);

  /**
   * Registers a callback to be invoked when a particular component (indicated by the template
   * parameter T, which should be a generated component API metaclass) is removed from an existing
   * entity in the worker's view of the simulation.
   */
  template <typename T>
  CallbackKey OnRemoveComponent(const Callback<RemoveComponentOp>& callback);

  /**
   * Registers a callback to be invoked when the worker is granted authority over a particular
   * component (indicated by the template parameter T, which should be a generated component API
   * metaclass) for some entity, or when the worker's authority over that component is revoked.
   */
  template <typename T>
  CallbackKey OnAuthorityChange(const Callback<AuthorityChangeOp>& callback);

  /**
   * Registers a callback to be invoked when a particular component (indicated by the template
   * parameter T, which should be a generated component API metaclass) is updated for an entity.
   */
  template <typename T>
  CallbackKey OnComponentUpdate(const Callback<ComponentUpdateOp<T>>& callback);

  /**
   * Registers a callback to be invoked when a command request is received for a particular
   * component (indicated by the template parameter T, which should be a generated command API
   * metaclass).
   */
  template <typename T>
  CallbackKey OnCommandRequest(const Callback<CommandRequestOp<T>>& callback);

  /**
   * Registers a callback to be invoked when a command response is received for a particular
   * component (indicated by the template parameter T, which should be a generated command API
   * metaclass).
   */
  template <typename T>
  CallbackKey OnCommandResponse(const Callback<CommandResponseOp<T>>& callback);

  /**
   * Unregisters a callback identified by its CallbackKey, as returned from the registration
   * function. If the key does not exist, the application will terminate.
   */
  void Remove(CallbackKey key);

  /** Processes an OpList and invokes registered callbacks. */
  void Process(const OpList& op_list) const;

private:
  detail::DispatcherImpl impl;
};

/**
 * Load a snapshot from a file. Returns an error message if an error occurred.
 * This function is DEPRECATED; use SnapshotInputStream instead.
 */
Option<std::string> LoadSnapshot(const ComponentRegistry& registry, const std::string& path,
                                 std::unordered_map<EntityId, Entity>& entities_output);

/**
 * Saves a snapshot to a file. Returns an error message if an error occurred.
 * This function is DEPRECATED; use SnapshotOutputStream instead.
 */
Option<std::string> SaveSnapshot(const ComponentRegistry& registry, const std::string& path,
                                 const std::unordered_map<EntityId, Entity>& entities);

/** A stream for reading snapshots one entity at a time. */
class SnapshotInputStream {
public:
  /** Creates a SnapshotInputStream to read the snapshot file from the given path. */
  explicit SnapshotInputStream(const ComponentRegistry& registry, const std::string& path);

  /** Returns true if SnapshotInputStream has not reached EOF. */
  bool HasNext();

  /**
   * Loads the next EntityId and Entity pair from the Snapshot. Returns an
   * error message if error occurred.
   */
  Option<std::string> ReadEntity(EntityId& entity_id, Entity& entity);

private:
  const detail::ComponentInfo& component_info;
  std::unique_ptr<detail::internal::WorkerProtocol_SnapshotInputStream,
                  void (*)(detail::internal::WorkerProtocol_SnapshotInputStream*)>
      input_stream;
};

/** A stream for writing snapshots one entity at a time. */
class SnapshotOutputStream {
public:
  /** Creates a SnapshotOutputStream to read the snapshot file from the given path. */
  explicit SnapshotOutputStream(const ComponentRegistry& registry, const std::string& path);

  /*
   * Writes the EntityId and Entity pair to the output stream. Returns an
   * error message if error occurred.
   */
  Option<std::string> WriteEntity(EntityId entity_id, const Entity& entity);

private:
  const detail::ComponentInfo& component_info;
  std::unique_ptr<detail::internal::WorkerProtocol_SnapshotOutputStream,
                  void (*)(detail::internal::WorkerProtocol_SnapshotOutputStream*)>
      output_stream;
};

/**
 * Invokes Accept<T>() on the provided handler for the component in the given
 * list of components whose ID matches the given component ID.
 */
template <typename... T, typename Handler>
void ForComponent(const Components<T...>& components, ComponentId component_id, Handler&& handler);

/**
 * Invokes Accept<T>() on the provided handler for each component in the given list of components.
 */
template <typename... T, typename Handler>
void ForEachComponent(const Components<T...>& components, Handler&& handler);

}  // ::worker

// Implementations files.
#include <improbable/detail/callbacks.i.h>
#include <improbable/detail/components.i.h>
#include <improbable/detail/connection.i.h>
#include <improbable/detail/dispatcher.i.h>
#include <improbable/detail/dynamic.i.h>
#include <improbable/detail/entity.i.h>
#include <improbable/detail/future.i.h>
#include <improbable/detail/metrics.i.h>
#include <improbable/detail/request_id.i.h>
#include <improbable/detail/snapshot.i.h>
#endif  // WORKER_SDK_CPP_INCLUDE_IMPROBABLE_WORKER_H
