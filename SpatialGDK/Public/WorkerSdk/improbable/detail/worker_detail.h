// Copyright (c) Improbable Worlds Ltd, All Rights Reserved
#ifndef WORKER_SDK_CPP_INCLUDE_IMPROBABLE_DETAIL_WORKER_DETAIL_H
#define WORKER_SDK_CPP_INCLUDE_IMPROBABLE_DETAIL_WORKER_DETAIL_H
#include <improbable/collections.h>
#include <functional>
#include <memory>
#include <type_traits>
#include <unordered_map>
#include <unordered_set>

#ifndef IMPROBABLE_DLL_API
#define IMPROBABLE_DLL_API
#endif  // IMPROBABLE_DLL_API

struct Pbio_Object;
struct WorkerProtocol_Connection;
struct WorkerProtocol_Dispatcher;
struct WorkerProtocol_Locator;
struct WorkerProtocol_OpList;
struct WorkerProtocol_SnapshotInputStream;
struct WorkerProtocol_SnapshotOutputStream;

namespace worker {
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
struct CreateEntityResponseOp;
struct CriticalSectionOp;
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
struct UpdateParameters;

namespace detail {
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

class ClientHandleBase;
// So that generated code need not include worker_protocol.h.
struct ComponentVtable {
  enum ClientHandleType : std::uint8_t {
    Update = 1,
    Snapshot = 2,
    Request = 3,
    Response = 4,
  };
  worker::ComponentId ComponentId;
  void (*Free)(worker::ComponentId component_id, std::uint8_t handle_type,
               ClientHandleBase* handle);
  ClientHandleBase* (*Copy)(worker::ComponentId component_id, std::uint8_t handle_type,
                            ClientHandleBase* handle);
  std::uint8_t (*Deserialize)(worker::ComponentId component_id, std::uint8_t handle_type,
                              Pbio_Object* source, ClientHandleBase** handle_out);
  void (*Serialize)(worker::ComponentId component_id, std::uint8_t handle_type,
                    const ClientHandleBase* handle, Pbio_Object* target);
};

// A smart pointer used to implement the client vtable functions while avoiding unnecessary
// allocation. It allows a raw pointer to client-owned memory to be promoted to a shared_ptr on
// demand when the vtable Copy function is called. This shared_ptr is created by copy- or
// move-constructing from the source depending on whether ownership has been assumed or not.
class ClientHandleBase {
public:
  virtual ~ClientHandleBase(){};
};

template <typename T>
class ClientHandle : public ClientHandleBase {
public:
  ~ClientHandle() override = default;
  ClientHandle(ClientHandle&&) = default;
  ClientHandle(const ClientHandle& ref) = delete;
  ClientHandle& operator=(ClientHandle&&) = default;
  ClientHandle& operator=(const ClientHandle& ref) = delete;

  template <typename... Args>
  static ClientHandle* allocate(Args&&... args) {
    auto* result = new ClientHandle{nullptr, nullptr};
    result->shared = std::shared_ptr<T>{new T{std::forward<Args>(args)...}};
    return result;
  }

  static ClientHandle copyable(const T* raw) {
    return ClientHandle{raw, nullptr};
  }

  static ClientHandle movable(T* raw) {
    return ClientHandle{nullptr, raw};
  }

  static void free(ClientHandleBase* handle) {
    delete static_cast<ClientHandle*>(handle);
  }

  static ClientHandle* new_copy(ClientHandleBase* handle) {
    auto& ref = *static_cast<ClientHandle*>(handle);
    ref.PromoteToShared();
    auto* result = new ClientHandle{nullptr, nullptr};
    result->shared = ref.shared;
    return result;
  }

  static const T& get(const ClientHandleBase* handle) {
    return static_cast<const ClientHandle*>(handle)->get();
  }

  static T& get_mutable(ClientHandleBase* handle) {
    return static_cast<ClientHandle*>(handle)->get_mutable();
  }

  const T& get() const {
    return copyable_raw ? *copyable_raw : movable_raw ? *movable_raw : *shared;
  }

  T& get_mutable() {
    if (copyable_raw) {
      PromoteToShared();
    }
    return movable_raw ? *movable_raw : *shared;
  }

private:
  explicit ClientHandle(const T* copyable, T* movable)
  : copyable_raw{copyable}, movable_raw{movable} {}

  void PromoteToShared() {
    if (copyable_raw) {
      shared = std::make_shared<T>(*copyable_raw);
      copyable_raw = nullptr;
    }
    if (movable_raw) {
      shared = std::make_shared<T>(std::move(*movable_raw));
      movable_raw = nullptr;
    }
  }

  const T* copyable_raw;
  T* movable_raw;
  std::shared_ptr<T> shared;
};

// Storage for arbitrary component data.
class ComponentStorageBase {
public:
  virtual ~ComponentStorageBase(){};
  virtual std::unique_ptr<ComponentStorageBase> Copy() const = 0;
};

template <typename T>
class ComponentStorage : public ComponentStorageBase {
public:
  explicit ComponentStorage(const typename T::Data& data) : data{data} {}
  explicit ComponentStorage(typename T::Data&& data) : data{std::move(data)} {}
  ~ComponentStorage() override {}

  std::unique_ptr<ComponentStorageBase> Copy() const override;
  const typename T::Data& Get() const;
  typename T::Data& Get();

private:
  typename T::Data data;
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
class CallbackMap {
public:
  CallbackMap();
  void Add(CallbackKey key, const Callback<T>& callback);
  bool Remove(CallbackKey key);
  void InvokeAll(const T& op) const;
  void ReverseInvokeAll(const T& op) const;

private:
  Map<CallbackKey, Callback<T>> map;
  Map<CallbackKey, Callback<T>> to_add;
  Map<CallbackKey, std::size_t> to_remove;
  // Current call level with 0 for the top-level call.
  std::int_fast32_t call_depth;

  // Merge to_add and to_remove with map.
  void UpdateCallbacks();

  // Registers that a call is entered and exited via constructor and destructor, respectively. The
  // state of the given callback map is updated accordingly.
  class UpdateGuard {
  public:
    UpdateGuard(CallbackMap<T>& callback_map) : callback_map{callback_map} {
      ++callback_map.call_depth;
    }

    ~UpdateGuard() {
      if (--callback_map.call_depth == 0) {
        callback_map.UpdateCallbacks();
      }
    }

    // Noncopyable.
    UpdateGuard(const UpdateGuard&) = delete;
    UpdateGuard& operator=(const UpdateGuard&) = delete;

  private:
    CallbackMap<T>& callback_map;
  };
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

template <typename T>
struct FutureImpl {
  ~FutureImpl() {
    destroy();
  }

  std::function<Option<T>(const Option<std::uint32_t>&)> get;
  std::function<void()> destroy;
  bool moved_from;
  Option<T> result;
};

struct DispatcherImpl {
  DispatcherImpl(const ComponentInfo& component_info, WorkerProtocol_Dispatcher* ptr,
                 void (*deleter)(WorkerProtocol_Dispatcher*))
  : component_info{component_info}, dispatcher{ptr, deleter}, current_callback_key{0} {}

  const ComponentInfo& component_info;
  std::unique_ptr<WorkerProtocol_Dispatcher, void (*)(WorkerProtocol_Dispatcher*)> dispatcher;

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

IMPROBABLE_DLL_API void SendClientComponentUpdate(WorkerProtocol_Connection* connection,
                                                  EntityId entity_id, ComponentId component_id,
                                                  const ClientHandleBase* update,
                                                  const UpdateParameters& parameters);
IMPROBABLE_DLL_API std::uint32_t
SendClientCommandRequest(WorkerProtocol_Connection* connection, EntityId entity_id,
                         ComponentId component_id, const ClientHandleBase* request,
                         std::uint32_t command_id, const Option<std::uint32_t>& timeout_millis,
                         const CommandParameters& parameters);
IMPROBABLE_DLL_API void SendClientCommandResponse(WorkerProtocol_Connection* connection,
                                                  std::uint32_t request_id,
                                                  ComponentId component_id,
                                                  const ClientHandleBase* response);
IMPROBABLE_DLL_API void SendClientCommandFailure(WorkerProtocol_Connection* connection,
                                                 std::uint32_t request_id,
                                                 const std::string& message);

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
