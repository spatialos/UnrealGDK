#pragma once
#include <improbable/worker.h>

#include "LogMacros.h"
#include "TestSchema.h"
#include "improbable/worker_protocol.h"
#include <assert.h>
#include <queue>

class MockConnection;
class MockDispatcher;
template <typename T>
class MockFuture;
class MockLocator;
class MockOpList;

using SpatialOSConnection = MockConnection;
using SpatialOSLocator = MockLocator;
using SpatialOSOpList = MockOpList;
using SpatialOSDispatcher = MockDispatcher;
template <class T>
using SpatialOSFuture = MockFuture<T>;

DECLARE_LOG_CATEGORY_EXTERN(LogSpatialOSMock, Log, All);

template <typename T>
class MockFuture
{
public:
  // Noncopyable, but movable.
  MockFuture(MockFuture&&) = default;
  MockFuture(const MockFuture&) = delete;
  MockFuture& operator=(MockFuture&&) = default;
  MockFuture& operator=(const MockFuture&) = delete;

  bool Wait(const worker::Option<std::uint32_t>& timeout_millis);

  T Get();

private:
  friend class MockConnection;
  friend class MockLocator;
  MockFuture(const std::function<worker::Option<T>(const worker::Option<std::uint32_t>&)> get,
             const std::function<void()>& destroy);
  std::unique_ptr<worker::detail::FutureImpl<T>> impl;
};

using ComponentDataType =
    worker::Variant<improbable::EntityAclData, improbable::MetadataData, improbable::PositionData,
                    improbable::PersistenceData, improbable::TestType1, improbable::TestType2,
                    improbable::BuiltInTypesData, improbable::CommandWithSameDelegateData>;

using ComponentUpdateType =
    worker::Variant<improbable::EntityAcl::Update, improbable::Metadata::Update,
                    improbable::Position::Update, improbable::Persistence::Update,
                    improbable::TestData1::Update, improbable::TestData2::Update,
                    improbable::BuiltInTypes::Update, improbable::CommandWithSameDelegate::Update>;

struct MockAddComponentOp
{
  worker::EntityId EntityId;
  worker::detail::internal::WorkerProtocol_ComponentId ComponentId;
  ComponentDataType Object;
};

struct MockUpdateComponentOp
{
  worker::EntityId EntityId;
  worker::detail::internal::WorkerProtocol_ComponentId ComponentId;
  ComponentUpdateType Object;
};

using ComponentCommandObject =
    worker::Variant<improbable::detail::GenericCommandObject_TestData2,
                    improbable::detail::GenericCommandObject_TestData1,
                    improbable::detail::GenericCommandObject_BuiltInTypes,
                    improbable::detail::GenericCommandObject_CommandWithSameDelegate>;

struct MockCommandRequest
{
  ComponentCommandObject Request;
};

struct MockCommandResponse
{
  ComponentCommandObject Response;
};

struct MockCommandRequestWrapperOp
{
  worker::detail::internal::WorkerProtocol_ComponentId ComponentId;
  std::uint32_t RequestId;
  worker::EntityId EntityId;
  std::uint32_t TimeoutMillis;
  const char* CallerWorkerId;
  std::uint32_t CallerAttributeCount;
  const char** CallerAttribute;
  MockCommandRequest Request;
};
struct MockCommandResponseWrapperOp
{
  worker::detail::internal::WorkerProtocol_ComponentId ComponentId;
  std::uint32_t RequestId;
  worker::EntityId EntityId;
  std::uint8_t StatusCode;
  const char* Message;
  MockCommandResponse Response;
  std::uint32_t CommandId;
};

using MockOp = worker::Variant<worker::ReserveEntityIdResponseOp, worker::AddEntityOp,
                               MockAddComponentOp, MockUpdateComponentOp,
                               MockCommandRequestWrapperOp, MockCommandResponseWrapperOp>;

class SDK_API MockOpList
{
public:
  MockOpList(const MockOpList&) = default;
  MockOpList(MockOpList&&) = default;
  MockOpList& operator=(const MockOpList&) = delete;
  MockOpList& operator=(MockOpList&&) = default;

  // mock section
  std::vector<MockOp> Ops;

  MockOpList() = default;
};

class SDK_API MockConnection
{
public:
  static MockFuture<MockConnection> ConnectAsync(const worker::ComponentRegistry& registry,
                                                 const std::string& hostname, std::uint16_t port,
                                                 const std::string& worker_id,
                                                 const worker::ConnectionParameters& params);

  // Noncopyable, but movable.
  MockConnection(const MockConnection&) = delete;
  MockConnection(MockConnection&&) = default;
  MockConnection& operator=(const MockConnection&) = delete;
  MockConnection& operator=(MockConnection&&) = default;

  bool IsConnected() const;

  std::string GetWorkerId() const;

  worker::List<std::string> GetWorkerAttributes() const;

  worker::Option<std::string> GetWorkerFlag(const std::string& flag_name) const;

  MockOpList GetOpList(std::uint32_t timeout_millis);

  /** Sends a log message for the worker to SpatialOS. */
  void SendLogMessage(worker::LogLevel level, const std::string& logger_name,
                      const std::string& message,
                      const worker::Option<worker::EntityId>& entity_id = {});

  void SendMetrics(worker::Metrics& metrics);

  worker::RequestId<worker::ReserveEntityIdRequest>
  SendReserveEntityIdRequest(const worker::Option<std::uint32_t>& timeout_millis);

  worker::RequestId<worker::ReserveEntityIdsRequest>
  SendReserveEntityIdsRequest(std::uint32_t number_of_entity_ids,
                              const worker::Option<std::uint32_t>& timeout_millis);

  worker::RequestId<worker::CreateEntityRequest>
  SendCreateEntityRequest(const worker::Entity& entity,
                          const worker::Option<worker::EntityId>& entity_id,
                          const worker::Option<std::uint32_t>& timeout_millis);

  worker::RequestId<worker::DeleteEntityRequest>
  SendDeleteEntityRequest(worker::EntityId entity_id,
                          const worker::Option<std::uint32_t>& timeout_millis);

  worker::RequestId<worker::EntityQueryRequest>
  SendEntityQueryRequest(const worker::query::EntityQuery& entity_query,
                         const worker::Option<std::uint32_t>& timeout_millis);

  void SendComponentInterest(
      worker::EntityId entity_id,
      const worker::Map<worker::ComponentId, worker::InterestOverride>& interest_overrides);

  void SendAuthorityLossImminentAcknowledgement(worker::EntityId entity_id,
                                                worker::ComponentId component_id);

  template <typename T>
  void SendComponentUpdate(worker::EntityId entity_id, const typename T::Update& update);

  template <typename T>
  void SendComponentUpdate(worker::EntityId entity_id, typename T::Update&& update);

  template <typename T>
  worker::RequestId<worker::OutgoingCommandRequest<T>>
  SendCommandRequest(worker::EntityId entity_id, const typename T::Request& request,
                     const worker::Option<std::uint32_t>& timeout_millis,
                     const worker::CommandParameters& parameters = {false});

  template <typename T>
  worker::RequestId<worker::OutgoingCommandRequest<T>>
  SendCommandRequest(worker::EntityId entity_id, typename T::Request&& request,
                     const worker::Option<std::uint32_t>& timeout_millis,
                     const worker::CommandParameters& parameters = {false});

  template <typename T>
  void SendCommandResponse(worker::RequestId<worker::IncomingCommandRequest<T>> request_id,
                           const typename T::Response& response);

  template <typename T>
  void SendCommandResponse(worker::RequestId<worker::IncomingCommandRequest<T>> request_id,
                           typename T::Response&& response);

  template <typename T>
  void SendCommandFailure(worker::RequestId<worker::IncomingCommandRequest<T>> request_id,
                          const std::string& message);

  void SetProtocolLoggingEnabled(bool enabled);

  /*
  * mock section
  */
  worker::detail::internal::WorkerProtocol_RequestId next_request_id;
  MockConnection::MockConnection() = default;
  MockConnection::MockConnection(const worker::detail::ComponentInfo& component_info);
  MockOpList CurrentOpList;
  std::queue<MockCommandResponse> MockCommandResponses;

  worker::RequestId<worker::ReserveEntityIdRequest> MockSendReserveEntityIdRequestReturn;

  /*
  * Mock helper functions
  */

  void PushAddEntityOpToOpsList(worker::EntityId entity_id);

  template <typename T>
  void PushAddComponentOpToOpsList(const worker::EntityId entity_id, const typename T::Data& data);

  template <typename T>
  void PushUpdateComponentOpToOpsList(const worker::EntityId entity_id,
                                      const typename T::Update& update);
};

class SDK_API MockLocator
{
public:
  MockLocator(const std::string& hostname, const worker::LocatorParameters& params);

  // Noncopyable, but movable.
  MockLocator(const MockLocator&) = delete;
  MockLocator(MockLocator&&) = default;
  MockLocator& operator=(const MockLocator&) = delete;
  MockLocator& operator=(MockLocator&&) = default;

  using QueueStatusCallback = std::function<bool(const worker::QueueStatus&)>;

  MockFuture<worker::DeploymentList> GetDeploymentListAsync();

  MockFuture<MockConnection> ConnectAsync(const worker::ComponentRegistry& registry,
                                          const std::string& deployment_name,
                                          const worker::ConnectionParameters& params,
                                          const QueueStatusCallback& callback);
};

class SDK_API MockDispatcher
{
public:
  template <typename T>
  using Callback = std::function<void(const T&)>;
  using CallbackKey = std::uint64_t;

  MockDispatcher(const worker::ComponentRegistry& registry);

  // Not copyable or movable.
  MockDispatcher(const MockDispatcher&) = delete;
  MockDispatcher(MockDispatcher&&) = delete;
  MockDispatcher& operator=(const MockDispatcher&) = delete;
  MockDispatcher& operator=(MockDispatcher&&) = delete;

  CallbackKey OnDisconnect(const Callback<worker::DisconnectOp>& callback);

  CallbackKey OnFlagUpdate(const Callback<worker::FlagUpdateOp>& callback);

  CallbackKey OnLogMessage(const Callback<worker::LogMessageOp>& callback);

  CallbackKey OnMetrics(const Callback<worker::MetricsOp>& callback);

  CallbackKey OnCriticalSection(const Callback<worker::CriticalSectionOp>& callback);

  CallbackKey OnAddEntity(const Callback<worker::AddEntityOp>& callback);

  CallbackKey OnRemoveEntity(const Callback<worker::RemoveEntityOp>& callback);

  CallbackKey
  OnReserveEntityIdResponse(const Callback<worker::ReserveEntityIdResponseOp>& callback);

  CallbackKey
  OnReserveEntityIdsResponse(const Callback<worker::ReserveEntityIdsResponseOp>& callback);

  CallbackKey OnCreateEntityResponse(const Callback<worker::CreateEntityResponseOp>& callback);

  CallbackKey OnDeleteEntityResponse(const Callback<worker::DeleteEntityResponseOp>& callback);

  CallbackKey OnEntityQueryResponse(const Callback<worker::EntityQueryResponseOp>& callback);

  template <typename T>
  CallbackKey OnAddComponent(const Callback<worker::AddComponentOp<T>>& callback);

  template <typename T>
  CallbackKey OnRemoveComponent(const Callback<worker::RemoveComponentOp>& callback);

  template <typename T>
  CallbackKey OnAuthorityChange(const Callback<worker::AuthorityChangeOp>& callback);

  template <typename T>
  CallbackKey OnComponentUpdate(const Callback<worker::ComponentUpdateOp<T>>& callback);

  template <typename T>
  CallbackKey OnCommandRequest(const Callback<worker::CommandRequestOp<T>>& callback);

  template <typename T>
  CallbackKey OnCommandResponse(const Callback<worker::CommandResponseOp<T>>& callback);

  void Remove(CallbackKey key);

  void Process(const MockOpList& op_list) const;

  // mock section
  CallbackKey current_callback_key;
  worker::detail::CallbackMap<worker::DisconnectOp> disconnect_callbacks;
  worker::detail::CallbackMap<worker::FlagUpdateOp> flag_update_callbacks;
  worker::detail::CallbackMap<worker::LogMessageOp> log_message_callbacks;
  worker::detail::CallbackMap<worker::MetricsOp> metrics_callbacks;
  worker::detail::CallbackMap<worker::CriticalSectionOp> critical_section_callbacks;
  worker::detail::CallbackMap<worker::AddEntityOp> add_entity_callbacks;
  worker::detail::CallbackMap<worker::RemoveEntityOp> remove_entity_callbacks;
  worker::detail::CallbackMap<worker::ReserveEntityIdResponseOp>
      reserve_entity_id_response_callbacks;
  worker::detail::CallbackMap<worker::ReserveEntityIdsResponseOp>
      reserve_entity_ids_response_callbacks;
  worker::detail::CallbackMap<worker::CreateEntityResponseOp> create_entity_response_callbacks;
  worker::detail::CallbackMap<worker::DeleteEntityResponseOp> delete_entity_response_callbacks;
  worker::detail::CallbackMap<worker::EntityQueryResponseOp> entity_query_response_callbacks;
  worker::detail::ComponentCallbackMap<MockAddComponentOp> add_component_callbacks;
  worker::detail::ComponentCallbackMap<worker::RemoveComponentOp> remove_component_callbacks;
  worker::detail::ComponentCallbackMap<worker::AuthorityChangeOp> authority_change_callbacks;
  worker::detail::ComponentCallbackMap<MockUpdateComponentOp> component_update_callbacks;
  worker::detail::ComponentCallbackMap<MockCommandRequestWrapperOp> command_request_callbacks;
  worker::detail::ComponentCallbackMap<MockCommandResponseWrapperOp> command_response_callbacks;
};

// template function headers
template <typename T>
bool MockFuture<T>::Wait(const worker::Option<std::uint32_t>& timeout_millis)
{
  if (!impl->result)
  {
    impl->result = impl->get(timeout_millis);
  }
  return static_cast<bool>(impl->result);
}

template <typename T>
T MockFuture<T>::Get()
{
  assert(!impl->moved_from);
  if (!impl->result)
  {
    Wait({});
  }
  impl->moved_from = true;
  return std::move(*impl->result);
}

template <typename T>
MockFuture<T>::MockFuture(
    const std::function<worker::Option<T>(const worker::Option<std::uint32_t>&)> get,
    const std::function<void()>& destroy)
: impl{new worker::detail::FutureImpl<T>{get, destroy, false, {}}}
{
}

template <typename T>
void MockConnection::SendComponentUpdate(worker::EntityId entity_id,
                                         const typename T::Update& update)
{
  // TODO mock it
}

template <typename T>
void MockConnection::SendComponentUpdate(worker::EntityId entity_id, typename T::Update&& update)
{
  // TODO mock it
}

template <typename T>
worker::RequestId<worker::OutgoingCommandRequest<T>>
MockConnection::SendCommandRequest(worker::EntityId entity_id, const typename T::Request& request,
                                   const worker::Option<std::uint32_t>& timeout_millis,
                                   const worker::CommandParameters& parameters)
{
  auto* generic_request =
      new typename T::ComponentMetaclass::GenericCommandObject{T::CommandId, request};

  // TODO mock it
  auto response = MockCommandResponses.front();
  MockCommandResponses.pop();
  auto requestId = next_request_id++;
  MockCommandResponseWrapperOp op{T::ComponentMetaclass::ComponentId,
                                  requestId,
                                  entity_id,
                                  static_cast<uint8_t>(worker::StatusCode::kSuccess),  // StatusCode
                                  "mock-command-response",
                                  response,
                                  T::CommandId};
  CurrentOpList.Ops.push_back(op);
  return worker::RequestId<worker::OutgoingCommandRequest<T>>{requestId};
}

template <typename T>
worker::RequestId<worker::OutgoingCommandRequest<T>>
MockConnection::SendCommandRequest(worker::EntityId entity_id, typename T::Request&& request,
                                   const worker::Option<std::uint32_t>& timeout_millis,
                                   const worker::CommandParameters& parameters)
{
  auto* generic_request =
      new typename T::ComponentMetaclass::GenericCommandObject{T::CommandId, std::move(request)};

  // TODO mock it
  return worker::RequestId<worker::OutgoingCommandRequest<T>>{next_request_id++};
}

template <typename T>
void MockConnection::SendCommandResponse(
    worker::RequestId<worker::IncomingCommandRequest<T>> request_id,
    const typename T::Response& response)
{
  auto* generic_response =
      new typename T::ComponentMetaclass::GenericCommandObject{T::CommandId, response};

  // TODO mock it
}

template <typename T>
void MockConnection::SendCommandResponse(
    worker::RequestId<worker::IncomingCommandRequest<T>> request_id,
    typename T::Response&& response)
{
  auto* generic_response =
      new typename T::ComponentMetaclass::GenericCommandObject{T::CommandId, std::move(response)};
  // TODO mock it
}

template <typename T>
void MockConnection::SendCommandFailure(
    worker::RequestId<worker::IncomingCommandRequest<T>> request_id, const std::string& message)
{
  // TODO mock it
}

template <typename T>
void MockConnection::PushAddComponentOpToOpsList(const worker::EntityId entity_id,
                                                 const typename T::Data& data)
{
  MockAddComponentOp add_component_op{entity_id, T::ComponentId, ComponentDataType{data}};
  CurrentOpList.Ops.push_back(add_component_op);
}

template <typename T>
void MockConnection::PushUpdateComponentOpToOpsList(const worker::EntityId entity_id,
                                                    const typename T::Update& update)
{
  MockUpdateComponentOp UpdateOp{entity_id, improbable::TestData2::ComponentId,
                                 ComponentUpdateType{update}};
  CurrentOpList.Ops.push_back(UpdateOp);
}

template <typename T>
MockDispatcher::CallbackKey
MockDispatcher::OnAddComponent(const Callback<worker::AddComponentOp<T>>& callback)
{
  auto wrapper_callback = [callback](const MockAddComponentOp& op) {
    auto data = op.Object.data<T::Data>();
    worker::AddComponentOp<T> addOp{op.EntityId, *data};
    callback(addOp);
  };
  add_component_callbacks.Add(T::ComponentId, current_callback_key, wrapper_callback);
  return current_callback_key++;
}

template <typename T>
MockDispatcher::CallbackKey
MockDispatcher::OnRemoveComponent(const Callback<worker::RemoveComponentOp>& callback)
{
  remove_component_callbacks.Add(T::ComponentId, current_callback_key, callback);
  return current_callback_key++;
}

template <typename T>
MockDispatcher::CallbackKey
MockDispatcher::OnAuthorityChange(const Callback<worker::AuthorityChangeOp>& callback)
{
  authority_change_callbacks.Add(T::ComponentId, current_callback_key, callback);
  return current_callback_key++;
}

template <typename T>
MockDispatcher::CallbackKey
MockDispatcher::OnComponentUpdate(const Callback<worker::ComponentUpdateOp<T>>& callback)
{
  auto wrapper_callback = [callback](const MockUpdateComponentOp& op) {
    auto data = op.Object.data<T::Update>();
    callback(worker::ComponentUpdateOp<T>{op.EntityId, *data});
  };
  component_update_callbacks.Add(T::ComponentId, current_callback_key, wrapper_callback);
  return current_callback_key++;
}

template <typename T>
MockDispatcher::CallbackKey
MockDispatcher::OnCommandRequest(const Callback<worker::CommandRequestOp<T>>& callback)
{
  auto wrapper_callback = [callback](const MockCommandRequestWrapperOp& op) {
    auto generic_request = op.Request.Request.data<T::ComponentMetaclass::GenericCommandObject>();
    if (generic_request == nullptr)
    {
      return;
    }
    if (generic_request->CommandId != T::CommandId)
    {
      return;
    }
    worker::CommandRequestOp<T> wrapper{
        worker::RequestId<worker::IncomingCommandRequest<T>>{op.RequestId},
        op.EntityId,
        op.TimeoutMillis,
        op.CallerWorkerId,
        /* CallerAttributeSet */ {},
        *generic_request->CommandObject.template data<typename T::Request>()};
    for (std::uint32_t i = 0; i < op.CallerAttributeCount; ++i)
    {
      wrapper.CallerAttributeSet.emplace_back(op.CallerAttribute[i]);
    }
    callback(wrapper);
  };
  command_request_callbacks.Add(T::ComponentMetaclass::ComponentId, current_callback_key,
                                wrapper_callback);
  return current_callback_key++;
}

template <typename T>
MockDispatcher::CallbackKey
MockDispatcher::OnCommandResponse(const Callback<worker::CommandResponseOp<T>>& callback)
{
  auto wrapper_callback = [callback](const MockCommandResponseWrapperOp& op) {
    if (op.CommandId != T::CommandId)
    {
      return;
    }
    worker::CommandResponseOp<T> wrapper{
        worker::RequestId<worker::OutgoingCommandRequest<T>>{op.RequestId},
        op.EntityId,
        static_cast<worker::StatusCode>(op.StatusCode),
        op.Message,
        {}};

    const auto generic_response =
        op.Response.Response.data<T::ComponentMetaclass::GenericCommandObject>();
    if (generic_response)
    {
      wrapper.Response.emplace(
          *generic_response->CommandObject.template data<typename T::Response>());
    }
    callback(wrapper);
  };
  command_response_callbacks.Add(T::ComponentMetaclass::ComponentId, current_callback_key,
                                 wrapper_callback);
  return current_callback_key++;
}
