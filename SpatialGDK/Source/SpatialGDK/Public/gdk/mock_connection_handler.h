#ifndef GDK_MOCK_CONNECTION_HANDLER_H
#define GDK_MOCK_CONNECTION_HANDLER_H
#include "gdk/abstract_connection_handler.h"
#include "gdk/built_op_list.h"
#include <cstdint>
#include <queue>
#include <unordered_map>
#include <string>
#include <vector>

namespace gdk {
class CommandResponse;
class CommandRequest;
class ComponentUpdate;
class ComponentData;
class EntityState;
class EntitySnapshot;

class MockConnectionHandler : public AbstractConnectionHandler {
public:
  MockConnectionHandler() = default;
  ~MockConnectionHandler() = default;

  // Not copyable or moveable
  MockConnectionHandler(const MockConnectionHandler&) = delete;
  MockConnectionHandler(MockConnectionHandler&&) = delete;
  MockConnectionHandler& operator=(const MockConnectionHandler&) = delete;
  MockConnectionHandler& operator=(MockConnectionHandler&&) = delete;

  void Advance() override;
  size_t GetOpListCount() override;
  OpList GetNextOpList() override;
  void SendMessages(MessagesToSend&& messages) override;
  const std::string& GetWorkerId() const override;
  const std::vector<std::string>& GetWorkerAttributes() const override;


  void AddOpListAtTick(BuiltOpList&& opList, std::int64_t tick);
  void AddOpListNextTick(BuiltOpList&& opList);

  std::int64_t GetCurrentTick() const;

private:
  std::int64_t currentTick = 0;
  std::unordered_map<std::int64_t, std::queue<OpList>> opsAtTick;
};

}  // namespace gdk
#endif  // GDK_MOCK_CONNECTION_HANDLER_H
