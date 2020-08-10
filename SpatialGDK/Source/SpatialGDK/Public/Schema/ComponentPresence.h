// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "Schema/Component.h"
#include "SpatialCommonTypes.h"
#include "SpatialConstants.h"
#include "Utils/SchemaUtils.h"

#include "Containers/Array.h"
#include "HAL/UnrealMemory.h"
#include "Templates/UnrealTemplate.h"

#include <WorkerSDK/improbable/c_schema.h>
#include <WorkerSDK/improbable/c_worker.h>

namespace SpatialGDK
{
struct ComponentPresence : Component
{
	static const Worker_ComponentId ComponentId = SpatialConstants::COMPONENT_PRESENCE_COMPONENT_ID;

	ComponentPresence() = default;

	ComponentPresence(TArray<Worker_ComponentId>&& InComponentList)
		: ComponentList(MoveTemp(InComponentList))
	{
	}

	ComponentPresence(const Worker_ComponentData& Data)
	{
		Schema_Object* ComponentObject = Schema_GetComponentDataFields(Data.schema_type);
		CopyListFromComponentObject(ComponentObject);
	}

	Worker_ComponentData CreateComponentPresenceData() { return CreateComponentPresenceData(ComponentList); }

	static Worker_ComponentData CreateComponentPresenceData(const TArray<Worker_ComponentId>& ComponentList)
	{
		Worker_ComponentData Data = {};
		Data.component_id = ComponentId;
		Data.schema_type = Schema_CreateComponentData();
		Schema_Object* ComponentObject = Schema_GetComponentDataFields(Data.schema_type);

		uint32 BufferCount = ComponentList.Num();
		uint32 BufferSize = BufferCount * sizeof(uint32);
		uint32* Buffer = reinterpret_cast<uint32*>(Schema_AllocateBuffer(ComponentObject, BufferSize));
		FMemory::Memcpy(Buffer, ComponentList.GetData(), BufferSize);
		Schema_AddUint32List(ComponentObject, SpatialConstants::COMPONENT_PRESENCE_COMPONENT_LIST_ID, Buffer, BufferCount);

		return Data;
	}

	Worker_ComponentUpdate CreateComponentPresenceUpdate() { return CreateComponentPresenceUpdate(ComponentList); }

	static Worker_ComponentUpdate CreateComponentPresenceUpdate(const TArray<Worker_ComponentId>& ComponentList)
	{
		Worker_ComponentUpdate Update = {};
		Update.component_id = ComponentId;
		Update.schema_type = Schema_CreateComponentUpdate();
		Schema_Object* ComponentObject = Schema_GetComponentUpdateFields(Update.schema_type);

		uint32 BufferCount = ComponentList.Num();
		uint32 BufferSize = BufferCount * sizeof(uint32);
		uint32* Buffer = reinterpret_cast<uint32*>(Schema_AllocateBuffer(ComponentObject, BufferSize));
		FMemory::Memcpy(Buffer, ComponentList.GetData(), BufferSize);
		Schema_AddUint32List(ComponentObject, SpatialConstants::COMPONENT_PRESENCE_COMPONENT_LIST_ID, Buffer, BufferCount);

		return Update;
	}

	void ApplyComponentUpdate(const Worker_ComponentUpdate& Update)
	{
		Schema_Object* ComponentObject = Schema_GetComponentUpdateFields(Update.schema_type);
		CopyListFromComponentObject(ComponentObject);
	}

	void CopyListFromComponentObject(Schema_Object* ComponentObject)
	{
		ComponentList.SetNum(Schema_GetUint32Count(ComponentObject, SpatialConstants::COMPONENT_PRESENCE_COMPONENT_LIST_ID), true);
		Schema_GetUint32List(ComponentObject, SpatialConstants::COMPONENT_PRESENCE_COMPONENT_LIST_ID, ComponentList.GetData());
	}

	void AddComponentDataIds(const TArray<FWorkerComponentData>& ComponentDatas)
	{
		TArray<Worker_ComponentId> ComponentIds;
		ComponentIds.Reserve(ComponentDatas.Num());
		for (const FWorkerComponentData& ComponentData : ComponentDatas)
		{
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
		ComponentList.RemoveAll([&](Worker_ComponentId PresentComponent) {
			return ComponentsToRemove.Contains(PresentComponent);
		});
	}

	// List of component IDs that exist on an entity.
	TArray<Worker_ComponentId> ComponentList;
};

} // namespace SpatialGDK
