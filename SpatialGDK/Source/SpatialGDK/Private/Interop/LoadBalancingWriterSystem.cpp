// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "Interop/LoadBalancingWriterSystem.h"

#include "EngineClasses/SpatialActorChannel.h"
#include "EngineClasses/SpatialNetDriver.h"
#include "EngineClasses/SpatialPackageMapClient.h"
#include "LoadBalancing/AbstractLBStrategy.h"
#include "SpatialView/EntityComponentTypes.h"
#include "SpatialView/SubView.h"
namespace SpatialGDK
{
Worker_EntityId FLoadBalancingWriterBase::GetEntityId(AActor* Actor) const
{
	return NetDriver->PackageMap->GetEntityIdFromObject(Actor);
}

const EntityViewElement& FLoadBalancingWriterBase::GetEntityView(Worker_EntityId EntityId) const
{
	return SubView->GetView()[EntityId];
}

void FLoadBalancingWriterBase::SendComponentUpdate(Worker_EntityId EntityId, FWorkerComponentUpdate& ComponentUpdate)
{
	NetDriver->Connection->SendComponentUpdate(EntityId, &ComponentUpdate);
}

class LoadBalancingWriterActorGroup : public TLoadBalancingWriter<ActorGroupMember>
{
public:
	explicit LoadBalancingWriterActorGroup(USpatialNetDriver* NetDriver, const FSubView* InSubView)
		: TLoadBalancingWriter<ActorGroupMember>(NetDriver, InSubView)
	{
	}

private:
	virtual ActorGroupMember GetLoadBalancingData(AActor* Actor) const override
	{
		return ActorGroupMember(NetDriver->LoadBalanceStrategy->GetActorGroupId(*Actor));
	}
};

class LoadBalancingWriterActorSet : public TLoadBalancingWriter<ActorSetMember>
{
public:
	explicit LoadBalancingWriterActorSet(USpatialNetDriver* NetDriver, const FSubView* InSubView)
		: TLoadBalancingWriter<ActorSetMember>(NetDriver, InSubView)
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

LoadBalancingWriter::LoadBalancingWriter(USpatialNetDriver* InNetDriver, const FSubView* InSubView)
	: NetDriver(InNetDriver)
	, SubView(InSubView)
	, ActorSetWriter(MakeUnique<LoadBalancingWriterActorSet>(InNetDriver, InSubView))
	, ActorGroupWriter(MakeUnique<LoadBalancingWriterActorGroup>(InNetDriver, InSubView))

{
}

void LoadBalancingWriter::Advance()
{
	ActorSetWriter->Advance();
	ActorGroupWriter->Advance();
}

void LoadBalancingWriter::OnActorReplicated(Worker_EntityId ActorEntityId, AActor* Actor)
{
	ActorSetWriter->ReplicateActor(ActorEntityId, Actor);
	ActorGroupWriter->ReplicateActor(ActorEntityId, Actor);
}

} // namespace SpatialGDK