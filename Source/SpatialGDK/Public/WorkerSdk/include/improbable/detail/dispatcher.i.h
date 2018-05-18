// Copyright (c) Improbable Worlds Ltd, All Rights Reserved
#ifndef WORKER_SDK_CPP_INCLUDE_IMPROBABLE_DETAIL_DISPATCHER_I_H
#define WORKER_SDK_CPP_INCLUDE_IMPROBABLE_DETAIL_DISPATCHER_I_H
#include <improbable/detail/client_handle.i.h>
#include <improbable/worker.h>

namespace worker {
namespace detail {

inline void DispatcherDisconnectThunk(void* user_data,
                                      const internal::WorkerProtocol_DisconnectOp* op) {
  const auto& impl = *static_cast<detail::DispatcherImpl*>(user_data);
  DisconnectOp wrapper{op->Reason};
  impl.disconnect_callbacks.InvokeAll(wrapper);
}

inline void DispatcherFlagUpdateThunk(void* user_data,
                                      const internal::WorkerProtocol_FlagUpdateOp* op) {
  const auto& impl = *static_cast<detail::DispatcherImpl*>(user_data);
  FlagUpdateOp wrapper{op->Name, {}};
  if (op->Value) {
    wrapper.Value.emplace(op->Value);
    impl.flag_update_callbacks.InvokeAll(wrapper);
  } else {
    impl.flag_update_callbacks.ReverseInvokeAll(wrapper);
  }
}

inline void DispatcherLogMessageThunk(void* user_data,
                                      const internal::WorkerProtocol_LogMessageOp* op) {
  const auto& impl = *static_cast<detail::DispatcherImpl*>(user_data);
  LogMessageOp wrapper{static_cast<LogLevel>(op->LogLevel), op->Message};
  impl.log_message_callbacks.InvokeAll(wrapper);
}

inline void DispatcherMetricsThunk(void* user_data, const internal::WorkerProtocol_MetricsOp* op) {
  const auto& impl = *static_cast<detail::DispatcherImpl*>(user_data);
  MetricsOp wrapper;
  for (std::size_t i = 0; i < op->Metrics.GaugeMetricCount; ++i) {
    wrapper.Metrics.GaugeMetrics[op->Metrics.GaugeMetric[i].Key] = op->Metrics.GaugeMetric[i].Value;
  }
  // We do not have any built-in histogram metrics.
  impl.metrics_callbacks.InvokeAll(wrapper);
}

inline void DispatcherCriticalSectionThunk(void* user_data,
                                           const internal::WorkerProtocol_CriticalSectionOp* op) {
  const auto& impl = *static_cast<detail::DispatcherImpl*>(user_data);
  CriticalSectionOp wrapper{op->InCriticalSection != 0};
  if (op->InCriticalSection) {
    impl.critical_section_callbacks.InvokeAll(wrapper);
  } else {
    impl.critical_section_callbacks.ReverseInvokeAll(wrapper);
  }
}

inline void DispatcherAddEntityThunk(void* user_data,
                                     const internal::WorkerProtocol_AddEntityOp* op) {
  const auto& impl = *static_cast<detail::DispatcherImpl*>(user_data);
  AddEntityOp wrapper{op->EntityId};
  impl.add_entity_callbacks.InvokeAll(wrapper);
}

inline void DispatcherRemoveEntityThunk(void* user_data,
                                        const internal::WorkerProtocol_RemoveEntityOp* op) {
  const auto& impl = *static_cast<detail::DispatcherImpl*>(user_data);
  RemoveEntityOp wrapper{op->EntityId};
  impl.remove_entity_callbacks.ReverseInvokeAll(wrapper);
}

inline void DispatcherReserveEntityIdResponseThunk(
    void* user_data, const internal::WorkerProtocol_ReserveEntityIdResponseOp* op) {
  const auto& impl = *static_cast<detail::DispatcherImpl*>(user_data);
  ReserveEntityIdResponseOp wrapper{RequestId<ReserveEntityIdRequest>{op->RequestId},
                                    static_cast<worker::StatusCode>(op->StatusCode), op->Message,
                                    op->StatusCode == internal::WORKER_PROTOCOL_STATUS_CODE_SUCCESS
                                        ? op->EntityId
                                        : Option<EntityId>{}};
  impl.reserve_entity_id_response_callbacks.InvokeAll(wrapper);
}

inline void DispatcherReserveEntityIdsResponseThunk(
    void* user_data, const internal::WorkerProtocol_ReserveEntityIdsResponseOp* op) {
  const auto& impl = *static_cast<detail::DispatcherImpl*>(user_data);
  ReserveEntityIdsResponseOp wrapper{RequestId<ReserveEntityIdsRequest>{op->RequestId},
                                     static_cast<worker::StatusCode>(op->StatusCode), op->Message,
                                     op->StatusCode == internal::WORKER_PROTOCOL_STATUS_CODE_SUCCESS
                                         ? op->FirstEntityId
                                         : Option<EntityId>{},
                                     op->NumberOfEntityIds};
  impl.reserve_entity_ids_response_callbacks.InvokeAll(wrapper);
}

inline void
DispatcherCreateEntityResponseThunk(void* user_data,
                                    const internal::WorkerProtocol_CreateEntityResponseOp* op) {
  const auto& impl = *static_cast<detail::DispatcherImpl*>(user_data);
  CreateEntityResponseOp wrapper{RequestId<CreateEntityRequest>{op->RequestId},
                                 static_cast<worker::StatusCode>(op->StatusCode), op->Message,
                                 op->StatusCode == internal::WORKER_PROTOCOL_STATUS_CODE_SUCCESS
                                     ? op->EntityId
                                     : Option<EntityId>{}};
  impl.create_entity_response_callbacks.InvokeAll(wrapper);
}

inline void
DispatcherDeleteEntityResponseThunk(void* user_data,
                                    const internal::WorkerProtocol_DeleteEntityResponseOp* op) {
  const auto& impl = *static_cast<detail::DispatcherImpl*>(user_data);
  DeleteEntityResponseOp wrapper{RequestId<DeleteEntityRequest>{op->RequestId}, op->EntityId,
                                 static_cast<worker::StatusCode>(op->StatusCode), op->Message};
  impl.delete_entity_response_callbacks.InvokeAll(wrapper);
}

inline void
DispatcherEntityQueryResponseThunk(void* user_data,
                                   const internal::WorkerProtocol_EntityQueryResponseOp* op) {
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

inline void DispatcherAddComponentThunk(void* user_data,
                                        const internal::WorkerProtocol_AddComponentOp* op) {
  const auto& impl = *static_cast<detail::DispatcherImpl*>(user_data);
  detail::DispatcherImpl::ComponentWrapperOp wrapper{
      op->EntityId, static_cast<const detail::ClientHandleBase*>(op->InitialComponent.Handle)};
  impl.add_component_callbacks.InvokeAll(op->InitialComponent.ComponentId, wrapper);
}

inline void DispatcherRemoveComponentThunk(void* user_data,
                                           const internal::WorkerProtocol_RemoveComponentOp* op) {
  const auto& impl = *static_cast<detail::DispatcherImpl*>(user_data);
  RemoveComponentOp wrapper{op->EntityId};
  impl.remove_component_callbacks.ReverseInvokeAll(op->ComponentId, wrapper);
}

inline void DispatcherAuthorityChangeThunk(void* user_data,
                                           const internal::WorkerProtocol_AuthorityChangeOp* op) {
  const auto& impl = *static_cast<detail::DispatcherImpl*>(user_data);
  AuthorityChangeOp wrapper{op->EntityId, static_cast<worker::Authority>(op->Authority)};
  if (wrapper.Authority == Authority::kAuthoritative) {
    impl.authority_change_callbacks.InvokeAll(op->ComponentId, wrapper);
  } else {
    impl.authority_change_callbacks.ReverseInvokeAll(op->ComponentId, wrapper);
  }
}

inline void DispatcherComponentUpdateThunk(void* user_data,
                                           const internal::WorkerProtocol_ComponentUpdateOp* op) {
  const auto& impl = *static_cast<detail::DispatcherImpl*>(user_data);
  detail::DispatcherImpl::ComponentWrapperOp wrapper{
      op->EntityId, static_cast<const detail::ClientHandleBase*>(op->Update.Handle)};
  impl.component_update_callbacks.InvokeAll(op->Update.ComponentId, wrapper);
}

inline void DispatcherCommandRequestThunk(void* user_data,
                                          const internal::WorkerProtocol_CommandRequestOp* op) {
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

inline void DispatcherCommandResponseThunk(void* user_data,
                                           const internal::WorkerProtocol_CommandResponseOp* op) {
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

}  // ::detail

// Implementation of Dispatcher.
inline Dispatcher::Dispatcher(const ComponentRegistry& registry)
: impl{registry.GetInternalComponentInfo(), detail::internal::WorkerProtocol_Dispatcher_Create(),
       &detail::internal::WorkerProtocol_Dispatcher_Destroy} {
  // Free function callbacks that pass control back to the Dispatcher. This allows the Dispatcher to
  // take std::function callbacks despite the C API being defined in terms of raw function pointers.
  detail::internal::WorkerProtocol_Dispatcher_RegisterDisconnectCallback(
      impl.dispatcher.get(), &impl, &detail::DispatcherDisconnectThunk);
  detail::internal::WorkerProtocol_Dispatcher_RegisterFlagUpdateCallback(
      impl.dispatcher.get(), &impl, &detail::DispatcherFlagUpdateThunk);
  detail::internal::WorkerProtocol_Dispatcher_RegisterLogMessageCallback(
      impl.dispatcher.get(), &impl, &detail::DispatcherLogMessageThunk);
  detail::internal::WorkerProtocol_Dispatcher_RegisterMetricsCallback(
      impl.dispatcher.get(), &impl, &detail::DispatcherMetricsThunk);
  detail::internal::WorkerProtocol_Dispatcher_RegisterCriticalSectionCallback(
      impl.dispatcher.get(), &impl, &detail::DispatcherCriticalSectionThunk);
  detail::internal::WorkerProtocol_Dispatcher_RegisterAddEntityCallback(
      impl.dispatcher.get(), &impl, &detail::DispatcherAddEntityThunk);
  detail::internal::WorkerProtocol_Dispatcher_RegisterRemoveEntityCallback(
      impl.dispatcher.get(), &impl, &detail::DispatcherRemoveEntityThunk);
  detail::internal::WorkerProtocol_Dispatcher_RegisterReserveEntityIdResponseCallback(
      impl.dispatcher.get(), &impl, &detail::DispatcherReserveEntityIdResponseThunk);
  detail::internal::WorkerProtocol_Dispatcher_RegisterReserveEntityIdsResponseCallback(
      impl.dispatcher.get(), &impl, &detail::DispatcherReserveEntityIdsResponseThunk);
  detail::internal::WorkerProtocol_Dispatcher_RegisterCreateEntityResponseCallback(
      impl.dispatcher.get(), &impl, &detail::DispatcherCreateEntityResponseThunk);
  detail::internal::WorkerProtocol_Dispatcher_RegisterDeleteEntityResponseCallback(
      impl.dispatcher.get(), &impl, &detail::DispatcherDeleteEntityResponseThunk);
  detail::internal::WorkerProtocol_Dispatcher_RegisterEntityQueryResponseCallback(
      impl.dispatcher.get(), &impl, &detail::DispatcherEntityQueryResponseThunk);
  detail::internal::WorkerProtocol_Dispatcher_RegisterAddComponentCallback(
      impl.dispatcher.get(), &impl, &detail::DispatcherAddComponentThunk);
  detail::internal::WorkerProtocol_Dispatcher_RegisterRemoveComponentCallback(
      impl.dispatcher.get(), &impl, &detail::DispatcherRemoveComponentThunk);
  detail::internal::WorkerProtocol_Dispatcher_RegisterAuthorityChangeCallback(
      impl.dispatcher.get(), &impl, &detail::DispatcherAuthorityChangeThunk);
  detail::internal::WorkerProtocol_Dispatcher_RegisterComponentUpdateCallback(
      impl.dispatcher.get(), &impl, &detail::DispatcherComponentUpdateThunk);
  detail::internal::WorkerProtocol_Dispatcher_RegisterCommandRequestCallback(
      impl.dispatcher.get(), &impl, &detail::DispatcherCommandRequestThunk);
  detail::internal::WorkerProtocol_Dispatcher_RegisterCommandResponseCallback(
      impl.dispatcher.get(), &impl, &detail::DispatcherCommandResponseThunk);
}

inline Dispatcher::CallbackKey Dispatcher::OnDisconnect(const Callback<DisconnectOp>& callback) {
  impl.disconnect_callbacks.Add(impl.current_callback_key, callback);
  return impl.current_callback_key++;
}

inline Dispatcher::CallbackKey Dispatcher::OnFlagUpdate(const Callback<FlagUpdateOp>& callback) {
  impl.flag_update_callbacks.Add(impl.current_callback_key, callback);
  return impl.current_callback_key++;
}

inline Dispatcher::CallbackKey Dispatcher::OnLogMessage(const Callback<LogMessageOp>& callback) {
  impl.log_message_callbacks.Add(impl.current_callback_key, callback);
  return impl.current_callback_key++;
}

inline Dispatcher::CallbackKey Dispatcher::OnMetrics(const Callback<MetricsOp>& callback) {
  impl.metrics_callbacks.Add(impl.current_callback_key, callback);
  return impl.current_callback_key++;
}

inline Dispatcher::CallbackKey
Dispatcher::OnCriticalSection(const Callback<CriticalSectionOp>& callback) {
  impl.critical_section_callbacks.Add(impl.current_callback_key, callback);
  return impl.current_callback_key++;
}

inline Dispatcher::CallbackKey Dispatcher::OnAddEntity(const Callback<AddEntityOp>& callback) {
  impl.add_entity_callbacks.Add(impl.current_callback_key, callback);
  return impl.current_callback_key++;
}

inline Dispatcher::CallbackKey
Dispatcher::OnRemoveEntity(const Callback<RemoveEntityOp>& callback) {
  impl.remove_entity_callbacks.Add(impl.current_callback_key, callback);
  return impl.current_callback_key++;
}

inline Dispatcher::CallbackKey
Dispatcher::OnReserveEntityIdResponse(const Callback<ReserveEntityIdResponseOp>& callback) {
  impl.reserve_entity_id_response_callbacks.Add(impl.current_callback_key, callback);
  return impl.current_callback_key++;
}

inline Dispatcher::CallbackKey
Dispatcher::OnReserveEntityIdsResponse(const Callback<ReserveEntityIdsResponseOp>& callback) {
  impl.reserve_entity_ids_response_callbacks.Add(impl.current_callback_key, callback);
  return impl.current_callback_key++;
}

inline Dispatcher::CallbackKey
Dispatcher::OnCreateEntityResponse(const Callback<CreateEntityResponseOp>& callback) {
  impl.create_entity_response_callbacks.Add(impl.current_callback_key, callback);
  return impl.current_callback_key++;
}

inline Dispatcher::CallbackKey
Dispatcher::OnDeleteEntityResponse(const Callback<DeleteEntityResponseOp>& callback) {
  impl.delete_entity_response_callbacks.Add(impl.current_callback_key, callback);
  return impl.current_callback_key++;
}

inline Dispatcher::CallbackKey
Dispatcher::OnEntityQueryResponse(const Callback<EntityQueryResponseOp>& callback) {
  impl.entity_query_response_callbacks.Add(impl.current_callback_key, callback);
  return impl.current_callback_key++;
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

inline void Dispatcher::Remove(CallbackKey key) {
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

inline void Dispatcher::Process(const OpList& op_list) const {
  detail::internal::WorkerProtocol_Dispatcher_Process(impl.dispatcher.get(), op_list.op_list.get());
}

}  // ::worker

#endif  // WORKER_SDK_CPP_INCLUDE_IMPROBABLE_DETAIL_DISPATCHER_I_H
