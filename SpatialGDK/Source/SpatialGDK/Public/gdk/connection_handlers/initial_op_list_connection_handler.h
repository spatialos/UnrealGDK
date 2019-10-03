#ifndef GDK_INITIAL_OP_LIST_CONNECTION_HANDLER_H
#define GDK_INITIAL_OP_LIST_CONNECTION_HANDLER_H
#include "gdk/connection_handlers/abstract_connection_handler.h"
#include "gdk/op_lists/built_op_list.h"
#include "gdk/op_lists/extracted_op_list.h"
#include <cstddef>
#include <functional>
#include <memory>
#include <queue>

namespace gdk {

/** A connection handler that can present selected ops early, holding back all others. */
class InitialOpListConnectionHandler : public AbstractConnectionHandler {
public:
  explicit InitialOpListConnectionHandler(
      std::unique_ptr<AbstractConnectionHandler> handler,
      std::function<bool(OpList*, ExtractedOpList*)> opExtractor);

  void Advance() override;
  size_t GetOpListCount() override;
  OpList GetNextOpList() override;
  void SendMessages(MessagesToSend&& messages) override;
  const std::string& GetWorkerId() const override;
  const std::vector<std::string>& GetWorkerAttributes() const override;

private:
  enum { kFiltering, kFilterFinished, kFlushingQueuedOpLists, kPassThrough } state;

  void QueueAndExtractOps();

  std::unique_ptr<AbstractConnectionHandler> internalHandler;

  std::unique_ptr<ExtractedOpList> extractedOpList;
  std::function<bool(OpList*, ExtractedOpList*)> opExtractor;

  std::queue<OpList> queuedOpLists;
};

}  // namespace gdk
#endif  // GDK_INITIAL_OP_LIST_CONNECTION_HANDLER_H