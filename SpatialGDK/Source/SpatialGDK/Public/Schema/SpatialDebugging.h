// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "Schema/Component.h"
#include "SpatialCommonTypes.h"
#include "Utils/SchemaUtils.h"

#include <WorkerSDK/improbable/c_schema.h>

namespace SpatialGDK
{
// The SpatialDebugging component exists to hold information which needs to be displayed by the
// SpatialDebugger on clients but which would not normally be available to clients.
struct SpatialDebugging : Component
{
	static const Worker_ComponentId ComponentId = SpatialConstants::SPATIAL_DEBUGGING_COMPONENT_ID;

	SpatialDebugging()
		: AuthoritativeVirtualWorkerId(SpatialConstants::INVALID_VIRTUAL_WORKER_ID)
		, AuthoritativeColor()
		, IntentVirtualWorkerId(SpatialConstants::INVALID_VIRTUAL_WORKER_ID)
		, IntentColor()
		, IsLocked(false)
	{
	}

	SpatialDebugging(const VirtualWorkerId AuthoritativeVirtualWorkerIdIn, const FColor& AuthoritativeColorIn,
					 const VirtualWorkerId IntentVirtualWorkerIdIn, const FColor& IntentColorIn, bool IsLockedIn)
	{
		AuthoritativeVirtualWorkerId = AuthoritativeVirtualWorkerIdIn;
		AuthoritativeColor = AuthoritativeColorIn;
		IntentVirtualWorkerId = IntentVirtualWorkerIdIn;
		IntentColor = IntentColorIn;
		IsLocked = IsLockedIn;
	}

	SpatialDebugging(const Worker_ComponentData& Data)
	{
		Schema_Object* ComponentObject = Schema_GetComponentDataFields(Data.schema_type);

		AuthoritativeVirtualWorkerId =
			Schema_GetUint32(ComponentObject, SpatialConstants::SPATIAL_DEBUGGING_AUTHORITATIVE_VIRTUAL_WORKER_ID);
		AuthoritativeColor = FColor(Schema_GetUint32(ComponentObject, SpatialConstants::SPATIAL_DEBUGGING_AUTHORITATIVE_COLOR));
		IntentVirtualWorkerId = Schema_GetUint32(ComponentObject, SpatialConstants::SPATIAL_DEBUGGING_INTENT_VIRTUAL_WORKER_ID);
		IntentColor = FColor(Schema_GetUint32(ComponentObject, SpatialConstants::SPATIAL_DEBUGGING_INTENT_COLOR));
		IsLocked = Schema_GetBool(ComponentObject, SpatialConstants::SPATIAL_DEBUGGING_IS_LOCKED) != 0;
	}

	Worker_ComponentData CreateSpatialDebuggingData()
	{
		Worker_ComponentData Data = {};
		Data.component_id = ComponentId;
		Data.schema_type = Schema_CreateComponentData();
		Schema_Object* ComponentObject = Schema_GetComponentDataFields(Data.schema_type);

		Schema_AddUint32(ComponentObject, SpatialConstants::SPATIAL_DEBUGGING_AUTHORITATIVE_VIRTUAL_WORKER_ID,
						 AuthoritativeVirtualWorkerId);
		Schema_AddUint32(ComponentObject, SpatialConstants::SPATIAL_DEBUGGING_AUTHORITATIVE_COLOR, AuthoritativeColor.DWColor());
		Schema_AddUint32(ComponentObject, SpatialConstants::SPATIAL_DEBUGGING_INTENT_VIRTUAL_WORKER_ID, IntentVirtualWorkerId);
		Schema_AddUint32(ComponentObject, SpatialConstants::SPATIAL_DEBUGGING_INTENT_COLOR, IntentColor.DWColor());
		Schema_AddBool(ComponentObject, SpatialConstants::SPATIAL_DEBUGGING_IS_LOCKED, IsLocked);

		return Data;
	}

	Worker_ComponentUpdate CreateSpatialDebuggingUpdate()
	{
		Worker_ComponentUpdate Update = {};
		Update.component_id = ComponentId;
		Update.schema_type = Schema_CreateComponentUpdate();
		Schema_Object* ComponentObject = Schema_GetComponentUpdateFields(Update.schema_type);

		Schema_AddUint32(ComponentObject, SpatialConstants::SPATIAL_DEBUGGING_AUTHORITATIVE_VIRTUAL_WORKER_ID,
						 AuthoritativeVirtualWorkerId);
		Schema_AddUint32(ComponentObject, SpatialConstants::SPATIAL_DEBUGGING_AUTHORITATIVE_COLOR, AuthoritativeColor.DWColor());
		Schema_AddUint32(ComponentObject, SpatialConstants::SPATIAL_DEBUGGING_INTENT_VIRTUAL_WORKER_ID, IntentVirtualWorkerId);
		Schema_AddUint32(ComponentObject, SpatialConstants::SPATIAL_DEBUGGING_INTENT_COLOR, IntentColor.DWColor());
		Schema_AddBool(ComponentObject, SpatialConstants::SPATIAL_DEBUGGING_IS_LOCKED, IsLocked);

		return Update;
	}

	void ApplyComponentUpdate(const Worker_ComponentUpdate& Update)
	{
		Schema_Object* ComponentObject = Schema_GetComponentUpdateFields(Update.schema_type);

		AuthoritativeVirtualWorkerId =
			Schema_GetUint32(ComponentObject, SpatialConstants::SPATIAL_DEBUGGING_AUTHORITATIVE_VIRTUAL_WORKER_ID);
		AuthoritativeColor = FColor(Schema_GetUint32(ComponentObject, SpatialConstants::SPATIAL_DEBUGGING_AUTHORITATIVE_COLOR));
		IntentVirtualWorkerId = Schema_GetUint32(ComponentObject, SpatialConstants::SPATIAL_DEBUGGING_INTENT_VIRTUAL_WORKER_ID);
		IntentColor = FColor(Schema_GetUint32(ComponentObject, SpatialConstants::SPATIAL_DEBUGGING_INTENT_COLOR));
		IsLocked = Schema_GetBool(ComponentObject, SpatialConstants::SPATIAL_DEBUGGING_IS_LOCKED) != 0;
	}

	// Id of the Unreal server worker which is authoritative for the entity.
	// 0 is reserved as an invalid/unset value.
	VirtualWorkerId AuthoritativeVirtualWorkerId;

	// The color for the authoritative virtual worker.
	FColor AuthoritativeColor;

	// Id of the Unreal server worker which should be authoritative for the entity.
	// 0 is reserved as an invalid/unset value.
	VirtualWorkerId IntentVirtualWorkerId;

	// The color for the intended virtual worker.
	FColor IntentColor;

	// Whether or not the entity is locked.
	bool IsLocked;
};

} // namespace SpatialGDK
