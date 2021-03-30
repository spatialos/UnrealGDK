// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "SpatialView/Callbacks.h"

namespace SpatialGDK
{
class FDispatcher;

class FScopedDispatcherCallback final
{
public:
	FScopedDispatcherCallback(FDispatcher& InDispatcher, const CallbackId InCallbackId);
	~FScopedDispatcherCallback();

	// Non-copyable, non-movable
	FScopedDispatcherCallback(const FScopedDispatcherCallback&) = delete;
	FScopedDispatcherCallback(FScopedDispatcherCallback&&) = delete;
	FScopedDispatcherCallback& operator=(const FScopedDispatcherCallback&) = delete;
	FScopedDispatcherCallback& operator=(FScopedDispatcherCallback&&) = delete;

private:
	FDispatcher* Dispatcher;
	CallbackId ScopedCallbackId;
};
} // namespace SpatialGDK
