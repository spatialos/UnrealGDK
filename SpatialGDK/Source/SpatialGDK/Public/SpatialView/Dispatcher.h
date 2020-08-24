#pragma once

#include "SpatialView/Callbacks.h"
#include "SpatialView/ViewDelta.h"

#include "Containers/Array.h"
#include "Templates/Function.h"

namespace SpatialGDK
{
struct FEntityComponentChange
{
	Worker_EntityId EntityId;
	const ComponentChange& Change;
};

using FEntityCallback = TCallbacks<Worker_EntityId>::CallbackType;
using FComponentValueCallback = TCallbacks<FEntityComponentChange>::CallbackType;

class FDispatcher
{
public:
	explicit FDispatcher();

	void InvokeCallbacks(const TArray<EntityDelta>& Deltas);

	CallbackId RegisterAndInvokeComponentAddedCallback(Worker_ComponentId ComponentId, FComponentValueCallback Callback, const EntityView* View);
	CallbackId RegisterAndInvokeComponentRemovedCallback(Worker_ComponentId ComponentId, FComponentValueCallback Callback, const EntityView* View);
	CallbackId RegisterAndInvokeComponentValueCallback(Worker_ComponentId ComponentId, FComponentValueCallback Callback, const EntityView* View);
	CallbackId RegisterAndInvokeAuthorityGainedCallback(Worker_ComponentId ComponentId, FEntityCallback Callback, const EntityView* View);
	CallbackId RegisterAndInvokeAuthorityLostCallback(Worker_ComponentId ComponentId, FEntityCallback Callback, const EntityView* View);

	CallbackId RegisterComponentAddedCallback(Worker_ComponentId ComponentId, FComponentValueCallback Callback);
	CallbackId RegisterComponentRemovedCallback(Worker_ComponentId ComponentId, FComponentValueCallback Callback);
	CallbackId RegisterComponentValueCallback(Worker_ComponentId ComponentId, FComponentValueCallback Callback);
	CallbackId RegisterAuthorityGainedCallback(Worker_ComponentId ComponentId, FEntityCallback Callback);
	CallbackId RegisterAuthorityLostCallback(Worker_ComponentId ComponentId, FEntityCallback Callback);
	CallbackId RegisterAuthorityLostTempCallback(Worker_ComponentId ComponentId, FEntityCallback Callback);

	void RemoveCallback(CallbackId Id);

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
		TCallbacks<Worker_EntityId> AuthorityGainedCallbacks;
		TCallbacks<Worker_EntityId> AuthorityLostCallbacks;
		TCallbacks<Worker_EntityId> AuthorityLostTemporarilyCallbacks;

		struct ComponentIdComparator
		{
			bool operator()(const FAuthorityCallbacks& Callbacks, Worker_ComponentId ComponentId) const
			{
				return Callbacks.Id < ComponentId;
			}
		};
	};

	static void InvokeWithExistingValues(Worker_ComponentId ComponentId, const FComponentValueCallback& Callback, const EntityView* View);
	void HandleComponentPresenceChanges(Worker_EntityId EntityId, const ComponentSpan<ComponentChange>& ComponentChanges,
										TCallbacks<FEntityComponentChange> FComponentCallbacks::*Callbacks);
	void HandleComponentValueChanges(Worker_EntityId EntityId, const ComponentSpan<ComponentChange>& ComponentChanges);
	void HandleAuthorityChange(Worker_EntityId EntityId, const ComponentSpan<AuthorityChange>& AuthorityChanges,
							   TCallbacks<Worker_EntityId> FAuthorityCallbacks::*Callbacks);

	// Component callbacks sorted by component ID;
	TArray<FComponentCallbacks> ComponentCallbacks;
	// Authority callbacks sorted by component ID;
	TArray<FAuthorityCallbacks> AuthorityCallbacks;
	CallbackId NextCallbackId;
};
} // namespace SpatialGDK
