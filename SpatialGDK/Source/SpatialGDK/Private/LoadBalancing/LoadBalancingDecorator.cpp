#include "LoadBalancing/LoadBalancingDecorator.h"
#include "Schema/ActorGroupMember.h"
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
	if (uint32* Group = ClassToGroup.Get(Actor->GetClass()))
	{
		ActorGroupMember MembershipComponent(*Group);
		NewData.Add(MembershipComponent.CreateComponentData());
	}
	return NewData;
}
} // namespace SpatialGDK
