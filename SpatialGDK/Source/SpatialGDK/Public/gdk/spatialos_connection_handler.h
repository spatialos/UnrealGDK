#ifndef GDK_SPATIALOS_CONNECTION_HANDLER_H
#define GDK_SPATIALOS_CONNECTION_HANDLER_H
#include "gdk/abstract_connection_handler.h"
#include "gdk/common_types.h"
#include "gdk/op_list.h"
#include <condition_variable>
#include <mutex>
#include <queue>
#include <string>
#include <thread>
#include <unordered_map>
#include <vector>

namespace gdk {

class SpatialOsConnectionHandler : public AbstractConnectionHandler {
public:
  explicit SpatialOsConnectionHandler(Worker_Connection* connection);

  ~SpatialOsConnectionHandler();

  // Not copyable or moveable.
  SpatialOsConnectionHandler(const SpatialOsConnectionHandler&) = delete;
  SpatialOsConnectionHandler(SpatialOsConnectionHandler&&) = delete;
  SpatialOsConnectionHandler& operator=(const SpatialOsConnectionHandler&) = delete;
  SpatialOsConnectionHandler& operator=(SpatialOsConnectionHandler&&) = delete;

  void Advance() override;
  size_t GetOpListCount() override;
  OpList GetNextOpList() override;
  void SendMessages(MessagesToSend&& messages) override;
  const std::string& GetWorkerId() const override;
  const std::vector<std::string>& GetWorkerAttributes() const override;

private:
  struct ConnectionDeleter {
    void operator()(Worker_Connection* connection) const noexcept;
  };

  void PollOpsInBackground();
  void PollOps();
  void SendAndClearMessages(MessagesToSend* messages);

  std::mutex connectionMutex;
  std::condition_variable connectionCV;
  std::thread pollOpsThread;

  std::unique_ptr<Worker_Connection, ConnectionDeleter> connection;
  std::queue<OpList> receivedOpsQueue;

  std::unordered_map<RequestId, RequestId> internalToPublicRequestIds;

  std::string workerId;
  std::vector<std::string> workerAttributes;
};

}  // namespace gdk
#endif  // GDK_SPATIALOS_CONNECTION_HANDLER_H
