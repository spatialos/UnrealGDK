#include "gdk/internal/command_metadata.h"

namespace gdk {

void CommandMetadataManager::AddCommandMetaData(RequestId internalRequestId,
                                                CommandMetadata&& metadata) {
  internalRequestIdToMetaData.emplace(internalRequestId, metadata);
}

const CommandMetadata&
CommandMetadataManager::GetCommandMetaData(RequestId internalRequestId) const {
  return internalRequestIdToMetaData.at(internalRequestId);
}

void CommandMetadataManager::RemoveCommandMetaData(RequestId internalRequestId) {
  internalRequestIdToMetaData.erase(internalRequestId);
}

}  // namespace gdk