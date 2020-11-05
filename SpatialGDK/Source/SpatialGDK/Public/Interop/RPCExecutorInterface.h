// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CrossServerRPCParams.h"

namespace SpatialGDK
{
class RPCExecutorInterface
{
public:
	virtual ~RPCExecutorInterface() = default;
	virtual FCrossServerRPCParams TryRetrieveCrossServerRPCParams(const Worker_Op& Op) = 0;
	virtual bool ExecuteCommand(const FCrossServerRPCParams& Params) = 0;
};
} // namespace SpatialGDK
