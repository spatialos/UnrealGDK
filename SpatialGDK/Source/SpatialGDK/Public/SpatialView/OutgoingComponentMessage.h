// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once
#include "SpatialView/ComponentData.h"
#include "SpatialView/ComponentUpdate.h"
#include "SpatialView/EntityComponentId.h"

namespace SpatialGDK
{

// Represents one of a component addition, update, or removal.
// Interally schema data is stored using raw pointers. However the interface exclusively uses explicitly owning objects to denote ownership.
// Not safe to use after being moved from.
class OutgoingComponentMessage
{
public:
	enum MessageType {ADD, UPDATE, REMOVE};

	explicit OutgoingComponentMessage(Worker_EntityId EntityId, ComponentData ComponentAdded)
		: EntityId(EntityId), ComponentId(ComponentAdded.GetComponentId()), ComponentAdded(MoveTemp(ComponentAdded).Release()), Type(ADD)
	{
	}

	OutgoingComponentMessage(Worker_EntityId EntityId, ComponentUpdate ComponentUpdated)
		: EntityId(EntityId), ComponentId(ComponentUpdated.GetComponentId()), ComponentUpdated(MoveTemp(ComponentUpdated).Release()), Type(UPDATE)
	{
	}

	OutgoingComponentMessage(Worker_EntityId EntityId, Worker_ComponentId RemovedComponentId)
		: EntityId(EntityId), ComponentId(RemovedComponentId), Type(REMOVE)
	{
	}

	~OutgoingComponentMessage()
	{
		// As data is stored in owning raw pointers we need to make sure resources are realsed.
		switch (Type)
		{
		case ADD:
			if (ComponentAdded)
			{
				Schema_DestroyComponentData(ComponentAdded);
			}
			break;
		case UPDATE:
			if (ComponentUpdated)
			{
				Schema_DestroyComponentUpdate(ComponentUpdated);
			}
			break;
		case REMOVE:
			break;
		}
	}

	// Moveable, not copyable.
	OutgoingComponentMessage(const OutgoingComponentMessage&) = delete;
	OutgoingComponentMessage(OutgoingComponentMessage&& Other) = default;
	OutgoingComponentMessage& operator=(const OutgoingComponentMessage& Other) = delete;
	OutgoingComponentMessage& operator=(OutgoingComponentMessage&& Other) = default;

	MessageType GetType() const
	{
		return Type;
	}

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
	union
	{
		Schema_ComponentData* ComponentAdded;
		Schema_ComponentUpdate* ComponentUpdated;
	};

	MessageType Type;
};

}  // namespace SpatialGDK
