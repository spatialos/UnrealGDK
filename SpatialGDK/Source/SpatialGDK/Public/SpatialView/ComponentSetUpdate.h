// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "SpatialView/ComponentUpdate.h"

#include "Algo/Transform.h"

namespace SpatialGDK
{
// An owning wrapper for a component-set-update.
class SPATIALGDK_API FComponentSetUpdate
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
	const TArray<ComponentUpdate>& GetUpdates() const { return Updates; }

	// Releases ownership of the component-set-update.
	TArray<ComponentUpdate> Release() && { return MoveTemp(Updates); }

	// Adds an update to the component-set-update.
	void AddUpdate(ComponentUpdate Update) { Updates.Add(MoveTemp(Update)); }

	Worker_ComponentId GetComponentSetId() const;

private:
	Worker_ComponentSetId ComponentSetId;
	TArray<ComponentUpdate> Updates;
};

} // namespace SpatialGDK
