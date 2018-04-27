#pragma once

#include "UObject/Class.h"

UENUM()
// Flags controlling the behavior of sent commands
enum class ECommandDelivery : uint8
{
  // Default behavior, always sends commands over the network. A successful
  // command response
  // guarantees that the command was fully executed on the target worker.
  ROUNDTRIP = 0,
  // Do not send commands over the network if the target entity is on the same
  // worker as the
  // command
  // originator and the worker is authoritative over the target entity's
  // component.
  // Faster, but less reliable as SpatialOS will not guarantee successful
  // execution in certain
  // edge
  // cases.
  SHORT_CIRCUIT = 1,
};

UENUM(BlueprintType)
// Corresponds to worker::StatusCode
enum class ECommandResponseCode : uint8
{
  Success = 1 UMETA(DisplayName = "Success"),
  Timeout UMETA(DisplayName = "Timed Out"),
  NotFound UMETA(DisplayName = "Not Found"),
  AuthorityLost UMETA(DisplayName = "Authority Lost"),
  PermissionDenied UMETA(DisplayName = "Permission Denied"),
  ApplicationError UMETA(DisplayName = "Application Error"),
  InternalError UMETA(DisplayName = "Internal Error"),
  Unknown UMETA(DisplayName = "Unknown"),
};