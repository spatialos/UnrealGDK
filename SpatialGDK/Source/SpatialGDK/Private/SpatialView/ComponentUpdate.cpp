// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "SpatialView/ComponentUpdate.h"

namespace SpatialGDK
{
ComponentUpdate::ComponentUpdate(Worker_ComponentId Id)
	: ComponentId(Id)
	, Update(Schema_CreateComponentUpdate())
{
}

ComponentUpdate::ComponentUpdate(OwningComponentUpdatePtr Update, Worker_ComponentId Id)
	: ComponentId(Id)
	, Update(MoveTemp(Update))
{
}

ComponentUpdate ComponentUpdate::CreateCopy(const Schema_ComponentUpdate* Update, Worker_ComponentId Id)
{
	return ComponentUpdate(OwningComponentUpdatePtr(Schema_CopyComponentUpdate(Update)), Id);
}

ComponentUpdate ComponentUpdate::DeepCopy() const
{
	check(Update.IsValid());
	return CreateCopy(Update.Get(), ComponentId);
}

Schema_ComponentUpdate* ComponentUpdate::Release() &&
{
	check(Update.IsValid());
	return Update.Release();
}

bool ComponentUpdate::Merge(ComponentUpdate Other)
{
	check(Other.GetComponentId() == GetComponentId());
	check(Other.Update.IsValid());
	// Calling GetUnderlying instead of Release
	// as we still need to manually destroy Other.
	return Schema_MergeComponentUpdateIntoUpdate(Other.GetUnderlying(), Update.Get()) != 0;
}

Schema_Object* ComponentUpdate::GetFields() const
{
	check(Update.IsValid());
	return Schema_GetComponentUpdateFields(Update.Get());
}

Schema_Object* ComponentUpdate::GetEvents() const
{
	check(Update.IsValid());
	return Schema_GetComponentUpdateEvents(Update.Get());
}

Schema_ComponentUpdate* ComponentUpdate::GetUnderlying() const
{
	check(Update.IsValid());
	return Update.Get();
}

Worker_ComponentUpdate ComponentUpdate::GetWorkerComponentUpdate() const
{
	check(Update.IsValid());
	return { nullptr, ComponentId, Update.Get(), nullptr };
}

Worker_ComponentId ComponentUpdate::GetComponentId() const
{
	return ComponentId;
}

} // namespace SpatialGDK
