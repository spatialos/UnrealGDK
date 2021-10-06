// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "SpatialView/Callbacks.h"

namespace SpatialGDK
{
class IDispatcher;

class FScopedDispatcherCallback final
{
public:
	FScopedDispatcherCallback() = delete;
	FScopedDispatcherCallback(IDispatcher& InDispatcher, const CallbackId InCallbackId);
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
	IDispatcher* Dispatcher;
	CallbackId ScopedCallbackId;
};
} // namespace SpatialGDK
