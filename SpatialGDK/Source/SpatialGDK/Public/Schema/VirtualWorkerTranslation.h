// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "Schema/Component.h"
#include "SpatialCommonTypes.h"
#include "Utils/SchemaUtils.h"

#include <WorkerSDK/improbable/c_schema.h>

#include "EngineClasses/SpatialVirtualWorkerTranslator.h"

namespace SpatialGDK
{
struct VirtualWorkerInfo
{
	VirtualWorkerId VirtualWorkerId;
	FString PhysicalWorkerName;
	Worker_EntityId ServerWorkerEntity;
	Worker_PartitionId PartitionId;
	Worker_EntityId ServerSystemWorkerEntity;
};

// The SpatialDebugging component exists to hold information which needs to be displayed by the
// SpatialDebugger on clients but which would not normally be available to clients.
struct VirtualWorkerTranslation : AbstractMutableComponent
{
	static const Worker_ComponentId ComponentId = SpatialConstants::VIRTUAL_WORKER_TRANSLATION_COMPONENT_ID;

	VirtualWorkerTranslation()
		: VirtualWorkerMapping({})
	{
	}

	VirtualWorkerTranslation(const TArray<VirtualWorkerInfo>& InVirtualWorkerMapping) { VirtualWorkerMapping = InVirtualWorkerMapping; }

	VirtualWorkerTranslation(const Worker_ComponentData& Data)
		: VirtualWorkerTranslation(Data.schema_type)
	{
	}

	VirtualWorkerTranslation(Schema_ComponentData* Data)
	{
		Schema_Object* ComponentObject = Schema_GetComponentDataFields(Data);

		// Resize the map to accept the new data.
		const uint32 VirtualWorkerCount = Schema_GetObjectCount(ComponentObject, SpatialConstants::VIRTUAL_WORKER_TRANSLATION_MAPPING_ID);
		VirtualWorkerMapping.Empty(VirtualWorkerCount);

		for (uint32 i = 0; i < VirtualWorkerCount; i++)
		{
			// Get each entry of the list and then unpack the virtual and physical IDs from the entry.
			Schema_Object* MappingObject = Schema_IndexObject(ComponentObject, SpatialConstants::VIRTUAL_WORKER_TRANSLATION_MAPPING_ID, i);
			const VirtualWorkerId VirtualWorkerId = Schema_GetUint32(MappingObject, SpatialConstants::MAPPING_VIRTUAL_WORKER_ID);
			const PhysicalWorkerName WorkerName =
				SpatialGDK::GetStringFromSchema(MappingObject, SpatialConstants::MAPPING_PHYSICAL_WORKER_NAME_ID);
			const Worker_EntityId ServerWorkerEntityId =
				Schema_GetEntityId(MappingObject, SpatialConstants::MAPPING_SERVER_WORKER_ENTITY_ID);
			const Worker_PartitionId PartitionEntityId = Schema_GetEntityId(MappingObject, SpatialConstants::MAPPING_PARTITION_ID);
			const Worker_EntityId SystemWorkerEntityId =
				Schema_GetEntityId(MappingObject, SpatialConstants::MAPPING_SYSTEM_WORKER_ENTITY_ID);

			VirtualWorkerMapping.Emplace(
				VirtualWorkerInfo{ VirtualWorkerId, WorkerName, ServerWorkerEntityId, PartitionEntityId, SystemWorkerEntityId });
		}
	}

	VirtualWorkerTranslation(const Worker_ComponentUpdate& Update)
		: VirtualWorkerTranslation()
	{
		ApplyComponentUpdate(Update);
	}

	VirtualWorkerTranslation(Schema_ComponentUpdate* Update)
		: VirtualWorkerTranslation()
	{
		ApplyComponentUpdate(Update);
	}

	Worker_ComponentData CreateComponentData() const
	{
		Worker_ComponentData Data = {};
		Data.component_id = ComponentId;
		Data.schema_type = Schema_CreateComponentData();
		Schema_Object* ComponentObject = Schema_GetComponentDataFields(Data.schema_type);

		for (const auto& Entry : VirtualWorkerMapping)
		{
			Schema_Object* EntryObject = Schema_AddObject(ComponentObject, SpatialConstants::VIRTUAL_WORKER_TRANSLATION_MAPPING_ID);
			Schema_AddUint32(EntryObject, SpatialConstants::MAPPING_VIRTUAL_WORKER_ID, Entry.VirtualWorkerId);
			SpatialGDK::AddStringToSchema(EntryObject, SpatialConstants::MAPPING_PHYSICAL_WORKER_NAME_ID, Entry.PhysicalWorkerName);
			Schema_AddEntityId(EntryObject, SpatialConstants::MAPPING_SERVER_WORKER_ENTITY_ID, Entry.ServerWorkerEntity);
			Schema_AddEntityId(EntryObject, SpatialConstants::MAPPING_PARTITION_ID, Entry.PartitionId);
			Schema_AddEntityId(EntryObject, SpatialConstants::MAPPING_SYSTEM_WORKER_ENTITY_ID, Entry.ServerSystemWorkerEntity);
		}

		return Data;
	}

	Worker_ComponentUpdate CreateVirtualWorkerTranslationUpdate() const
	{
		Worker_ComponentUpdate Update = {};
		Update.component_id = ComponentId;
		Update.schema_type = Schema_CreateComponentUpdate();
		Schema_Object* ComponentObject = Schema_GetComponentUpdateFields(Update.schema_type);

		for (const auto& Entry : VirtualWorkerMapping)
		{
			Schema_Object* EntryObject = Schema_AddObject(ComponentObject, SpatialConstants::VIRTUAL_WORKER_TRANSLATION_MAPPING_ID);
			Schema_AddUint32(EntryObject, SpatialConstants::MAPPING_VIRTUAL_WORKER_ID, Entry.VirtualWorkerId);
			SpatialGDK::AddStringToSchema(EntryObject, SpatialConstants::MAPPING_PHYSICAL_WORKER_NAME_ID, Entry.PhysicalWorkerName);
			Schema_AddEntityId(EntryObject, SpatialConstants::MAPPING_SERVER_WORKER_ENTITY_ID, Entry.ServerWorkerEntity);
			Schema_AddEntityId(EntryObject, SpatialConstants::MAPPING_PARTITION_ID, Entry.PartitionId);
			Schema_AddEntityId(EntryObject, SpatialConstants::MAPPING_SYSTEM_WORKER_ENTITY_ID, Entry.ServerSystemWorkerEntity);
		}

		return Update;
	}

	void ApplyComponentUpdate(const Worker_ComponentUpdate& Update) { ApplyComponentUpdate(Update.schema_type); }

	void ApplyComponentUpdate(Schema_ComponentUpdate* Update)
	{
		Schema_Object* ComponentObject = Schema_GetComponentUpdateFields(Update);

		// Resize the map to accept the new data.
		const uint32 VirtualWorkerCount = Schema_GetObjectCount(ComponentObject, SpatialConstants::VIRTUAL_WORKER_TRANSLATION_MAPPING_ID);
		VirtualWorkerMapping.Empty(VirtualWorkerCount);

		for (uint32 i = 0; i < VirtualWorkerCount; i++)
		{
			// Get each entry of the list and then unpack the virtual and physical IDs from the entry.
			Schema_Object* MappingObject = Schema_IndexObject(ComponentObject, SpatialConstants::VIRTUAL_WORKER_TRANSLATION_MAPPING_ID, i);
			const VirtualWorkerId VirtualWorkerId = Schema_GetUint32(MappingObject, SpatialConstants::MAPPING_VIRTUAL_WORKER_ID);
			const PhysicalWorkerName WorkerName =
				SpatialGDK::GetStringFromSchema(MappingObject, SpatialConstants::MAPPING_PHYSICAL_WORKER_NAME_ID);
			const Worker_EntityId ServerWorkerEntityId =
				Schema_GetEntityId(MappingObject, SpatialConstants::MAPPING_SERVER_WORKER_ENTITY_ID);
			const Worker_PartitionId PartitionEntityId = Schema_GetEntityId(MappingObject, SpatialConstants::MAPPING_PARTITION_ID);
			const Worker_EntityId SystemWorkerEntityId =
				Schema_GetEntityId(MappingObject, SpatialConstants::MAPPING_SYSTEM_WORKER_ENTITY_ID);

			VirtualWorkerMapping.Emplace(
				VirtualWorkerInfo{ VirtualWorkerId, WorkerName, ServerWorkerEntityId, PartitionEntityId, SystemWorkerEntityId });
		}
	}

	TArray<VirtualWorkerInfo> VirtualWorkerMapping;
};

} // namespace SpatialGDK
