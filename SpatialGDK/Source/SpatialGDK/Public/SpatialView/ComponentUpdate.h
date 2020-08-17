// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "Templates/UniquePtr.h"
#include <improbable/c_schema.h>
#include <improbable/c_worker.h>

namespace SpatialGDK
{
struct ComponentUpdateDeleter
{
	void operator()(Schema_ComponentUpdate* ComponentUpdate) const noexcept
	{
		if (ComponentUpdate != nullptr)
		{
			Schema_DestroyComponentUpdate(ComponentUpdate);
		}
	}
};

using OwningComponentUpdatePtr = TUniquePtr<Schema_ComponentUpdate, ComponentUpdateDeleter>;

// An RAII wrapper for component updates.
class ComponentUpdate
{
public:
	// Creates a new component update.
	explicit ComponentUpdate(Worker_ComponentId Id);
	// Takes ownership of a component update.
	explicit ComponentUpdate(OwningComponentUpdatePtr Update, Worker_ComponentId Id);

	~ComponentUpdate() = default;

	// Moveable, not copyable.
	ComponentUpdate(const ComponentUpdate&) = delete;
	ComponentUpdate(ComponentUpdate&&) = default;
	ComponentUpdate& operator=(const ComponentUpdate&) = delete;
	ComponentUpdate& operator=(ComponentUpdate&&) = default;

	static ComponentUpdate CreateCopy(const Schema_ComponentUpdate* Update, Worker_ComponentId Id);

	// Creates a copy of the component update.
	ComponentUpdate DeepCopy() const;
	// Releases ownership of the component update.
	Schema_ComponentUpdate* Release() &&;

	// Appends the fields and events from other to the update.
	bool Merge(ComponentUpdate Update);

	Schema_Object* GetFields() const;
	Schema_Object* GetEvents() const;

	Schema_ComponentUpdate* GetUnderlying() const;
	Worker_ComponentUpdate GetWorkerComponentUpdate() const;

	Worker_ComponentId GetComponentId() const;

private:
	Worker_ComponentId ComponentId;
	OwningComponentUpdatePtr Update;
};

} // namespace SpatialGDK
