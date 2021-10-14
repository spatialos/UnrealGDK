#include "Schema/SkeletonEntityManifest.h"

namespace SpatialGDK
{
namespace
{
constexpr Schema_FieldId SKELETON_ENTITY_MANIFEST_ENTITIES_TO_POPULATE_ID = 1;
constexpr Schema_FieldId SKELETON_ENTITY_MANIFEST_POPULATED_SKELETON_ENTITIES_ID = 2;
constexpr Schema_FieldId SKELETON_ENTITY_MANIFEST_ACK_ID = 3;
} // namespace

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
	ApplyCollection(EntitiesToPopulate, ComponentObject, SKELETON_ENTITY_MANIFEST_ENTITIES_TO_POPULATE_ID);
	ApplyCollection(PopulatedEntities, ComponentObject, SKELETON_ENTITY_MANIFEST_POPULATED_SKELETON_ENTITIES_ID);
	bAckedManifest = Schema_GetBool(&ComponentObject, SKELETON_ENTITY_MANIFEST_ACK_ID) != 0;
}

static void WriteCollection(const TSet<Worker_EntityId_Key>& EntityIds, Schema_Object& ComponentObject, Schema_FieldId FieldId)
{
	for (const auto CollectionEntityId : EntityIds)
	{
		const Worker_EntityId EntityId = CollectionEntityId;
		Schema_AddEntityId(&ComponentObject, FieldId, EntityId);
	}
}

void FSkeletonEntityManifest::WriteSchema(Schema_Object& ComponentObject) const
{
	WriteCollection(EntitiesToPopulate, ComponentObject, SKELETON_ENTITY_MANIFEST_ENTITIES_TO_POPULATE_ID);
	WriteCollection(PopulatedEntities, ComponentObject, SKELETON_ENTITY_MANIFEST_POPULATED_SKELETON_ENTITIES_ID);
	uint8_t AckedManifest = bAckedManifest ? 1 : 0;
	Schema_AddBool(&ComponentObject, SKELETON_ENTITY_MANIFEST_ACK_ID, AckedManifest);
}
} // namespace SpatialGDK
