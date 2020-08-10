// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "SpatialView/ComponentData.h"
#include "SpatialView/ComponentUpdate.h"
#include "SpatialView/EntityComponentId.h"

namespace SpatialGDK
{
// Represents one of a component addition, update, or removal.
// Internally schema data is stored using raw pointers. However the interface exclusively uses explicitly owning objects to denote
// ownership.
class OutgoingComponentMessage
{
public:
	enum MessageType
	{
		NONE,
		ADD,
		UPDATE,
		REMOVE
	};

	explicit OutgoingComponentMessage()
		: EntityId(0)
		, ComponentId(0)
		, Type(NONE)
	{
	}

	explicit OutgoingComponentMessage(Worker_EntityId EntityId, ComponentData ComponentAdded)
		: EntityId(EntityId)
		, ComponentId(ComponentAdded.GetComponentId())
		, ComponentAdded(MoveTemp(ComponentAdded).Release())
		, Type(ADD)
	{
	}

	explicit OutgoingComponentMessage(Worker_EntityId EntityId, ComponentUpdate ComponentUpdated)
		: EntityId(EntityId)
		, ComponentId(ComponentUpdated.GetComponentId())
		, ComponentUpdated(MoveTemp(ComponentUpdated).Release())
		, Type(UPDATE)
	{
	}

	explicit OutgoingComponentMessage(Worker_EntityId EntityId, Worker_ComponentId RemovedComponentId)
		: EntityId(EntityId)
		, ComponentId(RemovedComponentId)
		, Type(REMOVE)
	{
	}

	~OutgoingComponentMessage()
	{
		// As data is stored in owning raw pointers we need to make sure resources are released.
		DeleteSchemaObjects();
	}

	// Moveable, not copyable.
	OutgoingComponentMessage(const OutgoingComponentMessage&) = delete;
	OutgoingComponentMessage& operator=(const OutgoingComponentMessage& Other) = delete;

	OutgoingComponentMessage(OutgoingComponentMessage&& Other) noexcept
		: EntityId(Other.EntityId)
		, ComponentId(Other.ComponentId)
		, Type(Other.Type)
	{
		switch (Other.Type)
		{
		case NONE:
			break;
		case ADD:
			ComponentAdded = Other.ComponentAdded;
			Other.ComponentAdded = nullptr;
			break;
		case UPDATE:
			ComponentUpdated = Other.ComponentUpdated;
			Other.ComponentUpdated = nullptr;
			break;
		case REMOVE:
			break;
		}
		Other.Type = NONE;
	}

	OutgoingComponentMessage& operator=(OutgoingComponentMessage&& Other) noexcept
	{
		EntityId = Other.EntityId;
		ComponentId = Other.ComponentId;

		// As data is stored in owning raw pointers we need to make sure resources are released.
		DeleteSchemaObjects();
		switch (Other.Type)
		{
		case NONE:
			break;
		case ADD:
			ComponentAdded = Other.ComponentAdded;
			Other.ComponentAdded = nullptr;
			break;
		case UPDATE:
			ComponentUpdated = Other.ComponentUpdated;
			Other.ComponentUpdated = nullptr;
			break;
		case REMOVE:
			break;
		}

		Other.Type = NONE;

		return *this;
	}

	MessageType GetType() const { return Type; }

	ComponentData ReleaseComponentAdded() &&
	{
		check(Type == ADD);
		ComponentData Data(OwningComponentDataPtr(ComponentAdded), ComponentId);
		ComponentAdded = nullptr;
		return Data;
	}

	ComponentUpdate ReleaseComponentUpdate() &&
	{
		check(Type == UPDATE);
		ComponentUpdate Update(OwningComponentUpdatePtr(ComponentUpdated), ComponentId);
		ComponentUpdated = nullptr;
		return Update;
	}

	Worker_EntityId EntityId;
	Worker_ComponentId ComponentId;

private:
	void DeleteSchemaObjects()
	{
		switch (Type)
		{
		case NONE:
			break;
		case ADD:
			Schema_DestroyComponentData(ComponentAdded);
			break;
		case UPDATE:
			Schema_DestroyComponentUpdate(ComponentUpdated);
			break;
		case REMOVE:
			break;
		}
	}

	union
	{
		Schema_ComponentData* ComponentAdded;
		Schema_ComponentUpdate* ComponentUpdated;
	};

	MessageType Type;
};

} // namespace SpatialGDK
