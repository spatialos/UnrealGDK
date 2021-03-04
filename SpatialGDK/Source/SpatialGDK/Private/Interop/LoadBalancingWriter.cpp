// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "Interop/LoadBalancingWriter.h"

#include "EngineClasses/SpatialActorChannel.h"
#include "EngineClasses/SpatialNetDriver.h"
#include "EngineClasses/SpatialPackageMapClient.h"
#include "LoadBalancing/AbstractLBStrategy.h"
#include "Utils/SpatialActorUtils.h"

namespace SpatialGDK
{
void FLoadBalancingWriterBase::SendComponentUpdate(Worker_EntityId EntityId, FWorkerComponentUpdate& ComponentUpdate)
{
	NetDriver->Connection->SendComponentUpdate(EntityId, &ComponentUpdate);
}

class LoadBalancingWriterActorSet : public TLoadBalancingWriter<ActorSetMember>
{
public:
	explicit LoadBalancingWriterActorSet(USpatialNetDriver* NetDriver)
		: TLoadBalancingWriter<ActorSetMember>(NetDriver)
	{
	}

private:
	virtual ActorSetMember GetLoadBalancingData(AActor* Actor) const override
	{
		const AActor* LeaderActor = GetReplicatedHierarchyRoot(Actor);
		check(IsValid(LeaderActor));

		const Worker_EntityId LeaderEntityId = NetDriver->PackageMap->GetEntityIdFromObject(LeaderActor);
		check(LeaderEntityId != SpatialConstants::INVALID_ENTITY_ID);

		return ActorSetMember(LeaderEntityId);
	}
};

class LoadBalancingWriterActorGroup : public TLoadBalancingWriter<ActorGroupMember>
{
public:
	explicit LoadBalancingWriterActorGroup(USpatialNetDriver* NetDriver)
		: TLoadBalancingWriter<ActorGroupMember>(NetDriver)
	{
	}

private:
	virtual ActorGroupMember GetLoadBalancingData(AActor* Actor) const override
	{
		return ActorGroupMember(NetDriver->LoadBalanceStrategy->GetActorGroupId(*Actor));
	}
};

LoadBalancingWriter::LoadBalancingWriter(USpatialNetDriver* InNetDriver)
	: NetDriver(InNetDriver)
	, ActorSetWriter(MakeUnique<LoadBalancingWriterActorSet>(InNetDriver))
	, ActorGroupWriter(MakeUnique<LoadBalancingWriterActorGroup>(InNetDriver))

{
}

void LoadBalancingWriter::OnActorReplicated(Worker_EntityId ActorEntityId, AActor* Actor) const
{
	ActorSetWriter->ReplicateActor(ActorEntityId, Actor);
}

} // namespace SpatialGDK
