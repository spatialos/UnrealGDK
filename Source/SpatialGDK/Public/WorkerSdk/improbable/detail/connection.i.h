// Copyright (c) Improbable Worlds Ltd, All Rights Reserved
#ifndef WORKER_SDK_CPP_INCLUDE_IMPROBABLE_DETAIL_CONNECTION_I_H
#define WORKER_SDK_CPP_INCLUDE_IMPROBABLE_DETAIL_CONNECTION_I_H
#include <improbable/detail/client_handle.i.h>
#include <improbable/detail/extract.i.h>
#include <improbable/worker.h>
#include <cstdint>
#include <functional>
#include <memory>
#include <vector>

namespace worker {
namespace detail {

inline void
LocatorDeploymentListThunk(void* user_data,
                           const internal::WorkerProtocol_DeploymentList* deployment_list) {
  const auto& callback = *static_cast<std::function<void(const DeploymentList&)>*>(user_data);
  DeploymentList wrapper;
  for (std::uint32_t i = 0; i < deployment_list->DeploymentCount; ++i) {
    const auto& protocol_deployment = deployment_list->Deployment[i];
    wrapper.Deployments.emplace_back();
    auto& deployment = wrapper.Deployments.back();
    deployment.DeploymentName = protocol_deployment.DeploymentName;
    deployment.AssemblyName = protocol_deployment.AssemblyName;
    deployment.Description = protocol_deployment.Description;
    deployment.UsersConnected = protocol_deployment.UsersConnected;
    deployment.UsersCapacity = protocol_deployment.UsersCapacity;
  }
  if (deployment_list->Error) {
    wrapper.Error.emplace(deployment_list->Error);
  }
  callback(wrapper);
}

inline std::uint8_t
LocatorQueueStatusThunk(void* user_data, const internal::WorkerProtocol_QueueStatus* queue_status) {
  const auto& callback = *static_cast<Locator::QueueStatusCallback*>(user_data);
  QueueStatus wrapper;
  wrapper.PositionInQueue = queue_status->PositionInQueue;
  if (queue_status->Error) {
    wrapper.Error.emplace(queue_status->Error);
  }
  return callback(wrapper) ? 1 : 0;
}

inline void ConnectionGetWorkerFlagThunk(void* user_data, const char* value) {
  const auto& callback = *static_cast<std::function<void(const Option<std::string>&)>*>(user_data);
  Option<std::string> cpp_flag_value;
  if (value) {
    cpp_flag_value.emplace(value);
  }
  callback(cpp_flag_value);
}

inline internal::WorkerProtocol_LocatorParameters
ConvertLocatorParameters(const LocatorParameters& params) {
  internal::WorkerProtocol_LocatorParameters protocol_params = {};
  protocol_params.ProjectName = params.ProjectName.c_str();
  if (params.CredentialsType == LocatorCredentialsType::kLoginToken) {
    protocol_params.Flags |= internal::WORKER_PROTOCOL_LOCATOR_LOGIN_TOKEN_CREDENTIALS;
    protocol_params.LoginToken.Token = params.LoginToken.Token.c_str();
  }
  if (params.CredentialsType == LocatorCredentialsType::kSteam) {
    protocol_params.Flags |= internal::WORKER_PROTOCOL_LOCATOR_STEAM_CREDENTIALS;
    protocol_params.Steam.Ticket = params.Steam.Ticket.c_str();
    protocol_params.Steam.DeploymentTag = params.Steam.DeploymentTag.c_str();
  }
  return protocol_params;
}

internal::WorkerProtocol_ConnectionParameters inline ConvertConnectionParameters(
    const ConnectionParameters& params, const detail::ComponentInfo& component_info) {
  internal::WorkerProtocol_ConnectionParameters protocol_params = {};
  protocol_params.WorkerType = params.WorkerType.c_str();

  protocol_params.Network.Flags =
      static_cast<std::uint8_t>(params.Network.ConnectionType == NetworkConnectionType::kTcp
                                    ? internal::WORKER_PROTOCOL_NETWORK_TCP
                                    : internal::WORKER_PROTOCOL_NETWORK_RAKNET);
  if (params.Network.UseExternalIp) {
    protocol_params.Network.Flags =
        protocol_params.Network.Flags | internal::WORKER_PROTOCOL_NETWORK_USE_EXTERNAL_IP;
  }

  protocol_params.Network.Tcp.MultiplexLevel = params.Network.Tcp.MultiplexLevel;
  protocol_params.Network.Tcp.NoDelay = params.Network.Tcp.NoDelay;
  protocol_params.Network.Tcp.SendBufferSize = params.Network.Tcp.SendBufferSize;
  protocol_params.Network.Tcp.ReceiveBufferSize = params.Network.Tcp.ReceiveBufferSize;
  protocol_params.Network.RakNet.HeartbeatTimeoutMillis =
      params.Network.RakNet.HeartbeatTimeoutMillis;
  protocol_params.Sdk.SendQueueCapacity = params.SendQueueCapacity;
  protocol_params.Sdk.ReceiveQueueCapacity = params.ReceiveQueueCapacity;
  protocol_params.Sdk.LogMessageQueueCapacity = params.LogMessageQueueCapacity;
  protocol_params.Sdk.BuiltInMetricsReportPeriodMillis = params.BuiltInMetricsReportPeriodMillis;
  protocol_params.Sdk.EnableProtocolLoggingAtStartup = params.EnableProtocolLoggingAtStartup;
  protocol_params.Sdk.LogPrefix = params.ProtocolLogging.LogPrefix.c_str();
  protocol_params.Sdk.MaxLogFileSizeBytes = params.ProtocolLogging.MaxLogFileSizeBytes;
  protocol_params.Sdk.MaxLogFiles = params.ProtocolLogging.MaxLogFiles;

  protocol_params.ComponentVtableCount = static_cast<std::uint32_t>(component_info.Vtables.size());
  protocol_params.ComponentVtable =
      reinterpret_cast<const internal::WorkerProtocol_ComponentVtable*>(
          component_info.Vtables.data());
  return protocol_params;
}

internal::WorkerProtocol_Constraint inline ConvertConstraint(
    const query::Constraint& constraint,
    std::vector<std::unique_ptr<internal::WorkerProtocol_Constraint[]>>& storage) {
  internal::WorkerProtocol_Constraint result = {};
  if (const auto* entity_id = constraint.data<query::EntityIdConstraint>()) {
    result.ConstraintType = internal::WORKER_PROTOCOL_CONSTRAINT_TYPE_ENTITY_ID;
    result.EntityIdConstraint.EntityId = entity_id->EntityId;
  } else if (const auto* component = constraint.data<query::ComponentConstraint>()) {
    result.ConstraintType = internal::WORKER_PROTOCOL_CONSTRAINT_TYPE_COMPONENT;
    result.ComponentConstraint.ComponentId = component->ComponentId;
  } else if (const auto* sphere = constraint.data<query::SphereConstraint>()) {
    result.ConstraintType = internal::WORKER_PROTOCOL_CONSTRAINT_TYPE_SPHERE;
    result.SphereConstraint.PositionX = sphere->X;
    result.SphereConstraint.PositionY = sphere->Y;
    result.SphereConstraint.PositionZ = sphere->Z;
    result.SphereConstraint.Radius = sphere->Radius;
  } else if (const auto* and_constraint = constraint.data<query::AndConstraint>()) {
    auto size = static_cast<std::uint32_t>(and_constraint->size());
    storage.emplace_back(new internal::WorkerProtocol_Constraint[size]);
    auto ptr = storage.back().get();
    for (std::uint32_t i = 0; i < size; ++i) {
      ptr[i] = ConvertConstraint((*and_constraint)[i], storage);
    }

    result.ConstraintType = internal::WORKER_PROTOCOL_CONSTRAINT_TYPE_AND;
    result.AndConstraint.ConstraintCount = size;
    result.AndConstraint.Constraint = ptr;
  } else if (const auto* or_constraint = constraint.data<query::OrConstraint>()) {
    auto size = static_cast<std::uint32_t>(or_constraint->size());
    storage.emplace_back(new internal::WorkerProtocol_Constraint[size]);
    auto ptr = storage.back().get();
    for (std::uint32_t i = 0; i < size; ++i) {
      ptr[i] = ConvertConstraint((*or_constraint)[i], storage);
    }

    result.ConstraintType = internal::WORKER_PROTOCOL_CONSTRAINT_TYPE_OR;
    result.OrConstraint.ConstraintCount = size;
    result.OrConstraint.Constraint = ptr;
  } else if (const auto* not_constraint = constraint.data<query::NotConstraint>()) {
    storage.emplace_back(new internal::WorkerProtocol_Constraint[1]);
    auto ptr = storage.back().get();
    ptr[0] = ConvertConstraint(not_constraint->Constraint, storage);

    result.ConstraintType = internal::WORKER_PROTOCOL_CONSTRAINT_TYPE_NOT;
    result.NotConstraint.Constraint = ptr;
  }
  return result;
}

inline void SendClientComponentUpdate(internal::WorkerProtocol_Connection* connection,
                                      EntityId entity_id, ComponentId component_id,
                                      const ClientHandleBase* update) {
  internal::WorkerProtocol_ComponentHandle component_update{component_id, update};
  internal::WorkerProtocol_Connection_SendComponentUpdate(connection, entity_id, &component_update,
                                                          /* legacy semantics */ 0);
}

inline std::uint32_t SendClientCommandRequest(internal::WorkerProtocol_Connection* connection,
                                              EntityId entity_id, ComponentId component_id,
                                              const ClientHandleBase* request,
                                              std::uint32_t command_id,
                                              const Option<std::uint32_t>& timeout_millis,
                                              const CommandParameters& parameters) {
  internal::WorkerProtocol_ComponentHandle command_request{component_id, request};
  internal::WorkerProtocol_CommandParameters command_parameters{
      static_cast<std::uint8_t>(parameters.AllowShortCircuit)};
  return internal::WorkerProtocol_Connection_SendCommandRequest(
      connection, entity_id, &command_request, command_id, timeout_millis.data(),
      &command_parameters);
}

inline void SendClientCommandResponse(internal::WorkerProtocol_Connection* connection,
                                      std::uint32_t request_id, ComponentId component_id,
                                      const ClientHandleBase* response) {
  internal::WorkerProtocol_ComponentHandle command_response{component_id, response};
  internal::WorkerProtocol_Connection_SendCommandResponse(connection, request_id,
                                                          &command_response);
}

inline void SendClientCommandFailure(internal::WorkerProtocol_Connection* connection,
                                     std::uint32_t request_id, const std::string& message) {
  internal::WorkerProtocol_Connection_SendCommandFailure(connection, request_id, message.c_str());
}

}  // ::detail

inline Locator::Locator(const std::string& hostname, const LocatorParameters& params)
: locator{nullptr, &detail::internal::WorkerProtocol_Locator_Destroy} {
  auto protocol_params = detail::ConvertLocatorParameters(params);
  locator.reset(
      detail::internal::WorkerProtocol_Locator_Create(hostname.c_str(), &protocol_params));
}

inline Future<DeploymentList> Locator::GetDeploymentListAsync() {
  auto future = WorkerProtocol_Locator_GetDeploymentListAsync(locator.get());
  return Future<DeploymentList>(
      [future](const Option<std::uint32_t>& timeout_millis) {
        Option<DeploymentList> result;
        std::function<void(const DeploymentList& callback)> callback =
            [&](const DeploymentList& list) { result.emplace(list); };
        detail::internal::WorkerProtocol_DeploymentListFuture_Get(
            future, timeout_millis.data(), static_cast<void*>(&callback),
            &detail::LocatorDeploymentListThunk);
        return result;
      },
      [future] { detail::internal::WorkerProtocol_DeploymentListFuture_Destroy(future); });
}

inline Future<Connection> Locator::ConnectAsync(const ComponentRegistry& registry,
                                                const std::string& deployment_name,
                                                const ConnectionParameters& params,
                                                const QueueStatusCallback& callback) {
  const auto& component_info = registry.GetInternalComponentInfo();
  std::unique_ptr<detail::internal::WorkerProtocol_ComponentVtable[]> component_array;
  auto protocol_params = detail::ConvertConnectionParameters(params, component_info);
  auto shared_callback = std::make_shared<Locator::QueueStatusCallback>(callback);

  auto future = detail::internal::WorkerProtocol_Locator_ConnectAsync(
      locator.get(), deployment_name.c_str(), &protocol_params,
      static_cast<void*>(shared_callback.get()), &detail::LocatorQueueStatusThunk);

  return Future<Connection>(
      [future, shared_callback, &component_info](const Option<std::uint32_t>& timeout_millis) {
        Option<Connection> result;
        auto connection_ptr =
            detail::internal::WorkerProtocol_ConnectionFuture_Get(future, timeout_millis.data());
        if (connection_ptr) {
          result.emplace(Connection{component_info, connection_ptr});
        }
        return result;
      },
      [future] { detail::internal::WorkerProtocol_ConnectionFuture_Destroy(future); });
}

inline Future<Connection> Connection::ConnectAsync(const ComponentRegistry& registry,
                                                   const std::string& hostname, std::uint16_t port,
                                                   const std::string& worker_id,
                                                   const ConnectionParameters& params) {
  const auto& component_info = registry.GetInternalComponentInfo();
  auto protocol_params = ConvertConnectionParameters(params, component_info);

  auto future = detail::internal::WorkerProtocol_ConnectAsync(hostname.c_str(), port,
                                                              worker_id.c_str(), &protocol_params);

  return Future<Connection>(
      [future, &component_info](const Option<std::uint32_t>& timeout_millis) {
        Option<Connection> result;
        auto connection_ptr =
            detail::internal::WorkerProtocol_ConnectionFuture_Get(future, timeout_millis.data());
        if (connection_ptr) {
          result.emplace(Connection{component_info, connection_ptr});
        }
        return result;
      },
      [future] { detail::internal::WorkerProtocol_ConnectionFuture_Destroy(future); });
}

inline bool Connection::IsConnected() const {
  return detail::internal::WorkerProtocol_Connection_IsConnected(connection.get()) != 0;
}

inline std::string Connection::GetWorkerId() const {
  return detail::internal::WorkerProtocol_Connection_GetWorkerId(connection.get());
}

inline worker::List<std::string> Connection::GetWorkerAttributes() const {
  auto worker_attributes =
      detail::internal::WorkerProtocol_Connection_GetWorkerAttributes(connection.get());
  worker::List<std::string> result;
  for (std::uint32_t i = 0; i < worker_attributes->AttributeCount; ++i) {
    result.emplace_back(worker_attributes->Attribute[i]);
  }
  return result;
}

inline Option<std::string> Connection::GetWorkerFlag(const std::string& flag_name) const {
  Option<std::string> flag_value;
  std::function<void(const Option<std::string>&)> callback = [&](const Option<std::string>& val) {
    flag_value = val;
  };
  auto wrapper = static_cast<void*>(&callback);
  detail::internal::WorkerProtocol_Connection_GetFlag(connection.get(), flag_name.c_str(), wrapper,
                                                      &detail::ConnectionGetWorkerFlagThunk);
  return flag_value;
}

inline OpList Connection::GetOpList(std::uint32_t timeout_millis) {
  return OpList{
      detail::internal::WorkerProtocol_Connection_GetOpList(connection.get(), timeout_millis)};
}

inline void Connection::SendLogMessage(LogLevel level, const std::string& logger_name,
                                       const std::string& message,
                                       const Option<EntityId>& entity_id) {
  detail::internal::WorkerProtocol_LogMessage log_message;
  log_message.LogLevel = static_cast<std::uint8_t>(level);
  log_message.LoggerName = logger_name.c_str();
  log_message.Message = message.c_str();
  log_message.EntityId = entity_id.data();
  detail::internal::WorkerProtocol_Connection_SendLogMessage(connection.get(), &log_message);
}

inline void Connection::SendMetrics(Metrics& metrics) {
  detail::internal::WorkerProtocol_Metrics protocol_metrics = {};
  protocol_metrics.Load = metrics.Load.data();

  auto gauge_count = metrics.GaugeMetrics.size();
  auto histogram_count = metrics.HistogramMetrics.size();
  std::size_t total_bucket_count = 0;
  for (const auto& pair : metrics.HistogramMetrics) {
    total_bucket_count += pair.second.Buckets().size();
  }

  std::unique_ptr<detail::internal::WorkerProtocol_GaugeMetric[]> gauge_data;
  std::unique_ptr<detail::internal::WorkerProtocol_HistogramMetric[]> histogram_data;
  std::unique_ptr<detail::internal::WorkerProtocol_HistogramMetricBucket[]> bucket_data;

  if (gauge_count) {
    gauge_data.reset(new detail::internal::WorkerProtocol_GaugeMetric[gauge_count]);
    std::size_t index = 0;
    for (const auto& pair : metrics.GaugeMetrics) {
      gauge_data[index].Key = pair.first.c_str();
      gauge_data[index].Value = pair.second;
      ++index;
    }
    protocol_metrics.GaugeMetricCount = static_cast<std::uint32_t>(gauge_count);
    protocol_metrics.GaugeMetric = gauge_data.get();
  } else {
    protocol_metrics.GaugeMetricCount = 0;
    protocol_metrics.GaugeMetric = nullptr;
  }

  if (histogram_count) {
    histogram_data.reset(new detail::internal::WorkerProtocol_HistogramMetric[histogram_count]);
    bucket_data.reset(
        new detail::internal::WorkerProtocol_HistogramMetricBucket[total_bucket_count]);

    std::size_t index = 0;
    std::size_t bucket_index = 0;
    for (const auto& pair : metrics.HistogramMetrics) {
      histogram_data[index].Key = pair.first.c_str();
      histogram_data[index].Sum = pair.second.Sum();
      histogram_data[index].BucketCount = static_cast<std::uint32_t>(pair.second.Buckets().size());
      histogram_data[index].Bucket = bucket_index + bucket_data.get();

      for (const auto& bucket : pair.second.Buckets()) {
        bucket_data[bucket_index].UpperBound = bucket.UpperBound;
        bucket_data[bucket_index].Samples = bucket.Samples;
        ++bucket_index;
      }
      ++index;
    }
    protocol_metrics.HistogramMetricCount = static_cast<std::uint32_t>(histogram_count);
    protocol_metrics.HistogramMetric = histogram_data.get();
  } else {
    protocol_metrics.HistogramMetricCount = 0;
    protocol_metrics.HistogramMetric = nullptr;
  }

  detail::internal::WorkerProtocol_Connection_SendMetrics(connection.get(), &protocol_metrics);
  for (auto& pair : metrics.HistogramMetrics) {
    pair.second.ClearObservations();
  }
}

inline RequestId<ReserveEntityIdRequest>
Connection::SendReserveEntityIdRequest(const Option<std::uint32_t>& timeout_millis) {
  return RequestId<ReserveEntityIdRequest>{
      detail::internal::WorkerProtocol_Connection_SendReserveEntityIdRequest(
          connection.get(), timeout_millis.data())};
}

inline RequestId<ReserveEntityIdsRequest>
Connection::SendReserveEntityIdsRequest(std::uint32_t number_of_entity_ids,
                                        const Option<std::uint32_t>& timeout_millis) {
  return RequestId<ReserveEntityIdsRequest>{
      detail::internal::WorkerProtocol_Connection_SendReserveEntityIdsRequest(
          connection.get(), number_of_entity_ids, timeout_millis.data())};
}

inline RequestId<CreateEntityRequest>
Connection::SendCreateEntityRequest(const Entity& entity, const Option<EntityId>& entity_id,
                                    const Option<std::uint32_t>& timeout_millis) {
  std::vector<std::unique_ptr<detail::ClientHandleBase>> handle_storage;
  std::unique_ptr<detail::internal::WorkerProtocol_ComponentHandle[]> component_snapshot_array;
  std::uint32_t component_count;
  ExtractEntityComponents(component_info, entity, component_count, handle_storage,
                          component_snapshot_array);
  return RequestId<CreateEntityRequest>{
      detail::internal::WorkerProtocol_Connection_SendCreateEntityRequest(
          connection.get(), static_cast<std::uint32_t>(component_count),
          component_snapshot_array.get(), entity_id.data(), timeout_millis.data())};
}

inline RequestId<DeleteEntityRequest>
Connection::SendDeleteEntityRequest(EntityId entity_id,
                                    const Option<std::uint32_t>& timeout_millis) {
  return RequestId<DeleteEntityRequest>{
      detail::internal::WorkerProtocol_Connection_SendDeleteEntityRequest(
          connection.get(), entity_id, timeout_millis.data())};
}

inline RequestId<EntityQueryRequest>
Connection::SendEntityQueryRequest(const query::EntityQuery& entity_query,
                                   const Option<std::uint32_t>& timeout_millis) {
  detail::internal::WorkerProtocol_EntityQuery protocol_query;
  protocol_query.SnapshotResultTypeComponentIdCount = 0;
  protocol_query.SnapshotResultTypeComponentId = nullptr;
  if (entity_query.ResultType.data<query::CountResultType>()) {
    protocol_query.ResultType = detail::internal::WORKER_PROTOCOL_RESULT_TYPE_COUNT;
  }
  std::unique_ptr<detail::internal::WorkerProtocol_ComponentId[]> snapshot_result_type_storage;
  const auto* snapshot_result_type = entity_query.ResultType.data<query::SnapshotResultType>();
  if (snapshot_result_type) {
    protocol_query.ResultType = detail::internal::WORKER_PROTOCOL_RESULT_TYPE_SNAPSHOT;
    if (!snapshot_result_type->ComponentIds.empty()) {
      auto size = static_cast<std::uint32_t>(snapshot_result_type->ComponentIds->size());
      snapshot_result_type_storage.reset(new detail::internal::WorkerProtocol_ComponentId[size]);
      for (std::uint32_t i = 0; i < size; ++i) {
        snapshot_result_type_storage[i] = (*snapshot_result_type->ComponentIds)[i];
      }
      protocol_query.SnapshotResultTypeComponentIdCount = size;
      protocol_query.SnapshotResultTypeComponentId = snapshot_result_type_storage.get();
    }
  }
  std::vector<std::unique_ptr<detail::internal::WorkerProtocol_Constraint[]>> storage;
  protocol_query.Constraint = detail::ConvertConstraint(entity_query.Constraint, storage);
  return RequestId<EntityQueryRequest>{
      detail::internal::WorkerProtocol_Connection_SendEntityQueryRequest(
          connection.get(), &protocol_query, timeout_millis.data())};
}

inline void
Connection::SendComponentInterest(EntityId entity_id,
                                  const Map<ComponentId, InterestOverride>& interest_overrides) {
  std::vector<detail::internal::WorkerProtocol_InterestOverride> protocol_overrides;
  for (const auto& pair : interest_overrides) {
    protocol_overrides.push_back({pair.first, pair.second.IsInterested});
  }
  detail::internal::WorkerProtocol_Connection_SendComponentInterest(
      connection.get(), entity_id, protocol_overrides.data(),
      static_cast<std::uint32_t>(protocol_overrides.size()));
}

inline void Connection::SendAuthorityLossImminentAcknowledgement(EntityId entity_id,
                                                                 ComponentId component_id) {
  detail::internal::WorkerProtocol_Connection_SendAuthorityLossImminentAcknowledgement(
      connection.get(), entity_id, component_id);
}

template <typename T>
void Connection::SendComponentUpdate(EntityId entity_id, const typename T::Update& update) {
  auto handle = detail::ClientHandle<typename T::Update>::copyable(&update);
  detail::SendClientComponentUpdate(connection.get(), entity_id, T::ComponentId, &handle);
}

template <typename T>
void Connection::SendComponentUpdate(EntityId entity_id, typename T::Update&& update) {
  auto handle = detail::ClientHandle<typename T::Update>::movable(&update);
  detail::SendClientComponentUpdate(connection.get(), entity_id, T::ComponentId, &handle);
}

template <typename T>
RequestId<OutgoingCommandRequest<T>>
Connection::SendCommandRequest(EntityId entity_id, const typename T::Request& request,
                               const Option<std::uint32_t>& timeout_millis,
                               const CommandParameters& parameters) {
  using GenericRequest = typename T::ComponentMetaclass::GenericCommandObject;
  GenericRequest generic_request{T::CommandId, request};
  auto handle = detail::ClientHandle<GenericRequest>::movable(&generic_request);

  return RequestId<OutgoingCommandRequest<T>>{detail::SendClientCommandRequest(
      connection.get(), entity_id, T::ComponentMetaclass::ComponentId, &handle, T::CommandId,
      timeout_millis, parameters)};
}

template <typename T>
RequestId<OutgoingCommandRequest<T>>
Connection::SendCommandRequest(EntityId entity_id, typename T::Request&& request,
                               const Option<std::uint32_t>& timeout_millis,
                               const CommandParameters& parameters) {
  using GenericRequest = typename T::ComponentMetaclass::GenericCommandObject;
  GenericRequest generic_request{T::CommandId, std::move(request)};
  auto handle = detail::ClientHandle<GenericRequest>::movable(&generic_request);

  return RequestId<OutgoingCommandRequest<T>>{detail::SendClientCommandRequest(
      connection.get(), entity_id, T::ComponentMetaclass::ComponentId, &handle, T::CommandId,
      timeout_millis, parameters)};
}

template <typename T>
void Connection::SendCommandResponse(RequestId<IncomingCommandRequest<T>> request_id,
                                     const typename T::Response& response) {
  using GenericResponse = typename T::ComponentMetaclass::GenericCommandObject;
  GenericResponse generic_response{T::CommandId, response};
  auto handle = detail::ClientHandle<GenericResponse>::movable(&generic_response);

  detail::SendClientCommandResponse(connection.get(), request_id.Id,
                                    T::ComponentMetaclass::ComponentId, &handle);
}

template <typename T>
void Connection::SendCommandResponse(RequestId<IncomingCommandRequest<T>> request_id,
                                     typename T::Response&& response) {
  using GenericResponse = typename T::ComponentMetaclass::GenericCommandObject;
  GenericResponse generic_response{T::CommandId, std::move(response)};
  auto handle = detail::ClientHandle<GenericResponse>::movable(&generic_response);

  detail::SendClientCommandResponse(connection.get(), request_id.Id,
                                    T::ComponentMetaclass::ComponentId, &handle);
}

template <typename T>
void Connection::SendCommandFailure(RequestId<IncomingCommandRequest<T>> request_id,
                                    const std::string& message) {
  detail::SendClientCommandFailure(connection.get(), request_id.Id, message);
}

inline void Connection::SetProtocolLoggingEnabled(bool enabled) {
  detail::internal::WorkerProtocol_Connection_SetProtocolLoggingEnabled(connection.get(), enabled);
}

inline Connection::Connection(const detail::ComponentInfo& component_info,
                              detail::internal::WorkerProtocol_Connection* connection)
: component_info{component_info}
, connection{connection, &detail::internal::WorkerProtocol_Connection_Destroy} {}

inline OpList::OpList(detail::internal::WorkerProtocol_OpList* op_list)
: op_list{op_list, &detail::internal::WorkerProtocol_OpList_Destroy} {}

}  // ::worker

#endif  // WORKER_SDK_CPP_INCLUDE_IMPROBABLE_DETAIL_CONNECTION_I_H
