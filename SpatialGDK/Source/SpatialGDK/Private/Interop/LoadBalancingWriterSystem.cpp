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

bool FLoadBalancingWriterBase::IsCreatingNewEntity(Worker_EntityId EntityId) const
{
	return NetDriver->GetActorChannelByEntityId(EntityId)->bCreatingNewEntity;
}

const EntityViewElement& FLoadBalancingWriterBase::GetEntityView(Worker_EntityId EntityId) const
{
	return NetDriver->Connection->GetView()[EntityId];
}

void FLoadBalancingWriterBase::SendComponentUpdate(Worker_EntityId EntityId, FWorkerComponentUpdate& ComponentUpdate)
{
	NetDriver->Connection->SendComponentUpdate(EntityId, &ComponentUpdate);
}

class LoadBalancingWriterActorGroup : public TLoadBalancingWriter<ActorGroupMember>
{
public:
	explicit LoadBalancingWriterActorGroup(USpatialNetDriver* NetDriver)
		: TLoadBalancingWriter<ActorGroupMember>(NetDriver)
	{
	}

private:
	virtual LoadBalancingData GetLoadBalancingData(AActor* Actor) const override
	{
		return { ActorGroupMember(NetDriver->LoadBalanceStrategy->GetActorGroupId(*Actor)) };
	}
};

class LoadBalancingWriterActorSet : public TLoadBalancingWriter<ActorSetMember>
{
public:
	explicit LoadBalancingWriterActorSet(USpatialNetDriver* NetDriver)
		: TLoadBalancingWriter<ActorSetMember>(NetDriver)
	{
	}

private:
	virtual LoadBalancingData GetLoadBalancingData(AActor* Actor) const override
	{
		const AActor* Owner = Actor;
		for (; IsValid(Owner->GetOwner()); Owner = Owner->GetOwner())
			;
		check(IsValid(Owner));
		const Worker_EntityId OwnerEntityId = NetDriver->PackageMap->GetEntityIdFromObject(Owner);
		check(OwnerEntityId != SpatialConstants::INVALID_ENTITY_ID);
		// Actor Set IDs are implemented as owner entity IDs
		return { ActorSetMember(OwnerEntityId) };
	}
};

LoadBalancingWriter::LoadBalancingWriter(USpatialNetDriver* InNetDriver)
	: NetDriver(InNetDriver)
	, ActorSetWriter(MakeUnique<LoadBalancingWriterActorSet>(InNetDriver))
	, ActorGroupWriter(MakeUnique<LoadBalancingWriterActorGroup>(InNetDriver))

{
}

void LoadBalancingWriter::Advance()
{
	ActorSetWriter->Advance(*SubView);
	ActorGroupWriter->Advance(*SubView);
}

void LoadBalancingWriter::OnActorReplicated(AActor* Actor)
{
	ActorSetWriter->ReplicateActor(Actor);
	ActorGroupWriter->ReplicateActor(Actor);
}

LoadBalancingWriter::FLoadBalancingStuff LoadBalancingWriter::GetOrCreateLoadBalancingData(const AActor* Actor)
{
	FLoadBalancingStuff Stuff;

	Stuff.ActorGroup.ActorGroupId = NetDriver->LoadBalanceStrategy->GetActorGroupId(*Actor);

	const AActor* Owner = Actor;
	for (; IsValid(Owner->GetOwner()); Owner = Owner->GetOwner())
		;
	check(IsValid(Owner));
	const Worker_EntityId OwnerEntityId = NetDriver->PackageMap->GetEntityIdFromObject(Owner);
	check(OwnerEntityId != SpatialConstants::INVALID_ENTITY_ID);
	// Actor Set IDs are implemented as owner entity IDs
	Stuff.ActorSet.ActorSetId = OwnerEntityId;

	return Stuff;
}
} // namespace SpatialGDK