#ifndef GDK_COMMAND_METADATA_H
#define GDK_COMMAND_METADATA_H
#include "command_request.h"
#include <unordered_map>

namespace gdk {

struct CommandMetadata {
  CommandMetadata(RequestId requestId)  //, CommandRequest&& request)
  : PublicRequestId(requestId) {}
  //, Request(std::move(request)) {}

  RequestId PublicRequestId;
  // todo add retry and timeout data
  // todo should the request payload be there even if not retried
  // CommandRequest Request;
};

class CommandMetadataManager {
public:
  CommandMetadataManager() = default;
  ~CommandMetadataManager() = default;

  // Moveable, not copyable.
  CommandMetadataManager(const CommandMetadataManager&) = delete;
  CommandMetadataManager(CommandMetadataManager&&) = default;
  CommandMetadataManager& operator=(const CommandMetadataManager&) = delete;
  CommandMetadataManager& operator=(CommandMetadataManager&&) = default;

  void AddCommandMetaData(RequestId internalRequestId, CommandMetadata&& metadata);
  const CommandMetadata& GetCommandMetaData(RequestId internalRequestId) const;
  void RemoveCommandMetaData(RequestId internalRequestId);

private:
  std::unordered_map<RequestId, CommandMetadata> internalRequestIdToMetaData;
};

}  // namespace gdk
#endif  // GDK_COMMAND_METADATA_H