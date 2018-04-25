#include "WorkerEntityIdToTestTestType1Map.h"
// ===========
// DO NOT EDIT - this file is automatically regenerated.
// =========== 

UWorkerEntityIdToTestTestType1Map::UWorkerEntityIdToTestTestType1Map()
{
}

UWorkerEntityIdToTestTestType1Map* UWorkerEntityIdToTestTestType1Map::Init(const worker::Map<worker::EntityId, test::TestType1>& underlying)
{
	Underlying = worker::Map<worker::EntityId, test::TestType1>(underlying);
	return this;
}

UWorkerEntityIdToTestTestType1Map* UWorkerEntityIdToTestTestType1Map::Emplace(FEntityId key, UTestType1* value)
{
	worker::EntityId underlyingKey = (key).ToSpatialEntityId();
	auto underlyingValue = value->GetUnderlying();
	Underlying.emplace(underlyingKey, underlyingValue);
	return this;
}

UWorkerEntityIdToTestTestType1Map* UWorkerEntityIdToTestTestType1Map::Remove(FEntityId key)
{
	worker::EntityId underlyingKey = (key).ToSpatialEntityId();
	Underlying.erase(underlyingKey);
	return this;
}

bool UWorkerEntityIdToTestTestType1Map::Contains(FEntityId key)
{
	worker::EntityId underlyingKey = (key).ToSpatialEntityId();
	return Underlying.find(underlyingKey) != Underlying.end();
}

UTestType1* UWorkerEntityIdToTestTestType1Map::Get(FEntityId key)
{
	worker::EntityId underlyingKey = (key).ToSpatialEntityId();
	auto iterator = Underlying.find(underlyingKey);
	return NewObject<UTestType1>()->Init(iterator->second);
}

UWorkerEntityIdToTestTestType1Map* UWorkerEntityIdToTestTestType1Map::Clear()
{
	Underlying.clear();
	return this;
}

bool UWorkerEntityIdToTestTestType1Map::IsEmpty()
{
	return Underlying.empty();
}

bool UWorkerEntityIdToTestTestType1Map::operator==(const worker::Map<worker::EntityId, test::TestType1>& OtherUnderlying) const
{
	return Underlying == OtherUnderlying;
}

bool UWorkerEntityIdToTestTestType1Map::operator!=(const worker::Map<worker::EntityId, test::TestType1>& OtherUnderlying) const
{
	return Underlying != OtherUnderlying;
}

worker::Map<worker::EntityId, test::TestType1> UWorkerEntityIdToTestTestType1Map::GetUnderlying()
{
	return Underlying;
}