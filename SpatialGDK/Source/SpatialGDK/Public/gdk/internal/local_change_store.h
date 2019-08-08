#ifndef GDK_INTERNAL_LOCAL_CHANGE_STORE_H
#define GDK_INTERNAL_LOCAL_CHANGE_STORE_H
#include "gdk/messages_to_send.h"
#include <queue>
#include <stack>

namespace gdk {

class LocalChangeStore {
public:
  MessagesToSend& GetNewMessagesContainer();

private:
  std::stack<std::shared_ptr<MessagesToSend>> messagesPool;
  std::queue<std::shared_ptr<MessagesToSend>> inFlightMessages;

  MessagesToSend* currentMessages;
};

inline MessagesToSend& LocalChangeStore::GetNewMessagesContainer() {
  if (!messagesPool.empty()) {
    inFlightMessages.push(messagesPool.top());
  }
}

}  // namespace gdk
#endif  // GDK_INTERNAL_LOCAL_CHANGE_STORE_H