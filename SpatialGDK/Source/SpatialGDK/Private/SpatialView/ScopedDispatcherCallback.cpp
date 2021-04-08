// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "SpatialView/ScopedDispatcherCallback.h"

#include "SpatialView/Dispatcher.h"

namespace SpatialGDK
{
FScopedDispatcherCallback::FScopedDispatcherCallback(IDispatcher& InDispatcher, const CallbackId InCallbackId)
	: Dispatcher(&InDispatcher)
	, ScopedCallbackId(InCallbackId)
{
	check(Dispatcher);
}

FScopedDispatcherCallback::~FScopedDispatcherCallback()
{
	if (IsValid())
	{
		Dispatcher->RemoveCallback(ScopedCallbackId);
	}
}

bool FScopedDispatcherCallback::IsValid() const
{
	if (Dispatcher)
	{
		check(ScopedCallbackId != InvalidCallbackId);
		return true;
	}
	return false;
}

} // namespace SpatialGDK
