// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "Containers/Array.h"
#include <improbable/c_worker.h>
#include <improbable/c_schema.h>

namespace SpatialGDK
{

struct CompleteUpdateData {
	Schema_ComponentData* Data;
	Schema_Object* Events;
};

struct ComponentChange
{
	explicit ComponentChange(Worker_ComponentId Id, Schema_ComponentData* Data)
		: ComponentId(Id), Type(ADD), Data(Data)
	{
	}

	explicit ComponentChange(Worker_ComponentId Id, Schema_ComponentUpdate* Update)
		: ComponentId(Id), Type(UPDATE), Update(Update)
	{
	}

	explicit ComponentChange(Worker_ComponentId Id, Schema_ComponentData* Data, Schema_Object* Events)
		: ComponentId(Id), Type(COMPLETE_UPDATE), CompleteUpdate{Data, Events}
	{
	}

	explicit ComponentChange(Worker_ComponentId id) : ComponentId(id), Type(REMOVE)
	{
	}

	Worker_ComponentId ComponentId;
	enum { ADD, REMOVE, UPDATE, COMPLETE_UPDATE } Type;
	union
	{
		Schema_ComponentData* Data;
		Schema_ComponentUpdate* Update;
		CompleteUpdateData CompleteUpdate;
	};
};

struct AuthorityChange
{
	AuthorityChange(Worker_ComponentId Id, int Type)
		: ComponentId(Id), Type(static_cast<AuthorityType>(Type))
	{
	}

	Worker_ComponentId ComponentId;
	enum AuthorityType
	{
		AUTHORITY_GAINED = 1,
		AUTHORITY_LOST = 2,
		AUTHORITY_LOST_TEMPORARILY = 3
	} Type;
};

/** Pointer to an array and a size that can be used in a ranged based for.  */
template <typename T>
class ComponentSpan {
public:
	ComponentSpan() : Elements(nullptr), Count(0) {}
	ComponentSpan(const T* Elements, int32 Count) : Elements(Elements), Count(Count) {}

	const T* begin() const
	{
		return Elements;
	}

	const T* end() const
	{
		return Elements + Count;
	}

	int32 Num() const
	{
		return Count;
	}

	const T& operator[](int32 Index) const
	{
		return Elements[Index];
	}

	const T* GetData() const
	{
		return Elements;
	}

private:
	const T* Elements;
	int32 Count;
};

struct EntityDelta
{
	Worker_EntityId EntityId;
	bool bAdded;
	bool bRemoved;
	ComponentSpan<ComponentChange> ComponentsAdded;
	ComponentSpan<ComponentChange> ComponentsRemoved;
	ComponentSpan<ComponentChange> ComponentUpdates;
	ComponentSpan<ComponentChange> ComponentsRefreshed;
	ComponentSpan<AuthorityChange> AuthorityGained;
	ComponentSpan<AuthorityChange> AuthorityLost;
	ComponentSpan<AuthorityChange> AuthorityLostTemporarily;
};

} // namespace SpatialGDK
