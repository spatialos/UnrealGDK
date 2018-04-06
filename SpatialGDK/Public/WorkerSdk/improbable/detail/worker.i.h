// Copyright (c) Improbable Worlds Ltd, All Rights Reserved
#ifndef WORKER_SDK_CPP_INCLUDE_IMPROBABLE_DETAIL_WORKER_I_H
#define WORKER_SDK_CPP_INCLUDE_IMPROBABLE_DETAIL_WORKER_I_H
// Not strictly necessary, since this file shouldn't be included on its own, but may as well make it
// standalone for the benefit of automated tooling.
#include <improbable/worker.h>

namespace std {
template <typename T>
struct hash<worker::RequestId<T>> {
  std::size_t operator()(const worker::RequestId<T>& key) const {
    return std::hash<std::uint32_t>{}(key.Id);
  }
};
}  // ::std

namespace worker {
namespace detail {

template <typename H>
struct DynamicDispatch {
  virtual void InvokeHandler(H& handler) const = 0;
};

template <typename H, typename T>
struct DispatchImpl : DynamicDispatch<H> {
  void InvokeHandler(H& handler) const override {
    handler.template Accept<T>();
  }
};

template <typename H>
using DynamicDispatchMap = std::unordered_map<ComponentId, std::unique_ptr<DynamicDispatch<H>>>;

template <typename H>
struct RegisterHandler {
  DynamicDispatchMap<H>& dispatch_map;

  template <typename T>
  void Accept() const {
    dispatch_map.emplace(T::ComponentId,
                         std::unique_ptr<DynamicDispatch<H>>{new DispatchImpl<H, T>});
  }
};

template <typename... T>
struct MultiDispatch {
  // MultiDispatch<T0, T1, ...>::InvokeForAll(handler) calls each of
  // handler.Accept<T0>(), handler.Accept<T1>() and so on in order.
  template <typename H>
  static void InvokeForAll(H&& handler) {
    using evaluate = int[];
    (void)handler;  // MSVC incorrectly thinks this is an unused variable.
    (void)evaluate{0, ((void)DispatchImpl<H, T>{}.InvokeHandler(handler), 0)...};
  }

  template <typename H>
  static DynamicDispatchMap<H> CreateDynamicDispatchMap() {
    DynamicDispatchMap<H> result;
    InvokeForAll(RegisterHandler<H>{result});
    return result;
  }
};

struct ComponentInfo {
  std::vector<ComponentVtable> Vtables;
  std::unordered_map<ComponentId, std::function<std::unique_ptr<ClientHandleBase>(const Entity&)>>
      ExtractSnapshot;
  std::unordered_map<ComponentId, std::function<void(ClientHandleBase*, Entity&)>>
      MoveSnapshotIntoEntity;
};

struct CreateComponentInfoHandler {
  ComponentInfo& component_info;
  template <typename T>
  void Accept() const {
    using DataHandle = ClientHandle<typename T::Data>;
    component_info.Vtables.emplace_back(T::Vtable());
    component_info.ExtractSnapshot[T::ComponentId] = [](const Entity& entity) {
      return std::unique_ptr<ClientHandleBase>{
          new DataHandle{DataHandle::copyable(entity.Get<T>().data())}};
    };
    component_info.MoveSnapshotIntoEntity[T::ComponentId] = [](ClientHandleBase* snapshot,
                                                               Entity& entity) {
      entity.Add<T>(std::move(DataHandle::get_mutable(snapshot)));
    };
  }
};

template <typename T>
std::unique_ptr<ComponentStorageBase> ComponentStorage<T>::Copy() const {
  return std::unique_ptr<ComponentStorageBase>{new ComponentStorage{data}};
}

template <typename T>
const typename T::Data& ComponentStorage<T>::Get() const {
  return data;
}

template <typename T>
typename T::Data& ComponentStorage<T>::Get() {
  return data;
}

template <typename T>
CallbackMap<T>::CallbackMap() : call_depth{0} {}

template <typename T>
void CallbackMap<T>::Add(CallbackKey key, const Callback<T>& callback) {
  UpdateGuard update_guard{*this};
  to_add[key] = callback;
  to_remove.erase(key);
}

template <typename T>
bool CallbackMap<T>::Remove(CallbackKey key) {
  UpdateGuard update_guard{*this};
  bool is_in_map = map.find(key) != map.end();
  bool had_pending_add = to_add.erase(key);

  if (is_in_map) {
    bool created_pending_remove = to_remove.emplace(key, /* ignored */ 0).second;
    return created_pending_remove;
  } else {
    return had_pending_add;
  }
}

template <typename T>
void CallbackMap<T>::InvokeAll(const T& op) const {
  UpdateGuard update_guard{*const_cast<CallbackMap<T>*>(this)};
  for (const auto& pair : map) {
    pair.second(op);
  }
}

template <typename T>
void CallbackMap<T>::ReverseInvokeAll(const T& op) const {
  UpdateGuard update_guard{*const_cast<CallbackMap<T>*>(this)};
  for (auto it = map.rbegin(), end = map.rend(); it != end; ++it) {
    it->second(op);
  }
}

template <typename T>
void CallbackMap<T>::UpdateCallbacks() {
  map.insert(to_add.begin(), to_add.end());
  to_add.clear();
  for (const auto& pair : to_remove) {
    map.erase(pair.first);
  }
  to_remove.clear();
}

template <typename T>
void ComponentCallbackMap<T>::Add(ComponentId component_id, CallbackKey key,
                                  const Callback<T>& callback) {
  map[component_id].Add(key, callback);
}

template <typename T>
bool ComponentCallbackMap<T>::Remove(CallbackKey key) {
  for (auto& pair : map) {
    if (pair.second.Remove(key)) {
      return true;
    }
  }
  return false;
}

template <typename T>
void ComponentCallbackMap<T>::InvokeAll(ComponentId component_id, const T& op) const {
  auto it = map.find(component_id);
  if (it != map.end()) {
    it->second.InvokeAll(op);
  }
}

template <typename T>
void ComponentCallbackMap<T>::ReverseInvokeAll(ComponentId component_id, const T& op) const {
  auto it = map.find(component_id);
  if (it != map.end()) {
    it->second.ReverseInvokeAll(op);
  }
}

}  // ::detail

template <typename... Args>
detail::ComponentInfo& Components<Args...>::GetInternalComponentInfo() const {
  static auto component_info = [this] {
    detail::ComponentInfo result;
    ForEachComponent(*this, detail::CreateComponentInfoHandler{result});
    return result;
  }();
  return component_info;
}

template <typename T>
RequestId<T>::RequestId() : Id{0} {}

template <typename T>
RequestId<T>::RequestId(std::uint32_t id) : Id{id} {}

template <typename T>
bool RequestId<T>::operator==(const RequestId& other) const {
  return Id == other.Id;
}

template <typename T>
bool RequestId<T>::operator!=(const RequestId& other) const {
  return !operator==(other);
}

template <typename T>
bool Future<T>::Wait(const Option<std::uint32_t>& timeout_millis) {
  if (!impl->result) {
    impl->result = impl->get(timeout_millis);
  }
  return static_cast<bool>(impl->result);
}

template <typename T>
T Future<T>::Get() {
  if (impl->moved_from) {
    std::terminate();
  }
  if (!impl->result) {
    Wait({});
  }
  impl->moved_from = true;
  return std::move(*impl->result);
}

template <typename T>
Future<T>::Future(const std::function<Option<T>(const Option<std::uint32_t>&)> get,
                  const std::function<void()>& destroy)
: impl{new detail::FutureImpl<T>{get, destroy, false, {}}} {}

template <typename T>
void Connection::SendAuthorityLossImminentAcknowledgement(EntityId entity_id) {
  SendAuthorityLossImminentAcknowledgement(entity_id, T::ComponentId);
}

template <typename T>
void Connection::SendComponentUpdate(EntityId entity_id, const typename T::Update& update,
                                     const UpdateParameters& parameters) {
  auto handle = detail::ClientHandle<typename T::Update>::copyable(&update);
  detail::SendClientComponentUpdate(connection.get(), entity_id, T::ComponentId, &handle,
                                    parameters);
}

template <typename T>
void Connection::SendComponentUpdate(EntityId entity_id, typename T::Update&& update,
                                     const UpdateParameters& parameters) {
  auto handle = detail::ClientHandle<typename T::Update>::movable(&update);
  detail::SendClientComponentUpdate(connection.get(), entity_id, T::ComponentId, &handle,
                                    parameters);
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

template <typename T>
Option<typename T::Data&> Entity::Get() {
  auto it = components.find(T::ComponentId);
  if (it == components.end()) {
    return {};
  }
  return static_cast<detail::ComponentStorage<T>&>(*it->second).Get();
}

template <typename T>
Option<const typename T::Data&> Entity::Get() const {
  auto it = components.find(T::ComponentId);
  if (it == components.end()) {
    return {};
  }
  return static_cast<const detail::ComponentStorage<T>&>(*it->second).Get();
}

template <typename T>
void Entity::Add(const typename T::Data& data) {
  auto it = components.find(T::ComponentId);
  if (it == components.end()) {
    components.emplace(T::ComponentId, std::unique_ptr<detail::ComponentStorageBase>{
                                           new detail::ComponentStorage<T>{data}});
  }
}

template <typename T>
void Entity::Add(typename T::Data&& data) {
  auto it = components.find(T::ComponentId);
  if (it == components.end()) {
    components.emplace(T::ComponentId, std::unique_ptr<detail::ComponentStorageBase>{
                                           new detail::ComponentStorage<T>{std::move(data)}});
  }
}

template <typename T>
void Entity::Update(const typename T::Update& update) {
  auto it = components.find(T::ComponentId);
  if (it != components.end()) {
    update.ApplyTo(static_cast<detail::ComponentStorage<T>&>(*it->second).Get());
  }
}

template <typename T>
void Entity::Remove() {
  components.erase(T::ComponentId);
}

template <typename T>
Dispatcher::CallbackKey Dispatcher::OnAddComponent(const Callback<AddComponentOp<T>>& callback) {
  auto wrapper_callback = [callback](const detail::DispatcherImpl::ComponentWrapperOp& op) {
    callback(
        AddComponentOp<T>{op.EntityId, detail::ClientHandle<typename T::Data>::get(op.Handle)});
  };
  impl.add_component_callbacks.Add(T::ComponentId, impl.current_callback_key, wrapper_callback);
  return impl.current_callback_key++;
}

template <typename T>
Dispatcher::CallbackKey Dispatcher::OnRemoveComponent(const Callback<RemoveComponentOp>& callback) {
  impl.remove_component_callbacks.Add(T::ComponentId, impl.current_callback_key, callback);
  return impl.current_callback_key++;
}

template <typename T>
Dispatcher::CallbackKey Dispatcher::OnAuthorityChange(const Callback<AuthorityChangeOp>& callback) {
  impl.authority_change_callbacks.Add(T::ComponentId, impl.current_callback_key, callback);
  return impl.current_callback_key++;
}

template <typename T>
Dispatcher::CallbackKey
Dispatcher::OnComponentUpdate(const Callback<ComponentUpdateOp<T>>& callback) {
  auto wrapper_callback = [callback](const detail::DispatcherImpl::ComponentWrapperOp& op) {
    callback(ComponentUpdateOp<T>{op.EntityId,
                                  detail::ClientHandle<typename T::Update>::get(op.Handle)});
  };
  impl.component_update_callbacks.Add(T::ComponentId, impl.current_callback_key, wrapper_callback);
  return impl.current_callback_key++;
}

template <typename T>
Dispatcher::CallbackKey
Dispatcher::OnCommandRequest(const Callback<CommandRequestOp<T>>& callback) {
  auto wrapper_callback = [callback](const detail::DispatcherImpl::CommandRequestWrapperOp& op) {
    const auto& generic_request =
        detail::ClientHandle<typename T::ComponentMetaclass::GenericCommandObject>::get(op.Request);
    if (generic_request.CommandId != T::CommandId) {
      return;
    }
    CommandRequestOp<T> wrapper{
        RequestId<IncomingCommandRequest<T>>{op.RequestId},
        op.EntityId,
        op.TimeoutMillis,
        op.CallerWorkerId,
        /* CallerAttributeSet */ {},
        *generic_request.CommandObject.template data<typename T::Request>()};
    for (std::uint32_t i = 0; i < op.CallerAttributeCount; ++i) {
      wrapper.CallerAttributeSet.emplace_back(op.CallerAttribute[i]);
    }
    callback(wrapper);
  };
  impl.command_request_callbacks.Add(T::ComponentMetaclass::ComponentId, impl.current_callback_key,
                                     wrapper_callback);
  return impl.current_callback_key++;
}

template <typename T>
Dispatcher::CallbackKey
Dispatcher::OnCommandResponse(const Callback<CommandResponseOp<T>>& callback) {
  auto wrapper_callback = [callback](const detail::DispatcherImpl::CommandResponseWrapperOp& op) {
    if (op.CommandId != T::CommandId) {
      return;
    }
    CommandResponseOp<T> wrapper{RequestId<OutgoingCommandRequest<T>>{op.RequestId},
                                 op.EntityId,
                                 static_cast<StatusCode>(op.StatusCode),
                                 op.Message,
                                 {}};
    if (op.Response) {
      const auto& generic_response =
          detail::ClientHandle<typename T::ComponentMetaclass::GenericCommandObject>::get(
              op.Response);
      wrapper.Response.emplace(
          *generic_response.CommandObject.template data<typename T::Response>());
    }
    callback(wrapper);
  };
  impl.command_response_callbacks.Add(T::ComponentMetaclass::ComponentId, impl.current_callback_key,
                                      wrapper_callback);
  return impl.current_callback_key++;
}

template <typename... T, typename Handler>
void ForComponent(const Components<T...>&, ComponentId component_id, Handler&& handler) {
  static const auto dynamic_dispatch_map =
      detail::MultiDispatch<T...>::template CreateDynamicDispatchMap<Handler>();
  auto it = dynamic_dispatch_map.find(component_id);
  if (it != dynamic_dispatch_map.end()) {
    it->second->InvokeHandler(std::forward<Handler>(handler));
  }
}

template <typename... T, typename Handler>
void ForEachComponent(const Components<T...>&, Handler&& handler) {
  detail::MultiDispatch<T...>::InvokeForAll(std::forward<Handler>(handler));
}

}  // ::worker

#endif  // WORKER_SDK_CPP_INCLUDE_IMPROBABLE_DETAIL_WORKER_I_H
