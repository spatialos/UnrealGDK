#ifndef GDK_INITIAL_OP_LIST_CONNECTION_HANDLER_H
#define GDK_INITIAL_OP_LIST_CONNECTION_HANDLER_H
#include "gdk/abstract_connection_handler.h"
#include "gdk/built_op_list.h"
#include <cstddef>
#include <functional>
#include <memory>
#include <queue>

namespace gdk {
class ExtractedOpList : public AbstractOpList {
public:
  size_t GetCount() const override;
  Worker_Op& operator[](size_t index) override;

  void AddOp(OpList* opList, size_t index);

private:
  std::vector<Worker_Op> ops;
};

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
  enum {
    kFiltering,
    kFilterFinished,
    kFlushingQueuedOpLists,
    kPassThrough
  } state;

  std::unique_ptr<AbstractConnectionHandler> internalHandler;

  std::unique_ptr<ExtractedOpList> extractedOpList;
  std::function<bool(OpList*, ExtractedOpList*)> opExtractor;

  std::queue<OpList> queuedOpLists;
};

}  // namespace gdk
#endif  // GDK_INITIAL_OP_LIST_CONNECTION_HANDLER_H
