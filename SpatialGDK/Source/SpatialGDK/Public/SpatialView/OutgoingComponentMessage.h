// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "SpatialView/ComponentData.h"
#include "SpatialView/ComponentUpdate.h"

namespace SpatialGDK
{
// Represents one of a component addition, update, or removal.
class OutgoingComponentMessage
{
public:
	enum MessageType
	{
		REMOVE,
		ADD,
		UPDATE
	};

	OutgoingComponentMessage(Worker_EntityId EntityId, ComponentData ComponentAdded)
		: EntityId(EntityId)
		, Type(ADD)
	{
		new (&Message.ComponentAdded) ComponentData(MoveTemp(ComponentAdded));
	}

	OutgoingComponentMessage(Worker_EntityId EntityId, ComponentUpdate ComponentUpdated)
		: EntityId(EntityId)
		, Type(UPDATE)
	{
		new (&Message.Update) ComponentUpdate(MoveTemp(ComponentUpdated));
	}

	OutgoingComponentMessage(Worker_EntityId EntityId, Worker_ComponentId ComponentIdRemoved)
		: EntityId(EntityId)
		, Type(REMOVE)
	{
		Message.ComponentRemoved = ComponentIdRemoved;
	}

	~OutgoingComponentMessage() { DeleteCurrent(); }

	// Moveable, not copyable.
	OutgoingComponentMessage(const OutgoingComponentMessage&) = delete;
	OutgoingComponentMessage& operator=(const OutgoingComponentMessage&) = delete;

	OutgoingComponentMessage(OutgoingComponentMessage&& Other) noexcept
		: EntityId(Other.EntityId)
		, Type(REMOVE)
	{
		switch (Other.Type)
		{
		case REMOVE:
			SetComponentRemoved(Other.Message.ComponentRemoved);
			break;
		case ADD:
			SetComponentAdded(MoveTemp(Other.Message.ComponentAdded));
			break;
		case UPDATE:
			SetComponentUpdate(MoveTemp(Other.Message.Update));
			break;
		}
	}

	OutgoingComponentMessage& operator=(OutgoingComponentMessage&& Other) noexcept
	{
		EntityId = Other.EntityId;
		switch (Other.Type)
		{
		case REMOVE:
			SetComponentRemoved(Other.Message.ComponentRemoved);
			break;
		case ADD:
			SetComponentAdded(MoveTemp(Other.Message.ComponentAdded));
			break;
		case UPDATE:
			SetComponentUpdate(MoveTemp(Other.Message.Update));
			break;
		}
		return *this;
	}

	MessageType GetType() const { return Type; }

	const Worker_ComponentId& GetComponentRemoved() const
	{
		check(Type == REMOVE);
		return Message.ComponentRemoved;
	}

	const ComponentData& GetComponentAdded() const
	{
		check(Type == ADD);
		return Message.ComponentAdded;
	}

	const ComponentUpdate& GetComponentUpdate() const
	{
		check(Type == UPDATE);
		return Message.Update;
	}

	ComponentData ReleaseComponentAdded() &&
	{
		check(Type == ADD);
		return MoveTemp(Message.ComponentAdded);
	}

	ComponentUpdate ReleaseComponentUpdate() &&
	{
		check(Type == UPDATE);
		return MoveTemp(Message.Update);
	}

	Worker_EntityId EntityId;

private:
	union ComponentMessage
	{
		ComponentMessage()
			: ComponentRemoved(0)
		{
		}
		// The union destructor shouldn't try to clean up anything; that should be done by the tagged class. So disable the warning.
#pragma warning(disable : 4583)
		~ComponentMessage() {}
#pragma warning(default : 4583)
		Worker_ComponentId ComponentRemoved;
		ComponentData ComponentAdded;
		ComponentUpdate Update;
	};

	void DeleteCurrent()
	{
		switch (Type)
		{
		case REMOVE:
			break;
		case ADD:
			Message.ComponentAdded.~ComponentData();
			break;
		case UPDATE:
			Message.Update.~ComponentUpdate();
			break;
		}
	}

	void SetComponentAdded(ComponentData ComponentAdded)
	{
		DeleteCurrent();
		Type = ADD;
		new (&Message.ComponentAdded) ComponentData(MoveTemp(ComponentAdded));
	}

	void SetComponentUpdate(ComponentUpdate Update)
	{
		DeleteCurrent();
		Type = UPDATE;
		new (&Message.Update) ComponentUpdate(MoveTemp(Update));
	}

	void SetComponentRemoved(Worker_ComponentId IdRemoved)
	{
		DeleteCurrent();
		Type = UPDATE;
		Message.ComponentRemoved = IdRemoved;
	}

	MessageType Type;
	ComponentMessage Message;
};

} // namespace SpatialGDK
