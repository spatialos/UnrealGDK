#pragma once

#include "SpatialCommonTypes.h"
#include "SpatialConstants.h"
#include "Utils/SchemaUtils.h"

#include "WorkerSDK/improbable/c_worker.h"

namespace SpatialGDK
{
struct FSkeletonEntityManifest : public TSpatialComponent<FSkeletonEntityManifest>
{
	using Super = TSpatialComponent<FSkeletonEntityManifest>;

	static constexpr Worker_ComponentId ComponentId = SpatialConstants::SKELETON_ENTITY_MANIFEST_COMPONENT_ID;

	FSkeletonEntityManifest() = default;

	explicit FSkeletonEntityManifest(const ComponentData& ComponentData) { ApplyComponentData(*this, ComponentData); }

	void ApplySchema(const Schema_Object& ComponentObject);
	void WriteSchema(Schema_Object& ComponentObject) const;

	TArray<Worker_EntityId> EntitiesToPopulate;
	TArray<Worker_EntityId> PopulatedEntities;
};
} // namespace SpatialGDK
