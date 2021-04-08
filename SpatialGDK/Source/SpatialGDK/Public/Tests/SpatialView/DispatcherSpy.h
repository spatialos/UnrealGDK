// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "SpatialView/Callbacks.h"
#include "SpatialView/DispatcherInterface.h"
#include "SpatialView/ScopedDispatcherCallback.h"
#include "SpatialView/ViewDelta.h"

#include "Containers/Array.h"
#include "Templates/Function.h"

namespace SpatialGDK
{
class FDispatcherSpy : public IDispatcher
{
public:
	FDispatcherSpy()
		: NextCallbackId(FirstValidCallbackId)
	{
	}

	virtual void InvokeCallbacks(const TArray<EntityDelta>& Deltas) override {}

	virtual CallbackId RegisterAndInvokeComponentAddedCallback(Worker_ComponentId ComponentId, FComponentValueCallback Callback,
															   const EntityView& View) override
	{
		return RegisterAndReturnNewCallback();
	}
	virtual CallbackId RegisterAndInvokeComponentRemovedCallback(Worker_ComponentId ComponentId, FComponentValueCallback Callback,
																 const EntityView& View) override
	{
		return RegisterAndReturnNewCallback();
	}
	virtual CallbackId RegisterAndInvokeComponentValueCallback(Worker_ComponentId ComponentId, FComponentValueCallback Callback,
															   const EntityView& View) override
	{
		return RegisterAndReturnNewCallback();
	}
	virtual CallbackId RegisterAndInvokeAuthorityGainedCallback(Worker_ComponentId ComponentId, FEntityCallback Callback,
																const EntityView& View) override
	{
		return RegisterAndReturnNewCallback();
	}
	virtual CallbackId RegisterAndInvokeAuthorityLostCallback(Worker_ComponentId ComponentId, FEntityCallback Callback,
															  const EntityView& View) override
	{
		return RegisterAndReturnNewCallback();
	}

	virtual CallbackId RegisterComponentAddedCallback(Worker_ComponentId ComponentId, FComponentValueCallback Callback) override
	{
		return RegisterAndReturnNewCallback();
	}
	virtual CallbackId RegisterComponentRemovedCallback(Worker_ComponentId ComponentId, FComponentValueCallback Callback) override
	{
		return RegisterAndReturnNewCallback();
	}
	virtual CallbackId RegisterComponentValueCallback(Worker_ComponentId ComponentId, FComponentValueCallback Callback) override
	{
		return RegisterAndReturnNewCallback();
	}
	virtual CallbackId RegisterAuthorityGainedCallback(Worker_ComponentId ComponentId, FEntityCallback Callback) override
	{
		return RegisterAndReturnNewCallback();
	}
	virtual CallbackId RegisterAuthorityLostCallback(Worker_ComponentId ComponentId, FEntityCallback Callback) override
	{
		return RegisterAndReturnNewCallback();
	}
	virtual CallbackId RegisterAuthorityLostTempCallback(Worker_ComponentId ComponentId, FEntityCallback Callback) override
	{
		return RegisterAndReturnNewCallback();
	}

	virtual void RemoveCallback(CallbackId Id) override { RegisteredCallbacks.Remove(Id); }

	int32 GetNumCallbacks() const { return RegisteredCallbacks.Num(); }

private:
	CallbackId RegisterAndReturnNewCallback()
	{
		RegisteredCallbacks.Emplace(NextCallbackId);
		return NextCallbackId++;
	}

	CallbackId NextCallbackId;
	TSet<CallbackId> RegisteredCallbacks;
};
} // namespace SpatialGDK
