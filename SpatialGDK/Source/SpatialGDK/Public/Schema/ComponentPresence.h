// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "Schema/Component.h"
#include "SpatialCommonTypes.h"
#include "SpatialConstants.h"
#include "Utils/SchemaUtils.h"

#include "Containers/Array.h"

#include <WorkerSDK/improbable/c_schema.h>

namespace SpatialGDK
{

struct ComponentPresence : Component
{
	static const Worker_ComponentId ComponentId = SpatialConstants::COMPONENT_PRESENCE_COMPONENT_ID;

	ComponentPresence() = default;

	ComponentPresence(TArray<Worker_ComponentId> InActorComponentList)
		: ActorComponentList(InActorComponentList)
	{}

	ComponentPresence(const Worker_ComponentData& Data)
	{
		Schema_Object* ComponentObject = Schema_GetComponentDataFields(Data.schema_type);
		CopyListFromComponentObject(ComponentObject);
	}

	Worker_ComponentData CreateComponentPresenceData()
	{
		return CreateComponentPresenceData(ActorComponentList);
	}

	static Worker_ComponentData CreateComponentPresenceData(const TArray<Worker_ComponentId>& ActorComponentList)
	{
		Worker_ComponentData Data = {};
		Data.component_id = ComponentId;
		Data.schema_type = Schema_CreateComponentData();
		Schema_Object* ComponentObject = Schema_GetComponentDataFields(Data.schema_type);

		for (int32 i = 0; i < ActorComponentList.Num(); ++i)
		{
			Schema_AddUint32(ComponentObject, SpatialConstants::COMPONENT_PRESENCE_COMPONENT_LIST_ID, ActorComponentList[i]);
		}

		return Data;
	}

	Worker_ComponentUpdate CreateComponentPresenceUpdate()
	{
		return CreateComponentPresenceUpdate(ActorComponentList);
	}

	static Worker_ComponentUpdate CreateComponentPresenceUpdate(const TArray<Worker_ComponentId>& ActorComponentList)
	{
		Worker_ComponentUpdate Update = {};
		Update.component_id = ComponentId;
		Update.schema_type = Schema_CreateComponentUpdate();
		Schema_Object* ComponentObject = Schema_GetComponentUpdateFields(Update.schema_type);

		Schema_AddUint32List(ComponentObject, SpatialConstants::COMPONENT_PRESENCE_COMPONENT_LIST_ID,
			ActorComponentList.GetData(), ActorComponentList.Num());

		return Update;
	}

	void ApplyComponentUpdate(const Worker_ComponentUpdate& Update)
	{
		Schema_Object* ComponentObject = Schema_GetComponentUpdateFields(Update.schema_type);
		CopyListFromComponentObject(ComponentObject);
	}

	void CopyListFromComponentObject(Schema_Object* ComponentObject)
	{
		ActorComponentList.SetNum(Schema_GetUint32Count(ComponentObject, SpatialConstants::COMPONENT_PRESENCE_COMPONENT_LIST_ID), true);
		Schema_GetUint32List(ComponentObject, SpatialConstants::COMPONENT_PRESENCE_COMPONENT_LIST_ID, ActorComponentList.GetData());
	}

	// List of component IDs on a SpatialOS entity representing an Actor that a worker simulating
	// should have authority over.
	TArray<Worker_ComponentId> ActorComponentList;
};

} // namespace SpatialGDK

