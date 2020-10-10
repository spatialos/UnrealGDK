#pragma once

#include "SpatialView/Callbacks.h"
#include "SpatialView/ViewDelta.h"

#include "Containers/Array.h"
#include "Templates/Function.h"

namespace SpatialGDK
{
struct FEntityComponentChange
{
	FEntityId EntityId;
	const ComponentChange& Change;
};

using FEntityCallback = TCallbacks<FEntityId>::CallbackType;
using FComponentValueCallback = TCallbacks<FEntityComponentChange>::CallbackType;

class FDispatcher
{
public:
	FDispatcher();

	void InvokeCallbacks(const TArray<EntityDelta>& Deltas);

	CallbackId RegisterAndInvokeComponentAddedCallback(FComponentId ComponentId, FComponentValueCallback Callback, const EntityView& View);
	CallbackId RegisterAndInvokeComponentRemovedCallback(FComponentId ComponentId, FComponentValueCallback Callback,
														 const EntityView& View);
	CallbackId RegisterAndInvokeComponentValueCallback(FComponentId ComponentId, FComponentValueCallback Callback, const EntityView& View);
	CallbackId RegisterAndInvokeAuthorityGainedCallback(FComponentId ComponentId, FEntityCallback Callback, const EntityView& View);
	CallbackId RegisterAndInvokeAuthorityLostCallback(FComponentId ComponentId, FEntityCallback Callback, const EntityView& View);

	CallbackId RegisterComponentAddedCallback(FComponentId ComponentId, FComponentValueCallback Callback);
	CallbackId RegisterComponentRemovedCallback(FComponentId ComponentId, FComponentValueCallback Callback);
	CallbackId RegisterComponentValueCallback(FComponentId ComponentId, FComponentValueCallback Callback);
	CallbackId RegisterAuthorityGainedCallback(FComponentId ComponentId, FEntityCallback Callback);
	CallbackId RegisterAuthorityLostCallback(FComponentId ComponentId, FEntityCallback Callback);
	CallbackId RegisterAuthorityLostTempCallback(FComponentId ComponentId, FEntityCallback Callback);

	void RemoveCallback(CallbackId Id);

private:
	struct FComponentCallbacks
	{
		explicit FComponentCallbacks(FComponentId Id)
			: Id(Id)
		{
		}
		FComponentId Id;
		TCallbacks<FEntityComponentChange> ComponentAddedCallbacks;
		TCallbacks<FEntityComponentChange> ComponentRemovedCallbacks;
		TCallbacks<FEntityComponentChange> ComponentValueCallbacks;

		struct ComponentIdComparator
		{
			bool operator()(const FComponentCallbacks& Callbacks, FComponentId ComponentId) const { return Callbacks.Id < ComponentId; }
		};
	};

	struct FAuthorityCallbacks
	{
		explicit FAuthorityCallbacks(FComponentId Id)
			: Id(Id)
		{
		}
		FComponentId Id;
		TCallbacks<FEntityId> AuthorityGainedCallbacks;
		TCallbacks<FEntityId> AuthorityLostCallbacks;
		TCallbacks<FEntityId> AuthorityLostTemporarilyCallbacks;

		struct ComponentIdComparator
		{
			bool operator()(const FAuthorityCallbacks& Callbacks, FComponentId ComponentId) const { return Callbacks.Id < ComponentId; }
		};
	};

	static void InvokeWithExistingValues(FComponentId ComponentId, const FComponentValueCallback& Callback, const EntityView& View);
	void HandleComponentPresenceChanges(FEntityId EntityId, const ComponentSpan<ComponentChange>& ComponentChanges,
										TCallbacks<FEntityComponentChange> FComponentCallbacks::*Callbacks);
	void HandleComponentValueChanges(FEntityId EntityId, const ComponentSpan<ComponentChange>& ComponentChanges);
	void HandleAuthorityChange(FEntityId EntityId, const ComponentSpan<AuthorityChange>& AuthorityChanges,
							   TCallbacks<FEntityId> FAuthorityCallbacks::*Callbacks);

	// Component callbacks sorted by component ID;
	TArray<FComponentCallbacks> ComponentCallbacks;
	// Authority callbacks sorted by component ID;
	TArray<FAuthorityCallbacks> AuthorityCallbacks;
	CallbackId NextCallbackId;
};
} // namespace SpatialGDK
