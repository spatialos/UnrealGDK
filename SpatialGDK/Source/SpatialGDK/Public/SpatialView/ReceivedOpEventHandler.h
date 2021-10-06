// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "Interop/Connection/SpatialEventTracer.h"
#include "OpList/OpList.h"

namespace SpatialGDK
{
class FReceivedOpEventHandler
{
public:
	explicit FReceivedOpEventHandler(TSharedPtr<SpatialEventTracer> EventTracer = nullptr);
	void ProcessOpLists(const OpList& Ops);

private:
	TSharedPtr<SpatialEventTracer> EventTracer;
};
} // namespace SpatialGDK
