// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "SpatialView/Callbacks.h"

namespace SpatialGDK
{
class FDispatcher;

class FScopedDispatcherCallback final
{
public:
	FScopedDispatcherCallback() = delete;
	FScopedDispatcherCallback(FDispatcher& InDispatcher, const CallbackId InCallbackId);
	~FScopedDispatcherCallback();

	// Non-copyable
	FScopedDispatcherCallback(const FScopedDispatcherCallback&) = delete;
	FScopedDispatcherCallback& operator=(const FScopedDispatcherCallback&) = delete;

	// Movable
	FScopedDispatcherCallback(FScopedDispatcherCallback&& InOther)
	{
		Dispatcher = InOther.Dispatcher;
		ScopedCallbackId = InOther.ScopedCallbackId;
		InOther.Dispatcher = nullptr;
		InOther.ScopedCallbackId = InvalidCallbackId;
	}
	FScopedDispatcherCallback& operator=(FScopedDispatcherCallback&& InOther)
	{
		Dispatcher = InOther.Dispatcher;
		ScopedCallbackId = InOther.ScopedCallbackId;
		InOther.Dispatcher = nullptr;
		InOther.ScopedCallbackId = InvalidCallbackId;
		return *this;
	}

	bool IsValid() const;

private:
	FDispatcher* Dispatcher;
	CallbackId ScopedCallbackId;
};
} // namespace SpatialGDK
