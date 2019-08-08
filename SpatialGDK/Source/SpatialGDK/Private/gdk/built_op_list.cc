#include "gdk/built_op_list.h"

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

  opList.stringStorage.emplace_back(message);
  op.disconnect =
      Worker_DisconnectOp{static_cast<std::uint8_t>(status), opList.stringStorage.back().c_str()};
  opList.ops.emplace_back(op);
  return *this;
}

OpListBuilder& OpListBuilder::AddFlag(const std::string& name, const std::string& value) {
  Worker_Op op;
  op.op_type = static_cast<std::uint8_t>(WORKER_OP_TYPE_FLAG_UPDATE);

  opList.stringStorage.emplace_back(name);
  op.flag_update.name = opList.stringStorage.back().c_str();

  opList.stringStorage.emplace_back(value);
  op.flag_update.value = opList.stringStorage.back().c_str();

  opList.ops.emplace_back(op);
  return *this;
}

OpListBuilder& OpListBuilder::AddCriticalSection(bool inCriticalSection) {
  Worker_Op op;
  op.op_type = static_cast<std::uint8_t>(WORKER_OP_TYPE_CRITICAL_SECTION);
  op.critical_section = Worker_CriticalSectionOp{static_cast<std::uint8_t>(inCriticalSection)};
  opList.ops.emplace_back(op);
  return *this;
}

OpListBuilder& OpListBuilder::AddEntity(EntityId entityId, EntityState entity) {
  Worker_Op addEntityOp;
  addEntityOp.op_type = static_cast<std::uint8_t>(WORKER_OP_TYPE_ADD_ENTITY);
  addEntityOp.add_entity = Worker_AddEntityOp{entityId};
  opList.ops.emplace_back(addEntityOp);

  for (auto& component : entity.GetComponents()) {
    AddComponent(entityId, std::move(component));
  }
  return *this;
}

OpListBuilder& OpListBuilder::RemoveEntity(EntityId entityId) {
  Worker_Op removeEntityOp;
  removeEntityOp.op_type = static_cast<std::uint8_t>(WORKER_OP_TYPE_REMOVE_ENTITY);
  removeEntityOp.remove_entity = Worker_RemoveEntityOp{entityId};
  opList.ops.emplace_back(removeEntityOp);
  return *this;
}

OpListBuilder& OpListBuilder::AddComponent(EntityId entityId, ComponentData&& component) {
  Worker_ComponentData workerData{nullptr, component.GetComponentId(), component.GetUnderlying(),
                                  nullptr};
  Worker_Op op;
  op.op_type = static_cast<std::uint8_t>(WORKER_OP_TYPE_ADD_COMPONENT);
  op.add_component = Worker_AddComponentOp{entityId, workerData};
  opList.dataStorage.emplace_back(std::move(component));
  opList.ops.emplace_back(op);
  return *this;
}

OpListBuilder& OpListBuilder::RemoveComponent(EntityId entityId, ComponentId componentId) {
  Worker_Op removeComponentOp;
  removeComponentOp.op_type = static_cast<std::uint8_t>(WORKER_OP_TYPE_REMOVE_COMPONENT);
  removeComponentOp.remove_component = Worker_RemoveComponentOp{entityId, componentId};
  opList.ops.emplace_back(removeComponentOp);
  return *this;
}

OpListBuilder& OpListBuilder::UpdateComponent(EntityId entityId, ComponentUpdate&& update) {
  Worker_ComponentUpdate workerUpdate{nullptr, update.GetComponentId(), update.GetUnderlying(),
                                      nullptr};
  Worker_Op op;
  op.op_type = static_cast<std::uint8_t>(WORKER_OP_TYPE_COMPONENT_UPDATE);
  op.component_update = Worker_ComponentUpdateOp{entityId, workerUpdate};
  opList.updateStorage.emplace_back(std::move(update));
  opList.ops.emplace_back(op);
  return *this;
}

OpListBuilder& OpListBuilder::SetAuthority(EntityId entityId, ComponentId componentId,
                                           Worker_Authority authority) {
  Worker_Op op;
  op.op_type = static_cast<std::uint8_t>(WORKER_OP_TYPE_AUTHORITY_CHANGE);
  op.authority_change =
      Worker_AuthorityChangeOp{entityId, componentId, static_cast<uint8_t>(authority)};
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
  op.command_request.request_id = requestId;
  op.command_request.entity_id = entityId;
  op.command_request.timeout_millis = timeout;

  opList.stringStorage.emplace_back(callerWorkerId);
  op.command_request.caller_worker_id = opList.stringStorage.back().c_str();

  opList.attributeSetStorage.emplace_back(std::vector<const char*>(callerAttributeSet.size()));
  for (size_t i = 0; i < callerAttributeSet.size(); ++i) {
    opList.stringStorage.emplace_back(callerAttributeSet[i]);
    opList.attributeSetStorage.back()[i] = opList.stringStorage.back().c_str();
    // opList.attributeSetStorage.back().emplace_back(attribute.c_str());
  }
  op.command_request.caller_attribute_set.attributes = opList.attributeSetStorage.back().data();
  op.command_request.caller_attribute_set.attribute_count =
      static_cast<std::uint32_t>(callerAttributeSet.size());

  op.command_request.request =
      Worker_CommandRequest{nullptr, request.GetComponentId(), request.GetUnderlying(), nullptr};
  opList.requestStorage.emplace_back(std::move(request));
  opList.ops.emplace_back(op);
  return *this;
}

OpListBuilder& OpListBuilder::AddCommandResponse(RequestId requestId, EntityId entityId,
                                                 Worker_StatusCode status,
                                                 const std::string& message,
                                                 CommandResponse&& response,
                                                 std::uint32_t commandId) {
  Worker_Op op{};
  op.op_type = static_cast<std::uint8_t>(WORKER_OP_TYPE_COMMAND_RESPONSE);
  op.command_response.request_id = requestId;
  op.command_response.entity_id = entityId;
  op.command_response.status_code = static_cast<std::uint8_t>(status);

  opList.stringStorage.emplace_back(message);
  op.command_response.message = opList.stringStorage.back().c_str();

  op.command_response.response =
      Worker_CommandResponse{nullptr, response.GetComponentId(), response.GetUnderlying(), nullptr};
  opList.responseStorage.emplace_back(std::move(response));

  op.command_response.command_id = commandId;
  opList.ops.emplace_back(op);
  return *this;
}

OpListBuilder& OpListBuilder::AddCommandFailure(RequestId requestId, EntityId entityId,
                                                Worker_StatusCode status,
                                                const std::string& message,
                                                std::uint32_t commandId) {
  Worker_Op op{};
  op.op_type = static_cast<std::uint8_t>(WORKER_OP_TYPE_COMMAND_RESPONSE);
  op.command_response.request_id = requestId;
  op.command_response.entity_id = entityId;
  op.command_response.status_code = static_cast<std::uint8_t>(status);

  opList.stringStorage.emplace_back(message);
  op.command_response.message = opList.stringStorage.back().c_str();

  op.command_response.response = {};
  op.command_response.command_id = commandId;
  opList.ops.emplace_back(op);
  return *this;
}

}  // namespace gdk