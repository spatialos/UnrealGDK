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

	NetOwningClientWorker(const TSchemaOption<PhysicalWorkerName>& InWorkerId)
		: WorkerId(InWorkerId)
	{
	}

	NetOwningClientWorker(const Worker_ComponentData& Data)
	{
		Schema_Object* ComponentObject = Schema_GetComponentDataFields(Data.schema_type);
		if (Schema_GetBytesCount(ComponentObject, SpatialConstants::NET_OWNING_CLIENT_WORKER_FIELD_ID) == 1)
		{
			WorkerId = GetStringFromSchema(ComponentObject, SpatialConstants::NET_OWNING_CLIENT_WORKER_FIELD_ID);
		}
	}

	Worker_ComponentData CreateNetOwningClientWorkerData() { return CreateNetOwningClientWorkerData(WorkerId); }

	static Worker_ComponentData CreateNetOwningClientWorkerData(const TSchemaOption<PhysicalWorkerName>& WorkerId)
	{
		Worker_ComponentData Data = {};
		Data.component_id = ComponentId;
		Data.schema_type = Schema_CreateComponentData();
		Schema_Object* ComponentObject = Schema_GetComponentDataFields(Data.schema_type);

		if (WorkerId.IsSet())
		{
			AddStringToSchema(ComponentObject, SpatialConstants::NET_OWNING_CLIENT_WORKER_FIELD_ID, *WorkerId);
		}

		return Data;
	}

	Worker_ComponentUpdate CreateNetOwningClientWorkerUpdate() { return CreateNetOwningClientWorkerUpdate(WorkerId); }

	static Worker_ComponentUpdate CreateNetOwningClientWorkerUpdate(const TSchemaOption<PhysicalWorkerName>& WorkerId)
	{
		Worker_ComponentUpdate Update = {};
		Update.component_id = ComponentId;
		Update.schema_type = Schema_CreateComponentUpdate();
		Schema_Object* ComponentObject = Schema_GetComponentUpdateFields(Update.schema_type);

		if (WorkerId.IsSet())
		{
			AddStringToSchema(ComponentObject, SpatialConstants::NET_OWNING_CLIENT_WORKER_FIELD_ID, *WorkerId);
		}
		else
		{
			Schema_AddComponentUpdateClearedField(Update.schema_type, SpatialConstants::NET_OWNING_CLIENT_WORKER_FIELD_ID);
		}

		return Update;
	}

	void ApplyComponentUpdate(const Worker_ComponentUpdate& Update)
	{
		Schema_Object* ComponentObject = Schema_GetComponentUpdateFields(Update.schema_type);
		if (Schema_GetBytesCount(ComponentObject, SpatialConstants::NET_OWNING_CLIENT_WORKER_FIELD_ID) == 1)
		{
			WorkerId = GetStringFromSchema(ComponentObject, SpatialConstants::NET_OWNING_CLIENT_WORKER_FIELD_ID);
		}
		else if (Schema_IsComponentUpdateFieldCleared(Update.schema_type, SpatialConstants::NET_OWNING_CLIENT_WORKER_FIELD_ID))
		{
			WorkerId = TSchemaOption<PhysicalWorkerName>();
		}
	}

	// Client worker ID corresponding to the owning net connection (if exists).
	TSchemaOption<PhysicalWorkerName> WorkerId;
};

} // namespace SpatialGDK
