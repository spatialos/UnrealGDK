// Copyright (c) Improbable Worlds Ltd, All Rights Reserved
#ifndef WORKER_SDK_CPP_INCLUDE_IMPROBABLE_DETAIL_WORKER_DETAIL_H
#define WORKER_SDK_CPP_INCLUDE_IMPROBABLE_DETAIL_WORKER_DETAIL_H
#include <improbable/collections.h>
#include <functional>
#include <memory>
#include <type_traits>

extern "C" {
#include <stddef.h>
#include <stdint.h>
}  // extern "C"

namespace worker {
namespace detail {
namespace internal {
#include <improbable/pbio.h>
#include <improbable/worker_protocol.h>
}  // :internal
}  // ::detail

using EntityId = std::int64_t;
using ComponentId = std::uint32_t;

class Connection;
class Dispatcher;
class Entity;
class OpList;
struct Metrics;
namespace query {
struct EntityQuery;
}  // ::query

template <typename>
struct AddComponentOp;
struct AddEntityOp;
struct AuthorityChangeOp;
struct CommandParameters;
template <typename>
struct ComponentUpdateOp;
struct CriticalSectionOp;
struct CreateEntityResponseOp;
struct DeleteEntityResponseOp;
struct DisconnectOp;
struct EntityQueryResponseOp;
struct FlagUpdateOp;
struct LogMessageOp;
struct MetricsOp;
struct RemoveComponentOp;
struct RemoveEntityOp;
struct ReserveEntityIdResponseOp;
struct ReserveEntityIdsResponseOp;

namespace detail {
class ClientHandleBase;
class ComponentStorageBase;
template <typename T>
struct FutureImpl;

struct ComponentInfo;
class ComponentMetaclass;
template <typename T>
using IsComponentMetaclass = std::is_base_of<ComponentMetaclass, T>;

template <typename... Args>
struct AllComponentMetaclasses {
  template <bool...>
  struct BoolArray {};
  template <bool... B>
  using AllOf = std::is_same<BoolArray<B...>, BoolArray<(B || true)...>>;

  static_assert(AllOf<IsComponentMetaclass<Args>::value...>::value,
                "All template parameters to worker::Components must be component metaclasses.");
};

// Callback containers.
template <typename T>
using Callback = std::function<void(const T&)>;
using CallbackKey = std::uint64_t;

// We need to avoid changing the underlying map while iterating over it. For example, if a callback
// removes itself, a call to InvokeAll iterates over the map to invoke each callback. This then
// indirectly invokes Remove, which makes the iteration crash.
//
// A simple solution to this problem would be to copy the map for each iteration, but this allocates
// a significant amount of memory each time. Instead, we fix this issue by postponing changes to the
// map until the end of the top-level call to this object. In the mentioned example, when InvokeAll
// indirectly calls Remove, the removal is not done immediately, but postponed to the end of the
// (top-level) InvokeAll call.
//
// The current callbacks are those in the map, plus to_add, minus to_remove.
// The changes are merged via the UpdateGuard.
template <typename T>
class UpdateGuard;
template <typename T>
class CallbackMap {
public:
  CallbackMap();
  void Add(CallbackKey key, const Callback<T>& callback);
  bool Remove(CallbackKey key);
  void InvokeAll(const T& op) const;
  void ReverseInvokeAll(const T& op) const;

private:
  friend class UpdateGuard<T>;

  Map<CallbackKey, Callback<T>> map;
  Map<CallbackKey, Callback<T>> to_add;
  Map<CallbackKey, std::size_t> to_remove;
  // Current call level with 0 for the top-level call.
  std::int_fast32_t call_depth;

  // Merge to_add and to_remove with map.
  void UpdateCallbacks();
};

template <typename T>
class ComponentCallbackMap {
public:
  void Add(ComponentId component_id, CallbackKey key, const Callback<T>& callback);
  bool Remove(CallbackKey key);
  void InvokeAll(ComponentId component_id, const T& op) const;
  void ReverseInvokeAll(ComponentId component_id, const T& op) const;

private:
  Map<ComponentId, CallbackMap<T>> map;
};

struct DispatcherImpl {
  DispatcherImpl(const ComponentInfo& component_info, internal::WorkerProtocol_Dispatcher* ptr,
                 void (*deleter)(internal::WorkerProtocol_Dispatcher*))
  : component_info{component_info}, dispatcher{ptr, deleter}, current_callback_key{0} {}

  const ComponentInfo& component_info;
  std::unique_ptr<internal::WorkerProtocol_Dispatcher,
                  void (*)(internal::WorkerProtocol_Dispatcher*)>
      dispatcher;

  struct ComponentWrapperOp {
    worker::EntityId EntityId;
    // A WorkerProtocol_ClientHandle provided by C API.
    const ClientHandleBase* Handle;
  };
  struct CommandRequestWrapperOp {
    std::uint32_t RequestId;
    worker::EntityId EntityId;
    std::uint32_t TimeoutMillis;
    const char* CallerWorkerId;
    std::uint32_t CallerAttributeCount;
    const char** CallerAttribute;
    // A WorkerProtocol_ClientHandle provided by C API.
    const ClientHandleBase* Request;
  };
  struct CommandResponseWrapperOp {
    std::uint32_t RequestId;
    worker::EntityId EntityId;
    std::uint8_t StatusCode;
    const char* Message;
    // A WorkerProtocol_ClientHandle provided by C API. Will be nullptr if StatusCode is not
    // StatucCode::kSuccess.
    const ClientHandleBase* Response;
    std::uint32_t CommandId;
  };

  CallbackKey current_callback_key;
  CallbackMap<DisconnectOp> disconnect_callbacks;
  CallbackMap<FlagUpdateOp> flag_update_callbacks;
  CallbackMap<LogMessageOp> log_message_callbacks;
  CallbackMap<MetricsOp> metrics_callbacks;
  CallbackMap<CriticalSectionOp> critical_section_callbacks;
  CallbackMap<AddEntityOp> add_entity_callbacks;
  CallbackMap<RemoveEntityOp> remove_entity_callbacks;
  CallbackMap<ReserveEntityIdResponseOp> reserve_entity_id_response_callbacks;
  CallbackMap<ReserveEntityIdsResponseOp> reserve_entity_ids_response_callbacks;
  CallbackMap<CreateEntityResponseOp> create_entity_response_callbacks;
  CallbackMap<DeleteEntityResponseOp> delete_entity_response_callbacks;
  CallbackMap<EntityQueryResponseOp> entity_query_response_callbacks;
  ComponentCallbackMap<ComponentWrapperOp> add_component_callbacks;
  ComponentCallbackMap<RemoveComponentOp> remove_component_callbacks;
  ComponentCallbackMap<AuthorityChangeOp> authority_change_callbacks;
  ComponentCallbackMap<ComponentWrapperOp> component_update_callbacks;
  ComponentCallbackMap<CommandRequestWrapperOp> command_request_callbacks;
  ComponentCallbackMap<CommandResponseWrapperOp> command_response_callbacks;
};

// Base class for component metaclasses.
class ComponentMetaclass {
public:
  virtual ~ComponentMetaclass() {}

private:
  ComponentMetaclass() = default;
};

}  // ::detail
}  // ::worker

#endif  // WORKER_SDK_CPP_INCLUDE_IMPROBABLE_DETAIL_WORKER_DETAIL_H
