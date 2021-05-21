#include "LoadBalancing/LoadBalancingDecorator.h"
#include "Utils/SchemaUtils.h"

namespace SpatialGDK
{
FLayerLoadBalancingDecorator::FLayerLoadBalancingDecorator(TClassMap<uint32>&& InClassToGroup)
	: ClassToGroup(MoveTemp(InClassToGroup))
{
}

TArray<SpatialGDK::ComponentData> FLayerLoadBalancingDecorator::OnCreate(/*Worker_EntityId EntityId,*/ AActor* Actor)
{
	TArray<SpatialGDK::ComponentData> NewData;
	if (uint32_t* Group = ClassToGroup.Get(Actor->GetClass()))
	{
		Schema_ComponentData* Data = Schema_CreateComponentData();
		Schema_Object* GroupObject = Schema_GetComponentDataFields(Data);
		Schema_AddUint32(GroupObject, SpatialConstants::ACTOR_GROUP_MEMBER_COMPONENT_ACTOR_GROUP_ID, *Group);

		NewData.Add(SpatialGDK::ComponentData(SpatialGDK::OwningComponentDataPtr(Data), SpatialConstants::ACTOR_GROUP_MEMBER_COMPONENT_ID));
	}
	return NewData;
}
} // namespace SpatialGDK
