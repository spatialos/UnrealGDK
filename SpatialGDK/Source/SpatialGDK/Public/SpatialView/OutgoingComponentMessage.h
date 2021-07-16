// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "SpatialView/ComponentData.h"
#include "SpatialView/ComponentUpdate.h"

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

	explicit OutgoingComponentMessage(Worker_EntityId EntityId, ComponentData ComponentAdded, const FSpatialGDKSpanId& SpanId)
		: EntityId(EntityId)
		, SpanId(SpanId)
		, Type(ADD)
	{
		new (&Message.Data) ComponentData(MoveTemp(ComponentAdded));
	}

	explicit OutgoingComponentMessage(Worker_EntityId EntityId, ComponentUpdate ComponentUpdated, const FSpatialGDKSpanId& SpanId)
		: EntityId(EntityId)
		, SpanId(SpanId)
		, Type(UPDATE)
	{
		new (&Message.Update) ComponentUpdate(MoveTemp(ComponentUpdated));
	}

	explicit OutgoingComponentMessage(Worker_EntityId EntityId, Worker_ComponentId RemovedComponentId, const FSpatialGDKSpanId& SpanId)
		: EntityId(EntityId)
		, SpanId(SpanId)
		, Type(REMOVE)
	{
		Message.RemovedId = RemovedComponentId;
	}

	~OutgoingComponentMessage()
	{
		DeleteCurrent();
	}

	// Moveable, not copyable.
	OutgoingComponentMessage(const OutgoingComponentMessage&) = delete;
	OutgoingComponentMessage& operator=(const OutgoingComponentMessage& Other) = delete;

	OutgoingComponentMessage(OutgoingComponentMessage&& Other) noexcept
		: EntityId(Other.EntityId)
		, SpanId(Other.SpanId)
		, Type(Other.Type)
	{
		check(Other.Type != NONE);
		switch (Other.Type)
		{
		case ADD:
			new (&Message.Data) ComponentData(MoveTemp(Other.Message.Data));
			break;
		case UPDATE:
			new (&Message.Update) ComponentUpdate(MoveTemp(Other.Message.Update));
			break;
		case REMOVE:
			Message.RemovedId = Other.Message.RemovedId;
			break;
		default:
			checkNoEntry();
		}
	}

	OutgoingComponentMessage& operator=(OutgoingComponentMessage&& Other) noexcept
	{
		check(Other.Type != NONE);
		DeleteCurrent();

		EntityId = Other.EntityId;
		SpanId = Other.SpanId;
		Type = Other.Type;

		switch (Other.Type)
		{
		case ADD:
			new (&Message.Data) ComponentData(MoveTemp(Other.Message.Data));
			break;
		case UPDATE:
			new (&Message.Update) ComponentUpdate(MoveTemp(Other.Message.Update));
			break;
		case REMOVE:
			Message.RemovedId = Other.Message.RemovedId;
			break;
		default:
			checkNoEntry();
		}

		Other.Type = NONE;

		return *this;
	}

	MessageType GetType() const { return Type; }

	Worker_ComponentId GetRemovedId() const
	{
		check(Type == ADD);
		return Message.RemovedId;
	}

	ComponentData ReleaseComponentAdded() &&
	{
		check(Type == ADD);
		return MoveTemp(Message.Data);
	}

	ComponentUpdate ReleaseComponentUpdate() &&
	{
		check(Type == UPDATE);
		return MoveTemp(Message.Update);
	}

	Worker_EntityId EntityId;
	FSpatialGDKSpanId SpanId;

private:
	void DeleteCurrent()
	{
		switch (Type)
		{
		case NONE:
			break;
		case ADD:
			Message.Data.~ComponentData();
			break;
		case UPDATE:
			Message.Update.~ComponentUpdate();
			break;
		case REMOVE:
			break;
		}
	}

	union ComponentMessage
	{
		ComponentMessage() : RemovedId(0) {}
#if defined(_MSC_VER)
#pragma warning(disable : 4583)
#endif // defined(_MSC_VER)
		~ComponentMessage() {}
#if defined(_MSC_VER)
#pragma warning(default : 4583)
#endif // defined(_MSC_VER)
		ComponentData Data;
		ComponentUpdate Update;
		Worker_ComponentId RemovedId;
	} Message;

	MessageType Type;
};

} // namespace SpatialGDK
