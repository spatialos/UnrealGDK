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
class FDispatcher : public IDispatcher
{
public:
	FDispatcher();

	virtual void InvokeCallbacks(const TArray<EntityDelta>& Deltas) override;

	virtual CallbackId RegisterAndInvokeComponentAddedCallback(Worker_ComponentId ComponentId, FComponentValueCallback Callback,
															   const EntityView& View) override;
	virtual CallbackId RegisterAndInvokeComponentRemovedCallback(Worker_ComponentId ComponentId, FComponentValueCallback Callback,
																 const EntityView& View) override;
	virtual CallbackId RegisterAndInvokeComponentValueCallback(Worker_ComponentId ComponentId, FComponentValueCallback Callback,
															   const EntityView& View) override;
	virtual CallbackId RegisterAndInvokeAuthorityGainedCallback(Worker_ComponentId ComponentId, FEntityCallback Callback,
																const EntityView& View) override;
	virtual CallbackId RegisterAndInvokeAuthorityLostCallback(Worker_ComponentId ComponentId, FEntityCallback Callback,
															  const EntityView& View) override;

	virtual CallbackId RegisterComponentAddedCallback(Worker_ComponentId ComponentId, FComponentValueCallback Callback) override;
	virtual CallbackId RegisterComponentRemovedCallback(Worker_ComponentId ComponentId, FComponentValueCallback Callback) override;
	virtual CallbackId RegisterComponentValueCallback(Worker_ComponentId ComponentId, FComponentValueCallback Callback) override;
	virtual CallbackId RegisterAuthorityGainedCallback(Worker_ComponentId ComponentId, FEntityCallback Callback) override;
	virtual CallbackId RegisterAuthorityLostCallback(Worker_ComponentId ComponentId, FEntityCallback Callback) override;
	virtual CallbackId RegisterAuthorityLostTempCallback(Worker_ComponentId ComponentId, FEntityCallback Callback) override;

	virtual void RemoveCallback(CallbackId Id) override;

private:
	struct FComponentCallbacks
	{
		explicit FComponentCallbacks(Worker_ComponentId Id)
			: Id(Id)
		{
		}
		Worker_ComponentId Id;
		TCallbacks<FEntityComponentChange> ComponentAddedCallbacks;
		TCallbacks<FEntityComponentChange> ComponentRemovedCallbacks;
		TCallbacks<FEntityComponentChange> ComponentValueCallbacks;

		struct ComponentIdComparator
		{
			bool operator()(const FComponentCallbacks& Callbacks, Worker_ComponentId ComponentId) const
			{
				return Callbacks.Id < ComponentId;
			}
		};
	};

	struct FAuthorityCallbacks
	{
		explicit FAuthorityCallbacks(Worker_ComponentId Id)
			: Id(Id)
		{
		}
		Worker_ComponentId Id;
		TCallbacks<FSpatialEntityId> AuthorityGainedCallbacks;
		TCallbacks<FSpatialEntityId> AuthorityLostCallbacks;
		TCallbacks<FSpatialEntityId> AuthorityLostTemporarilyCallbacks;

		struct ComponentIdComparator
		{
			bool operator()(const FAuthorityCallbacks& Callbacks, Worker_ComponentId ComponentId) const
			{
				return Callbacks.Id < ComponentId;
			}
		};
	};

	static void InvokeWithExistingValues(Worker_ComponentId ComponentId, const FComponentValueCallback& Callback, const EntityView& View);
	void HandleComponentPresenceChanges(FSpatialEntityId EntityId, const ComponentSpan<ComponentChange>& ComponentChanges,
										TCallbacks<FEntityComponentChange> FComponentCallbacks::*Callbacks);
	void HandleComponentValueChanges(FSpatialEntityId EntityId, const ComponentSpan<ComponentChange>& ComponentChanges);
	void HandleAuthorityChange(FSpatialEntityId EntityId, const ComponentSpan<AuthorityChange>& AuthorityChanges,
							   TCallbacks<FSpatialEntityId> FAuthorityCallbacks::*Callbacks);

	// Component callbacks sorted by component ID;
	TArray<FComponentCallbacks> ComponentCallbacks;
	// Authority callbacks sorted by component ID;
	TArray<FAuthorityCallbacks> AuthorityCallbacks;
	CallbackId NextCallbackId;
};
} // namespace SpatialGDK
