// Copyright (c) Improbable Worlds Ltd, All Rights Reserved
#ifndef WORKER_SDK_CPP_INCLUDE_IMPROBABLE_DEFAULTS_H
#define WORKER_SDK_CPP_INCLUDE_IMPROBABLE_DEFAULTS_H
#include <cstddef>
#include <cstdint>
#include <string>

namespace worker {
namespace defaults {
namespace {
// General asynchronous IO.
const std::uint32_t kSendQueueCapacity = 4096;
const std::uint32_t kReceiveQueueCapacity = 4096;
const std::uint32_t kLogMessageQueueCapacity = 256;
const std::uint32_t kBuiltInMetricsReportPeriodMillis = 5000;
// General networking.
const bool kUseExternalIp = false;
const std::uint32_t kConnectionTimeoutMillis = 60000;
// TCP.
const std::uint8_t kTcpMultiplexLevel = 32;
const std::uint32_t kTcpSendBufferSize = 65536;
const std::uint32_t kTcpReceiveBufferSize = 65536;
const bool kTcpNoDelay = false;
// RakNet.
const uint32_t kRakNetHeartbeatTimeoutMillis = 60000;
// Protocol logging.
const std::string kLogPrefix = "protocol-log-";
const std::uint32_t kMaxLogFiles = 10;
const std::uint32_t kMaxLogFileSizeBytes = 1024 * 1024;
}  // anonymous namespace
}  // ::defaults
}  // ::worker

#endif  // WORKER_SDK_CPP_INCLUDE_IMPROBABLE_DEFAULTS_H
