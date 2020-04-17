// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "SpatialView/EntityComponentTypes.h"

namespace SpatialGDK
{

static const Schema_FieldId kEventId = 1;
static const Schema_FieldId kEventIntFieldId = 2;

static const Schema_FieldId kTestDoubleFieldId = 1;

inline ComponentData CreateTestComponentData(Worker_ComponentId id, double value)
{
	ComponentData componentData{ id };
	auto* fields = componentData.GetFields();
	Schema_AddDouble(fields, kTestDoubleFieldId, value);
	return componentData;
}

inline ComponentUpdate CreateTestComponentUpdate(Worker_ComponentId id, double value)
{
	ComponentUpdate componentUpdate{ id };
	auto* fields = componentUpdate.GetFields();
	Schema_AddDouble(fields, kTestDoubleFieldId, value);
	return componentUpdate;
}

inline void AddTestEvent(ComponentUpdate* update, int value) {
	auto* events = update->GetEvents();
	auto* eventData = Schema_AddObject(events, kEventId);
	Schema_AddInt32(eventData, kEventIntFieldId, value);
}

inline ComponentUpdate CreateTestComponentEvent(Worker_ComponentId id, int value) {
	ComponentUpdate update{ id };
	AddTestEvent(&update, value);
	return update;
}

/** Returns true if lhs and rhs have the same serialized form. */
inline bool CompareSchemaObjects(const Schema_Object* lhs, const Schema_Object* rhs)
{
	const auto length = Schema_GetWriteBufferLength(lhs);
	if (Schema_GetWriteBufferLength(rhs) != length)
	{
		return false;
	}
	const TUniquePtr<unsigned char[]> lhsBuffer = MakeUnique<unsigned char[]>(length);
	const TUniquePtr<unsigned char[]> rhsBuffer = MakeUnique<unsigned char[]>(length);
	Schema_SerializeToBuffer(lhs, lhsBuffer.Get(), length);
	Schema_SerializeToBuffer(rhs, rhsBuffer.Get(), length);
	return FMemory::Memcmp(lhsBuffer.Get(), rhsBuffer.Get(), length) == 0;
}

/** Returns true if lhs and rhs have the same component ID and state. */
inline bool CompareComponentData(const ComponentData& lhs, const ComponentData& rhs)
{
	if (lhs.GetComponentId() != rhs.GetComponentId())
	{
		return false;
	}
	return CompareSchemaObjects(lhs.GetFields(), rhs.GetFields());
}

/** Returns true if lhs and rhs have the same component ID and events. */
inline bool CompareComponentUpdateEvents(const ComponentUpdate& lhs, const ComponentUpdate& rhs)
{
	if (lhs.GetComponentId() != rhs.GetComponentId())
	{
		return false;
	}
	return CompareSchemaObjects(lhs.GetEvents(), rhs.GetEvents());
}

/** Returns true if lhs and rhs have the same component ID and state. */
inline bool CompareComponentUpdates(const ComponentUpdate& lhs, const ComponentUpdate& rhs)
{
	if (lhs.GetComponentId() != rhs.GetComponentId())
	{
		return false;
	}
	return CompareSchemaObjects(lhs.GetFields(), rhs.GetFields()) &&
		CompareSchemaObjects(lhs.GetEvents(), rhs.GetEvents());
}

/** Returns true if lhs and rhs have the same entity ID, component ID, and state. */
inline bool CompareEntityComponentData(const EntityComponentData& lhs,
	const EntityComponentData& rhs)
{
	if (lhs.EntityId != rhs.EntityId)
	{
		return false;
	}
	return CompareComponentData(lhs.Data, rhs.Data);
}

/** Returns true if lhs and rhs have the same entity ID, component ID, and events. */
inline bool CompareEntityComponentUpdateEvents(const EntityComponentUpdate& lhs,
	const EntityComponentUpdate& rhs)
{
	if (lhs.EntityId != rhs.EntityId)
	{
		return false;
	}
	return CompareComponentUpdateEvents(lhs.Update, rhs.Update);
}

/** Returns true if lhs and rhs have the same entity ID, component ID, state, and events. */
inline bool CompareEntityComponentUpdates(const EntityComponentUpdate& lhs,
	const EntityComponentUpdate& rhs)
{
	if (lhs.EntityId != rhs.EntityId)
	{
		return false;
	}
	return CompareComponentUpdates(lhs.Update, rhs.Update);
}

/** Returns true if lhs and rhs have the same ID, component ID, data, state and events. */
inline bool CompareEntityComponentCompleteUpdates(const EntityComponentCompleteUpdate& lhs,
	const EntityComponentCompleteUpdate& rhs)
{
	if (lhs.EntityId != rhs.EntityId)
	{
		return false;
	}
	return CompareComponentData(lhs.CompleteUpdate, rhs.CompleteUpdate) && CompareComponentUpdateEvents(lhs.Events, rhs.Events);
}

inline bool CompareEntityComponentId(const EntityComponentId& lhs, const EntityComponentId& rhs)
{
	return lhs == rhs;
}

template<typename T>
using ComparisonFunction = bool(*)(const T& lhs, const T& rhs);

template<typename T>
bool AreEquivalent(const TArray<T>& lhs, const TArray<T>& rhs, ComparisonFunction<T> Compare)
{
	if (lhs.Num() != rhs.Num())
	{
		return false;
	}

	for (int i = 0; i < lhs.Num(); i++)
	{
		if (!Compare(lhs[i], rhs[i]))
		{
			return false;
		}
	}

	return true;
}

inline bool AreEquivalent(const TArray<EntityComponentUpdate>& lhs, const TArray<EntityComponentUpdate>& rhs)
{
	return AreEquivalent(lhs, rhs, CompareEntityComponentUpdates);
}

inline bool AreEquivalent(const TArray<EntityComponentCompleteUpdate>& lhs, const TArray<EntityComponentCompleteUpdate>& rhs)
{
	return AreEquivalent(lhs, rhs, CompareEntityComponentCompleteUpdates);
}

inline bool AreEquivalent(const TArray<EntityComponentData>& lhs, const TArray<EntityComponentData>& rhs)
{
	return AreEquivalent(lhs, rhs, CompareEntityComponentData);
}

inline bool AreEquivalent(const TArray<EntityComponentId>& lhs, const TArray<EntityComponentId>& rhs)
{
	return AreEquivalent(lhs, rhs, CompareEntityComponentId);
}

}
