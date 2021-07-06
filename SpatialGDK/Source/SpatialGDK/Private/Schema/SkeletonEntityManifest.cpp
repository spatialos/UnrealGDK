#include "Schema/SkeletonEntityManifest.h"

namespace SpatialGDK
{
static void ApplyCollection(TArray<Worker_EntityId>& Array, const Schema_Object& ComponentObject, Schema_FieldId FieldId)
{
	const uint32 EntitiesToPopulateCount = Schema_GetEntityIdCount(&ComponentObject, FieldId);
	Array.SetNumUninitialized(EntitiesToPopulateCount);
	for (uint32 EntityToPopulateIndex = 0; EntityToPopulateIndex < EntitiesToPopulateCount; ++EntityToPopulateIndex)
	{
		Array[EntityToPopulateIndex] = Schema_IndexEntityId(&ComponentObject, FieldId, EntityToPopulateIndex);
	}
}

static void ApplyCollection(TSet<Worker_EntityId_Key>& Set, const Schema_Object& ComponentObject, Schema_FieldId FieldId)
{
	const uint32 EntitiesToPopulateCount = Schema_GetEntityIdCount(&ComponentObject, FieldId);
	for (uint32 EntityToPopulateIndex = 0; EntityToPopulateIndex < EntitiesToPopulateCount; ++EntityToPopulateIndex)
	{
		const Worker_EntityId EntityId = Schema_IndexEntityId(&ComponentObject, FieldId, EntityToPopulateIndex);
		Set.Emplace(EntityId);
	}
}

void FSkeletonEntityManifest::ApplySchema(const Schema_Object& ComponentObject)
{
	ApplyCollection(EntitiesToPopulate, ComponentObject, SpatialConstants::SKELETON_ENTITY_MANIFEST_ENTITIES_TO_POPULATE_ID);
	ApplyCollection(PopulatedEntities, ComponentObject, SpatialConstants::SKELETON_ENTITY_MANIFEST_POPULATED_SKELETON_ENTITIES_ID);
}

template <class TCollection>
void WriteCollection(const TCollection& EntityIds, Schema_Object& ComponentObject, Schema_FieldId FieldId)
{
	for (const auto CollectionEntityId : EntityIds)
	{
		const Worker_EntityId EntityId = CollectionEntityId;
		Schema_AddEntityId(&ComponentObject, FieldId, EntityId);
	}
}

void FSkeletonEntityManifest::WriteSchema(Schema_Object& ComponentObject) const
{
	WriteCollection(EntitiesToPopulate, ComponentObject, SpatialConstants::SKELETON_ENTITY_MANIFEST_ENTITIES_TO_POPULATE_ID);
	WriteCollection(PopulatedEntities, ComponentObject, SpatialConstants::SKELETON_ENTITY_MANIFEST_POPULATED_SKELETON_ENTITIES_ID);
}
} // namespace SpatialGDK
