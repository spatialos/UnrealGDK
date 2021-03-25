// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "EngineClasses/SpatialNetConnection.h"
#include "SpatialCommonTypes.h"

DECLARE_LOG_CATEGORY_EXTERN(LogServerCrashHandler, Log, All)

class USpatialNetDriver;

namespace SpatialGDK
{
class FSubView;

// The server crash handler informs client workers when their server worker crashes and disconnects. While the server
// worker restarts, we want to try and avoid the client crashing due to network inactivity / RPC overloads and this
// class functionality gives us a way to detect this.
class ServerCrashHandler
{
public:
	ServerCrashHandler(const FSubView& InWellKnownSubView, const FSubView& InPlayerControllerSubView, USpatialNetDriver* InNetDriver);

	void Advance();

private:
	const FSubView* WellKnownSubView;
	const FSubView* PlayerControllerSubView;

	USpatialNetDriver* NetDriver;

	Worker_EntityId AuthServerWorkerEntityId;
};

} // namespace SpatialGDK
