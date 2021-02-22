// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "Interop/LoadBalancingWriterSystem.h"

#include "EngineClasses/SpatialActorChannel.h"
#include "EngineClasses/SpatialNetDriver.h"
#include "EngineClasses/SpatialPackageMapClient.h"
#include "LoadBalancing/AbstractLBStrategy.h"

namespace SpatialGDK
{
const EntityViewElement& FLoadBalancingWriterBase::GetEntityView(Worker_EntityId EntityId) const
{
	return NetDriver->Connection->GetView()[EntityId];
}

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
		const AActor* Owner = Actor;
		for (; IsValid(Owner->GetOwner()); Owner = Owner->GetOwner())
			;
		check(IsValid(Owner));
		const Worker_EntityId OwnerEntityId = NetDriver->PackageMap->GetEntityIdFromObject(Owner);
		check(OwnerEntityId != SpatialConstants::INVALID_ENTITY_ID);
		// Actor Set IDs are implemented as owner entity IDs
		return ActorSetMember(OwnerEntityId);
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

void LoadBalancingWriter::OnActorReplicated(Worker_EntityId ActorEntityId, AActor* Actor)
{
	ActorSetWriter->ReplicateActor(ActorEntityId, Actor);
	ActorGroupWriter->ReplicateActor(ActorEntityId, Actor);
}

} // namespace SpatialGDK