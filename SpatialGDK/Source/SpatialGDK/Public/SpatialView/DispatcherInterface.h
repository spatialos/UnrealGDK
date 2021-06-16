// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "SpatialView/Callbacks.h"
#include "SpatialView/ViewDelta.h"

namespace SpatialGDK
{
struct FEntityComponentChange
{
	FSpatialEntityId EntityId;
	const ComponentChange& Change;
};

using FEntityCallback = TCallbacks<FSpatialEntityId>::CallbackType;
using FComponentValueCallback = TCallbacks<FEntityComponentChange>::CallbackType;

class IDispatcher
{
public:
	virtual ~IDispatcher() {}

	virtual void InvokeCallbacks(const TArray<EntityDelta>& Deltas) = 0;

	virtual CallbackId RegisterAndInvokeComponentAddedCallback(Worker_ComponentId ComponentId, FComponentValueCallback Callback,
															   const EntityView& View) = 0;
	virtual CallbackId RegisterAndInvokeComponentRemovedCallback(Worker_ComponentId ComponentId, FComponentValueCallback Callback,
																 const EntityView& View) = 0;
	virtual CallbackId RegisterAndInvokeComponentValueCallback(Worker_ComponentId ComponentId, FComponentValueCallback Callback,
															   const EntityView& View) = 0;
	virtual CallbackId RegisterAndInvokeAuthorityGainedCallback(Worker_ComponentId ComponentId, FEntityCallback Callback,
																const EntityView& View) = 0;
	virtual CallbackId RegisterAndInvokeAuthorityLostCallback(Worker_ComponentId ComponentId, FEntityCallback Callback,
															  const EntityView& View) = 0;

	virtual CallbackId RegisterComponentAddedCallback(Worker_ComponentId ComponentId, FComponentValueCallback Callback) = 0;
	virtual CallbackId RegisterComponentRemovedCallback(Worker_ComponentId ComponentId, FComponentValueCallback Callback) = 0;
	virtual CallbackId RegisterComponentValueCallback(Worker_ComponentId ComponentId, FComponentValueCallback Callback) = 0;
	virtual CallbackId RegisterAuthorityGainedCallback(Worker_ComponentId ComponentId, FEntityCallback Callback) = 0;
	virtual CallbackId RegisterAuthorityLostCallback(Worker_ComponentId ComponentId, FEntityCallback Callback) = 0;
	virtual CallbackId RegisterAuthorityLostTempCallback(Worker_ComponentId ComponentId, FEntityCallback Callback) = 0;

	virtual void RemoveCallback(CallbackId Id) = 0;
};
} // namespace SpatialGDK
