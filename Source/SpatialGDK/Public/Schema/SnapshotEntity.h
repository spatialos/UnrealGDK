// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "Platform.h"

#include "Schema/Component.h"

const Worker_ComponentId SNAPSHOT_ENTITY_COMPONENT_ID = 100006;

struct SnapshotEntity : Component
{
	static const Worker_ComponentId ComponentId = SNAPSHOT_ENTITY_COMPONENT_ID;

	SnapshotEntity() = default;

	Worker_ComponentData CreateSnapshotEntityData()
	{
		Worker_ComponentData Data = {};
		Data.component_id = SNAPSHOT_ENTITY_COMPONENT_ID;
		Data.schema_type = Schema_CreateComponentData(SNAPSHOT_ENTITY_COMPONENT_ID);

		return Data;
	}
};

