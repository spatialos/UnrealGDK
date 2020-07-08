// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "Containers/Array.h"
#include <improbable/c_worker.h>
#include <improbable/c_schema.h>

namespace SpatialGDK
{

struct CompleteUpdateData {
	Schema_ComponentData* data;
	Schema_Object* events;
};

struct ComponentChange
{
	explicit ComponentChange(Worker_ComponentId id, Schema_ComponentData* data)
		: component_id(id), type(kAdd), data(data)
	{
	}

	explicit ComponentChange(Worker_ComponentId id, Schema_ComponentUpdate* update)
		: component_id(id), type(kUpdate), update(update)
	{
	}

	explicit ComponentChange(Worker_ComponentId id, Schema_ComponentData* data, Schema_Object* events)
		: component_id(id), type(kCompleteUpdate), complete_update{data, events}
	{
	}

	explicit ComponentChange(Worker_ComponentId id) : component_id(id), type(kRemove)
	{
	}

	Worker_ComponentId component_id;
	enum { kAdd, kRemove, kUpdate, kCompleteUpdate } type;
	union
	{
		Schema_ComponentData* data;
		Schema_ComponentUpdate* update;
		CompleteUpdateData complete_update;
	};
};

struct AuthorityChange
{
	AuthorityChange(Worker_ComponentId id, int type)
		: component_id(id), type(static_cast<AuthorityType>(type))
	{
	}

	Worker_ComponentId component_id;
	enum AuthorityType
	{
		kAuthorityGained = 1,
		kAuthorityLost = 2,
		kAuthorityLostTemporarily = 3
	} type;
};

/** Pointer to an array and a size that can be used in a ranged based for.  */
template <typename T>
class ComponentSpan {
public:
	ComponentSpan() : elements(nullptr), count(0) {}
	ComponentSpan(const T* elements, int32 count) : elements(elements), count(count) {}

	const T* begin() const
	{
		return elements;
	}

	const T* end() const
	{
		return elements + count;
	}

	size_t size() const
	{
		return count;
	}

	const T& operator[](size_t i) const
	{
		return elements[i];
	}

	const T* data() const
	{
		return elements;
	}

private:
	const T* elements;
	int32 count;
};

struct EntityDelta
{
	Worker_EntityId entity_id;
	bool added;
	bool removed;
	ComponentSpan<ComponentChange> component_changes;
	ComponentSpan<AuthorityChange> authority_changes;
};

} // namespace SpatialGDK
