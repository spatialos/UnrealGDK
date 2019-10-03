#include "gdk/op_lists/built_op_list.h"

#include <algorithm>
#include <cstddef>

namespace gdk {

size_t BuiltOpList::GetCount() const {
  return ops.size();
}

Worker_Op& BuiltOpList::operator[](size_t index) {
  return ops[index];
}

BuiltOpList OpListBuilder::CreateOpList() && {
  return std::move(opList);
}

OpListBuilder& OpListBuilder::Disconnect(Worker_ConnectionStatusCode status,
                                         const std::string& message) {
  Worker_Op op;
  op.op_type = static_cast<std::uint8_t>(WORKER_OP_TYPE_DISCONNECT);
  op.op.disconnect.connection_status_code = static_cast<std::uint8_t>(status);
  op.op.disconnect.reason = StoreString(message);
  opList.ops.emplace_back(op);
  return *this;
}

OpListBuilder& OpListBuilder::AddFlag(const std::string& name, const std::string& value) {
  Worker_Op op;
  op.op_type = static_cast<std::uint8_t>(WORKER_OP_TYPE_FLAG_UPDATE);
  op.op.flag_update.name = StoreString(name);
  op.op.flag_update.value = StoreString(value);
  opList.ops.emplace_back(op);
  return *this;
}

OpListBuilder& OpListBuilder::AddCriticalSection(bool inCriticalSection) {
  Worker_Op op;
  op.op_type = static_cast<std::uint8_t>(WORKER_OP_TYPE_CRITICAL_SECTION);
  op.op.critical_section = Worker_CriticalSectionOp{static_cast<std::uint8_t>(inCriticalSection)};
  opList.ops.emplace_back(op);
  return *this;
}

OpListBuilder& OpListBuilder::AddEntity(EntityId entityId, EntityState entity) {
  Worker_Op addEntityOp;
  addEntityOp.op_type = static_cast<std::uint8_t>(WORKER_OP_TYPE_ADD_ENTITY);
  addEntityOp.op.add_entity = Worker_AddEntityOp{entityId};
  opList.ops.emplace_back(addEntityOp);

  for (auto& component : entity.GetComponents()) {
    AddComponent(entityId, std::move(component));
  }
  return *this;
}

OpListBuilder& OpListBuilder::RemoveEntity(EntityId entityId) {
  Worker_Op removeEntityOp;
  removeEntityOp.op_type = static_cast<std::uint8_t>(WORKER_OP_TYPE_REMOVE_ENTITY);
  removeEntityOp.op.remove_entity = Worker_RemoveEntityOp{entityId};
  opList.ops.emplace_back(removeEntityOp);
  return *this;
}

OpListBuilder& OpListBuilder::AddComponent(EntityId entityId, ComponentData&& component) {
  Worker_Op op;
  op.op_type = static_cast<std::uint8_t>(WORKER_OP_TYPE_ADD_COMPONENT);
  op.op.add_component.entity_id = entityId;
  op.op.add_component.data = StoreComponentData(std::move(component));
  opList.ops.emplace_back(op);
  return *this;
}

OpListBuilder& OpListBuilder::RemoveComponent(EntityId entityId, ComponentId componentId) {
  Worker_Op removeComponentOp;
  removeComponentOp.op_type = static_cast<std::uint8_t>(WORKER_OP_TYPE_REMOVE_COMPONENT);
  removeComponentOp.op.remove_component = Worker_RemoveComponentOp{entityId, componentId};
  opList.ops.emplace_back(removeComponentOp);
  return *this;
}

OpListBuilder& OpListBuilder::UpdateComponent(EntityId entityId, ComponentUpdate&& update) {
  Worker_Op op;
  op.op_type = static_cast<std::uint8_t>(WORKER_OP_TYPE_COMPONENT_UPDATE);
  op.op.component_update.entity_id = entityId;
  op.op.component_update.update = StoreComponentUpdate(std::move(update));
  opList.ops.emplace_back(op);
  return *this;
}

OpListBuilder& OpListBuilder::SetAuthority(EntityId entityId, ComponentId componentId,
                                           Worker_Authority authority) {
  Worker_Op op;
  op.op_type = static_cast<std::uint8_t>(WORKER_OP_TYPE_AUTHORITY_CHANGE);
  op.op.authority_change.entity_id = entityId;
  op.op.authority_change.authority = static_cast<uint8_t>(authority);
  op.op.authority_change.component_id = componentId;
  opList.ops.emplace_back(op);
  return *this;
}

OpListBuilder& OpListBuilder::AddCommandRequest(RequestId requestId, EntityId entityId,
                                                std::uint32_t timeout,
                                                const std::string& callerWorkerId,
                                                const std::vector<std::string>& callerAttributeSet,
                                                CommandRequest&& request) {
  Worker_Op op{};
  op.op_type = static_cast<std::uint8_t>(WORKER_OP_TYPE_COMMAND_REQUEST);
  op.op.command_request.request_id = requestId;
  op.op.command_request.entity_id = entityId;
  op.op.command_request.timeout_millis = timeout;
  op.op.command_request.caller_worker_id = StoreString(callerWorkerId);
  op.op.command_request.request = StoreCommandRequest(std::move(request));
  op.op.command_request.caller_attribute_set.attribute_count =
      static_cast<std::uint32_t>(callerAttributeSet.size());
  op.op.command_request.caller_attribute_set.attributes = StoreAttributeSet(callerAttributeSet);
  opList.ops.emplace_back(op);
  return *this;
}

OpListBuilder& OpListBuilder::AddCommandResponse(RequestId requestId, EntityId entityId,
                                                 Worker_StatusCode status,
                                                 const std::string& message,
                                                 CommandResponse&& response) {
  Worker_Op op{};
  op.op_type = static_cast<std::uint8_t>(WORKER_OP_TYPE_COMMAND_RESPONSE);
  op.op.command_response.request_id = requestId;
  op.op.command_response.entity_id = entityId;
  op.op.command_response.status_code = static_cast<std::uint8_t>(status);
  op.op.command_response.message = StoreString(message);
  op.op.command_response.response = StoreCommandResponse(std::move(response));
  opList.ops.emplace_back(op);
  return *this;
}

OpListBuilder& OpListBuilder::AddCommandFailure(RequestId requestId, EntityId entityId,
                                                Worker_StatusCode status,
                                                const std::string& message, ComponentId componentId,
                                                CommandIndex commandId) {
  Worker_Op op{};
  op.op_type = static_cast<std::uint8_t>(WORKER_OP_TYPE_COMMAND_RESPONSE);
  op.op.command_response.request_id = requestId;
  op.op.command_response.entity_id = entityId;
  op.op.command_response.status_code = static_cast<std::uint8_t>(status);
  op.op.command_response.message = StoreString(message);
  op.op.command_response.response = {};
  op.op.command_response.response.component_id = componentId;
  op.op.command_response.response.command_index = commandId;
  opList.ops.emplace_back(op);
  return *this;
}

OpListBuilder& OpListBuilder::AddReserveEntityIdsResponse(RequestId requestId,
                                                          Worker_StatusCode status,
                                                          const std::string& message,
                                                          EntityId firstEntityId,
                                                          std::uint32_t numberOfIds) {
  Worker_Op op{};
  op.op_type = static_cast<std::uint8_t>(WORKER_OP_TYPE_RESERVE_ENTITY_IDS_RESPONSE);
  op.op.reserve_entity_ids_response.request_id = requestId;
  op.op.reserve_entity_ids_response.status_code = static_cast<std::uint8_t>(status);
  op.op.reserve_entity_ids_response.message = StoreString(message);
  op.op.reserve_entity_ids_response.first_entity_id = firstEntityId;
  op.op.reserve_entity_ids_response.number_of_entity_ids = numberOfIds;
  opList.ops.emplace_back(op);
  return *this;
}

OpListBuilder& OpListBuilder::AddCreateEntityResponse(RequestId requestId, Worker_StatusCode status,
                                                      const std::string& message,
                                                      EntityId entityId) {
  Worker_Op op{};
  op.op_type = static_cast<std::uint8_t>(WORKER_OP_TYPE_CREATE_ENTITY_RESPONSE);
  op.op.create_entity_response.request_id = requestId;
  op.op.create_entity_response.status_code = static_cast<std::uint8_t>(status);
  op.op.create_entity_response.message = StoreString(message);
  op.op.create_entity_response.entity_id = entityId;
  opList.ops.emplace_back(op);
  return *this;
}

OpListBuilder& OpListBuilder::AddDeleteEntityResponse(RequestId requestId, EntityId entityId,
                                                      Worker_StatusCode status,
                                                      const std::string& message) {
  Worker_Op op{};
  op.op_type = static_cast<std::uint8_t>(WORKER_OP_TYPE_DELETE_ENTITY_RESPONSE);
  op.op.delete_entity_response.request_id = requestId;
  op.op.delete_entity_response.status_code = static_cast<std::uint8_t>(status);
  op.op.delete_entity_response.message = StoreString(message);
  op.op.delete_entity_response.entity_id = entityId;
  opList.ops.emplace_back(op);
  return *this;
}

OpListBuilder& OpListBuilder::AddEntityQueryResponse(RequestId requestId, Worker_StatusCode status,
                                                     const std::string& message,
                                                     std::uint32_t resultCount,
                                                     std::vector<EntitySnapshot> results) {
  Worker_Op op{};
  op.op_type = static_cast<std::uint8_t>(WORKER_OP_TYPE_ENTITY_QUERY_RESPONSE);
  op.op.entity_query_response.request_id = requestId;
  op.op.entity_query_response.status_code = static_cast<std::uint8_t>(status);
  op.op.entity_query_response.message = StoreString(message);
  op.op.entity_query_response.result_count = resultCount;
  op.op.entity_query_response.results = StoreQueryEntities(std::move(results)).data();
  opList.ops.emplace_back(op);
  return *this;
}

Worker_ComponentData OpListBuilder::StoreComponentData(ComponentData&& data) {
  opList.dataStorage.emplace_back(std::move(data));
  auto& storedData = opList.dataStorage.back();
  return Worker_ComponentData{nullptr, storedData.GetComponentId(), storedData.GetUnderlying(),
                              nullptr};
}

Worker_ComponentUpdate OpListBuilder::StoreComponentUpdate(ComponentUpdate&& update) {
  opList.updateStorage.emplace_back(std::move(update));
  auto& storedData = opList.updateStorage.back();
  return Worker_ComponentUpdate{nullptr, storedData.GetComponentId(), storedData.GetUnderlying(),
                                nullptr};
}

Worker_CommandRequest OpListBuilder::StoreCommandRequest(CommandRequest&& request) {
  opList.requestStorage.emplace_back(std::move(request));
  auto& storedRequest = opList.requestStorage.back();
  return Worker_CommandRequest{nullptr, storedRequest.GetComponentId(),
                               storedRequest.GetCommandIndex(), storedRequest.GetUnderlying(),
                               nullptr};
}

Worker_CommandResponse OpListBuilder::StoreCommandResponse(CommandResponse&& response) {
  opList.responseStorage.emplace_back(std::move(response));
  auto& storedRequest = opList.responseStorage.back();
  return Worker_CommandResponse{nullptr, storedRequest.GetComponentId(),
                                storedRequest.GetCommandIndex(), storedRequest.GetUnderlying(),
                                nullptr};
}

const std::vector<Worker_Entity>&
OpListBuilder::StoreQueryEntities(std::vector<EntitySnapshot> entities) {
  opList.queryEntityStorage.emplace_back(std::vector<Worker_Entity>(entities.size()));
  for (auto& entity : entities) {
    Worker_Entity workerEntity;
    workerEntity.entity_id = entity.GetEntityId();

    // Store the components.
    auto&& entityState = std::move(entity).ReleaseEntityState();
    const auto& storedSnapshot = StoreSnapshotComponents(std::move(entityState));

    // Set the entity values from the stored components.
    workerEntity.component_count = static_cast<std::uint32_t>(storedSnapshot.size());
    workerEntity.components = storedSnapshot.data();

    // Store the entity.
    opList.queryEntityStorage.back().emplace_back(workerEntity);
  }
  return opList.queryEntityStorage.back();
}

const std::vector<Worker_ComponentData>&
OpListBuilder::StoreSnapshotComponents(EntityState entity) {
  auto& components = entity.GetComponents();
  opList.snapshotStorage.emplace_back(std::vector<Worker_ComponentData>(components.size()));
  auto& storedSnapshot = opList.snapshotStorage.back();
  for (auto& component : components) {
    storedSnapshot.emplace_back(StoreComponentData(std::move(component)));
  }
  return opList.snapshotStorage.back();
}

const char* OpListBuilder::StoreString(const std::string& string) {
  opList.stringStorage.emplace_back(string);
  return opList.stringStorage.back().c_str();
}

const char** OpListBuilder::StoreAttributeSet(const std::vector<std::string>& attributeSet) {
  opList.attributeSetStorage.emplace_back(std::vector<const char*>(attributeSet.size()));
  for (size_t i = 0; i < attributeSet.size(); ++i) {
    opList.attributeSetStorage.back()[i] = StoreString(attributeSet[i]);
  }
  return opList.attributeSetStorage.back().data();
}

}  // namespace gdk
