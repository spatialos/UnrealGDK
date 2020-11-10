// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "Schema/Component.h"
#include "SpatialCommonTypes.h"
#include "SpatialConstants.h"
#include "Utils/SchemaOption.h"
#include "Utils/SchemaUtils.h"

#include <WorkerSDK/improbable/c_schema.h>
#include <WorkerSDK/improbable/c_worker.h>

namespace SpatialGDK
{
struct NetOwningClientWorker : Component
{
	static const Worker_ComponentId ComponentId = SpatialConstants::NET_OWNING_CLIENT_WORKER_COMPONENT_ID;

	NetOwningClientWorker() = default;

	NetOwningClientWorker(const TSchemaOption<Worker_PartitionId>& InPartitionId)
		: ClientPartitionId(InPartitionId)
	{
	}

	NetOwningClientWorker(const Worker_ComponentData& Data)
		: NetOwningClientWorker(Data.schema_type)
	{
	}

	NetOwningClientWorker(Schema_ComponentData* Data)
	{
		Schema_Object* ComponentObject = Schema_GetComponentDataFields(Data);
		if (Schema_GetEntityIdCount(ComponentObject, SpatialConstants::NET_OWNING_CLIENT_PARTITION_ENTITY_FIELD_ID) == 1)
		{
			ClientPartitionId = Schema_GetEntityId(ComponentObject, SpatialConstants::NET_OWNING_CLIENT_PARTITION_ENTITY_FIELD_ID);
		}
	}

	Worker_ComponentData CreateNetOwningClientWorkerData() { return CreateNetOwningClientWorkerData(ClientPartitionId); }

	static Worker_ComponentData CreateNetOwningClientWorkerData(const TSchemaOption<Worker_PartitionId>& PartitionId)
	{
		Worker_ComponentData Data = {};
		Data.component_id = ComponentId;
		Data.schema_type = Schema_CreateComponentData();
		Schema_Object* ComponentObject = Schema_GetComponentDataFields(Data.schema_type);

		if (PartitionId.IsSet())
		{
			Schema_AddEntityId(ComponentObject, SpatialConstants::NET_OWNING_CLIENT_PARTITION_ENTITY_FIELD_ID, *PartitionId);
		}

		return Data;
	}

	Worker_ComponentUpdate CreateNetOwningClientWorkerUpdate() const { return CreateNetOwningClientWorkerUpdate(ClientPartitionId); }

	static Worker_ComponentUpdate CreateNetOwningClientWorkerUpdate(const TSchemaOption<Worker_PartitionId>& PartitionId)
	{
		Worker_ComponentUpdate Update = {};
		Update.component_id = ComponentId;
		Update.schema_type = Schema_CreateComponentUpdate();
		Schema_Object* ComponentObject = Schema_GetComponentUpdateFields(Update.schema_type);

		if (PartitionId.IsSet())
		{
			Schema_AddEntityId(ComponentObject, SpatialConstants::NET_OWNING_CLIENT_PARTITION_ENTITY_FIELD_ID, *PartitionId);
		}
		else
		{
			Schema_AddComponentUpdateClearedField(Update.schema_type, SpatialConstants::NET_OWNING_CLIENT_PARTITION_ENTITY_FIELD_ID);
		}

		return Update;
	}

	void ApplyComponentUpdate(const Worker_ComponentUpdate& Update) { ApplyComponentUpdate(Update.schema_type); }

	void ApplyComponentUpdate(Schema_ComponentUpdate* Update)
	{
		Schema_Object* ComponentObject = Schema_GetComponentUpdateFields(Update);

		if (Schema_GetEntityIdCount(ComponentObject, SpatialConstants::NET_OWNING_CLIENT_PARTITION_ENTITY_FIELD_ID) == 1)
		{
			ClientPartitionId = Schema_GetEntityId(ComponentObject, SpatialConstants::NET_OWNING_CLIENT_PARTITION_ENTITY_FIELD_ID);
		}
		else if (Schema_IsComponentUpdateFieldCleared(Update, SpatialConstants::NET_OWNING_CLIENT_PARTITION_ENTITY_FIELD_ID))
		{
			ClientPartitionId = TSchemaOption<Worker_PartitionId>();
		}
	}

	void SetPartitionId(const Worker_PartitionId& InPartitionId)
	{
		ClientPartitionId = InPartitionId == SpatialConstants::INVALID_ENTITY_ID ? TSchemaOption<Worker_PartitionId>() : InPartitionId;
	}

	// Client partition entity ID corresponding to the owning net connection (if exists).
	TSchemaOption<Worker_PartitionId> ClientPartitionId;
};

} // namespace SpatialGDK
