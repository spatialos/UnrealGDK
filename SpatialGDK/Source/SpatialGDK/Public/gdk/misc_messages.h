#ifndef GDK_MISC_MESSAGES_H
#define GDK_MISC_MESSAGES_H
#include "gdk/metrics.h"
#include <string>
#include <vector>

namespace gdk {

struct LogMessage {
  Worker_LogLevel Level;
  gdk::EntityId EntityId;
  std::string LoggerName;
  std::string Message;
};

struct MetricsMessage {
  gdk::Metrics Metrics;
};

struct InterestChange {
  gdk::EntityId EntityId;
  std::vector<Worker_InterestOverride> Override;
};

}  // namespace gdk
#endif  // GDK_MISC_MESSAGES_H