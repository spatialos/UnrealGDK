// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "SpatialView/EntityComponentTypes.h"

// TODO(Alex): remove std include
#include <memory>
#include <vector>
#include <type_traits>

namespace SpatialGDK
{

static const Schema_FieldId kEventId = 1;
static const Schema_FieldId kEventIntFieldId = 2;

// TODO(Alex): remove
template <typename T, typename Value>
void PopulateVector(std::vector<T>* vec, Value&& value);
template <typename T, typename Head, typename... Tail>
void PopulateVector(std::vector<T>* vec, Head&& head, Tail&&... tail);

/**
 * Syntactic sugar for creating a vector of move only types.
 * Allows creation of a const vector of move-only objects without defining it in a lambda.
 * All arguments passed must be convertible to VectorType.
 */
template <typename VectorType, typename... Values>
std::vector<VectorType> CreateVector(Values&&... values) {
	std::vector<VectorType> vec;
	PopulateVector(&vec, std::forward<Values>(values)...);
	return vec;
}

template <typename T, typename Value>
void PopulateVector(std::vector<T>* vec, Value&& value) {
	static_assert(std::is_convertible<Value&&, T>::value,
		"All values passed to CreateVector must be convertible To VectorType");
	vec->emplace_back(std::forward<Value>(value));
}

template <typename T, typename Head, typename... Tail>
void PopulateVector(std::vector<T>* vec, Head&& head, Tail&&... tail) {
	static_assert(std::is_convertible<Head&&, T>::value,
		"All values passed to CreateVector must be convertible To VectorType");
	vec->emplace_back(std::forward<Head>(head));
	PopulateVector(vec, std::forward<Tail>(tail)...);
}
// TODO(Alex): end

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
  ComponentUpdate update{id};
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
	const auto lhsBuffer = std::make_unique<std::uint8_t[]>(length);
	const auto rhsBuffer = std::make_unique<std::uint8_t[]>(length);
	Schema_SerializeToBuffer(lhs, lhsBuffer.get(), length);
	Schema_SerializeToBuffer(rhs, rhsBuffer.get(), length);
	return std::memcmp(lhsBuffer.get(), rhsBuffer.get(), length) == 0;
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

}
