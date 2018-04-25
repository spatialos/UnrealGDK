#include "WorkerEntityIdList.h"
// ===========
// DO NOT EDIT - this file is automatically regenerated.
// =========== 

UWorkerEntityIdList::UWorkerEntityIdList()
{
}

UWorkerEntityIdList* UWorkerEntityIdList::Init(const worker::List<worker::EntityId>& underlying)
{
	Underlying = worker::List<worker::EntityId>(underlying);
	return this;
}

UWorkerEntityIdList* UWorkerEntityIdList::Add(FEntityId value)
{
	auto underlyingValue = (value).ToSpatialEntityId();
	Underlying.emplace_back(underlyingValue);
	return this;
}

FEntityId UWorkerEntityIdList::Get(int pos)
{
	return FEntityId((Underlying)[pos]);
}

UWorkerEntityIdList* UWorkerEntityIdList::Clear()
{
	Underlying.clear();
	return this;
}

bool UWorkerEntityIdList::IsEmpty()
{
	return Underlying.empty();
}

int UWorkerEntityIdList::Size()
{
	return static_cast<int>(Underlying.size());
}

bool UWorkerEntityIdList::operator==(const worker::List<worker::EntityId>& OtherUnderlying) const
{
	return Underlying == OtherUnderlying;
}

bool UWorkerEntityIdList::operator!=(const worker::List<worker::EntityId>& OtherUnderlying) const
{
	return Underlying != OtherUnderlying;
}

worker::List<worker::EntityId> UWorkerEntityIdList::GetUnderlying()
{
	return Underlying;
}
