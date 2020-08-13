// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "SpatialView/EntityComponentTypes.h"

#include <algorithm>

namespace SpatialGDK
{
namespace EntityComponentTestUtils
{
const Schema_FieldId EVENT_ID = 1;
const Schema_FieldId EVENT_INT_FIELD_ID = 2;
const Schema_FieldId TEST_DOUBLE_FIELD_ID = 1;
} // namespace EntityComponentTestUtils

inline ComponentData CreateTestComponentData(const Worker_ComponentId Id, const double Value)
{
	ComponentData Data{ Id };
	Schema_Object* Fields = Data.GetFields();
	Schema_AddDouble(Fields, EntityComponentTestUtils::TEST_DOUBLE_FIELD_ID, Value);
	return Data;
}

inline ComponentUpdate CreateTestComponentUpdate(const Worker_ComponentId Id, const double Value)
{
	ComponentUpdate Update{ Id };
	Schema_Object* Fields = Update.GetFields();
	Schema_AddDouble(Fields, EntityComponentTestUtils::TEST_DOUBLE_FIELD_ID, Value);
	return Update;
}

inline void AddTestEvent(ComponentUpdate* Update, int Value)
{
	Schema_Object* events = Update->GetEvents();
	Schema_Object* eventData = Schema_AddObject(events, EntityComponentTestUtils::EVENT_ID);
	Schema_AddInt32(eventData, EntityComponentTestUtils::EVENT_INT_FIELD_ID, Value);
}

inline ComponentUpdate CreateTestComponentEvent(const Worker_ComponentId Id, int Value)
{
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
	const TUniquePtr<uint8_t[]> LhsBuffer = MakeUnique<uint8_t[]>(Length);
	const TUniquePtr<uint8_t[]> RhsBuffer = MakeUnique<uint8_t[]>(Length);
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
	return CompareSchemaObjects(Lhs.GetFields(), Rhs.GetFields()) && CompareSchemaObjects(Lhs.GetEvents(), Rhs.GetEvents());
}

/** Returns true if Lhs and Rhs have the same entity ID, component ID, and state. */
inline bool CompareEntityComponentData(const EntityComponentData& Lhs, const EntityComponentData& Rhs)
{
	if (Lhs.EntityId != Rhs.EntityId)
	{
		return false;
	}
	return CompareComponentData(Lhs.Data, Rhs.Data);
}

/** Returns true if Lhs and Rhs have the same entity ID, component ID, and events. */
inline bool CompareEntityComponentUpdateEvents(const EntityComponentUpdate& Lhs, const EntityComponentUpdate& Rhs)
{
	if (Lhs.EntityId != Rhs.EntityId)
	{
		return false;
	}
	return CompareComponentUpdateEvents(Lhs.Update, Rhs.Update);
}

/** Returns true if Lhs and Rhs have the same entity ID, component ID, state, and events. */
inline bool CompareEntityComponentUpdates(const EntityComponentUpdate& Lhs, const EntityComponentUpdate& Rhs)
{
	if (Lhs.EntityId != Rhs.EntityId)
	{
		return false;
	}
	return CompareComponentUpdates(Lhs.Update, Rhs.Update);
}

/** Returns true if Lhs and Rhs have the same ID, component ID, data, state and events. */
inline bool CompareEntityComponentCompleteUpdates(const EntityComponentCompleteUpdate& Lhs, const EntityComponentCompleteUpdate& Rhs)
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

template <typename T, typename Predicate>
bool AreEquivalent(const TArray<T>& Lhs, const TArray<T>& Rhs, Predicate&& Compare)
{
	return std::is_permutation(Lhs.GetData(), Lhs.GetData() + Lhs.Num(), Rhs.GetData(), std::forward<Predicate>(Compare));
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

} // namespace SpatialGDK
