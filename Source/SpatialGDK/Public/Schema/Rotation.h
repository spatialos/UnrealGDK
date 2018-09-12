// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "Platform.h"

#include "Schema/Component.h"
#include "Utils/SchemaUtils.h"

#include <improbable/c_schema.h>
#include <improbable/c_worker.h>

const Worker_ComponentId ROTATION_COMPONENT_ID = 100001;

struct Rotation : Component
{
	static const Worker_ComponentId ComponentId = ROTATION_COMPONENT_ID;

	Rotation() = default;

	Rotation(float InPitch, float InYaw, float InRoll)
		: Pitch(InPitch), Yaw(InYaw), Roll(InRoll) {}

	Rotation(const FRotator& Rotator)
		: Pitch(Rotator.Pitch), Yaw(Rotator.Yaw), Roll(Rotator.Roll) {}

	Rotation(const Worker_ComponentData& Data)
	{
		Schema_Object* ComponentObject = Schema_GetComponentDataFields(Data.schema_type);

		Pitch = Schema_GetFloat(ComponentObject, 1);
		Yaw = Schema_GetFloat(ComponentObject, 2);
		Roll = Schema_GetFloat(ComponentObject, 3);
	}

	FRotator ToFRotator()
	{
		return {Pitch, Yaw, Roll};
	}

	Worker_ComponentData CreateRotationData()
	{
		Worker_ComponentData Data = {};
		Data.component_id = ComponentId;
		Data.schema_type = Schema_CreateComponentData(ComponentId);
		Schema_Object* ComponentObject = Schema_GetComponentDataFields(Data.schema_type);

		Schema_AddFloat(ComponentObject, 1, Pitch);
		Schema_AddFloat(ComponentObject, 2, Yaw);
		Schema_AddFloat(ComponentObject, 3, Roll);

		return Data;
	}

	Worker_ComponentUpdate CreateRotationUpdate()
	{
		Worker_ComponentUpdate ComponentUpdate = {};
		ComponentUpdate.component_id = ComponentId;
		ComponentUpdate.schema_type = Schema_CreateComponentUpdate(ComponentId);
		Schema_Object* ComponentObject = Schema_GetComponentUpdateFields(ComponentUpdate.schema_type);

		Schema_AddFloat(ComponentObject, 1, Pitch);
		Schema_AddFloat(ComponentObject, 2, Yaw);
		Schema_AddFloat(ComponentObject, 3, Roll);

		return ComponentUpdate;
	}

	float Pitch;
	float Yaw;
	float Roll;
};
