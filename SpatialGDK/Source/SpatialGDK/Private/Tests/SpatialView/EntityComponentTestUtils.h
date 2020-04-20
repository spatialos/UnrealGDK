// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "SpatialView/EntityComponentTypes.h"

namespace SpatialGDK
{

static const Schema_FieldId kEventId = 1;
static const Schema_FieldId kEventIntFieldId = 2;

static const Schema_FieldId kTestDoubleFieldId = 1;

inline ComponentData CreateTestComponentData(Worker_ComponentId Id, double Value)
{
	ComponentData Data{ Id };
	Schema_Object* Fields = Data.GetFields();
	Schema_AddDouble(Fields, kTestDoubleFieldId, Value);
	return Data;
}

inline ComponentUpdate CreateTestComponentUpdate(Worker_ComponentId Id, double Value)
{
	ComponentUpdate Update{ Id };
	Schema_Object* Fields = Update.GetFields();
	Schema_AddDouble(Fields, kTestDoubleFieldId, Value);
	return Update;
}

inline void AddTestEvent(ComponentUpdate* Update, int Value) {
	Schema_Object* events = Update->GetEvents();
	Schema_Object* eventData = Schema_AddObject(events, kEventId);
	Schema_AddInt32(eventData, kEventIntFieldId, Value);
}

inline ComponentUpdate CreateTestComponentEvent(Worker_ComponentId Id, int Value) {
	ComponentUpdate Update{ Id };
	AddTestEvent(&Update, Value);
	return Update;
}

/** Returns true if Lhs and Rhs have the same serialized form. */
inline bool CompareSchemaObjects(const Schema_Object* Lhs, const Schema_Object* Rhs)
{
	const auto Length = Schema_GetWriteBufferLength(Lhs);
	if (Schema_GetWriteBufferLength(Rhs) != Length)
	{
		return false;
	}
	const TUniquePtr<unsigned char[]> LhsBuffer = MakeUnique<unsigned char[]>(Length);
	const TUniquePtr<unsigned char[]> RhsBuffer = MakeUnique<unsigned char[]>(Length);
	Schema_SerializeToBuffer(Lhs, LhsBuffer.Get(), Length);
	Schema_SerializeToBuffer(Rhs, RhsBuffer.Get(), Length);
	return FMemory::Memcmp(LhsBuffer.Get(), RhsBuffer.Get(), Length) == 0;
}

/** Returns true if Lhs and Rhs have the same component ID and state. */
inline bool CompareComponentData(const ComponentData& Lhs, const ComponentData& Rhs)
{
	if (Lhs.GetComponentId() != Rhs.GetComponentId())
	{
		return false;
	}
	return CompareSchemaObjects(Lhs.GetFields(), Rhs.GetFields());
}

/** Returns true if Lhs and Rhs have the same component ID and events. */
inline bool CompareComponentUpdateEvents(const ComponentUpdate& Lhs, const ComponentUpdate& Rhs)
{
	if (Lhs.GetComponentId() != Rhs.GetComponentId())
	{
		return false;
	}
	return CompareSchemaObjects(Lhs.GetEvents(), Rhs.GetEvents());
}

/** Returns true if Lhs and Rhs have the same component ID and state. */
inline bool CompareComponentUpdates(const ComponentUpdate& Lhs, const ComponentUpdate& Rhs)
{
	if (Lhs.GetComponentId() != Rhs.GetComponentId())
	{
		return false;
	}
	return CompareSchemaObjects(Lhs.GetFields(), Rhs.GetFields()) &&
		CompareSchemaObjects(Lhs.GetEvents(), Rhs.GetEvents());
}

/** Returns true if Lhs and Rhs have the same entity ID, component ID, and state. */
inline bool CompareEntityComponentData(const EntityComponentData& Lhs,
	const EntityComponentData& Rhs)
{
	if (Lhs.EntityId != Rhs.EntityId)
	{
		return false;
	}
	return CompareComponentData(Lhs.Data, Rhs.Data);
}

/** Returns true if Lhs and Rhs have the same entity ID, component ID, and events. */
inline bool CompareEntityComponentUpdateEvents(const EntityComponentUpdate& Lhs,
	const EntityComponentUpdate& Rhs)
{
	if (Lhs.EntityId != Rhs.EntityId)
	{
		return false;
	}
	return CompareComponentUpdateEvents(Lhs.Update, Rhs.Update);
}

/** Returns true if Lhs and Rhs have the same entity ID, component ID, state, and events. */
inline bool CompareEntityComponentUpdates(const EntityComponentUpdate& Lhs,
	const EntityComponentUpdate& Rhs)
{
	if (Lhs.EntityId != Rhs.EntityId)
	{
		return false;
	}
	return CompareComponentUpdates(Lhs.Update, Rhs.Update);
}

/** Returns true if Lhs and Rhs have the same ID, component ID, data, state and events. */
inline bool CompareEntityComponentCompleteUpdates(const EntityComponentCompleteUpdate& Lhs,
	const EntityComponentCompleteUpdate& Rhs)
{
	if (Lhs.EntityId != Rhs.EntityId)
	{
		return false;
	}
	return CompareComponentData(Lhs.CompleteUpdate, Rhs.CompleteUpdate) && CompareComponentUpdateEvents(Lhs.Events, Rhs.Events);
}

inline bool CompareEntityComponentId(const EntityComponentId& Lhs, const EntityComponentId& Rhs)
{
	return Lhs == Rhs;
}

template<typename T>
using ComparisonFunction = bool(*)(const T& Lhs, const T& Rhs);

template<typename T>
bool AreEquivalent(const TArray<T>& Lhs, const TArray<T>& Rhs, ComparisonFunction<T> Compare)
{
	if (Lhs.Num() != Rhs.Num())
	{
		return false;
	}

	for (int i = 0; i < Lhs.Num(); i++)
	{
		if (!Compare(Lhs[i], Rhs[i]))
		{
			return false;
		}
	}

	return true;
}

inline bool AreEquivalent(const TArray<EntityComponentUpdate>& Lhs, const TArray<EntityComponentUpdate>& Rhs)
{
	return AreEquivalent(Lhs, Rhs, CompareEntityComponentUpdates);
}

inline bool AreEquivalent(const TArray<EntityComponentCompleteUpdate>& Lhs, const TArray<EntityComponentCompleteUpdate>& Rhs)
{
	return AreEquivalent(Lhs, Rhs, CompareEntityComponentCompleteUpdates);
}

inline bool AreEquivalent(const TArray<EntityComponentData>& Lhs, const TArray<EntityComponentData>& Rhs)
{
	return AreEquivalent(Lhs, Rhs, CompareEntityComponentData);
}

inline bool AreEquivalent(const TArray<EntityComponentId>& Lhs, const TArray<EntityComponentId>& Rhs)
{
	return AreEquivalent(Lhs, Rhs, CompareEntityComponentId);
}

}
