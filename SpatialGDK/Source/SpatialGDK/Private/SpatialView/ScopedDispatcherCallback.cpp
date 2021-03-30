// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "SpatialView/ScopedDispatcherCallback.h"

#include "SpatialView/Dispatcher.h"

namespace SpatialGDK
{
FScopedDispatcherCallback::FScopedDispatcherCallback(FDispatcher& InDispatcher, const CallbackId InCallbackId)
	: Dispatcher(&InDispatcher)
	, ScopedCallbackId(InCallbackId)
{
	check(Dispatcher);
}

FScopedDispatcherCallback::~FScopedDispatcherCallback()
{
	check(Dispatcher);
	Dispatcher->RemoveCallback(ScopedCallbackId);
}
} // namespace SpatialGDK
