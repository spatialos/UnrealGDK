#include "Schema/SkeletonEntityManifest.h"

namespace SpatialGDK
{
static void ApplyArray(TArray<Worker_EntityId>& Array, const Schema_Object& ComponentObject, Schema_FieldId FieldId)
{
	const uint32 EntitiesToPopulateCount = Schema_GetEntityIdCount(&ComponentObject, FieldId);
	Array.SetNumUninitialized(EntitiesToPopulateCount);
	for (uint32 EntityToPopulateIndex = 0; EntityToPopulateIndex < EntitiesToPopulateCount; ++EntityToPopulateIndex)
	{
		Array[EntityToPopulateIndex] = Schema_IndexEntityId(&ComponentObject, FieldId, EntityToPopulateIndex);
	}
}

void FSkeletonEntityManifest::ApplySchema(const Schema_Object& ComponentObject)
{
	ApplyArray(EntitiesToPopulate, ComponentObject, SpatialConstants::SKELETON_ENTITY_MANIFEST_ENTITIES_TO_POPULATE_ID);
	ApplyArray(PopulatedEntities, ComponentObject, SpatialConstants::SKELETON_ENTITY_MANIFEST_POPULATED_SKELETON_ENTITIES_ID);
}

static void WriteArray(const TArray<Worker_EntityId>& EntityIds, Schema_Object& ComponentObject, Schema_FieldId FieldId)
{
	for (const Worker_EntityId EntityId : EntityIds)
	{
		Schema_AddEntityId(&ComponentObject, FieldId, EntityId);
	}
}

void FSkeletonEntityManifest::WriteSchema(Schema_Object& ComponentObject) const
{
	WriteArray(EntitiesToPopulate, ComponentObject, SpatialConstants::SKELETON_ENTITY_MANIFEST_ENTITIES_TO_POPULATE_ID);
	WriteArray(PopulatedEntities, ComponentObject, SpatialConstants::SKELETON_ENTITY_MANIFEST_POPULATED_SKELETON_ENTITIES_ID);
}
} // namespace SpatialGDK
