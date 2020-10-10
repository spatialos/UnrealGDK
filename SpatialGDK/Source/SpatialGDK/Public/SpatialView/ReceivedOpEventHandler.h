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
	void HandleAddEntity();
	void HandleRemoveEntity();
	void HandleAddComponent();
	void HandleComponentUpdate();
	void HandleRemoveComponent();
	void HandleAuthorityChange();

	TSharedPtr<SpatialEventTracer> EventTracer;
};
} // namespace SpatialGDK
