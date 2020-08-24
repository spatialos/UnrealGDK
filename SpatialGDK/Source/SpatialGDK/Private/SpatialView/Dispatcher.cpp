// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "SpatialView/Dispatcher.h"

#include "Algo/BinarySearch.h"
#include "SpatialView/EntityComponentTypes.h"

namespace SpatialGDK
{
FDispatcher::FDispatcher()
	: NextCallbackId(1)
{
}

void FDispatcher::InvokeCallbacks(const TArray<EntityDelta>& Deltas)
{
	for (const EntityDelta& Delta : Deltas)
	{
		HandleComponentPresenceChanges(Delta.EntityId, Delta.ComponentsAdded, &FComponentCallbacks::ComponentAddedCallbacks);
		HandleComponentPresenceChanges(Delta.EntityId, Delta.ComponentsRemoved, &FComponentCallbacks::ComponentRemovedCallbacks);
		HandleComponentValueChanges(Delta.EntityId, Delta.ComponentUpdates);
		HandleComponentValueChanges(Delta.EntityId, Delta.ComponentsRefreshed);

		HandleAuthorityChange(Delta.EntityId, Delta.AuthorityGained, &FAuthorityCallbacks::AuthorityGainedCallbacks);
		HandleAuthorityChange(Delta.EntityId, Delta.AuthorityLost, &FAuthorityCallbacks::AuthorityLostCallbacks);
		HandleAuthorityChange(Delta.EntityId, Delta.AuthorityLostTemporarily, &FAuthorityCallbacks::AuthorityLostTemporarilyCallbacks);
	}
}

CallbackId FDispatcher::RegisterAndInvokeComponentAddedCallback(Worker_ComponentId ComponentId, FComponentValueCallback Callback,
																const EntityView* View)
{
	InvokeWithExistingValues(ComponentId, Callback, View);
	return RegisterComponentAddedCallback(ComponentId, MoveTemp(Callback));
}

CallbackId FDispatcher::RegisterAndInvokeComponentRemovedCallback(Worker_ComponentId ComponentId, FComponentValueCallback Callback,
																  const EntityView* View)
{
	for (const auto& Entity : *View)
	{
		if (!Entity.Value.Components.ContainsByPredicate(ComponentIdEquality{ ComponentId }))
		{
			Callback({ Entity.Key, ComponentChange(ComponentId) });
		}
	}
	return RegisterComponentRemovedCallback(ComponentId, MoveTemp(Callback));
}

CallbackId FDispatcher::RegisterAndInvokeComponentValueCallback(Worker_ComponentId ComponentId, FComponentValueCallback Callback,
																const EntityView* View)
{
	InvokeWithExistingValues(ComponentId, Callback, View);
	return RegisterComponentValueCallback(ComponentId, MoveTemp(Callback));
}

CallbackId FDispatcher::RegisterAndInvokeAuthorityGainedCallback(Worker_ComponentId ComponentId, FEntityCallback Callback,
																 const EntityView* View)
{
	for (const auto& Entity : *View)
	{
		if (Entity.Value.Authority.Contains(ComponentId))
		{
			Callback(Entity.Key);
		}
	}
	return RegisterAuthorityGainedCallback(ComponentId, MoveTemp(Callback));
}

CallbackId FDispatcher::RegisterAndInvokeAuthorityLostCallback(Worker_ComponentId ComponentId, FEntityCallback Callback,
															   const EntityView* View)
{
	for (const auto& Entity : *View)
	{
		if (!Entity.Value.Authority.Contains(ComponentId))
		{
			Callback(Entity.Key);
		}
	}
	return RegisterAuthorityLostCallback(ComponentId, MoveTemp(Callback));
}

CallbackId FDispatcher::RegisterComponentAddedCallback(Worker_ComponentId ComponentId, FComponentValueCallback Callback)
{
	const int32 Index = Algo::LowerBound(ComponentCallbacks, ComponentId, FComponentCallbacks::ComponentIdComparator());
	if (Index == ComponentCallbacks.Num() || ComponentCallbacks[Index].Id != ComponentId)
	{
		ComponentCallbacks.EmplaceAt(Index, ComponentId);
	}
	ComponentCallbacks[Index].ComponentAddedCallbacks.Register(NextCallbackId, MoveTemp(Callback));
	return NextCallbackId++;
}

CallbackId FDispatcher::RegisterComponentRemovedCallback(Worker_ComponentId ComponentId, FComponentValueCallback Callback)
{
	const int32 Index = Algo::LowerBound(ComponentCallbacks, ComponentId, FComponentCallbacks::ComponentIdComparator());
	if (Index == ComponentCallbacks.Num() || ComponentCallbacks[Index].Id != ComponentId)
	{
		ComponentCallbacks.EmplaceAt(Index, ComponentId);
	}
	ComponentCallbacks[Index].ComponentRemovedCallbacks.Register(NextCallbackId, MoveTemp(Callback));
	return NextCallbackId++;
}

CallbackId FDispatcher::RegisterComponentValueCallback(Worker_ComponentId ComponentId, FComponentValueCallback Callback)
{
	const int32 Index = Algo::LowerBound(ComponentCallbacks, ComponentId, FComponentCallbacks::ComponentIdComparator());
	if (Index == ComponentCallbacks.Num() || ComponentCallbacks[Index].Id != ComponentId)
	{
		ComponentCallbacks.EmplaceAt(Index, ComponentId);
	}
	ComponentCallbacks[Index].ComponentValueCallbacks.Register(NextCallbackId, MoveTemp(Callback));
	return NextCallbackId++;
}

CallbackId FDispatcher::RegisterAuthorityGainedCallback(Worker_ComponentId ComponentId, FEntityCallback Callback)
{
	const int32 Index = Algo::LowerBound(AuthorityCallbacks, ComponentId, FAuthorityCallbacks::ComponentIdComparator());
	if (Index == AuthorityCallbacks.Num() || AuthorityCallbacks[Index].Id != ComponentId)
	{
		AuthorityCallbacks.EmplaceAt(Index, ComponentId);
	}
	AuthorityCallbacks[Index].AuthorityGainedCallbacks.Register(NextCallbackId, MoveTemp(Callback));
	return NextCallbackId++;
}

CallbackId FDispatcher::RegisterAuthorityLostCallback(Worker_ComponentId ComponentId, FEntityCallback Callback)
{
	const int32 Index = Algo::LowerBound(AuthorityCallbacks, ComponentId, FAuthorityCallbacks::ComponentIdComparator());
	if (Index == AuthorityCallbacks.Num() || AuthorityCallbacks[Index].Id != ComponentId)
	{
		AuthorityCallbacks.EmplaceAt(Index, ComponentId);
	}
	AuthorityCallbacks[Index].AuthorityLostCallbacks.Register(NextCallbackId, MoveTemp(Callback));
	return NextCallbackId++;
}

CallbackId FDispatcher::RegisterAuthorityLostTempCallback(Worker_ComponentId ComponentId, FEntityCallback Callback)
{
	const int32 Index = Algo::LowerBound(AuthorityCallbacks, ComponentId, FAuthorityCallbacks::ComponentIdComparator());
	if (Index == AuthorityCallbacks.Num() || AuthorityCallbacks[Index].Id != ComponentId)
	{
		AuthorityCallbacks.EmplaceAt(Index, ComponentId);
	}
	AuthorityCallbacks[Index].AuthorityLostTemporarilyCallbacks.Register(NextCallbackId, MoveTemp(Callback));
	return NextCallbackId++;
}

void FDispatcher::RemoveCallback(CallbackId Id)
{
	for (FComponentCallbacks& Callback : ComponentCallbacks)
	{
		Callback.ComponentAddedCallbacks.Remove(Id);
		Callback.ComponentRemovedCallbacks.Remove(Id);
	}

	for (FAuthorityCallbacks& Callback : AuthorityCallbacks)
	{
		Callback.AuthorityGainedCallbacks.Remove(Id);
		Callback.AuthorityLostCallbacks.Remove(Id);
		Callback.AuthorityLostTemporarilyCallbacks.Remove(Id);
	}
}

void FDispatcher::InvokeWithExistingValues(Worker_ComponentId ComponentId, const FComponentValueCallback& Callback, const EntityView* View)
{
	for (const auto& Entity : *View)
	{
		const ComponentData* It = Entity.Value.Components.FindByPredicate(ComponentIdEquality{ ComponentId });
		if (It != nullptr)
		{
			const ComponentChange Change(ComponentId, It->GetUnderlying());
			Callback({ Entity.Key, Change });
		}
	}
}

void FDispatcher::HandleComponentPresenceChanges(Worker_EntityId EntityId, const ComponentSpan<ComponentChange>& ComponentChanges,
												 TCallbacks<FEntityComponentChange> FComponentCallbacks::*Callbacks)
{
	auto* CallbackIt = ComponentCallbacks.GetData();
	auto* ChangeIt = ComponentChanges.GetData();

	const auto* CallbackEnd = ComponentCallbacks.GetData() + ComponentCallbacks.Num();
	const auto* ChangeEnd = ComponentChanges.GetData() + ComponentChanges.Num();

	// Find the intersection between callbacks and changes and invoke all such callbacks.
	while (CallbackIt != CallbackEnd && ChangeIt != ChangeEnd)
	{
		if (CallbackIt->Id < ChangeIt->ComponentId)
		{
			++CallbackIt;
		}
		else if (ChangeIt->ComponentId < CallbackIt->Id)
		{
			++ChangeIt;
		}
		else
		{
			const FEntityComponentChange EntityComponentChange = { EntityId, *ChangeIt };
			(CallbackIt->*Callbacks).Invoke(EntityComponentChange);
			CallbackIt->ComponentValueCallbacks.Invoke(EntityComponentChange);
		}
		++ChangeIt;
		++CallbackIt;
	}
}

void FDispatcher::HandleComponentValueChanges(Worker_EntityId EntityId, const ComponentSpan<ComponentChange>& ComponentChanges)
{
	auto* CallbackIt = ComponentCallbacks.GetData();
	auto* ChangeIt = ComponentChanges.GetData();

	const auto* CallbackEnd = ComponentCallbacks.GetData() + ComponentCallbacks.Num();
	const auto* ChangeEnd = ComponentChanges.GetData() + ComponentChanges.Num();

	// Find the intersection between callbacks and changes and invoke all such callbacks.
	while (CallbackIt != CallbackEnd && ChangeIt != ChangeEnd)
	{
		if (CallbackIt->Id < ChangeIt->ComponentId)
		{
			++CallbackIt;
		}
		else if (ChangeIt->ComponentId < CallbackIt->Id)
		{
			++ChangeIt;
		}
		else
		{
			const FEntityComponentChange EntityComponentChange = { EntityId, *ChangeIt };
			CallbackIt->ComponentValueCallbacks.Invoke(EntityComponentChange);
		}
		++ChangeIt;
		++CallbackIt;
	}
}

void FDispatcher::HandleAuthorityChange(Worker_EntityId EntityId, const ComponentSpan<AuthorityChange>& AuthorityChanges,
										TCallbacks<Worker_EntityId> FAuthorityCallbacks::*Callbacks)
{
	auto* CallbackIt = AuthorityCallbacks.GetData();
	auto* ChangeIt = AuthorityChanges.GetData();

	const auto* CallbackEnd = AuthorityCallbacks.GetData() + AuthorityCallbacks.Num();
	const auto* ChangeEnd = AuthorityChanges.GetData() + AuthorityChanges.Num();

	// Find the intersection between callbacks and changes and invoke all such callbacks.
	while (CallbackIt != CallbackEnd && ChangeIt != ChangeEnd)
	{
		if (CallbackIt->Id < ChangeIt->ComponentId)
		{
			++CallbackIt;
		}
		else if (ChangeIt->ComponentId < CallbackIt->Id)
		{
			++ChangeIt;
		}
		else
		{
			(CallbackIt->*Callbacks).Invoke(EntityId);
		}
		++ChangeIt;
		++CallbackIt;
	}
}
} // namespace SpatialGDK
