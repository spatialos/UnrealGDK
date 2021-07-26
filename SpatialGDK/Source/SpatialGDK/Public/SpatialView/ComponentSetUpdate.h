// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "SpatialView/ComponentUpdate.h"

#include "Algo/Transform.h"
#include "Containers/Map.h"
#include "Templates/TypeHash.h"

namespace SpatialGDK
{
// Set of functions that allows for ComponentUpdate to be used in a TSet with heterogeneous lookup.
struct ComponentUpdateIdKeyFuncs : BaseKeyFuncs<ComponentUpdate, Worker_ComponentId, false>
{
	// N.b. ConstPointerType is really a const Worker_ComponentId.
	// using KeyInitType = TTypeTraits<Worker_ComponentId>::ConstPointerType;
	// N.b. ElementInitType is really a const ComponentUpdate&.
	// using ElementInitType = TCallTraits<ComponentUpdate>::ParamType;

	using KeyInitType = Worker_ComponentId;
	using ElementInitType = const ComponentUpdate&;

	static KeyInitType GetSetKey(ElementInitType Update) { return Update.GetComponentId(); }

	static bool Matches(KeyInitType A, KeyInitType B) { return A == B; }

	static uint32 GetKeyHash(KeyInitType Id)
	{
		// Need to fully qualify the function name for it to be found.
		return ::GetTypeHash(Id);
	}
};

using ComponentUpdateHashSet = TSet<ComponentUpdate, ComponentUpdateIdKeyFuncs>;

// An owning wrapper for a component-set-update.
class FComponentSetUpdate
{
public:
	// Creates a new component-set-update.
	explicit FComponentSetUpdate(Worker_ComponentSetId Id)
		: ComponentSetId(Id)
	{
	}

	// Takes ownership of component updates provided.
	// The array provided will be cleared and is safe for the caller to reuse.
	explicit FComponentSetUpdate(Worker_ComponentId Id, TArray<ComponentUpdate> Updates)
		: ComponentSetId(Id)
		, Updates(MoveTemp(Updates))
	{
	}

	~FComponentSetUpdate() = default;

	// Moveable, not copyable.
	FComponentSetUpdate(const FComponentSetUpdate&) = delete;
	FComponentSetUpdate(FComponentSetUpdate&&) = default;
	FComponentSetUpdate& operator=(const FComponentSetUpdate&) = delete;
	FComponentSetUpdate& operator=(FComponentSetUpdate&&) = default;

	// Creates a copy of the component-set-update.
	FComponentSetUpdate DeepCopy() const
	{
		FComponentSetUpdate Copy(ComponentSetId);

		Algo::Transform(Updates, Copy.Updates, [](const ComponentUpdate& Update) {
			return Update.DeepCopy();
		});
		return Copy;
	}

	// Returns the updates currently added to the component-set-update.
	const ComponentUpdateHashSet& GetUpdates() const { return Updates; }

	// Releases ownership of the component-set-update.
	ComponentUpdateHashSet Release() && { return MoveTemp(Updates); }

	// Adds an update to the component-set-update.
	// If there is an existing update for this component, the new update will be merged into it.
	void AddUpdate(ComponentUpdate Update)
	{
		ComponentUpdate* ExistingUpdate = Updates.Find(Update.GetComponentId());
		if (ExistingUpdate)
		{
			ExistingUpdate->Merge(MoveTemp(Update));
		}
		else
		{
			Updates.Add(MoveTemp(Update));
		}
	}

	Worker_ComponentId GetComponentSetId() const { return ComponentSetId; }

private:
	Worker_ComponentSetId ComponentSetId;
	ComponentUpdateHashSet Updates;
};

} // namespace SpatialGDK
