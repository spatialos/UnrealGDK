// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "Containers/UnrealString.h"
#include "Schema/Component.h"
#include "SpatialConstants.h"
#include "Utils/SchemaUtils.h"

#include <WorkerSDK/improbable/c_schema.h>
#include <WorkerSDK/improbable/c_worker.h>

namespace SpatialGDK
{
struct DebugComponent
{
	static const Worker_ComponentId ComponentId = SpatialConstants::GDK_DEBUG_COMPONENT_ID;

	DebugComponent() {}

	DebugComponent(Schema_ComponentData* Data)
	{
		Schema_Object* ComponentObject = Schema_GetComponentDataFields(Data);
		ReadFromSchema(ComponentObject);
	}

	Worker_ComponentData CreateDebugComponent() const
	{
		Worker_ComponentData Data = {};
		Data.component_id = ComponentId;
		Data.schema_type = Schema_CreateComponentData();
		Schema_Object* ComponentObject = Schema_GetComponentDataFields(Data.schema_type);
		WriteToSchema(ComponentObject);

		return Data;
	}

	Worker_ComponentUpdate CreateDebugComponentUpdate() const
	{
		Worker_ComponentUpdate Data = {};
		Data.component_id = ComponentId;
		Data.schema_type = Schema_CreateComponentUpdate();
		Schema_Object* ComponentObject = Schema_GetComponentUpdateFields(Data.schema_type);
		WriteToSchema(ComponentObject);
		if (ActorTags.Num() == 0)
		{
			Schema_AddComponentUpdateClearedField(Data.schema_type, 2);
		}

		return Data;
	}

	void ApplyComponentUpdate(Schema_ComponentUpdate* Update)
	{
		Schema_Object* ComponentObject = Schema_GetComponentUpdateFields(Update);
		ReadFromSchema(ComponentObject);
	}

	TSchemaOption<VirtualWorkerId> DelegatedWorkerId;
	TSet<FName> ActorTags;

private:
	void ReadFromSchema(Schema_Object* ComponentObject)
	{
		if (Schema_GetObjectCount(ComponentObject, 1) == 1)
		{
			DelegatedWorkerId = Schema_GetInt32(ComponentObject, 1);
		}
		else
		{
			DelegatedWorkerId = TSchemaOption<VirtualWorkerId>();
		}

		const uint32 TagsCount = Schema_GetObjectCount(ComponentObject, 2);
		ActorTags.Empty();

		for (uint32 i = 0; i < TagsCount; ++i)
		{
			FString TagString = IndexStringFromSchema(ComponentObject, 2, i);
			ActorTags.Add(FName(*TagString));
		}
	}

	void WriteToSchema(Schema_Object* ComponentObject) const
	{
		if (DelegatedWorkerId.IsSet())
		{
			Schema_AddInt32(ComponentObject, 1, DelegatedWorkerId.GetValue());
		}
		for (const auto& Tag : ActorTags)
		{
			AddStringToSchema(ComponentObject, 2, Tag.ToString());
		}
	}
};

} // namespace SpatialGDK
