#ifndef GDK_MISC_MESSAGES_H
#define GDK_MISC_MESSAGES_H
#include "gdk/metrics.h"
#include <WorkerSDK/improbable/c_worker.h>
#include <string>
#include <vector>

namespace gdk {

struct LogMessage {
  Worker_LogLevel Level;
  EntityId EntityId;
  std::string LoggerName;
  std::string Message;
};

struct MetricsMessage {
  Metrics Metrics;
};

struct InterestChange {
  EntityId EntityId;
  std::vector<Worker_InterestOverride> Override;
};

}  // namespace gdk
#endif  // GDK_MISC_MESSAGES_H