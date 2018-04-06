// Copyright (c) Improbable Worlds Ltd, All Rights Reserved
#include "improbable/worker.h"
#include "improbable/worker_protocol.h"
#include <limits>

namespace worker {

namespace {

/** Extracts the components of an Entity into the C equivalent structures. */
void ExtractEntityComponents(
    const detail::ComponentInfo& component_info, const Entity& entity,
    std::uint32_t& component_count,
    std::vector<std::unique_ptr<detail::ClientHandleBase>>& handle_storage,
    std::unique_ptr<WorkerProtocol_ComponentHandle[]>& component_snapshot_storage) {
  // Create C-compatible continuous storage for the updates.
  const auto& component_ids = entity.GetComponentIds();
  component_count = static_cast<std::uint32_t>(component_ids.size());
  component_snapshot_storage.reset(new WorkerProtocol_ComponentHandle[component_count]);
  WorkerProtocol_ComponentHandle* entity_components_array = component_snapshot_storage.get();

  // Extract each component into a WorkerProtocol_ComponentUpdate.
  std::size_t component_index = 0;
  for (const auto& component_id : component_ids) {
    auto it = component_info.ExtractSnapshot.find(component_id);
    if (it != component_info.ExtractSnapshot.end()) {
      handle_storage.emplace_back(it->second(entity));
      entity_components_array[component_index].ComponentId = component_id;
      entity_components_array[component_index].Handle = handle_storage.back().get();
    } else {
      // This causes the C API to report an error for the unknown component.
      entity_components_array[component_index].ComponentId = component_id;
      entity_components_array[component_index].Handle = nullptr;
    }
    ++component_index;
  }
}

void DispatcherDisconnectThunk(void* user_data, const WorkerProtocol_DisconnectOp* op) {
  const auto& impl = *static_cast<detail::DispatcherImpl*>(user_data);
  DisconnectOp wrapper{op->Reason};
  impl.disconnect_callbacks.InvokeAll(wrapper);
}

void DispatcherFlagUpdateThunk(void* user_data, const WorkerProtocol_FlagUpdateOp* op) {
  const auto& impl = *static_cast<detail::DispatcherImpl*>(user_data);
  FlagUpdateOp wrapper{op->Name, {}};
  if (op->Value) {
    wrapper.Value.emplace(op->Value);
    impl.flag_update_callbacks.InvokeAll(wrapper);
  } else {
    impl.flag_update_callbacks.ReverseInvokeAll(wrapper);
  }
}

void DispatcherLogMessageThunk(void* user_data, const WorkerProtocol_LogMessageOp* op) {
  const auto& impl = *static_cast<detail::DispatcherImpl*>(user_data);
  LogMessageOp wrapper{static_cast<LogLevel>(op->LogLevel), op->Message};
  impl.log_message_callbacks.InvokeAll(wrapper);
}

void DispatcherMetricsThunk(void* user_data, const WorkerProtocol_MetricsOp* op) {
  const auto& impl = *static_cast<detail::DispatcherImpl*>(user_data);
  MetricsOp wrapper;
  for (std::size_t i = 0; i < op->Metrics.GaugeMetricCount; ++i) {
    wrapper.Metrics.GaugeMetrics[op->Metrics.GaugeMetric[i].Key] = op->Metrics.GaugeMetric[i].Value;
  }
  // We do not have any built-in histogram metrics.
  impl.metrics_callbacks.InvokeAll(wrapper);
}

void DispatcherCriticalSectionThunk(void* user_data, const WorkerProtocol_CriticalSectionOp* op) {
  const auto& impl = *static_cast<detail::DispatcherImpl*>(user_data);
  CriticalSectionOp wrapper{op->InCriticalSection != 0};
  if (op->InCriticalSection) {
    impl.critical_section_callbacks.InvokeAll(wrapper);
  } else {
    impl.critical_section_callbacks.ReverseInvokeAll(wrapper);
  }
}

void DispatcherAddEntityThunk(void* user_data, const WorkerProtocol_AddEntityOp* op) {
  const auto& impl = *static_cast<detail::DispatcherImpl*>(user_data);
  AddEntityOp wrapper{op->EntityId};
  impl.add_entity_callbacks.InvokeAll(wrapper);
}

void DispatcherRemoveEntityThunk(void* user_data, const WorkerProtocol_RemoveEntityOp* op) {
  const auto& impl = *static_cast<detail::DispatcherImpl*>(user_data);
  RemoveEntityOp wrapper{op->EntityId};
  impl.remove_entity_callbacks.ReverseInvokeAll(wrapper);
}

void DispatcherReserveEntityIdResponseThunk(void* user_data,
                                            const WorkerProtocol_ReserveEntityIdResponseOp* op) {
  const auto& impl = *static_cast<detail::DispatcherImpl*>(user_data);
  ReserveEntityIdResponseOp wrapper{
      RequestId<ReserveEntityIdRequest>{op->RequestId},
      static_cast<worker::StatusCode>(op->StatusCode), op->Message,
      op->StatusCode == WORKER_PROTOCOL_STATUS_CODE_SUCCESS ? op->EntityId : Option<EntityId>{}};
  impl.reserve_entity_id_response_callbacks.InvokeAll(wrapper);
}

void DispatcherReserveEntityIdsResponseThunk(void* user_data,
                                             const WorkerProtocol_ReserveEntityIdsResponseOp* op) {
  const auto& impl = *static_cast<detail::DispatcherImpl*>(user_data);
  ReserveEntityIdsResponseOp wrapper{RequestId<ReserveEntityIdsRequest>{op->RequestId},
                                     static_cast<worker::StatusCode>(op->StatusCode), op->Message,
                                     op->StatusCode == WORKER_PROTOCOL_STATUS_CODE_SUCCESS
                                         ? op->FirstEntityId
                                         : Option<EntityId>{},
                                     op->NumberOfEntityIds};
  impl.reserve_entity_ids_response_callbacks.InvokeAll(wrapper);
}

void DispatcherCreateEntityResponseThunk(void* user_data,
                                         const WorkerProtocol_CreateEntityResponseOp* op) {
  const auto& impl = *static_cast<detail::DispatcherImpl*>(user_data);
  CreateEntityResponseOp wrapper{
      RequestId<CreateEntityRequest>{op->RequestId},
      static_cast<worker::StatusCode>(op->StatusCode), op->Message,
      op->StatusCode == WORKER_PROTOCOL_STATUS_CODE_SUCCESS ? op->EntityId : Option<EntityId>{}};
  impl.create_entity_response_callbacks.InvokeAll(wrapper);
}

void DispatcherDeleteEntityResponseThunk(void* user_data,
                                         const WorkerProtocol_DeleteEntityResponseOp* op) {
  const auto& impl = *static_cast<detail::DispatcherImpl*>(user_data);
  DeleteEntityResponseOp wrapper{RequestId<DeleteEntityRequest>{op->RequestId}, op->EntityId,
                                 static_cast<worker::StatusCode>(op->StatusCode), op->Message};
  impl.delete_entity_response_callbacks.InvokeAll(wrapper);
}

void DispatcherEntityQueryResponseThunk(void* user_data,
                                        const WorkerProtocol_EntityQueryResponseOp* op) {
  const auto& impl = *static_cast<detail::DispatcherImpl*>(user_data);
  EntityQueryResponseOp wrapper{RequestId<EntityQueryRequest>{op->RequestId},
                                static_cast<worker::StatusCode>(op->StatusCode),
                                op->Message,
                                op->ResultCount,
                                {}};
  for (std::uint32_t i = 0; op->Result && i < op->ResultCount; ++i) {
    auto& entity = wrapper.Result[op->Result[i].EntityId];
    for (std::uint32_t j = 0; j < op->Result[i].ComponentCount; ++j) {
      const auto& component = op->Result[i].Component[j];
      auto it = impl.component_info.MoveSnapshotIntoEntity.find(component.ComponentId);
      if (it != impl.component_info.MoveSnapshotIntoEntity.end()) {
        // Moves the snapshot data out of the C-owned object. It's OK to steal the data here since
        // we know this is the only possible callback for the C dispatcher.
        it->second(const_cast<detail::ClientHandleBase*>(
                       static_cast<const detail::ClientHandleBase*>(component.Handle)),
                   entity);
      }
    }
  }
  impl.entity_query_response_callbacks.InvokeAll(wrapper);
}

void DispatcherAddComponentThunk(void* user_data, const WorkerProtocol_AddComponentOp* op) {
  const auto& impl = *static_cast<detail::DispatcherImpl*>(user_data);
  detail::DispatcherImpl::ComponentWrapperOp wrapper{
      op->EntityId, static_cast<const detail::ClientHandleBase*>(op->InitialComponent.Handle)};
  impl.add_component_callbacks.InvokeAll(op->InitialComponent.ComponentId, wrapper);
}

void DispatcherRemoveComponentThunk(void* user_data, const WorkerProtocol_RemoveComponentOp* op) {
  const auto& impl = *static_cast<detail::DispatcherImpl*>(user_data);
  RemoveComponentOp wrapper{op->EntityId};
  impl.remove_component_callbacks.ReverseInvokeAll(op->ComponentId, wrapper);
}

void DispatcherAuthorityChangeThunk(void* user_data, const WorkerProtocol_AuthorityChangeOp* op) {
  const auto& impl = *static_cast<detail::DispatcherImpl*>(user_data);
  AuthorityChangeOp wrapper{op->EntityId, static_cast<worker::Authority>(op->Authority)};
  if (wrapper.Authority == Authority::kAuthoritative) {
    impl.authority_change_callbacks.InvokeAll(op->ComponentId, wrapper);
  } else {
    impl.authority_change_callbacks.ReverseInvokeAll(op->ComponentId, wrapper);
  }
}

void DispatcherComponentUpdateThunk(void* user_data, const WorkerProtocol_ComponentUpdateOp* op) {
  const auto& impl = *static_cast<detail::DispatcherImpl*>(user_data);
  detail::DispatcherImpl::ComponentWrapperOp wrapper{
      op->EntityId, static_cast<const detail::ClientHandleBase*>(op->Update.Handle)};
  impl.component_update_callbacks.InvokeAll(op->Update.ComponentId, wrapper);
}

void DispatcherCommandRequestThunk(void* user_data, const WorkerProtocol_CommandRequestOp* op) {
  const auto& impl = *static_cast<detail::DispatcherImpl*>(user_data);
  detail::DispatcherImpl::CommandRequestWrapperOp wrapper{
      op->RequestId,
      op->EntityId,
      op->TimeoutMillis,
      op->CallerWorkerId,
      op->CallerAttributes.AttributeCount,
      op->CallerAttributes.Attribute,
      static_cast<const detail::ClientHandleBase*>(op->Request.Handle)};
  impl.command_request_callbacks.InvokeAll(op->Request.ComponentId, wrapper);
}

void DispatcherCommandResponseThunk(void* user_data, const WorkerProtocol_CommandResponseOp* op) {
  const auto& impl = *static_cast<detail::DispatcherImpl*>(user_data);
  detail::DispatcherImpl::CommandResponseWrapperOp wrapper{
      op->RequestId,
      op->EntityId,
      op->StatusCode,
      op->Message,
      static_cast<const detail::ClientHandleBase*>(op->Response.Handle),
      op->CommandId};
  impl.command_response_callbacks.InvokeAll(op->Response.ComponentId, wrapper);
}

void LocatorDeploymentListThunk(void* user_data,
                                const WorkerProtocol_DeploymentList* deployment_list) {
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

std::uint8_t LocatorQueueStatusThunk(void* user_data,
                                     const WorkerProtocol_QueueStatus* queue_status) {
  const auto& callback = *static_cast<Locator::QueueStatusCallback*>(user_data);
  QueueStatus wrapper;
  wrapper.PositionInQueue = queue_status->PositionInQueue;
  if (queue_status->Error) {
    wrapper.Error.emplace(queue_status->Error);
  }
  return callback(wrapper) ? 1 : 0;
}

void ConnectionGetWorkerFlagThunk(void* user_data, const char* value) {
  const auto& callback = *static_cast<std::function<void(const Option<std::string>&)>*>(user_data);
  Option<std::string> cpp_flag_value;
  if (value) {
    cpp_flag_value.emplace(value);
  }
  callback(cpp_flag_value);
}

WorkerProtocol_LocatorParameters ConvertLocatorParameters(const LocatorParameters& params) {
  WorkerProtocol_LocatorParameters protocol_params = {};
  protocol_params.ProjectName = params.ProjectName.c_str();
  if (params.CredentialsType == LocatorCredentialsType::kLoginToken) {
    protocol_params.Flags |= WORKER_PROTOCOL_LOCATOR_LOGIN_TOKEN_CREDENTIALS;
    protocol_params.LoginToken.Token = params.LoginToken.Token.c_str();
  }
  if (params.CredentialsType == LocatorCredentialsType::kSteam) {
    protocol_params.Flags |= WORKER_PROTOCOL_LOCATOR_STEAM_CREDENTIALS;
    protocol_params.Steam.Ticket = params.Steam.Ticket.c_str();
    protocol_params.Steam.DeploymentTag = params.Steam.DeploymentTag.c_str();
  }
  return protocol_params;
}

WorkerProtocol_ConnectionParameters
ConvertConnectionParameters(const ConnectionParameters& params,
                            const detail::ComponentInfo& component_info) {
  WorkerProtocol_ConnectionParameters protocol_params = {};
  protocol_params.WorkerType = params.WorkerType.c_str();

  protocol_params.Network.Flags =
      static_cast<std::uint8_t>(params.Network.ConnectionType == NetworkConnectionType::kTcp
                                    ? WORKER_PROTOCOL_NETWORK_TCP
                                    : WORKER_PROTOCOL_NETWORK_RAKNET);
  if (params.Network.UseExternalIp) {
    protocol_params.Network.Flags =
        protocol_params.Network.Flags | WORKER_PROTOCOL_NETWORK_USE_EXTERNAL_IP;
  }

  protocol_params.Network.ConnectionTimeoutMillis = params.Network.ConnectionTimeoutMillis;
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
      reinterpret_cast<const WorkerProtocol_ComponentVtable*>(component_info.Vtables.data());
  return protocol_params;
}

WorkerProtocol_Constraint
ConvertConstraint(const query::Constraint& constraint,
                  std::vector<std::unique_ptr<WorkerProtocol_Constraint[]>>& storage) {
  WorkerProtocol_Constraint result = {};
  if (const auto* entity_id = constraint.data<query::EntityIdConstraint>()) {
    result.ConstraintType = WORKER_PROTOCOL_CONSTRAINT_TYPE_ENTITY_ID;
    result.EntityIdConstraint.EntityId = entity_id->EntityId;
  } else if (const auto* component = constraint.data<query::ComponentConstraint>()) {
    result.ConstraintType = WORKER_PROTOCOL_CONSTRAINT_TYPE_COMPONENT;
    result.ComponentConstraint.ComponentId = component->ComponentId;
  } else if (const auto* sphere = constraint.data<query::SphereConstraint>()) {
    result.ConstraintType = WORKER_PROTOCOL_CONSTRAINT_TYPE_SPHERE;
    result.SphereConstraint.PositionX = sphere->X;
    result.SphereConstraint.PositionY = sphere->Y;
    result.SphereConstraint.PositionZ = sphere->Z;
    result.SphereConstraint.Radius = sphere->Radius;
  } else if (const auto* and_constraint = constraint.data<query::AndConstraint>()) {
    auto size = static_cast<std::uint32_t>(and_constraint->size());
    storage.emplace_back(new WorkerProtocol_Constraint[size]);
    auto ptr = storage.back().get();
    for (std::uint32_t i = 0; i < size; ++i) {
      ptr[i] = ConvertConstraint((*and_constraint)[i], storage);
    }

    result.ConstraintType = WORKER_PROTOCOL_CONSTRAINT_TYPE_AND;
    result.AndConstraint.ConstraintCount = size;
    result.AndConstraint.Constraint = ptr;
  } else if (const auto* or_constraint = constraint.data<query::OrConstraint>()) {
    auto size = static_cast<std::uint32_t>(or_constraint->size());
    storage.emplace_back(new WorkerProtocol_Constraint[size]);
    auto ptr = storage.back().get();
    for (std::uint32_t i = 0; i < size; ++i) {
      ptr[i] = ConvertConstraint((*or_constraint)[i], storage);
    }

    result.ConstraintType = WORKER_PROTOCOL_CONSTRAINT_TYPE_OR;
    result.OrConstraint.ConstraintCount = size;
    result.OrConstraint.Constraint = ptr;
  } else if (const auto* not_constraint = constraint.data<query::NotConstraint>()) {
    storage.emplace_back(new WorkerProtocol_Constraint[1]);
    auto ptr = storage.back().get();
    ptr[0] = ConvertConstraint(not_constraint->Constraint, storage);

    result.ConstraintType = WORKER_PROTOCOL_CONSTRAINT_TYPE_NOT;
    result.NotConstraint.Constraint = ptr;
  }
  return result;
}

WorkerProtocol_SnapshotInputStream*
MakeSnapshotInputStream(const detail::ComponentInfo& component_info, const std::string& path) {
  WorkerProtocol_SnapshotParameters parameters = {};
  parameters.ComponentVtableCount = static_cast<std::uint32_t>(component_info.Vtables.size());
  parameters.ComponentVtable =
      reinterpret_cast<const WorkerProtocol_ComponentVtable*>(component_info.Vtables.data());
  WorkerProtocol_SnapshotInputStream* input_stream =
      WorkerProtocol_SnapshotInputStream_Create(path.c_str(), &parameters);
  return input_stream;
}

WorkerProtocol_SnapshotOutputStream*
MakeSnapshotOutputStream(const detail::ComponentInfo& component_info, const std::string& path) {
  WorkerProtocol_SnapshotParameters parameters = {};
  parameters.ComponentVtableCount = static_cast<std::uint32_t>(component_info.Vtables.size());
  parameters.ComponentVtable =
      reinterpret_cast<const WorkerProtocol_ComponentVtable*>(component_info.Vtables.data());
  WorkerProtocol_SnapshotOutputStream* output_stream =
      WorkerProtocol_SnapshotOutputStream_Create(path.c_str(), &parameters);
  return output_stream;
}

}  // anonymous namespace

namespace detail {

void SendClientComponentUpdate(WorkerProtocol_Connection* connection, EntityId entity_id,
                               ComponentId component_id, const ClientHandleBase* update,
                               const UpdateParameters& parameters) {
  WorkerProtocol_ComponentHandle component_update{component_id, update};
  WorkerProtocol_UpdateParameters update_parameters{static_cast<std::uint8_t>(parameters.Loopback)};
  WorkerProtocol_Connection_SendComponentUpdate(connection, entity_id, &component_update,
                                                &update_parameters);
}

std::uint32_t SendClientCommandRequest(WorkerProtocol_Connection* connection, EntityId entity_id,
                                       ComponentId component_id, const ClientHandleBase* request,
                                       std::uint32_t command_id,
                                       const Option<std::uint32_t>& timeout_millis,
                                       const CommandParameters& parameters) {
  WorkerProtocol_ComponentHandle command_request{component_id, request};
  WorkerProtocol_CommandParameters command_parameters{
      static_cast<std::uint8_t>(parameters.AllowShortCircuit)};
  return WorkerProtocol_Connection_SendCommandRequest(connection, entity_id, &command_request,
                                                      command_id, timeout_millis.data(),
                                                      &command_parameters);
}

void SendClientCommandResponse(WorkerProtocol_Connection* connection, std::uint32_t request_id,
                               ComponentId component_id, const ClientHandleBase* response) {
  WorkerProtocol_ComponentHandle command_response{component_id, response};
  WorkerProtocol_Connection_SendCommandResponse(connection, request_id, &command_response);
}

void SendClientCommandFailure(WorkerProtocol_Connection* connection, std::uint32_t request_id,
                              const std::string& message) {
  WorkerProtocol_Connection_SendCommandFailure(connection, request_id, message.c_str());
}

}  // ::detail

// Implementation of Locator.
Locator::Locator(const std::string& hostname, const LocatorParameters& params)
: locator{nullptr, WorkerProtocol_Locator_Destroy} {
  auto protocol_params = ConvertLocatorParameters(params);
  locator.reset(WorkerProtocol_Locator_Create(hostname.c_str(), &protocol_params));
}

Future<DeploymentList> Locator::GetDeploymentListAsync() {
  auto future = WorkerProtocol_Locator_GetDeploymentListAsync(locator.get());
  return Future<DeploymentList>(
      [future](const Option<std::uint32_t>& timeout_millis) {
        Option<DeploymentList> result;
        std::function<void(const DeploymentList& callback)> callback =
            [&](const DeploymentList& list) { result.emplace(list); };
        WorkerProtocol_DeploymentListFuture_Get(future, timeout_millis.data(),
                                                static_cast<void*>(&callback),
                                                LocatorDeploymentListThunk);
        return result;
      },
      [future] { WorkerProtocol_DeploymentListFuture_Destroy(future); });
}

Future<Connection> Locator::ConnectAsync(const ComponentRegistry& registry,
                                         const std::string& deployment_name,
                                         const ConnectionParameters& params,
                                         const QueueStatusCallback& callback) {
  const auto& component_info = registry.GetInternalComponentInfo();
  std::unique_ptr<WorkerProtocol_ComponentVtable[]> component_array;
  auto protocol_params = ConvertConnectionParameters(params, component_info);
  auto shared_callback = std::make_shared<Locator::QueueStatusCallback>(callback);

  auto future = WorkerProtocol_Locator_ConnectAsync(
      locator.get(), deployment_name.c_str(), &protocol_params,
      static_cast<void*>(shared_callback.get()), LocatorQueueStatusThunk);

  return Future<Connection>(
      [future, shared_callback, &component_info](const Option<std::uint32_t>& timeout_millis) {
        Option<Connection> result;
        auto connection_ptr = WorkerProtocol_ConnectionFuture_Get(future, timeout_millis.data());
        if (connection_ptr) {
          result.emplace(Connection{component_info, connection_ptr});
        }
        return result;
      },
      [future] { WorkerProtocol_ConnectionFuture_Destroy(future); });
}

// Implementation of Connection.
Future<Connection> Connection::ConnectAsync(const ComponentRegistry& registry,
                                            const std::string& hostname, std::uint16_t port,
                                            const std::string& worker_id,
                                            const ConnectionParameters& params) {
  const auto& component_info = registry.GetInternalComponentInfo();
  auto protocol_params = ConvertConnectionParameters(params, component_info);

  auto future =
      WorkerProtocol_ConnectAsync(hostname.c_str(), port, worker_id.c_str(), &protocol_params);

  return Future<Connection>(
      [future, &component_info](const Option<std::uint32_t>& timeout_millis) {
        Option<Connection> result;
        auto connection_ptr = WorkerProtocol_ConnectionFuture_Get(future, timeout_millis.data());
        if (connection_ptr) {
          result.emplace(Connection{component_info, connection_ptr});
        }
        return result;
      },
      [future] { WorkerProtocol_ConnectionFuture_Destroy(future); });
}

bool Connection::IsConnected() const {
  return WorkerProtocol_Connection_IsConnected(connection.get()) != 0;
}

std::string Connection::GetWorkerId() const {
  return WorkerProtocol_Connection_GetWorkerId(connection.get());
}

worker::List<std::string> Connection::GetWorkerAttributes() const {
  auto worker_attributes = WorkerProtocol_Connection_GetWorkerAttributes(connection.get());
  worker::List<std::string> result;
  for (std::uint32_t i = 0; i < worker_attributes->AttributeCount; ++i) {
    result.emplace_back(worker_attributes->Attribute[i]);
  }
  return result;
}

Option<std::string> Connection::GetWorkerFlag(const std::string& flag_name) const {
  Option<std::string> flag_value;
  std::function<void(const Option<std::string>&)> callback = [&](const Option<std::string>& val) {
    flag_value = val;
  };
  auto wrapper = static_cast<void*>(&callback);
  WorkerProtocol_Connection_GetFlag(connection.get(), flag_name.c_str(), wrapper,
                                    ConnectionGetWorkerFlagThunk);
  return flag_value;
}

OpList Connection::GetOpList(std::uint32_t timeout_millis) {
  return OpList{WorkerProtocol_Connection_GetOpList(connection.get(), timeout_millis)};
}

void Connection::SendLogMessage(LogLevel level, const std::string& logger_name,
                                const std::string& message, const Option<EntityId>& entity_id) {
  WorkerProtocol_LogMessage log_message;
  log_message.LogLevel = static_cast<std::uint8_t>(level);
  log_message.LoggerName = logger_name.c_str();
  log_message.Message = message.c_str();
  log_message.EntityId = entity_id.data();
  WorkerProtocol_Connection_SendLogMessage(connection.get(), &log_message);
}

void Connection::SendMetrics(Metrics& metrics) {
  WorkerProtocol_Metrics protocol_metrics = {};
  protocol_metrics.Load = metrics.Load.data();

  auto gauge_count = metrics.GaugeMetrics.size();
  auto histogram_count = metrics.HistogramMetrics.size();
  std::size_t total_bucket_count = 0;
  for (const auto& pair : metrics.HistogramMetrics) {
    total_bucket_count += pair.second.Buckets().size();
  }

  std::unique_ptr<WorkerProtocol_GaugeMetric[]> gauge_data;
  std::unique_ptr<WorkerProtocol_HistogramMetric[]> histogram_data;
  std::unique_ptr<WorkerProtocol_HistogramMetricBucket[]> bucket_data;

  if (gauge_count) {
    gauge_data.reset(new WorkerProtocol_GaugeMetric[gauge_count]);
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
    histogram_data.reset(new WorkerProtocol_HistogramMetric[histogram_count]);
    bucket_data.reset(new WorkerProtocol_HistogramMetricBucket[total_bucket_count]);

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

  WorkerProtocol_Connection_SendMetrics(connection.get(), &protocol_metrics);
  for (auto& pair : metrics.HistogramMetrics) {
    pair.second.ClearObservations();
  }
}

RequestId<ReserveEntityIdRequest>
Connection::SendReserveEntityIdRequest(const Option<std::uint32_t>& timeout_millis) {
  return RequestId<ReserveEntityIdRequest>{WorkerProtocol_Connection_SendReserveEntityIdRequest(
      connection.get(), timeout_millis.data())};
}

RequestId<ReserveEntityIdsRequest>
Connection::SendReserveEntityIdsRequest(std::uint32_t number_of_entity_ids,
                                        const Option<std::uint32_t>& timeout_millis) {
  return RequestId<ReserveEntityIdsRequest>{WorkerProtocol_Connection_SendReserveEntityIdsRequest(
      connection.get(), number_of_entity_ids, timeout_millis.data())};
}

RequestId<CreateEntityRequest>
Connection::SendCreateEntityRequest(const Entity& entity, const Option<EntityId>& entity_id,
                                    const Option<std::uint32_t>& timeout_millis) {
  std::vector<std::unique_ptr<detail::ClientHandleBase>> handle_storage;
  std::unique_ptr<WorkerProtocol_ComponentHandle[]> component_snapshot_array;
  std::uint32_t component_count;
  ExtractEntityComponents(component_info, entity, component_count, handle_storage,
                          component_snapshot_array);
  return RequestId<CreateEntityRequest>{WorkerProtocol_Connection_SendCreateEntityRequest(
      connection.get(), static_cast<std::uint32_t>(component_count), component_snapshot_array.get(),
      entity_id.data(), timeout_millis.data())};
}

RequestId<DeleteEntityRequest>
Connection::SendDeleteEntityRequest(EntityId entity_id,
                                    const Option<std::uint32_t>& timeout_millis) {
  return RequestId<DeleteEntityRequest>{WorkerProtocol_Connection_SendDeleteEntityRequest(
      connection.get(), entity_id, timeout_millis.data())};
}

RequestId<EntityQueryRequest>
Connection::SendEntityQueryRequest(const query::EntityQuery& entity_query,
                                   const Option<std::uint32_t>& timeout_millis) {
  WorkerProtocol_EntityQuery protocol_query;
  protocol_query.SnapshotResultTypeComponentIdCount = 0;
  protocol_query.SnapshotResultTypeComponentId = nullptr;
  if (entity_query.ResultType.data<query::CountResultType>()) {
    protocol_query.ResultType = WORKER_PROTOCOL_RESULT_TYPE_COUNT;
  }
  std::unique_ptr<WorkerProtocol_ComponentId[]> snapshot_result_type_storage;
  const auto* snapshot_result_type = entity_query.ResultType.data<query::SnapshotResultType>();
  if (snapshot_result_type) {
    protocol_query.ResultType = WORKER_PROTOCOL_RESULT_TYPE_SNAPSHOT;
    if (!snapshot_result_type->ComponentIds.empty()) {
      auto size = static_cast<std::uint32_t>(snapshot_result_type->ComponentIds->size());
      snapshot_result_type_storage.reset(new WorkerProtocol_ComponentId[size]);
      for (std::uint32_t i = 0; i < size; ++i) {
        snapshot_result_type_storage[i] = (*snapshot_result_type->ComponentIds)[i];
      }
      protocol_query.SnapshotResultTypeComponentIdCount = size;
      protocol_query.SnapshotResultTypeComponentId = snapshot_result_type_storage.get();
    }
  }
  std::vector<std::unique_ptr<WorkerProtocol_Constraint[]>> storage;
  protocol_query.Constraint = ConvertConstraint(entity_query.Constraint, storage);
  return RequestId<EntityQueryRequest>{WorkerProtocol_Connection_SendEntityQueryRequest(
      connection.get(), &protocol_query, timeout_millis.data())};
}

void Connection::SendComponentInterest(
    EntityId entity_id, const Map<ComponentId, InterestOverride>& interest_overrides) {
  std::vector<WorkerProtocol_InterestOverride> protocol_overrides;
  for (const auto& pair : interest_overrides) {
    protocol_overrides.push_back({pair.first, pair.second.IsInterested});
  }
  WorkerProtocol_Connection_SendComponentInterest(
      connection.get(), entity_id, protocol_overrides.data(),
      static_cast<std::uint32_t>(protocol_overrides.size()));
}

void Connection::SendAuthorityLossImminentAcknowledgement(EntityId entity_id,
                                                          ComponentId component_id) {
  WorkerProtocol_Connection_SendAuthorityLossImminentAcknowledgement(connection.get(), entity_id,
                                                                     component_id);
}

void Connection::SetProtocolLoggingEnabled(bool enabled) {
  WorkerProtocol_Connection_SetProtocolLoggingEnabled(connection.get(), enabled);
}

Connection::Connection(const detail::ComponentInfo& component_info,
                       WorkerProtocol_Connection* connection)
: component_info{component_info}, connection{connection, WorkerProtocol_Connection_Destroy} {}

// Implementation of OpList.
OpList::OpList(WorkerProtocol_OpList* op_list) : op_list{op_list, WorkerProtocol_OpList_Destroy} {}

// Implementation of HistogramMetric.
HistogramMetric::HistogramMetric(const List<double>& bucket_boundaries) : sum{0} {
  static constexpr double infinity = std::numeric_limits<double>::infinity();
  bool has_infinity = false;
  for (double le : bucket_boundaries) {
    has_infinity |= le == infinity;
    buckets.emplace_back(Bucket{le, 0});
  }
  if (!has_infinity) {
    buckets.emplace_back(Bucket{infinity, 0});
  }
}

HistogramMetric::HistogramMetric() : HistogramMetric(List<double>{}) {}

void HistogramMetric::ClearObservations() {
  for (auto& bucket : buckets) {
    bucket.Samples = 0;
  }
  sum = 0;
}

void HistogramMetric::RecordObservation(double value) {
  for (auto& bucket : buckets) {
    if (value <= bucket.UpperBound) {
      ++bucket.Samples;
    }
  }
  sum += value;
}

const List<HistogramMetric::Bucket>& HistogramMetric::Buckets() const {
  return buckets;
}

double HistogramMetric::Sum() const {
  return sum;
}

// Implementation of Metrics.
void Metrics::Merge(const Metrics& metrics) {
  if (metrics.Load) {
    Load = metrics.Load;
  }
  for (const auto& pair : metrics.GaugeMetrics) {
    GaugeMetrics[pair.first] = pair.second;
  }
  for (const auto& pair : metrics.HistogramMetrics) {
    HistogramMetrics[pair.first] = pair.second;
  }
}

// Implementation of Entity.
Entity::Entity(const Entity& entity) {
  for (const auto& pair : entity.components) {
    components.emplace(pair.first, pair.second->Copy());
  }
}

Entity& Entity::operator=(const Entity& entity) {
  if (&entity != this) {
    components.clear();
    for (const auto& pair : entity.components) {
      components.emplace(pair.first, pair.second->Copy());
    }
  }
  return *this;
}

List<ComponentId> Entity::GetComponentIds() const {
  List<ComponentId> ids;
  for (const auto& kv : components) {
    ids.emplace_back(kv.first);
  }
  return ids;
}

// Implementation of Dispatcher.
Dispatcher::Dispatcher(const ComponentRegistry& registry)
: impl{registry.GetInternalComponentInfo(), WorkerProtocol_Dispatcher_Create(),
       WorkerProtocol_Dispatcher_Destroy} {
  // Free function callbacks that pass control back to the Dispatcher. This allows the Dispatcher to
  // take std::function callbacks despite the C API being defined in terms of raw function pointers.
  WorkerProtocol_Dispatcher_RegisterDisconnectCallback(impl.dispatcher.get(), &impl,
                                                       DispatcherDisconnectThunk);
  WorkerProtocol_Dispatcher_RegisterFlagUpdateCallback(impl.dispatcher.get(), &impl,
                                                       DispatcherFlagUpdateThunk);
  WorkerProtocol_Dispatcher_RegisterLogMessageCallback(impl.dispatcher.get(), &impl,
                                                       DispatcherLogMessageThunk);
  WorkerProtocol_Dispatcher_RegisterMetricsCallback(impl.dispatcher.get(), &impl,
                                                    DispatcherMetricsThunk);
  WorkerProtocol_Dispatcher_RegisterCriticalSectionCallback(impl.dispatcher.get(), &impl,
                                                            DispatcherCriticalSectionThunk);
  WorkerProtocol_Dispatcher_RegisterAddEntityCallback(impl.dispatcher.get(), &impl,
                                                      DispatcherAddEntityThunk);
  WorkerProtocol_Dispatcher_RegisterRemoveEntityCallback(impl.dispatcher.get(), &impl,
                                                         DispatcherRemoveEntityThunk);
  WorkerProtocol_Dispatcher_RegisterReserveEntityIdResponseCallback(
      impl.dispatcher.get(), &impl, DispatcherReserveEntityIdResponseThunk);
  WorkerProtocol_Dispatcher_RegisterReserveEntityIdsResponseCallback(
      impl.dispatcher.get(), &impl, DispatcherReserveEntityIdsResponseThunk);
  WorkerProtocol_Dispatcher_RegisterCreateEntityResponseCallback(
      impl.dispatcher.get(), &impl, DispatcherCreateEntityResponseThunk);
  WorkerProtocol_Dispatcher_RegisterDeleteEntityResponseCallback(
      impl.dispatcher.get(), &impl, DispatcherDeleteEntityResponseThunk);
  WorkerProtocol_Dispatcher_RegisterEntityQueryResponseCallback(impl.dispatcher.get(), &impl,
                                                                DispatcherEntityQueryResponseThunk);
  WorkerProtocol_Dispatcher_RegisterAddComponentCallback(impl.dispatcher.get(), &impl,
                                                         DispatcherAddComponentThunk);
  WorkerProtocol_Dispatcher_RegisterRemoveComponentCallback(impl.dispatcher.get(), &impl,
                                                            DispatcherRemoveComponentThunk);
  WorkerProtocol_Dispatcher_RegisterAuthorityChangeCallback(impl.dispatcher.get(), &impl,
                                                            DispatcherAuthorityChangeThunk);
  WorkerProtocol_Dispatcher_RegisterComponentUpdateCallback(impl.dispatcher.get(), &impl,
                                                            DispatcherComponentUpdateThunk);
  WorkerProtocol_Dispatcher_RegisterCommandRequestCallback(impl.dispatcher.get(), &impl,
                                                           DispatcherCommandRequestThunk);
  WorkerProtocol_Dispatcher_RegisterCommandResponseCallback(impl.dispatcher.get(), &impl,
                                                            DispatcherCommandResponseThunk);
}

Dispatcher::CallbackKey Dispatcher::OnDisconnect(const Callback<DisconnectOp>& callback) {
  impl.disconnect_callbacks.Add(impl.current_callback_key, callback);
  return impl.current_callback_key++;
}

Dispatcher::CallbackKey Dispatcher::OnFlagUpdate(const Callback<FlagUpdateOp>& callback) {
  impl.flag_update_callbacks.Add(impl.current_callback_key, callback);
  return impl.current_callback_key++;
}

Dispatcher::CallbackKey Dispatcher::OnLogMessage(const Callback<LogMessageOp>& callback) {
  impl.log_message_callbacks.Add(impl.current_callback_key, callback);
  return impl.current_callback_key++;
}

Dispatcher::CallbackKey Dispatcher::OnMetrics(const Callback<MetricsOp>& callback) {
  impl.metrics_callbacks.Add(impl.current_callback_key, callback);
  return impl.current_callback_key++;
}

Dispatcher::CallbackKey Dispatcher::OnCriticalSection(const Callback<CriticalSectionOp>& callback) {
  impl.critical_section_callbacks.Add(impl.current_callback_key, callback);
  return impl.current_callback_key++;
}

Dispatcher::CallbackKey Dispatcher::OnAddEntity(const Callback<AddEntityOp>& callback) {
  impl.add_entity_callbacks.Add(impl.current_callback_key, callback);
  return impl.current_callback_key++;
}

Dispatcher::CallbackKey Dispatcher::OnRemoveEntity(const Callback<RemoveEntityOp>& callback) {
  impl.remove_entity_callbacks.Add(impl.current_callback_key, callback);
  return impl.current_callback_key++;
}

Dispatcher::CallbackKey
Dispatcher::OnReserveEntityIdResponse(const Callback<ReserveEntityIdResponseOp>& callback) {
  impl.reserve_entity_id_response_callbacks.Add(impl.current_callback_key, callback);
  return impl.current_callback_key++;
}

Dispatcher::CallbackKey
Dispatcher::OnReserveEntityIdsResponse(const Callback<ReserveEntityIdsResponseOp>& callback) {
  impl.reserve_entity_ids_response_callbacks.Add(impl.current_callback_key, callback);
  return impl.current_callback_key++;
}

Dispatcher::CallbackKey
Dispatcher::OnCreateEntityResponse(const Callback<CreateEntityResponseOp>& callback) {
  impl.create_entity_response_callbacks.Add(impl.current_callback_key, callback);
  return impl.current_callback_key++;
}

Dispatcher::CallbackKey
Dispatcher::OnDeleteEntityResponse(const Callback<DeleteEntityResponseOp>& callback) {
  impl.delete_entity_response_callbacks.Add(impl.current_callback_key, callback);
  return impl.current_callback_key++;
}

Dispatcher::CallbackKey
Dispatcher::OnEntityQueryResponse(const Callback<EntityQueryResponseOp>& callback) {
  impl.entity_query_response_callbacks.Add(impl.current_callback_key, callback);
  return impl.current_callback_key++;
}

void Dispatcher::Remove(CallbackKey key) {
  if (!impl.disconnect_callbacks.Remove(key) && !impl.flag_update_callbacks.Remove(key) &&
      !impl.log_message_callbacks.Remove(key) && !impl.metrics_callbacks.Remove(key) &&
      !impl.critical_section_callbacks.Remove(key) && !impl.add_entity_callbacks.Remove(key) &&
      !impl.remove_entity_callbacks.Remove(key) &&
      !impl.reserve_entity_id_response_callbacks.Remove(key) &&
      !impl.reserve_entity_ids_response_callbacks.Remove(key) &&
      !impl.create_entity_response_callbacks.Remove(key) &&
      !impl.delete_entity_response_callbacks.Remove(key) &&
      !impl.entity_query_response_callbacks.Remove(key) &&
      !impl.add_component_callbacks.Remove(key) && !impl.remove_component_callbacks.Remove(key) &&
      !impl.authority_change_callbacks.Remove(key) &&
      !impl.component_update_callbacks.Remove(key) && !impl.command_request_callbacks.Remove(key) &&
      !impl.command_response_callbacks.Remove(key)) {
    std::terminate();
  }
}

void Dispatcher::Process(const OpList& op_list) const {
  WorkerProtocol_Dispatcher_Process(impl.dispatcher.get(), op_list.op_list.get());
}

SnapshotInputStream::SnapshotInputStream(const ComponentRegistry& registry, const std::string& path)
: component_info{registry.GetInternalComponentInfo()}
, input_stream{MakeSnapshotInputStream(component_info, path),
               WorkerProtocol_SnapshotInputStream_Destroy} {}

bool SnapshotInputStream::HasNext() {
  return 0 != WorkerProtocol_SnapshotInputStream_HasNext(input_stream.get());
}

Option<std::string> SnapshotInputStream::ReadEntity(EntityId& entity_id, Entity& entity) {
  entity = Entity{};
  // WorkerProtocol_SnapshotInputStream_ReadEntity manages the memory for wp_entity internally.
  const WorkerProtocol_Entity* wp_entity =
      WorkerProtocol_SnapshotInputStream_ReadEntity(input_stream.get());

  const char* error = WorkerProtocol_SnapshotInputStream_GetError(input_stream.get());
  if (error) {
    return {error};
  }
  entity_id = wp_entity->EntityId;
  for (std::uint32_t i = 0; i < wp_entity->ComponentCount; ++i) {
    const auto& component = wp_entity->Component[i];
    auto it = component_info.MoveSnapshotIntoEntity.find(component.ComponentId);
    if (it != component_info.MoveSnapshotIntoEntity.end()) {
      // Moves the snapshot data out of the C-owned object. Fine to steal the data here.
      it->second(const_cast<detail::ClientHandleBase*>(
                     static_cast<const detail::ClientHandleBase*>(component.Handle)),
                 entity);
    }
  }
  return {};
}

SnapshotOutputStream::SnapshotOutputStream(const ComponentRegistry& registry,
                                           const std::string& path)
: component_info{registry.GetInternalComponentInfo()}
, output_stream{MakeSnapshotOutputStream(component_info, path),
                WorkerProtocol_SnapshotOutputStream_Destroy} {
  // We are never handling failure in constructor, the caller will be notified when they
  // attempt to use stream again.
}

Option<std::string> SnapshotOutputStream::WriteEntity(EntityId entity_id, const Entity& entity) {
  WorkerProtocol_Entity snapshot_entity;
  snapshot_entity.EntityId = entity_id;
  std::vector<std::unique_ptr<detail::ClientHandleBase>> handle_storage;
  std::unique_ptr<WorkerProtocol_ComponentHandle[]> component_snapshots;
  ExtractEntityComponents(component_info, entity, snapshot_entity.ComponentCount, handle_storage,
                          component_snapshots);
  snapshot_entity.Component = component_snapshots.get();
  if (0 == WorkerProtocol_SnapshotOutputStream_WriteEntity(output_stream.get(), &snapshot_entity)) {
    const char* error = WorkerProtocol_SnapshotOutputStream_GetError(output_stream.get());
    if (error) {
      return {error};
    }
  }
  return {};
}

}  // ::worker
