#include "gdk/mock_connection_handler.h"

#include <memory>
#include <utility>

namespace {
const std::string kWorkerId = "test_worker_id";
const std::string kWorkerAttribute = "test_worker_attribute";
const std::vector<std::string> kWorkerAttributes{kWorkerAttribute,
                                                 std::string{"workerId:"} + kWorkerId};
}  // anonymous namespace

namespace gdk {

void MockConnectionHandler::Advance() {
  ++currentTick;
}

size_t MockConnectionHandler::GetOpListCount() {
  auto it = opsAtTick.find(currentTick);
  if (it == opsAtTick.end()) {
    return 0;
  }
  return it->second.size();
}

OpList MockConnectionHandler::GetNextOpList() {
  auto it = opsAtTick.find(currentTick);
  if (it == opsAtTick.end()) {
    return OpList{};
  }
  OpList opList = std::move(it->second.front());
  it->second.pop();
  if (it->second.empty()) {
    opsAtTick.erase(it);
  }
  return opList;
}

void MockConnectionHandler::SendMessages(MessagesToSend&&) {}

const std::string& MockConnectionHandler::GetWorkerId() const {
  return kWorkerId; 
}

const std::vector<std::string>& MockConnectionHandler::GetWorkerAttributes() const {
  return kWorkerAttributes;
}

void MockConnectionHandler::AddOpListAtTick(BuiltOpList&& opList, std::int64_t tick) {
  OpList ops{std::make_unique<BuiltOpList>(std::move(opList))};
  opsAtTick[tick].push(std::move(ops));
}

void MockConnectionHandler::AddOpListNextTick(BuiltOpList&& opList) {
  AddOpListAtTick(std::move(opList), currentTick + 1);
}

std::int64_t MockConnectionHandler::GetCurrentTick() const {
  return currentTick;
}

}  // namespace gdk
