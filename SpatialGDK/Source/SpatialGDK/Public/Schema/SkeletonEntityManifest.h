#pragma once

#include "SpatialCommonTypes.h"
#include "SpatialConstants.h"
#include "Utils/SchemaUtils.h"

#include "WorkerSDK/improbable/c_worker.h"

namespace SpatialGDK
{
struct FSkeletonEntityManifest
{
	static constexpr Worker_ComponentId ComponentId = SpatialConstants::SKELETON_ENTITY_MANIFEST_COMPONENT_ID;

	FSkeletonEntityManifest() = default;

	explicit FSkeletonEntityManifest(const ComponentData& ComponentData) { ApplyComponentData(ComponentData); }

	void ApplyComponentData(const ComponentData& Data) { ApplySchema(*Data.GetFields()); }

	void ApplyComponentUpdate(const ComponentUpdate& Update) { ApplySchema(*Update.GetFields()); }

	ComponentData CreateComponentData() const
	{
		ComponentData Data(ComponentId);
		WriteSchema(*Data.GetFields());
		return MoveTemp(Data);
	}

	ComponentUpdate CreateComponentUpdate() const
	{
		ComponentUpdate Update(ComponentId);
		WriteSchema(*Update.GetFields());
		return MoveTemp(Update);
	}

	void ApplySchema(const Schema_Object& ComponentObject);
	void WriteSchema(Schema_Object& ComponentObject) const;

	TSet<Worker_EntityId_Key> EntitiesToPopulate;
	TSet<Worker_EntityId_Key> PopulatedEntities;
	bool bAckedManifest = false;
};
} // namespace SpatialGDK
