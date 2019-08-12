#ifndef GDK_BUILDABLE_OP_LIST_H
#define GDK_BUILDABLE_OP_LIST_H
#include "gdk/abstract_op_list.h"
#include "gdk/command_request.h"
#include "gdk/command_response.h"
#include "gdk/common_types.h"
#include "gdk/component_data.h"
#include "gdk/component_update.h"
#include "gdk/entity_state.h"
#include <cstdint>
#include <deque>
#include <string>
#include <vector>
#include "entity_snapshot.h"

namespace gdk {

class OpListBuilder;

class BuiltOpList : public AbstractOpList {
public:
  ~BuiltOpList() = default;

  // Moveable, not copyable.
  BuiltOpList(const BuiltOpList&) = delete;
  BuiltOpList(BuiltOpList&&) = default;
  BuiltOpList& operator=(const BuiltOpList&) = delete;
  BuiltOpList& operator=(BuiltOpList&&) = default;

  size_t GetCount() const override;
  Worker_Op& operator[](size_t index) override;

private:
  BuiltOpList() = default;

  std::vector<Worker_Op> ops;

  // Storage objects used to create stable pointers.
  // Can't use vector for strings as pointers to small strings can be invalidated.
  std::deque<std::string> stringStorage;
  std::deque<std::vector<const char*>> attributeSetStorage;
  std::deque<ComponentData> dataStorage;
  std::deque<ComponentUpdate> updateStorage;
  std::deque<CommandRequest> requestStorage;
  std::deque<CommandResponse> responseStorage;
  std::deque<std::vector<Worker_ComponentData>> snapshotStorage;
  std::deque<std::vector<Worker_Entity>> queryEntityStorage;

  friend OpListBuilder;
};

class OpListBuilder {
public:
  OpListBuilder() = default;
  ~OpListBuilder() = default;

  // Moveable, not copyable
  OpListBuilder(const OpListBuilder&) = delete;
  OpListBuilder(OpListBuilder&&) = default;
  OpListBuilder& operator=(const OpListBuilder&) = delete;
  OpListBuilder& operator=(OpListBuilder&&) = default;

  BuiltOpList CreateOpList() &&;

  OpListBuilder& Disconnect(Worker_ConnectionStatusCode status, const std::string& message);
  OpListBuilder& AddFlag(const std::string& name, const std::string& value);
  OpListBuilder& AddCriticalSection(bool inCriticalSection);

  OpListBuilder& AddEntity(EntityId entityId, EntityState entity = {});
  OpListBuilder& RemoveEntity(EntityId entityId);
  OpListBuilder& AddComponent(EntityId entityId, ComponentData&& component);
  OpListBuilder& RemoveComponent(EntityId entityId, ComponentId componentId);
  OpListBuilder& UpdateComponent(EntityId entityId, ComponentUpdate&& update);
  OpListBuilder& SetAuthority(EntityId entityId, ComponentId componentId,
                              Worker_Authority authority);
  OpListBuilder& AddCommandRequest(RequestId requestId, EntityId entityId, std::uint32_t timeout,
                                   const std::string& callerWorkerId,
                                   const std::vector<std::string>& callerAttributeSet,
                                   CommandRequest&& request);
  OpListBuilder& AddCommandResponse(RequestId requestId, EntityId entityId,
                                    Worker_StatusCode status, const std::string& message,
                                    CommandResponse&& response, std::uint32_t commandId);
  OpListBuilder& AddCommandFailure(RequestId requestId, EntityId entityId, Worker_StatusCode status,
                                   const std::string& message, std::uint32_t commandId);
  OpListBuilder& AddReserveEntityIdsResponse(RequestId requestId, Worker_StatusCode status,
                                             const std::string& message, EntityId firstEntityId,
                                             std::uint32_t numberOfIds);
  OpListBuilder& AddCreateEntityResponse(RequestId requestId, Worker_StatusCode status,
                                         const std::string& message, EntityId entityId);
  OpListBuilder& AddDeleteEntityResponse(RequestId requestId, EntityId entityId,
                                         Worker_StatusCode status, const std::string& message);
  OpListBuilder& AddEntityQueryResponse(RequestId requestId, Worker_StatusCode status,
                                        const std::string& message, std::uint32_t resultCount,
                                        std::vector<EntitySnapshot> results);

private:
  Worker_ComponentData StoreAndConvertComponentData(ComponentData&& data);
  BuiltOpList opList;
};

}  // namespace gdk
#endif  // GDK_BUILDABLE_OP_LIST_H
