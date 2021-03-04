// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "Schema/Component.h"
#include "SpatialView/ComponentData.h"

namespace SpatialGDK
{

struct SnapshotVersion : AbstractMutableComponent
{
	static const Worker_ComponentId ComponentId = SpatialConstants::SNAPSHOT_VERSION_COMPONENT_ID;

	SnapshotVersion()
		: Version(0)
	{
	}

	SnapshotVersion(const uint64 InVersion)
		: Version(InVersion)
	{
	}

	SnapshotVersion(const Worker_ComponentData& Data)
	{
		Schema_Object* ComponentObject = Schema_GetComponentDataFields(Data.schema_type);
		check(ComponentObject);

		Version = Schema_GetUint64(ComponentObject, 1);
	}

	Worker_ComponentData CreateComponentData() const override
	{
		Worker_ComponentData Data = {};
		Data.component_id = ComponentId;
		Data.schema_type = Schema_CreateComponentData();

		Schema_Object* ComponentObject = Schema_GetComponentDataFields(Data.schema_type);
		check(ComponentObject);

		Schema_AddUint64(ComponentObject, 1, Version);

		return Data;
	}

	uint64 Version;
};

} // namespace SpatialGDK
