// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "SpatialView/ComponentData.h"
#include "SpatialView/ComponentUpdate.h"

namespace SpatialGDK
{

void ComponentDataDeleter::operator()(Schema_ComponentData* ComponentData) const noexcept
{
	if (ComponentData == nullptr)
	{
		return;
	}

	Schema_DestroyComponentData(ComponentData);
}

ComponentData::ComponentData(Worker_ComponentId Id)
	: ComponentId(Id)
	, Data(Schema_CreateComponentData())
{
}

ComponentData::ComponentData(OwningComponentDataPtr Data, Worker_ComponentId Id)
	: ComponentId(Id)
	, Data(MoveTemp(Data))
{
}

ComponentData ComponentData::CreateCopy(const Schema_ComponentData* Data, Worker_ComponentId Id)
{
	return ComponentData(OwningComponentDataPtr(Schema_CopyComponentData(Data)), Id);
}

ComponentData ComponentData::DeepCopy() const
{
	return CreateCopy(Data.Get(), ComponentId);
}

Schema_ComponentData* ComponentData::Release() &&
{
	return Data.Release();
}

bool ComponentData::ApplyUpdate(const ComponentUpdate& Update)
{
	check(Update.GetComponentId() == GetComponentId());
	check(Update.GetUnderlying() != nullptr);

	return Schema_ApplyComponentUpdateToData(Update.GetUnderlying(), Data.Get());
}

Schema_Object* ComponentData::GetFields() const
{
	check(Data.IsValid());
	return Schema_GetComponentDataFields(Data.Get());
}

Schema_ComponentData* ComponentData::GetUnderlying() const
{
	check(Data.IsValid());
	return Data.Get();
}

Worker_ComponentId ComponentData::GetComponentId() const
{
	return ComponentId;
}

} // namespace SpatialGDK
