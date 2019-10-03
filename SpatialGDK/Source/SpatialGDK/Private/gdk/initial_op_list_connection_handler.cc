#include "gdk/connection_handlers/initial_op_list_connection_handler.h"

namespace gdk {
InitialOpListConnectionHandler::InitialOpListConnectionHandler(
    std::unique_ptr<AbstractConnectionHandler> handler,
    std::function<bool(OpList*, ExtractedOpList*)> opExtractor)
: state(kFiltering), internalHandler(std::move(handler)), opExtractor(std::move(opExtractor)) {}

void InitialOpListConnectionHandler::Advance() {
  internalHandler->Advance();
  if (state == kPassThrough) {
    return;
  }

  switch (state) {
  case kFiltering: {
    OpList empty{};
    extractedOpList = std::make_unique<ExtractedOpList>();
    if (opExtractor(&empty, extractedOpList.get())) {
      state = kFilterFinished;
    }
    break;
  }
  case kFilterFinished:
    extractedOpList = nullptr;
    state = kFlushingQueuedOpLists;
    break;
  case kFlushingQueuedOpLists:
    state = kPassThrough;
    break;
  case kPassThrough:
    // Ignore.
    break;
  }

  if (state != kPassThrough) {
    QueueAndExtractOps();
  }
}

size_t InitialOpListConnectionHandler::GetOpListCount() {
  switch (state) {
  case kFiltering:
    return 1;
  case kFilterFinished:
    return 1;
  case kFlushingQueuedOpLists:
    return queuedOpLists.size();
  case kPassThrough:
    return internalHandler->GetOpListCount();
  default:
    // Not possible.
    return 0;
  }
}

OpList InitialOpListConnectionHandler::GetNextOpList() {
  switch (state) {
  case kFiltering:
    // Fallthrough.
  case kFilterFinished: {
    OpList tmp{std::move(extractedOpList)};
    extractedOpList = nullptr;
    return tmp;
  }
  case kFlushingQueuedOpLists: {
    OpList tmp{std::move(queuedOpLists.front())};
    queuedOpLists.pop();
    return tmp;
  }
  case kPassThrough:
    return internalHandler->GetNextOpList();
  default:
    // Not possible.
    return OpList{};
  }
}

void InitialOpListConnectionHandler::SendMessages(MessagesToSend&& messages) {
  internalHandler->SendMessages(std::move(messages));
}

const std::string& InitialOpListConnectionHandler::GetWorkerId() const {
  return internalHandler->GetWorkerId();
}

const std::vector<std::string>& InitialOpListConnectionHandler::GetWorkerAttributes() const {
  return internalHandler->GetWorkerAttributes();
}

void InitialOpListConnectionHandler::QueueAndExtractOps() {
  const auto opListsAvailable = internalHandler->GetOpListCount();
  for (size_t i = 0; i < opListsAvailable; ++i) {
    queuedOpLists.push(internalHandler->GetNextOpList());
    if (state == kFiltering && opExtractor(&queuedOpLists.back(), extractedOpList.get())) {
      state = kFilterFinished;
    }
  }
}
}  // namespace gdk
