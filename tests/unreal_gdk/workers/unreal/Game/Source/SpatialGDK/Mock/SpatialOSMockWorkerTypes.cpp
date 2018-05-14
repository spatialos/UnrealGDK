#include "SpatialOSMockWorkerTypes.h"
#include "improbable/worker_protocol.h"

DEFINE_LOG_CATEGORY(LogSpatialOSMock);

// Implementation of Locator.
MockLocator::MockLocator(const std::string& hostname, const worker::LocatorParameters& params)
{
  // TODO mock it
}

MockFuture<worker::DeploymentList> MockLocator::GetDeploymentListAsync()
{
  return MockFuture<worker::DeploymentList>(
      [this](const worker::Option<std::uint32_t>& timeout_millis) {
        worker::Option<worker::DeploymentList> result;
        // TODO return mocked future deployment list
        return result;
      },
      [this] {
        // TODO destroy future
      });
}

MockFuture<MockConnection> MockLocator::ConnectAsync(const worker::ComponentRegistry& registry,
                                                     const std::string& deployment_name,
                                                     const worker::ConnectionParameters& params,
                                                     const QueueStatusCallback& callback)
{
  return MockFuture<MockConnection>(
      [this](const worker::Option<std::uint32_t>& timeout_millis) {
        worker::Option<MockConnection> result = worker::Option<MockConnection>(MockConnection());
        UE_LOG(LogSpatialOSMock, Log, TEXT("created mocked connection MockLocator::ConnectAsync"));
        return result;
      },
      [this] {
        // TODO mock destroy connection future
      });
}

// Implementation of Connection.
MockFuture<MockConnection> MockConnection::ConnectAsync(const worker::ComponentRegistry& registry,
                                                        const std::string& hostname,
                                                        std::uint16_t port,
                                                        const std::string& worker_id,
                                                        const worker::ConnectionParameters& params)
{
  return MockFuture<MockConnection>(
      [](const worker::Option<std::uint32_t>& timeout_millis) {
        worker::Option<MockConnection> result = worker::Option<MockConnection>(MockConnection());
        UE_LOG(LogSpatialOSMock, Log,
               TEXT("created mocked connection from MockConnection::ConnectAsync"));

        return result;
      },
      [] {
        // TODO mock destroy connection future
      });
}

bool MockConnection::IsConnected() const
{
  // TODO mock connected
  return true;
}

std::string MockConnection::GetWorkerId() const
{
  // TODO mock worker id
  return "mock-worker-id";
}

worker::List<std::string> MockConnection::GetWorkerAttributes() const
{
  // TODO mock worker attributes
  worker::List<std::string> result;
  for (std::uint32_t i = 0; i < 0; ++i)
  {
    result.emplace_back("");
  }
  return result;
}

worker::Option<std::string> MockConnection::GetWorkerFlag(const std::string& flag_name) const
{
  worker::Option<std::string> flag_value;
  // TODO mock flag value
  return flag_value;
}

SpatialOSOpList MockConnection::GetOpList(std::uint32_t timeout_millis)
{
  auto opsList = CurrentOpList;
  CurrentOpList = MockOpList();
  return opsList;
}

void MockConnection::SendLogMessage(worker::LogLevel level, const std::string& logger_name,
                                    const std::string& message,
                                    const worker::Option<worker::EntityId>& entity_id)
{
  // TODO mock log
}

void MockConnection::SendMetrics(worker::Metrics& metrics)
{
  // TODO mock metrics
}

worker::RequestId<worker::ReserveEntityIdRequest>
MockConnection::SendReserveEntityIdRequest(const worker::Option<std::uint32_t>& timeout_millis)
{
  // mock opList
  worker::ReserveEntityIdResponseOp ops{MockSendReserveEntityIdRequestReturn,  // request id
                                        worker::StatusCode::kSuccess,          // status code
                                        "mock-response", worker::Option<worker::EntityId>(1)};
  CurrentOpList.Ops.push_back(ops);
  return MockSendReserveEntityIdRequestReturn;
}

worker::RequestId<worker::ReserveEntityIdsRequest>
MockConnection::SendReserveEntityIdsRequest(std::uint32_t number_of_entity_ids,
                                            const worker::Option<std::uint32_t>& timeout_millis)
{
  // mock request
  return worker::RequestId<worker::ReserveEntityIdsRequest>{};
}

worker::RequestId<worker::CreateEntityRequest>
MockConnection::SendCreateEntityRequest(const worker::Entity& entity,
                                        const worker::Option<worker::EntityId>& entity_id,
                                        const worker::Option<std::uint32_t>& timeout_millis)
{
  // mock request
  return worker::RequestId<worker::CreateEntityRequest>{};
}

worker::RequestId<worker::DeleteEntityRequest>
MockConnection::SendDeleteEntityRequest(worker::EntityId entity_id,
                                        const worker::Option<std::uint32_t>& timeout_millis)
{
  // mock request
  return worker::RequestId<worker::DeleteEntityRequest>{};
}

worker::RequestId<worker::EntityQueryRequest>
MockConnection::SendEntityQueryRequest(const worker::query::EntityQuery& entity_query,
                                       const worker::Option<std::uint32_t>& timeout_millis)
{
  // TODO mock request
  return worker::RequestId<worker::EntityQueryRequest>{};
}

void MockConnection::SendComponentInterest(
    worker::EntityId entity_id,
    const worker::Map<worker::ComponentId, worker::InterestOverride>& interest_overrides)
{
  // TODO mock component interest
}

void MockConnection::SendAuthorityLossImminentAcknowledgement(worker::EntityId entity_id,
                                                              worker::ComponentId component_id)
{
  // TODO mock ack
}

void MockConnection::SetProtocolLoggingEnabled(bool enabled)
{
  // TODO mock logging
}

MockConnection::MockConnection(const worker::detail::ComponentInfo& component_info)
{
  // TODO mock connection
}

void MockConnection::PushAddEntityOpToOpsList(worker::EntityId entity_id)
{
  worker::AddEntityOp AddEntityOp{entity_id};
  CurrentOpList.Ops.push_back(AddEntityOp);
}

// Implementation of Dispatcher.
MockDispatcher::MockDispatcher(const worker::ComponentRegistry& registry)
{
  // Free function callbacks that pass control back to the Dispatcher. This allows the Dispatcher to
  // take std::function callbacks despite the C API being defined in terms of raw function pointers.
}

MockDispatcher::CallbackKey
MockDispatcher::OnDisconnect(const Callback<worker::DisconnectOp>& callback)
{
  disconnect_callbacks.Add(current_callback_key, callback);
  return current_callback_key++;
}

MockDispatcher::CallbackKey
MockDispatcher::OnFlagUpdate(const Callback<worker::FlagUpdateOp>& callback)
{
  flag_update_callbacks.Add(current_callback_key, callback);
  return current_callback_key++;
}

MockDispatcher::CallbackKey
MockDispatcher::OnLogMessage(const Callback<worker::LogMessageOp>& callback)
{
  log_message_callbacks.Add(current_callback_key, callback);
  return current_callback_key++;
}

MockDispatcher::CallbackKey MockDispatcher::OnMetrics(const Callback<worker::MetricsOp>& callback)
{
  metrics_callbacks.Add(current_callback_key, callback);
  return current_callback_key++;
}

MockDispatcher::CallbackKey
MockDispatcher::OnCriticalSection(const Callback<worker::CriticalSectionOp>& callback)
{
  critical_section_callbacks.Add(current_callback_key, callback);
  return current_callback_key++;
}

MockDispatcher::CallbackKey
MockDispatcher::OnAddEntity(const Callback<worker::AddEntityOp>& callback)
{
  add_entity_callbacks.Add(current_callback_key, callback);
  return current_callback_key++;
}

MockDispatcher::CallbackKey
MockDispatcher::OnRemoveEntity(const Callback<worker::RemoveEntityOp>& callback)
{
  remove_entity_callbacks.Add(current_callback_key, callback);
  return current_callback_key++;
}

MockDispatcher::CallbackKey MockDispatcher::OnReserveEntityIdResponse(
    const Callback<worker::ReserveEntityIdResponseOp>& callback)
{
  reserve_entity_id_response_callbacks.Add(current_callback_key, callback);
  return current_callback_key++;
}

MockDispatcher::CallbackKey MockDispatcher::OnReserveEntityIdsResponse(
    const Callback<worker::ReserveEntityIdsResponseOp>& callback)
{
  reserve_entity_ids_response_callbacks.Add(current_callback_key, callback);
  return current_callback_key++;
}

MockDispatcher::CallbackKey
MockDispatcher::OnCreateEntityResponse(const Callback<worker::CreateEntityResponseOp>& callback)
{
  create_entity_response_callbacks.Add(current_callback_key, callback);
  return current_callback_key++;
}

MockDispatcher::CallbackKey
MockDispatcher::OnDeleteEntityResponse(const Callback<worker::DeleteEntityResponseOp>& callback)
{
  delete_entity_response_callbacks.Add(current_callback_key, callback);
  return current_callback_key++;
}

MockDispatcher::CallbackKey
MockDispatcher::OnEntityQueryResponse(const Callback<worker::EntityQueryResponseOp>& callback)
{
  entity_query_response_callbacks.Add(current_callback_key, callback);
  return current_callback_key++;
}

void MockDispatcher::Remove(CallbackKey key)
{
  if (!disconnect_callbacks.Remove(key) && !flag_update_callbacks.Remove(key) &&
      !log_message_callbacks.Remove(key) && !metrics_callbacks.Remove(key) &&
      !critical_section_callbacks.Remove(key) && !add_entity_callbacks.Remove(key) &&
      !remove_entity_callbacks.Remove(key) && !reserve_entity_id_response_callbacks.Remove(key) &&
      !reserve_entity_ids_response_callbacks.Remove(key) &&
      !create_entity_response_callbacks.Remove(key) &&
      !delete_entity_response_callbacks.Remove(key) &&
      !entity_query_response_callbacks.Remove(key) && !add_component_callbacks.Remove(key) &&
      !remove_component_callbacks.Remove(key) && !authority_change_callbacks.Remove(key) &&
      !component_update_callbacks.Remove(key) && !command_request_callbacks.Remove(key) &&
      !command_response_callbacks.Remove(key))
  {
    assert(false);
  }
}

void MockDispatcher::Process(const MockOpList& op_list) const
{
  for (const MockOp& op : op_list.Ops)
  {
    if (op.data<worker::ReserveEntityIdResponseOp>())
    {
      const auto& data = *op.data<worker::ReserveEntityIdResponseOp>();
      reserve_entity_id_response_callbacks.InvokeAll(data);
    }
    if (op.data<worker::AddEntityOp>())
    {
      const auto& data = *op.data<worker::AddEntityOp>();
      add_entity_callbacks.InvokeAll(data);
    }
    if (op.data<MockAddComponentOp>())
    {
      const auto& data = *op.data<MockAddComponentOp>();
      add_component_callbacks.InvokeAll(data.ComponentId, data);
    }
    if (op.data<MockUpdateComponentOp>())
    {
      const auto& data = *op.data<MockUpdateComponentOp>();
      component_update_callbacks.InvokeAll(data.ComponentId, data);
    }
    if (op.data<MockCommandRequestWrapperOp>())
    {
      const auto& data = *op.data<MockCommandRequestWrapperOp>();
      command_request_callbacks.InvokeAll(data.ComponentId, data);
    }
    if (op.data<MockCommandResponseWrapperOp>())
    {
      const auto& data = *op.data<MockCommandResponseWrapperOp>();
      command_response_callbacks.InvokeAll(data.ComponentId, data);
    }
  }
}