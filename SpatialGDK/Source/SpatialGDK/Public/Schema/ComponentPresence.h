// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "Schema/Component.h"
#include "SpatialCommonTypes.h"
#include "SpatialConstants.h"
#include "Utils/SchemaOption.h"
#include "Utils/SchemaUtils.h"

#include "Containers/Array.h"

#include <WorkerSDK/improbable/c_schema.h>

namespace SpatialGDK
{

struct ComponentPresence : Component
{
	static const Worker_ComponentId ComponentId = SpatialConstants::COMPONENT_PRESENCE_COMPONENT_ID;

	ComponentPresence() = default;

	ComponentPresence(TArray<Worker_ComponentId>&& InActorComponentList)
		: ComponentList(MoveTemp(InActorComponentList))
		, ClientWorkerId(InClientWorkerId)
	{}

	ComponentPresence(const Worker_ComponentData& Data)
	{
		Schema_Object* ComponentObject = Schema_GetComponentDataFields(Data.schema_type);
		CopyListFromComponentObject(ComponentObject);
		if (Schema_GetBytesCount(ComponentObject, SpatialConstants::COMPONENT_PRESENCE_CLIENT_WORKER_ID) == 1)
		{
			ClientWorkerId = GetObjectRefFromSchema(ComponentObject, 1);
		}
	}

	Worker_ComponentData CreateComponentPresenceData()
	{
		return CreateComponentPresenceData(ComponentList, ClientWorkerId);
	}

	static Worker_ComponentData CreateComponentPresenceData(const TArray<Worker_ComponentId>& ComponentList, const TSchemaOption<PhysicalWorkerName>& ClientWorkerId)
	{
		Worker_ComponentData Data = {};
		Data.component_id = ComponentId;
		Data.schema_type = Schema_CreateComponentData();
		Schema_Object* ComponentObject = Schema_GetComponentDataFields(Data.schema_type);

		for (const Worker_ComponentId& InComponentId : ComponentList)
		{
			Schema_AddUint32(ComponentObject, SpatialConstants::COMPONENT_PRESENCE_COMPONENT_LIST_ID, InComponentId);
		}

		if (ClientWorkerId.IsSet())
		{
			AddStringToSchema(ComponentObject, SpatialConstants::COMPONENT_PRESENCE_CLIENT_WORKER_ID, *ClientWorkerId)
		}

		return Data;
	}

	Worker_ComponentUpdate CreateComponentPresenceUpdate()
	{
		return CreateComponentPresenceUpdate(ComponentList, ClientWorkerId);
	}

	static Worker_ComponentUpdate CreateComponentPresenceUpdate(const TArray<Worker_ComponentId>& ComponentList, const TSchemaOption<PhysicalWorkerName>& ClientWorkerId)
	{
		Worker_ComponentUpdate Update = {};
		Update.component_id = ComponentId;
		Update.schema_type = Schema_CreateComponentUpdate();
		Schema_Object* ComponentObject = Schema_GetComponentUpdateFields(Update.schema_type);

		Schema_AddUint32List(ComponentObject, SpatialConstants::COMPONENT_PRESENCE_COMPONENT_LIST_ID,
			ComponentList.GetData(), ComponentList.Num());

		if (ClientWorkerId.IsSet())
		{
			AddStringToSchema(ComponentObject, SpatialConstants::COMPONENT_PRESENCE_CLIENT_WORKER_ID, *ClientWorkerId)
		}
		else
		{
			Schema_AddComponentUpdateClearedField(Update.schema_type, SpatialConstants::COMPONENT_PRESENCE_CLIENT_WORKER_ID);
		}

		return Update;
	}

	void ApplyComponentUpdate(const Worker_ComponentUpdate& Update)
	{
		Schema_Object* ComponentObject = Schema_GetComponentUpdateFields(Update.schema_type);
		CopyListFromComponentObject(ComponentObject);

		if (Schema_GetBytesCount(ComponentObject, SpatialConstants::COMPONENT_PRESENCE_CLIENT_WORKER_ID) == 1)
		{
			ClientWorkerId = GetStringFromSchema(ComponentObject, SpatialConstants::COMPONENT_PRESENCE_CLIENT_WORKER_ID);
		}
		else if (Schema_IsComponentUpdateFieldCleared(ComponentObject, SpatialConstants::COMPONENT_PRESENCE_CLIENT_WORKER_ID))
		{
			ClientWorkerId = FString();
		}
	}

	void CopyListFromComponentObject(Schema_Object* ComponentObject)
	{
		ComponentList.SetNum(Schema_GetUint32Count(ComponentObject, SpatialConstants::COMPONENT_PRESENCE_COMPONENT_LIST_ID), true);
		Schema_GetUint32List(ComponentObject, SpatialConstants::COMPONENT_PRESENCE_COMPONENT_LIST_ID, ComponentList.GetData());
	}

	void AddComponentDataIds(const TArray<FWorkerComponentData>& ComponentDatas)
	{
		TArray<Worker_ComponentId> ComponentIds;
		for (const FWorkerComponentData& ComponentData : ComponentDatas)
		{
			if (ComponentData.component_id == 0)
			{
				UE_LOG(LogTemp, Warning, TEXT("Unknoasdfasdfao KCP."));
			}
			ComponentIds.Add(ComponentData.component_id);
		}

		AddComponentIds(ComponentIds);
	}

	void AddComponentIds(const TArray<Worker_ComponentId>& ComponentsToAdd)
	{
		for (const Worker_ComponentId& NewComponentId : ComponentsToAdd)
		{
			ComponentList.AddUnique(NewComponentId);
		}
	}

	void RemoveComponentIds(const TArray<Worker_ComponentId>& ComponentsToRemove)
	{
		ComponentList.RemoveAll([&](Worker_ComponentId PresentComponent)
		{
			return ComponentsToRemove.Contains(PresentComponent);
		});
	}

	// List of component IDs that exist on an entity.
	TArray<Worker_ComponentId> ComponentList;

	// Client worker ID corresponding to the owning net connection (if exists).
	TSchemaOption<PhysicalWorkerName> ClientWorkerId;
};

} // namespace SpatialGDK
