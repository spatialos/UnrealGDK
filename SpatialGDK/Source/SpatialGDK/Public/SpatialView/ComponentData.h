// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "Templates/UniquePtr.h"
#include <improbable/c_schema.h>
#include <improbable/c_worker.h>

namespace SpatialGDK
{
class ComponentUpdate;

struct ComponentDataDeleter
{
	void operator()(Schema_ComponentData* ComponentData) const noexcept
	{
		if (ComponentData != nullptr)
		{
			Schema_DestroyComponentData(ComponentData);
		}
	}
};

using OwningComponentDataPtr = TUniquePtr<Schema_ComponentData, ComponentDataDeleter>;

// An RAII wrapper for component data.
class ComponentData
{
public:
	// Creates a new component data.
	explicit ComponentData(Worker_ComponentId Id);
	// Takes ownership of component data.
	explicit ComponentData(OwningComponentDataPtr Data, Worker_ComponentId Id);

	~ComponentData() = default;

	// Moveable, not copyable.
	ComponentData(const ComponentData&) = delete;
	ComponentData(ComponentData&&) = default;
	ComponentData& operator=(const ComponentData&) = delete;
	ComponentData& operator=(ComponentData&&) = default;

	static ComponentData CreateCopy(const Schema_ComponentData* Data, Worker_ComponentId Id);

	// Creates a copy of the component data.
	ComponentData DeepCopy() const;
	// Releases ownership of the component data.
	Schema_ComponentData* Release() &&;

	// Appends the fields from the provided update.
	// Returns true if the update was successfully applied and false otherwise.
	// This will cause the size of the component data to increase.
	// To resize use DeepCopy to create a new data object with the serialized size of the data.
	bool ApplyUpdate(const ComponentUpdate& Update);

	Schema_Object* GetFields() const;

	Schema_ComponentData* GetUnderlying() const;
	Worker_ComponentData GetWorkerComponentData() const;

	Worker_ComponentId GetComponentId() const;

private:
	Worker_ComponentId ComponentId;
	OwningComponentDataPtr Data;
};

} // namespace SpatialGDK
